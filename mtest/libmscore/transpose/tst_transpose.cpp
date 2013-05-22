//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

#define DIR QString("libmscore/transpose/")

using namespace Ms;

//---------------------------------------------------------
//   TestTranspose
//---------------------------------------------------------

class TestTranspose : public QObject, public MTest
      {
      Q_OBJECT

   private slots:
      void initTestCase();
      void undoTranspose();
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestTranspose::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   undoTranspose
//---------------------------------------------------------

void TestTranspose::undoTranspose()
      {
      QString readFile(DIR + "undoTranspose.mscx");
      QString writeFile1("undoTranspose01-test.mscx");
      QString reference1(DIR  + "undoTranspose01-ref.mscx");
      QString writeFile2("undoTranspose02-test.mscx");

      Score* score = readScore(readFile);

      // select all
      score->cmdSelectAll();
      
      // transpose major second up
      score->startCmd();
      score->transpose(TRANSPOSE_BY_INTERVAL, TRANSPOSE_UP, 0, 4,
                       true, true, true);
      score->endCmd();
      QVERIFY(saveCompareScore(score, writeFile1, reference1));

      // undo
      score->undo()->undo();
      QVERIFY(saveCompareScore(score, writeFile2, readFile));

      delete score;
      }

QTEST_MAIN(TestTranspose)
#include "tst_transpose.moc"

