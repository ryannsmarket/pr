//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "synthcontrol.h"
#include "musescore.h"
#include "seq.h"
#include "audio/midi/msynthesizer.h"
#include "audio/midi/synthesizer.h"
#include "audio/midi/synthesizergui.h"
#include "mixer/mixer.h"
#include "file.h"
#include "icons.h"
#include "libmscore/score.h"
#include "libmscore/mscore.h"
#include "libmscore/xml.h"
#include "libmscore/undo.h"
#include "effects/effectgui.h"
#include "libmscore/part.h"
#include "libmscore/instrument.h"

namespace Ms {
extern MasterSynthesizer* synti;
extern bool useFactorySettings;

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

SynthControl::SynthControl(QWidget* parent)
    : QWidget(parent, Qt::Dialog)
{
    setObjectName("SynthControl");
    setupUi(this);
    _score = 0;

    setWindowFlags(Qt::Tool);
//      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    int idx = 0;
    for (Synthesizer* s : synti->synthesizer()) {
        if (strcmp(s->name(), "Aeolus") == 0) {      // no gui for aeolus
            continue;
        }
        tabWidget->insertTab(idx++, s->gui(), tr(s->name()));
        s->gui()->synthesizerChanged();
        connect(s->gui(), SIGNAL(valueChanged()), SLOT(setDirty()));
    }

    // effectA        combo box
    // effectStackA   widget stack

    effectA->clear();
    for (Effect* e : synti->effectList(0)) {
        effectA->addItem(tr(e->name()));
        EffectGui* gui = e->gui();
        if (gui) {
            effectStackA->addWidget(gui);
            connect(gui, SIGNAL(valueChanged()), SLOT(setDirty()));
        }
    }

    effectB->clear();
    for (Effect* e : synti->effectList(1)) {
        effectB->addItem(tr(e->name()));
        EffectGui* gui = e->gui();
        if (gui) {
            effectStackB->addWidget(gui);
            connect(gui, SIGNAL(valueChanged()), SLOT(setDirty()));
        }
    }
    readSettings();

    updateGui();

    storeButton->setEnabled(false);
    recallButton->setEnabled(false);
    changeTuningButton->setEnabled(false);

    gainSlider->setLog(false);
    gainSlider->setRange(synti->minGainAsDecibels, synti->maxGainAsDecibels);
    gainSlider->setDclickValue1(synti->defaultGainAsDecibels);
    gainSlider->setDclickValue2(synti->defaultGainAsDecibels);
    gainSlider->setValue(synti->gainAsDecibels());

    enablePlay = new EnablePlayForWidget(this);
    connect(effectA,      SIGNAL(currentIndexChanged(int)), SLOT(effectAChanged(int)));
    connect(effectB,      SIGNAL(currentIndexChanged(int)), SLOT(effectBChanged(int)));
    connect(gainSlider,   SIGNAL(valueChanged(double,int)), SLOT(gainChanged(double,int)));
    connect(masterTuning, SIGNAL(valueChanged(double)),     SLOT(masterTuningChanged(double)));
    connect(changeTuningButton, SIGNAL(clicked()),          SLOT(changeMasterTuning()));
    connect(loadButton,   SIGNAL(clicked()),                SLOT(loadButtonClicked()));
    connect(saveButton,   SIGNAL(clicked()),                SLOT(saveButtonClicked()));
    connect(storeButton,  SIGNAL(clicked()),                SLOT(storeButtonClicked()));
    connect(recallButton, SIGNAL(clicked()),                SLOT(recallButtonClicked()));
    connect(gainSlider,         SIGNAL(valueChanged(double,int)), SLOT(setDirty()));
    connect(dynamicsMethodList, SIGNAL(currentIndexChanged(int)), SLOT(dynamicsMethodChanged(int)));
    connect(ccToUseList,        SIGNAL(currentIndexChanged(int)), SLOT(ccToUseChanged(int)));
    connect(switchExpr,   SIGNAL(clicked()),                SLOT(switchExprButtonClicked()));
    connect(switchNonExpr,SIGNAL(clicked()),                SLOT(switchNonExprButtonClicked()));
    connect(resetExpr,    SIGNAL(clicked()),                SLOT(resetExprButtonClicked()));
}

//---------------------------------------------------------
//   setGain
//---------------------------------------------------------

// synthesizer has signalled a gain change - update the slider
void SynthControl::setGain(float val)
{
    Q_UNUSED(val);
    gainSlider->setValue(synti->gainAsDecibels());
}

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void SynthControl::closeEvent(QCloseEvent* ev)
{
    emit closed(false);
    QWidget::closeEvent(ev);
}

//---------------------------------------------------------
//   showEvent
//---------------------------------------------------------

void SynthControl::showEvent(QShowEvent* e)
{
    enablePlay->showEvent(e);
    QWidget::showEvent(e);
    activateWindow();
    setFocus();
}

//---------------------------------------------------------
//   eventFilter
//---------------------------------------------------------

bool SynthControl::eventFilter(QObject* obj, QEvent* e)
{
    if (enablePlay->eventFilter(obj, e)) {
        return true;
    }
    return QWidget::eventFilter(obj, e);
}

void SynthControl::keyPressEvent(QKeyEvent* ev)
{
    if (ev->key() == Qt::Key_Escape && ev->modifiers() == Qt::NoModifier) {
        close();
        return;
    }
    QWidget::keyPressEvent(ev);
}

//---------------------------------------------------------
//   showSynthControl
//---------------------------------------------------------

void MuseScore::showSynthControl(bool val)
{
    QAction* a = getAction("synth-control");
    if (synthControl == 0) {
        synthControl = new SynthControl(this);
        mscore->stackUnder(synthControl);
        synthControl->setScore(cs);
        connect(synti,        SIGNAL(gainChanged(float)), synthControl, SLOT(setGain(float)));
        connect(synthControl, SIGNAL(closed(bool)), a,     SLOT(setChecked(bool)));
    }
    synthControl->setVisible(val);
}

//---------------------------------------------------------
//   gainChanged
//---------------------------------------------------------

// user has moved the gain control on this widget - update the synthesizer
void SynthControl::gainChanged(double val, int)
{
    synti->setGainAsDecibels(val);
}

//---------------------------------------------------------
//   masterTuningChanged
//---------------------------------------------------------

void SynthControl::masterTuningChanged(double /*val*/)
{
    changeTuningButton->setEnabled(true);
}

//---------------------------------------------------------
//   changeMasterTuning
//---------------------------------------------------------

void SynthControl::changeMasterTuning()
{
    synti->setMasterTuning(masterTuning->value());
    changeTuningButton->setEnabled(false);
    setDirty();
}

//---------------------------------------------------------
//   setMeter
//---------------------------------------------------------

void SynthControl::setMeter(float l, float r, float left_peak, float right_peak)
{
    gainSlider->setMeterVal(0, l, left_peak);
    gainSlider->setMeterVal(1, r, right_peak);
}

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------
void SynthControl::setScore(Score* s)
{
    _score = s;

    loadButton->setEnabled(true);
    saveButton->setEnabled(true);
}

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void SynthControl::stop()
{
    gainSlider->setMeterVal(0, .0, .0);
    gainSlider->setMeterVal(1, .0, .0);
}

//---------------------------------------------------------
//   effectAChanged
//---------------------------------------------------------

void SynthControl::effectAChanged(int idx)
{
    synti->setEffect(0, idx);
    effectStackA->setCurrentIndex(idx);
    setDirty();
}

//---------------------------------------------------------
//   effectBChanged
//---------------------------------------------------------

void SynthControl::effectBChanged(int idx)
{
    synti->setEffect(1, idx);
    effectStackB->setCurrentIndex(idx);
    setDirty();
}

//---------------------------------------------------------
//   dynamicsMethodChanged
//---------------------------------------------------------

void SynthControl::dynamicsMethodChanged(int val)
{
    ccToUseList->setEnabled(val != 0);
    synti->setDynamicsMethod(val);
    setDirty();
}

//---------------------------------------------------------
//   ccToUseChanged
//---------------------------------------------------------

void SynthControl::ccToUseChanged(int val)
{
    synti->setCcToUseIndex(val);
    setDirty();
}

//---------------------------------------------------------
//   switchExprButtonClicked
//---------------------------------------------------------

void SynthControl::switchExprButtonClicked()
{
    _score->masterScore()->updateExpressive(MuseScore::synthesizer("Fluid"), true, true);
    setAllUserBankController(true);
    updateMixer();
}

//---------------------------------------------------------
//   switchNonExprButtonClicked
//---------------------------------------------------------

void SynthControl::switchNonExprButtonClicked()
{
    _score->masterScore()->updateExpressive(MuseScore::synthesizer("Fluid"), false, true);
    setAllUserBankController(true);
    updateMixer();
}

//---------------------------------------------------------
//   resetExprButtonClicked
//---------------------------------------------------------

void SynthControl::resetExprButtonClicked()
{
    setAllUserBankController(false);
    _score->masterScore()->updateExpressive(MuseScore::synthesizer("Fluid"));
    updateMixer();
}

//---------------------------------------------------------
//   setAllUserBankController
//---------------------------------------------------------

void SynthControl::setAllUserBankController(bool val)
{
    _score->startCmd();
    for (Part* p : _score->parts()) {
        const InstrumentList* il = p->instruments();
        for (auto it = il->begin(); it != il->end(); it++) {
            Instrument* i = it->second;
            for (Channel* c : i->channel()) {
                _score->undo(new SetUserBankController(c, val));
            }
        }
    }
    _score->endCmd();
}

//---------------------------------------------------------
//   loadButtonClicked
//    load synthesizer settings from score
//---------------------------------------------------------

void SynthControl::loadButtonClicked()
{
    if (!_score) {
        return;
    }
    synti->setState(_score->synthesizerState());
    updateGui();
    loadButton->setEnabled(false);
    saveButton->setEnabled(false);
    storeButton->setEnabled(true);
    recallButton->setEnabled(true);
    changeTuningButton->setEnabled(false);
}

//---------------------------------------------------------
//   saveButtonClicked
//    save synthesizer settings to score
//---------------------------------------------------------

void SynthControl::saveButtonClicked()
{
    if (!_score) {
        return;
    }
    _score->startCmd();
    SynthesizerState ss = synti->state();
    if (_dirty || !_score->synthesizerState().isDefault()) {
        ss.setIsDefault(false);
    }
    _score->undo(new ChangeSynthesizerState(_score, ss));
    _score->endCmd();

    updateExpressivePatches();
    loadButton->setEnabled(false);
    saveButton->setEnabled(false);
    storeButton->setEnabled(true);
    recallButton->setEnabled(true);
    _dirty = false;
}

//---------------------------------------------------------
//   recallButtonClicked
//    load stored synthesizer settings
//---------------------------------------------------------

void SynthControl::recallButtonClicked()
{
    if (!_score) {
        qDebug("no score");
        return;
    }

    SynthesizerState state;
    QString s(dataPath + "/synthesizer.xml");
    QFile f(s);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug("cannot read synthesizer settings <%s>", qPrintable(s));
        return;
    }
    XmlReader e(&f);
    while (e.readNextStartElement()) {
        if (e.name() == "Synthesizer") {
            state.read(e);
        } else {
            e.unknown();
        }
    }
    state.setIsDefault(true);
    synti->setState(state);
    updateGui();

