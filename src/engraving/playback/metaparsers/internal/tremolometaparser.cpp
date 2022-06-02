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

#include "tremolometaparser.h"

#include "libmscore/tremolo.h"
#include "libmscore/chord.h"

using namespace mu::engraving;

void TremoloMetaParser::doParse(const mu::engraving::EngravingItem* item, const RenderingContext& ctx, mpe::ArticulationMap& result)
{
    IF_ASSERT_FAILED(item->type() == mu::engraving::ElementType::TREMOLO) {
        return;
    }

    const mu::engraving::Tremolo* tremolo = mu::engraving::toTremolo(item);

    mpe::ArticulationType type = mpe::ArticulationType::Undefined;

    switch (tremolo->tremoloType()) {
    case mu::engraving::TremoloType::R8:
    case mu::engraving::TremoloType::C8:
        type = mpe::ArticulationType::Tremolo8th;
        break;

    case mu::engraving::TremoloType::R16:
    case mu::engraving::TremoloType::C16:
        type = mpe::ArticulationType::Tremolo16th;
        break;

    case mu::engraving::TremoloType::R32:
    case mu::engraving::TremoloType::C32:
        type = mpe::ArticulationType::Tremolo32nd;
        break;

    case mu::engraving::TremoloType::R64:
    case mu::engraving::TremoloType::C64:
        type = mpe::ArticulationType::Tremolo64th;
        break;

    default:
        break;
    }

    if (type == mpe::ArticulationType::Undefined) {
        return;
    }

    int overallDurationTicks = ctx.nominalDurationTicks;
    if (tremolo->twoNotes() && tremolo->chord1() && tremolo->chord2()) {
        overallDurationTicks = tremolo->chord1()->ticks().ticks() + tremolo->chord2()->ticks().ticks();
    }

    mpe::ArticulationMeta articulationMeta;
    articulationMeta.type = type;
    articulationMeta.pattern = ctx.profile->pattern(type);
    articulationMeta.timestamp = ctx.nominalTimestamp;
    articulationMeta.overallDuration = durationFromTicks(ctx.beatsPerSecond.val, overallDurationTicks);

    appendArticulationData(std::move(articulationMeta), result);
}
