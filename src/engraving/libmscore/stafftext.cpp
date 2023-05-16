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

#include "stafftext.h"

#include "layout/tlayout.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   staffStyle
//---------------------------------------------------------

static const ElementStyle staffStyle {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Segment* parent, TextStyleType tid)
    : StaffTextBase(ElementType::STAFF_TEXT, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF)
{
    initElementStyle(&staffStyle);
}

//---------------------------------------------------------
<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
//   layout
//---------------------------------------------------------

void StaffText::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

//---------------------------------------------------------
>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
=======
>>>>>>> cd79de8b507ce5e52931bbfbce650f3fc04e0ae2
//   propertyDefault
//---------------------------------------------------------

engraving::PropertyValue StaffText::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::STAFF;
    default:
        return StaffTextBase::propertyDefault(id);
    }
}
}
