//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "testing/qtestsuite.h"
#include "testbase.h"
#include "libmscore/score.h"

static const QString CAPELLA_DIR("data/");

using namespace Ms;

//---------------------------------------------------------
//   TestCapellaIO
//---------------------------------------------------------

class TestCapellaIO : public QObject, public MTest
{
    Q_OBJECT

    void capReadTest(const char* file);
    void capxReadTest(const char* file);

private slots:
    void initTestCase();

    // The list of Capella regression tests
    // Currently failing tests are commented out and annotated with the failure reason
    // To extract the list in a shell script use:
    // cat tst_capella_io.cpp | grep "{ <test>" | awk -F\" '{print $2}'
    // where <test> is capReadTest or capxReadTest

    void capTest1() { capReadTest("test1"); }
    void capTest2() { capReadTest("test2"); }
    void capTest3() { capReadTest("test3"); }
    void capTest4() { capReadTest("test4"); }   // wrong enharmonic spelling
    void capTest5() { capReadTest("test5"); }
    void capTest6() { capReadTest("test6"); }
    void capTest7() { capReadTest("test7"); }   // double bar missing (auto-generated by Capella programs)
    void capTest8() { capReadTest("test8"); }
    void capTestTuplet2() { capReadTest("testTuplet2"); }   // generates different beaming with respect to the original
    void capxTest1() { capxReadTest("test1"); }
    void capxTest2() { capxReadTest("test2"); }
    void capxTest3() { capxReadTest("test3"); }
    void capxTest4() { capxReadTest("test4"); }   // wrong enharmonic spelling
    void capxTest5() { capxReadTest("test5"); }
    void capxTest6() { capxReadTest("test6"); }
    void capxTest7() { capxReadTest("test7"); }   // double bar missing (auto-generated by Capella programs)
    void capxTestEmptyStaff1() { capxReadTest("testEmptyStaff1"); }
    void capxTestEmptyStaff2() { capxReadTest("testEmptyStaff2"); }
    void capxTestPianoG4G5() { capxReadTest("testPianoG4G5"); }
    void capxTestScaleC4C5() { capxReadTest("testScaleC4C5"); }
    void capxTestSlurTie() { capxReadTest("testSlurTie"); }
    void capxTestText1() { capxReadTest("testText1"); }
    void capxTestTuplet1() { capxReadTest("testTuplet1"); }   // generates different (incorrect ?) l1 and l2 values in beams
    void capxTestTuplet2() { capxReadTest("testTuplet2"); }   // generates different beaming with respect to the original
    void capxTestVolta1() { capxReadTest("testVolta1"); }
    void capxTestBarline() { capxReadTest("testBarline"); }
};

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestCapellaIO::initTestCase()
{
    initMTest(QString(iex_capella_tests_DATA_ROOT));
}

//---------------------------------------------------------
//   capReadTest
//   read a Capella file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void TestCapellaIO::capReadTest(const char* file)
{
    MasterScore* score = readScore(CAPELLA_DIR + file + ".cap");
    QVERIFY(score);
    QVERIFY(saveCompareScore(score, QString("%1.cap.mscx").arg(file),
                             CAPELLA_DIR + QString("%1.cap-ref.mscx").arg(file)));
    delete score;
}

//---------------------------------------------------------
//   capxReadTest
//   read a CapellaXML file, write to a MuseScore file and verify against reference
//---------------------------------------------------------

void TestCapellaIO::capxReadTest(const char* file)
{
    MasterScore* score = readScore(CAPELLA_DIR + file + ".capx");
    QVERIFY(score);
    QVERIFY(saveCompareScore(score, QString("%1.capx.mscx").arg(file),
                             CAPELLA_DIR + QString("%1.capx-ref.mscx").arg(file)));
    delete score;
}

QTEST_MAIN(TestCapellaIO)
#include "tst_capella_io.moc"
