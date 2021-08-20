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
#ifndef MU_ENGRAVING_LAYOUTCONTEXT_H
#define MU_ENGRAVING_LAYOUTCONTEXT_H

#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/fraction.h"
#include "libmscore/system.h"
#include "libmscore/spanner.h"

namespace mu::engraving {
//---------------------------------------------------------
//   LayoutContext
//    temp values used during layout
//---------------------------------------------------------

class LayoutContext
{
public:
    LayoutContext(Ms::Score* s);
    LayoutContext(const LayoutContext&) = delete;
    LayoutContext& operator=(const LayoutContext&) = delete;
    ~LayoutContext();

    void layoutLinear();

    void layout();
    int adjustMeasureNo(Ms::MeasureBase*);
    void getNextPage();
    void collectPage();

    Ms::Score* score             { 0 };
    bool startWithLongNames  { true };
    bool firstSystem         { true };
    bool firstSystemIndent   { true };
    Ms::Page* page               { 0 };
    int curPage              { 0 };        // index in Score->page()s
    Ms::Fraction tick            { 0, 1 };

    QList<Ms::System*> systemList;            // reusable systems
    std::set<Ms::Spanner*> processedSpanners;

    Ms::System* prevSystem       { 0 };       // used during page layout
    Ms::System* curSystem        { 0 };

    Ms::MeasureBase* systemOldMeasure { 0 };
    Ms::MeasureBase* pageOldMeasure   { 0 };
    bool rangeDone           { false };

    Ms::MeasureBase* prevMeasure { 0 };
    Ms::MeasureBase* curMeasure  { 0 };
    Ms::MeasureBase* nextMeasure { 0 };
    int measureNo            { 0 };
    Ms::Fraction startTick;
    Ms::Fraction endTick;
};
}

#endif // MU_ENGRAVING_LAYOUTCONTEXT_H
