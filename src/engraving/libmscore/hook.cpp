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

#include "hook.h"
#include "chord.h"
#include "stem.h"
#include "score.h"
#include "symid.h"

using namespace mu;
using namespace Ms;

Hook::Hook(Chord* parent)
    : Symbol(ElementType::HOOK, parent, ElementFlag::NOTHING)
{
    setZ(int(type()) * 100);
}

EngravingItem* Hook::elementBase() const
{
    return parentItem();
}

void Hook::setHookType(int i)
{
    bool straight = score()->styleB(Sid::useStraightNoteFlags);
    _hookType = i;

    switch (i) {
    case 0:
        break;
    case 1:
        setSym(straight ? SymId::flag8thUpStraight : SymId::flag8thUp);
        break;
    case 2:
        setSym(straight ? SymId::flag16thUpStraight : SymId::flag16thUp);
        break;
    case 3:
        setSym(straight ? SymId::flag32ndUpStraight : SymId::flag32ndUp);
        break;
    case 4:
        setSym(straight ? SymId::flag64thUpStraight : SymId::flag64thUp);
        break;
    case 5:
        setSym(straight ? SymId::flag128thUpStraight : SymId::flag128thUp);
        break;
    case 6:
        setSym(straight ? SymId::flag256thUpStraight : SymId::flag256thUp);
        break;
    case 7:
        setSym(straight ? SymId::flag512thUpStraight : SymId::flag512thUp);
        break;
    case 8:
        setSym(straight ? SymId::flag1024thUpStraight : SymId::flag1024thUp);
        break;

    case -1:
        setSym(straight ? SymId::flag8thDownStraight : SymId::flag8thDown);
        break;
    case -2:
        setSym(straight ? SymId::flag16thDownStraight : SymId::flag16thDown);
        break;
    case -3:
        setSym(straight ? SymId::flag32ndDownStraight : SymId::flag32ndDown);
        break;
    case -4:
        setSym(straight ? SymId::flag64thDownStraight : SymId::flag64thDown);
        break;
    case -5:
        setSym(straight ? SymId::flag128thDownStraight : SymId::flag128thDown);
        break;
    case -6:
        setSym(straight ? SymId::flag256thDownStraight : SymId::flag256thDown);
        break;
    case -7:
        setSym(straight ? SymId::flag512thDownStraight : SymId::flag512thDown);
        break;
    case -8:
        setSym(straight ? SymId::flag1024thDownStraight : SymId::flag1024thDown);
        break;
    default:
        qDebug("no hook/flag for subtype %d", i);
        break;
    }
}

void Hook::layout()
{
    setbbox(symBbox(_sym));
}

void Hook::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    // hide if belonging to the second chord of a cross-measure pair
    if (chord() && chord()->crossMeasure() == CrossMeasure::SECOND) {
        return;
    }

    painter->setPen(curColor());
    drawSymbol(_sym, painter);
}

mu::PointF Hook::smuflAnchor() const
{
    return symSmuflAnchor(_sym, chord()->up() ? SmuflAnchorId::stemUpNW : SmuflAnchorId::stemDownSW);
}
