
//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: newwizard.cpp 5626 2012-05-13 18:33:52Z lasconic $
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "newwizard.h"
#include "musescore.h"
#include "preferences.h"
#include "palette.h"
#include "instrdialog.h"
#include "scoreBrowser.h"
#include "extension.h"

#include "libmscore/instrtemplate.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/clef.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/keysig.h"
#include "libmscore/measure.h"
#include "libmscore/stafftype.h"
#include "libmscore/timesig.h"
#include "libmscore/sym.h"

#include <QCompleter>

#define METATAGS_SAVED_COMPLETIONS 12

namespace Ms {

extern bool useFactorySettings;

extern Palette* newKeySigPalette();
extern void filterInstruments(QTreeWidget *instrumentList, const QString &searchPhrase = QString(""));

//---------------------------------------------------------
//   TimesigWizard
//---------------------------------------------------------

TimesigWizard::TimesigWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      connect(tsCommonTime, SIGNAL(toggled(bool)), SLOT(commonTimeToggled(bool)));
      connect(tsCutTime,    SIGNAL(toggled(bool)), SLOT(cutTimeToggled(bool)));
      connect(tsFraction,   SIGNAL(toggled(bool)), SLOT(fractionToggled(bool)));
      }

//---------------------------------------------------------
//   measures
//---------------------------------------------------------

