//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================


#include "musescore.h"
#include "startcenter.h"

namespace Ms {

//---------------------------------------------------------
//   showStartcenter
//---------------------------------------------------------

void MuseScore::showStartcenter(bool val)
      {
      QAction* a = getAction("startcenter");
      if (val && startcenter == nullptr) {
            startcenter = new Startcenter;
            startcenter->addAction(a);
            startcenter->readSettings(settings);
            connect(startcenter, SIGNAL(closed(bool)), a, SLOT(setChecked(bool)));
            }
      startcenter->setVisible(val);
      }

//---------------------------------------------------------
//   Startcenter
//---------------------------------------------------------

Startcenter::Startcenter()
 : QDialog(0)
      {
      setupUi(this);
//      setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Popup);
      setWindowFlags(Qt::WindowStaysOnTopHint);
      setWindowModality(Qt::ApplicationModal);
      connect(createNewScore, SIGNAL(clicked()),      getAction("file-new"), SLOT(trigger()));
      connect(recentScores,   SIGNAL(toggled(bool)),  SLOT(recentScoresToggled(bool)));
      connect(templates,      SIGNAL(toggled(bool)),  SLOT(templatesToggled(bool)));
      connect(demos,          SIGNAL(toggled(bool)),  SLOT(demosToggled(bool)));
      connect(connectWeb,     SIGNAL(toggled(bool)),  SLOT(connectWebToggled(bool)));

      connect(demosPage,      &ScoreBrowser::leave, this, &Startcenter::close);
      connect(templatesPage,  &ScoreBrowser::leave, this, &Startcenter::close);
      connect(recentPage,     &ScoreBrowser::leave, this, &Startcenter::close);
      recentScoresToggled(true);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Startcenter::closeEvent(QCloseEvent*)
      {
      emit closed(false);
      }

//---------------------------------------------------------
//   recentScoresToggled
//---------------------------------------------------------

void Startcenter::recentScoresToggled(bool val)
      {
      if (!val)
            return;
      if (!recentPageInitialized) {
            recentPage->setScores(mscore->recentScores());
            recentPageInitialized = true;
            }
      stack->setCurrentWidget(recentPage);
      }

//---------------------------------------------------------
//   updateRecentScores
//---------------------------------------------------------

void Startcenter::updateRecentScores()
      {
      recentPageInitialized = false;
      if (recentScores->isChecked())
            recentScoresToggled(true);
      }

//---------------------------------------------------------
//   templatesToggled
//---------------------------------------------------------

void Startcenter::templatesToggled(bool val)
      {
      if (!val)
            return;
      if (!templatesPageInitialized) {
            QDir dir(mscoreGlobalShare + "/templates");
            QStringList filter = { "*.mscz" };
            QFileInfoList fil;
            for (const QFileInfo& fi : dir.entryInfoList(filter, QDir::Files)) {
                  if (fi.exists())
                        fil.append(fi);
                  }
            templatesPage->setScores(fil);
            templatesPageInitialized = true;
            }
      stack->setCurrentWidget(templatesPage);
      }

//---------------------------------------------------------
//   demosToggled
//---------------------------------------------------------

void Startcenter::demosToggled(bool val)
      {
      if (!val)
            return;
      if (!demosPageInitialized) {
            QDir dir(mscoreGlobalShare + "/demos");
            QStringList filter = { "*.mscz" };
            QFileInfoList fil;
            for (const QFileInfo& fi : dir.entryInfoList(filter, QDir::Files)) {
                  if (fi.exists())
                        fil.append(fi);
                  }
            demosPage->setScores(fil);
            demosPageInitialized = true;
            }
      stack->setCurrentWidget(demosPage);
      }

//---------------------------------------------------------
//   connectWebToggled
//---------------------------------------------------------

void Startcenter::connectWebToggled(bool val)
      {
      if (!val)
            return;
      if (!webPageInitialized) {
            webView->setUrl(QUrl("http://www.musescore.org"));
            webPageInitialized = true;
            }
      stack->setCurrentWidget(webPage);
      }

//---------------------------------------------------------
//   writeSettings
//---------------------------------------------------------

void Startcenter::writeSettings(QSettings& settings)
      {
      settings.beginGroup("Startcenter");
      settings.setValue("size", size());
      settings.setValue("pos", pos());
      settings.endGroup();
      }

//---------------------------------------------------------
//   readSettings
//---------------------------------------------------------

void Startcenter::readSettings(QSettings& settings)
      {
      settings.beginGroup("Startcenter");
      resize(settings.value("size", QSize(1161, 694)).toSize());
      move(settings.value("pos", QPoint(200, 100)).toPoint());
      settings.endGroup();
      }

}

