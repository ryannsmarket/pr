/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>

#include "dom/chord.h"
#include "dom/masterscore.h"
#include "dom/segment.h"
#include "dom/undo.h"

#include "utils/scorerw.h"
#include "utils/scorecomp.h"

using namespace mu;
using namespace mu::engraving;

static const String EXCHVOICES_DATA_DIR("exchangevoices_data/");

class Engraving_ExchangevoicesTests : public ::testing::Test
{
};

TEST_F(Engraving_ExchangevoicesTests, slurs)
{
    Score* score = ScoreRW::readScore(EXCHVOICES_DATA_DIR + "exchangevoices-slurs.mscx");
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdExchangeVoice(0, 1);
    score->endCmd();

    // compare
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"exchangevoices-slurs.mscx", EXCHVOICES_DATA_DIR + u"exchangevoices-slurs-ref.mscx"));
}

TEST_F(Engraving_ExchangevoicesTests, glissandi)
{
    Score* score = ScoreRW::readScore(EXCHVOICES_DATA_DIR + u"exchangevoices-gliss.mscx");
    EXPECT_TRUE(score);
    score->doLayout();

    // select all
    score->startCmd();
    score->cmdSelectAll();
    score->endCmd();

    // do
    score->startCmd();
    score->cmdExchangeVoice(0, 1);
    score->endCmd();

    // compare
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, u"exchangevoices-gliss.mscx", EXCHVOICES_DATA_DIR + u"exchangevoices-gliss-ref.mscx"));
}

TEST_F(Engraving_ExchangevoicesTests, undoChangeVoice)
{
    String readFile(EXCHVOICES_DATA_DIR + u"undoChangeVoice.mscx");
    String writeFile1(u"undoChangeVoice01-test.mscx");
    String reference1(EXCHVOICES_DATA_DIR + u"undoChangeVoice01-ref.mscx");
    String writeFile2(u"undoChangeVoice02-test.mscx");
    String reference2(EXCHVOICES_DATA_DIR + u"undoChangeVoice02-ref.mscx");

    MasterScore* score = ScoreRW::readScore(readFile);
    EXPECT_TRUE(score);
    score->doLayout();

    std::vector<Note*> notes;

    for (Segment* s = score->firstSegment(SegmentType::ChordRest); s; s = s->next1()) {
        ChordRest* cr = static_cast<ChordRest*>(s->element(0));
        if (cr && cr->type() == ElementType::CHORD) {
            Chord* c = toChord(cr);
            Note* note = c->downNote();
            notes.push_back(note);
        }
    }

    // change voice
    score->startCmd();
    score->changeNotesVoice(notes, 1);
    score->endCmd(false, /*layoutAllParts = */ true);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile1, reference1));

    // undo
    EditData ed;
    score->undoStack()->undo(&ed);
    EXPECT_TRUE(ScoreComp::saveCompareScore(score, writeFile2, reference2));

    delete score;
}
