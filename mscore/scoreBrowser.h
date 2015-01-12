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

#ifndef __SCOREBROWSER_H__
#define __SCOREBROWSER_H__

#include "ui_scoreBrowser.h"
#include "scoreInfo.h"

namespace Ms {

class ScoreItem;

//---------------------------------------------------------
//   ScoreListWidget
//---------------------------------------------------------

class ScoreListWidget : public QListWidget
      {
      Q_OBJECT
      int CELLW           { 112   };
      int CELLH           { 224   };
      int SPACE           { 4    };

      virtual QSize sizeHint() const override;

   public:
      ScoreListWidget(QWidget* parent = 0) : QListWidget(parent) {}
      int cellWidth() const { return CELLW; }
      int cellHeight() const { return CELLH; }
      QSize cellSize() const { return QSize(CELLW, CELLH); }
      };

//---------------------------------------------------------
//   ScoreBrowser
//---------------------------------------------------------

class ScoreBrowser : public QWidget, public Ui::ScoreBrowser
      {
      Q_OBJECT

      QList<ScoreListWidget*> scoreLists;
      bool _stripNumbers  { false };
      bool _showPreview   { false };      // no preview: - no selection
                                          //             - single click action
      bool _boldTitle     { false }; // score title are displayed in bold

      ScoreListWidget* createScoreList();
      ScoreItem* genScoreItem(const QFileInfo&, ScoreListWidget*);

   private slots:
      void scoreChanged(QListWidgetItem*);
      void setScoreActivated(QListWidgetItem*);

   signals:
      void leave();
      void scoreSelected(QString);
      void scoreActivated(QString);

   public:
      ScoreBrowser(QWidget* parent = 0);
      void setScores(const QFileInfoList&);
      void setStripNumbers(bool val) { _stripNumbers = val; }
      void selectFirst();
      void selectLast();
      void setBoldTitle(bool bold) { _boldTitle = bold; }
      };
}

#endif