    storeButton->setEnabled(false);
    recallButton->setEnabled(false);

    loadButton->setEnabled(true);
    saveButton->setEnabled(true);
    changeTuningButton->setEnabled(false);
}

//---------------------------------------------------------
//   storeButtonClicked
//    save synthesizer settings
//---------------------------------------------------------

void SynthControl::storeButtonClicked()
{
    if (!_score) {
        qDebug("no score");
        return;
    }
    synti->storeState();
    updateExpressivePatches();
    storeButton->setEnabled(false);
    recallButton->setEnabled(false);
    _dirty = false;
}

//---------------------------------------------------------
//   Gui
//---------------------------------------------------------

void SynthControl::updateGui()
{
    masterTuning->setValue(synti->masterTuning());
    setGain(synti->gain());

    dynamicsMethodList->setCurrentIndex(synti->dynamicsMethod());
    ccToUseList->setCurrentIndex(synti->ccToUseIndex());
    if (dynamicsMethodList->currentIndex() == 0) {
        ccToUseList->setEnabled(false);
    } else {
        ccToUseList->setEnabled(true);
    }

    int idx = synti->indexOfEffect(0);
    effectA->setCurrentIndex(idx);
    effectStackA->setCurrentIndex(idx);
    if (synti->effect(0) && synti->effect(0)->gui()) {
        synti->effect(0)->gui()->updateValues();
    }
    if (synti->effect(1) && synti->effect(1)->gui()) {
        synti->effect(1)->gui()->updateValues();
    }

    idx = synti->indexOfEffect(1);
    effectB->setCurrentIndex(idx);
    effectStackB->setCurrentIndex(idx);
    for (Synthesizer* s : synti->synthesizer()) {
        if (strcmp(s->name(), "Aeolus") == 0) {      // no gui for aeolus
            continue;
        }
        s->gui()->synthesizerChanged();
    }
}