int TimesigWizard::measures() const
      {
      return measureCount->value();
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

Fraction TimesigWizard::timesig() const
      {
      if (tsFraction->isChecked())
            return Fraction(timesigZ->value(), 1 << timesigN->currentIndex());
      else if (tsCommonTime->isChecked())
            return Fraction(4, 4);
      else
            return Fraction(2, 2);
      }

//---------------------------------------------------------
//   pickupMeasure
//---------------------------------------------------------

bool TimesigWizard::pickup(int* z, int* n) const
      {
      *z = pickupTimesigZ->value();
      *n = 1 << pickupTimesigN->currentIndex();
      return pickupMeasure->isChecked();
      }

//---------------------------------------------------------
//   type
//---------------------------------------------------------

TimeSigType TimesigWizard::type() const
      {
      if (tsFraction->isChecked())
            return TimeSigType::NORMAL;
      if (tsCommonTime->isChecked())
            return TimeSigType::FOUR_FOUR;
      return TimeSigType::ALLA_BREVE;
      }

//---------------------------------------------------------
//   commonTimeToggled
//---------------------------------------------------------

void TimesigWizard::commonTimeToggled(bool val)
      {
      if (val) {
            // timesigZ->setValue(4);
            // timesigN->setValue(4);
            timesigZ->setEnabled(false);
            timesigN->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   cutTimeToggled
//---------------------------------------------------------

void TimesigWizard::cutTimeToggled(bool val)
      {
      if (val) {
            // timesigZ->setValue(2);
            // timesigN->setValue(2);
            timesigZ->setEnabled(false);
            timesigN->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   fractionToggled
//---------------------------------------------------------

void TimesigWizard::fractionToggled(bool val)
      {
      if (val) {
            timesigZ->setEnabled(true);
            timesigN->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   TitleWizard
//---------------------------------------------------------

TitleWizard::TitleWizard(QWidget* parent)
   : QWidget(parent),
     _moreOptionsVisible(true) // because they are visible in the .ui files.
      {
      setObjectName("TitleWizard");
      setupUi(this);

      readSettings();

      QCompleter* composerCompl = lineEditComposer->completer();
      composerCompl->setCaseSensitivity(Qt::CaseInsensitive);
      composerCompl->setCompletionMode(QCompleter::PopupCompletion);
      composerCompl->setFilterMode(Qt::MatchContains);
      QCompleter* arrangerCompl = lineEditArranger->completer();
      arrangerCompl->setCaseSensitivity(Qt::CaseInsensitive);
      arrangerCompl->setCompletionMode(QCompleter::PopupCompletion);
      arrangerCompl->setFilterMode(Qt::MatchContains);
      QCompleter* lyricistCompl = lineEditLyricist->completer();
      lyricistCompl->setCaseSensitivity(Qt::CaseInsensitive);
      lyricistCompl->setCompletionMode(QCompleter::PopupCompletion);
      lyricistCompl->setFilterMode(Qt::MatchContains);
      QCompleter* copyrightCompl = lineEditCopyright->completer();
      copyrightCompl->setCaseSensitivity(Qt::CaseInsensitive);
      copyrightCompl->setCompletionMode(QCompleter::PopupCompletion);
      copyrightCompl->setFilterMode(Qt::MatchContains);

      connect(buttonMore, SIGNAL(toggled(bool)), SLOT(setMoreOptionsVisible(bool)));
      }

TitleWizard::~TitleWizard()
      {
      writeSettings();
      }

void TitleWizard::addCompletions()
      {
      auto addCompletionToModel = [] (QStringListModel* model, QString completion)
            {
            if (completion.isEmpty())
                  return;
            QStringList sl = model->stringList();

            // to stack the existing string on top,
            // so it's the last on the discard pile,
            // remove it, then append it later.
            if (sl.contains(completion, Qt::CaseSensitive))
                  sl.removeAll(completion); // sl.removeOne(completion) would be faster, but less secure.

            // Limit the number max number of completions
            if (sl.size() > METATAGS_SAVED_COMPLETIONS)
                  sl.takeFirst();

            sl.append(completion);
            model->setStringList(sl);
            };

      // completions are added to composer, lyricist, arranger and copyright.
      addCompletionToModel(static_cast<QStringListModel*> (lineEditComposer->completer()->model()),  lineEditComposer->text());
      addCompletionToModel(static_cast<QStringListModel*> (lineEditLyricist->completer()->model()),  lineEditLyricist->text());
      addCompletionToModel(static_cast<QStringListModel*> (lineEditArranger->completer()->model()),  lineEditArranger->text());
      addCompletionToModel(static_cast<QStringListModel*> (lineEditCopyright->completer()->model()), lineEditCopyright->text());
      }

//   metatags which are saved in a QCompleter: composer, lyricist, arranger and copyright.
void TitleWizard::readSettings()
      {
      QSettings settings;
      settings.beginGroup(objectName());
      setMoreOptionsVisible(settings.value("moreOptionsVisible", false).toBool());

      /// Load the QCompleters
      {
      // composers
      int size = settings.beginReadArray("composers");
      QStringList sl;
      for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            sl << settings.value("composer", "").toString();
            }
      lineEditComposer->setCompleter(new QCompleter(new QStringListModel(sl)));
      settings.endArray();
      } // to free the ressources and change the scope so that no long name is required.

      {
      // lyricists
      int size = settings.beginReadArray("lyricists");
      QStringList sl;
      for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            sl << settings.value("lyricist", "").toString();
            }
      lineEditLyricist->setCompleter(new QCompleter(new QStringListModel(sl)));
      settings.endArray();
      } // to free the ressources and change the scope so that no long name is required.

      {
      // arrangers
      int size = settings.beginReadArray("arrangers");
      QStringList sl;
      for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            sl << settings.value("arranger", "").toString();
            }
      lineEditArranger->setCompleter(new QCompleter(new QStringListModel(sl)));
      settings.endArray();
      } // to free the ressources and change the scope so that no long name is required.

      {
      // copyrights
      int size = settings.beginReadArray("copyrights");
      QStringList sl;
      for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            sl << settings.value("copyright", "").toString();
            }
      lineEditCopyright->setCompleter(new QCompleter(new QStringListModel(sl)));
      settings.endArray();
      } // to free the ressources and change the scope so that no long name is required.

      settings.endGroup();
      }

//   metatags which are saved in a QCompleter: composer, lyricist, arranger and copyright.
void TitleWizard::writeSettings()
      {
      QSettings settings;
      settings.beginGroup(objectName());
      settings.setValue("moreOptionsVisible", _moreOptionsVisible);

      /// Save the QCompleters
      {
      // composers
      QStringList sl = static_cast<QStringListModel*> (lineEditComposer->completer()->model())->stringList();
      settings.beginWriteArray("composers");
      for (int i = 0; i < sl.length(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("composer", sl.at(i));
            }
      settings.endArray();
      } // to keep short names

      {
      // lyricists
      QStringList sl = static_cast<QStringListModel*> (lineEditLyricist->completer()->model())->stringList();
      settings.beginWriteArray("lyricists");
      for (int i = 0; i < sl.length(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("lyricist", sl.at(i));
            }
      settings.endArray();
      } // to keep short names

      {
      // arrangers
      QStringList sl = static_cast<QStringListModel*> (lineEditArranger->completer()->model())->stringList();
      settings.beginWriteArray("arrangers");
      for (int i = 0; i < sl.length(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("arrangers", sl.at(i));
            }
      settings.endArray();
      }

      {
      // copyrights
      QStringList sl = static_cast<QStringListModel*> (lineEditCopyright->completer()->model())->stringList();
      settings.beginWriteArray("copyrights");
      for (int i = 0; i < sl.length(); ++i) {
            settings.setArrayIndex(i);
            settings.setValue("copyright", sl.at(i));
            }
      settings.endArray();
      } // to keep short names

      settings.endGroup();
      }

//---------------------------------------------------------
//   setMoreOptionsVisible
//---------------------------------------------------------

void TitleWizard::setMoreOptionsVisible(bool visible)
      {
      if (visible == _moreOptionsVisible)
            return;

      lineEditArranger->setVisible(visible);
      labelArranger->setVisible(visible);
      lineEditMovementNumber->setVisible(visible);
      labelMovementNumber->setVisible(visible);
      lineEditMovementTitle->setVisible(visible);
      labelMovementTitle->setVisible(visible);
      lineEditSource->setVisible(visible);
      labelSource->setVisible(visible);
      lineEditPoet->setVisible(visible);
      labelPoet->setVisible(visible);
      lineEditTranslator->setVisible(visible);
      labelTranslator->setVisible(visible);
      lineEditWorkNumber->setVisible(visible);
      labelWorkNumber->setVisible(visible);

      if (visible)
            buttonMore->setText(tr("Less"));
      else
            buttonMore->setText(tr("More"));

      _moreOptionsVisible = visible;
      }

//---------------------------------------------------------
//   NewWizardPage1
//---------------------------------------------------------

NewWizardPage1::NewWizardPage1(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Enter score information:"));
      setAccessibleName(QWizardPage::title());
      setAccessibleDescription(QWizardPage::subTitle());

      w = new TitleWizard;

      QGridLayout* grid = new QGridLayout;
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   initializePage
//   This needs to be done because since the newWizard is
//   only deleted when musescore is closed, the editLines
//   are not reset if you use 2 times the newwizard on the
//   same session. But since (quite often) you don't want
//   to create two scores with the same title, it needs to
//   be reset.
//---------------------------------------------------------

void NewWizardPage1::initializePage()
      {
      w->lineEditTitle->setText("");
      w->lineEditSubtitle->setText("");
      w->lineEditComposer->setText("");
      w->lineEditLyricist->setText("");
      w->lineEditArranger->setText("");
      w->lineEditCopyright->setText("");
      if (preferences.getBool(PREF_SCORE_WORKNUMBER_TRACKWORKNUMBER)) {
            w->lineEditWorkNumber->setText(QString("%1%2%3").arg(preferences.getBool(PREF_SCORE_WORKNUMBER_USEPREFIX)
                                                                 ? (preferences.getString(PREF_SCORE_WORKNUMBER_PREFIX) + " ")
                                                                 : "")
                                                             .arg(preferences.getInt(PREF_SCORE_WORKNUMBER_NEXTWORKNUMBER))
                                                             .arg(preferences.getBool(PREF_SCORE_WORKNUMBER_USESUFFIX)
                                                                  ? (" " + preferences.getString(PREF_SCORE_WORKNUMBER_SUFFIX))
                                                                  : ""));
            }
      }

//---------------------------------------------------------
//   NewWizardPage2
//---------------------------------------------------------

NewWizardPage2::NewWizardPage2(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose instruments on the left to add to instrument list on the right:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());
      w        = new InstrumentsWidget;
      QGridLayout* grid = new QGridLayout;
      grid->setSpacing(0);
      grid->setContentsMargins(0, 0, 0, 0);
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      connect(w, SIGNAL(completeChanged(bool)), SLOT(setComplete(bool)));
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage2::initializePage()
      {
      complete = false;
      w->init();
      }

//---------------------------------------------------------
//   setComplete
//---------------------------------------------------------

void NewWizardPage2::setComplete(bool val)
      {
      if (complete == val)
            return;
      complete = val;
      emit completeChanged();
      }

//---------------------------------------------------------
//   isComplete
//---------------------------------------------------------

bool NewWizardPage2::isComplete() const
      {
      return complete;
      }

//---------------------------------------------------------
//   createInstruments
//---------------------------------------------------------

void NewWizardPage2::createInstruments(Score* s)
      {
      w->createInstruments(s);
      }

//---------------------------------------------------------
//   NewWizardPage3
//---------------------------------------------------------

NewWizardPage3::NewWizardPage3(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose time signature:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      w = new TimesigWizard;
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   NewWizardPage4
//---------------------------------------------------------

NewWizardPage4::NewWizardPage4(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose template file:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      templateFileBrowser = new ScoreBrowser;
      templateFileBrowser->setStripNumbers(true);
      templateFileBrowser->setShowCustomCategory(true);
      templateFileBrowser->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));
      buildTemplatesList();

      QVBoxLayout* layout = new QVBoxLayout;
      QHBoxLayout* searchLayout = new QHBoxLayout;
      QLineEdit* search = new QLineEdit;
      search->setPlaceholderText(tr("Search"));
      search->setClearButtonEnabled(true);
      search->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
      searchLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
      searchLayout->addWidget(search);

      layout->addLayout(searchLayout);
      layout->addWidget(templateFileBrowser);
      setLayout(layout);

      connect(templateFileBrowser, SIGNAL(scoreSelected(const QString&)), SLOT(templateChanged(const QString&)));
      connect(templateFileBrowser, SIGNAL(scoreActivated(const QString&)), SLOT(fileAccepted(const QString&)));
      connect(search, &QLineEdit::textChanged, [this] (const QString& searchString) {
            this->templateFileBrowser->filter(searchString);
            });
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage4::initializePage()
      {
      templateFileBrowser->show();
      path.clear();
      }

//---------------------------------------------------------
//   buildTemplatesList
//---------------------------------------------------------

void NewWizardPage4::buildTemplatesList()
      {

      QDir dir(mscoreGlobalShare + "/templates");
      QFileInfoList fil = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Dirs | QDir::Files, QDir::Name);
      if(fil.isEmpty()){
          fil.append(QFileInfo(QFile(":data/Empty_Score.mscz")));
          }

      QDir myTemplatesDir(preferences.getString(PREF_APP_PATHS_MYTEMPLATES));
      fil.append(myTemplatesDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Dirs | QDir::Files, QDir::Name));

      // append templates directories from extensions
      QStringList extensionsDir = Extension::getDirectoriesByType(Extension::templatesDir);
      for (QString extDir : extensionsDir) {
            QDir extTemplateDir(extDir);
            fil.append(extTemplateDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Readable | QDir::Dirs | QDir::Files, QDir::Name));
            }
      templateFileBrowser->setScores(fil);
      }

//---------------------------------------------------------
//   isComplete
//---------------------------------------------------------

bool NewWizardPage4::isComplete() const
      {
      return !path.isEmpty();
      }

//---------------------------------------------------------
//   fileAccepted
//---------------------------------------------------------

void NewWizardPage4::fileAccepted(const QString& s)
      {
      path = s;
      templateFileBrowser->show();
      if (wizard()->currentPage() == this)
            wizard()->next();
      }

//---------------------------------------------------------
//   templateChanged
//---------------------------------------------------------

void NewWizardPage4::templateChanged(const QString& s)
      {
      path = s;
      emit completeChanged();
      }

//---------------------------------------------------------
//   templatePath
//---------------------------------------------------------

QString NewWizardPage4::templatePath() const
      {
      return path;
      }

//---------------------------------------------------------
//   NewWizardPage5
//---------------------------------------------------------

NewWizardPage5::NewWizardPage5(QWidget* parent)
   : QWizardPage(parent)
      {
      setFinalPage(true);
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Choose key signature and tempo:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      QGroupBox* b1 = new QGroupBox;
      b1->setTitle(tr("Key Signature"));
      b1->setAccessibleName(title());
      sp = MuseScore::newKeySigPalette();
      sp->setMoreElements(false);
      sp->setShowContextMenu(false);
      sp->setSelectable(true);
      sp->setDisableDoubleClick(true);
      sp->setSelected(14);
      PaletteScrollArea* sa = new PaletteScrollArea(sp);
      QVBoxLayout* l1 = new QVBoxLayout;
      l1->addWidget(sa);
      b1->setLayout(l1);

      tempoGroup = new QGroupBox;
      tempoGroup->setCheckable(true);
      tempoGroup->setChecked(false);
      tempoGroup->setTitle(tr("Tempo"));
      QLabel* bpm = new QLabel;
      bpm->setText(tr("BPM:"));
      _tempo = new QDoubleSpinBox;
      _tempo->setAccessibleName(tr("Beats per minute"));
      _tempo->setRange(20.0, 400.0);
      _tempo->setValue(120.0);
      _tempo->setDecimals(1);
      QHBoxLayout* l2 = new QHBoxLayout;
      l2->addWidget(bpm);
      l2->addWidget(_tempo);
      l2->addStretch(100);
      tempoGroup->setLayout(l2);

      QVBoxLayout* l3 = new QVBoxLayout;
      l3->addWidget(b1);
      l3->addWidget(tempoGroup);
      l3->addStretch(100);
      setLayout(l3);
      setFocusPolicy(Qt::StrongFocus);
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

KeySigEvent NewWizardPage5::keysig() const
      {
      int idx    = sp->getSelectedIdx();
      Element* e = sp->element(idx);
      return static_cast<KeySig*>(e)->keySigEvent();
      }

//---------------------------------------------------------
//   NewWizard
//---------------------------------------------------------

NewWizard::NewWizard(QWidget* parent)
   : QWizard(parent)
      {
      setObjectName("NewWizard");
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWizardStyle(QWizard::ClassicStyle);
      setPixmap(QWizard::LogoPixmap, QPixmap(":/data/mscore.png"));
      setPixmap(QWizard::WatermarkPixmap, QPixmap());
      setWindowTitle(tr("New Score Wizard"));

      setOption(QWizard::NoCancelButton, false);
#ifdef Q_OS_MAC
      setOption(QWizard::CancelButtonOnLeft, true);
#endif
      setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
      setOption(QWizard::HaveNextButtonOnLastPage, true);

      p1 = new NewWizardPage1;
      p2 = new NewWizardPage2;
      p3 = new NewWizardPage3;
      p4 = new NewWizardPage4;
      p5 = new NewWizardPage5;

//  enum Page { Invalid = -1, Type, Instruments, Template, Keysig, Timesig};

      setPage(Page::Type,        p1);
      setPage(Page::Instruments, p2);
      setPage(Page::Template,    p4);
      setPage(Page::Keysig,      p5);
      setPage(Page::Timesig,     p3);

      resize(QSize(840, 560)); //ensure default size if no geometry in settings
      //connect(this, SIGNAL(currentIdChanged(int)), SLOT(idChanged(int)));
      readSettings();
      }

//---------------------------------------------------------
//   idChanged
//---------------------------------------------------------

void NewWizard::idChanged(int /*id*/)
      {
      // printf("\n===\nWizard: id changed %d\n", id);
      }

//---------------------------------------------------------
//   nextId
//---------------------------------------------------------

int NewWizard::nextId() const
      {
      int next;
      switch (Page(currentId())) {
            case Page::Type:
                  next = Page::Template;
                  break;
            case Page::Template:
                  next = emptyScore() ? Page::Instruments : Page::Keysig;
                  break;
            case Page::Instruments:
                  next = Page::Keysig;
                  break;
            case Page::Keysig:
                  next = Page::Timesig;
                  break;
            case Page::Timesig:
            default:
                  next = Page::Invalid;
                  break;
            }
      return next;
      }

//---------------------------------------------------------
//   emptyScore
//---------------------------------------------------------

bool NewWizard::emptyScore() const
      {
      QString p = p4->templatePath();
      QFileInfo fi(p);
      bool val = fi.completeBaseName() == "00-Blank";
      return val;
      }
//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void NewWizard::writeSettings()
      {
      QSettings settings;
      settings.beginGroup(objectName());
      settings.setValue("numberOfMeasures", p3->measures());
      settings.setValue("tempo", p5->tempo());
      settings.setValue("createTempo", p5->createTempo());
      settings.endGroup();

      MuseScore::saveGeometry(this);
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void NewWizard::readSettings()
      {
      if (!useFactorySettings) {
            QSettings settings;
            settings.beginGroup(objectName());
            p3->setMeasures(settings.value("numberOfMeasures").toInt());
            p5->setTempo(settings.value("tempo").toDouble());
            p5->setCreateTempo(settings.value("createTempo").toBool());
            settings.endGroup();
            }

      MuseScore::restoreGeometry(this);
      }

//---------------------------------------------------------
//   hideEvent
//---------------------------------------------------------

void NewWizard::hideEvent(QHideEvent* event)
      {
      writeSettings();
      QWidget::hideEvent(event);
      }

} // namesace Ms

