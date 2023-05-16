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

#include "playtechannotation.h"

#include "layout/tlayout.h"

#include "segment.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

static const ElementStyle annotationStyle {
    { Sid::staffTextPlacement, Pid::PLACEMENT },
    { Sid::staffTextMinDistance, Pid::MIN_DISTANCE },
};

PlayTechAnnotation::PlayTechAnnotation(Segment* parent, PlayingTechniqueType techniqueType, TextStyleType tid)
    : StaffTextBase(ElementType::PLAYTECH_ANNOTATION, parent, tid, ElementFlag::MOVABLE | ElementFlag::ON_STAFF),
    m_techniqueType(techniqueType)
{
    initElementStyle(&annotationStyle);
}

PlayingTechniqueType PlayTechAnnotation::techniqueType() const
{
    return m_techniqueType;
}

void PlayTechAnnotation::setTechniqueType(const PlayingTechniqueType techniqueType)
{
    m_techniqueType = techniqueType;
}

PlayTechAnnotation* PlayTechAnnotation::clone() const
{
    return new PlayTechAnnotation(*this);
}

<<<<<<< HEAD
<<<<<<< HEAD
<<<<<<< HEAD
=======
void PlayTechAnnotation::layout()
{
    UNREACHABLE;
    LayoutContext ctx(score());
    v0::TLayout::layout(this, ctx);
}

>>>>>>> 4f8a1b6dd0... [engraving] replaced item->layout() to TLayout::layout
=======
>>>>>>> 11610ff2b5... [engraving] removed item->layout method
=======
>>>>>>> cd79de8b507ce5e52931bbfbce650f3fc04e0ae2
PropertyValue PlayTechAnnotation::getProperty(Pid id) const
{
    switch (id) {
    case Pid::PLAY_TECH_TYPE:
        return m_techniqueType;
    default:
        return StaffTextBase::getProperty(id);
    }
}

bool PlayTechAnnotation::setProperty(Pid propertyId, const PropertyValue& val)
{
    switch (propertyId) {
    case Pid::PLAY_TECH_TYPE:
        m_techniqueType = PlayingTechniqueType(val.toInt());
        break;
    default:
        if (!StaffTextBase::setProperty(propertyId, val)) {
            return false;
        }
        break;
    }

    triggerLayout();
    return true;
}

PropertyValue PlayTechAnnotation::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::TEXT_STYLE:
        return TextStyleType::STAFF;
    case Pid::PLAY_TECH_TYPE:
        return PlayingTechniqueType::Natural;
    default:
        return StaffTextBase::propertyDefault(id);
    }
}