//---------------------------------------------------------
//   updateExpressivePatches
//---------------------------------------------------------

void SynthControl::updateExpressivePatches()
{
    _score->masterScore()->rebuildAndUpdateExpressive(MuseScore::synthesizer("Fluid"));
    updateMixer();
}

//---------------------------------------------------------
//   updateMixer
//---------------------------------------------------------

void SynthControl::updateMixer()
{
    if (mscore->getMixer()) {
        mscore->getMixer()->updateTracks();
    }
}

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void SynthControl::setDirty()
{
    _dirty = true;
    loadButton->setEnabled(true);
    saveButton->setEnabled(true);
    storeButton->setEnabled(true);
    recallButton->setEnabled(true);
}

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void SynthControl::writeSettings()
{
    QSettings settings;
    settings.beginGroup(objectName());
    settings.setValue("tab", tabWidget->currentIndex());
    settings.endGroup();

    MuseScore::saveGeometry(this);
}

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void SynthControl::readSettings()
{
    if (!useFactorySettings) {
        QSettings settings;
        settings.beginGroup(objectName());
        tabWidget->setCurrentIndex(settings.value("tab", 0).toInt());
        settings.endGroup();
    } else {
        tabWidget->setCurrentIndex(0);
    }

    MuseScore::restoreGeometry(this);
}

//---------------------------------------------------------
//   changeEvent
//---------------------------------------------------------

void SynthControl::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::LanguageChange) {
        retranslate();
    }
}
}
