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

#ifndef MU_ENGRAVING_PLAYBACKEVENTSRENDERER_H
#define MU_ENGRAVING_PLAYBACKEVENTSRENDERER_H

#include "mpe/events.h"

#include "playbackcontext.h"

namespace Ms {
class Chord;
class Rest;
}

namespace mu::engraving {
class PlaybackEventsRenderer
{
public:
    PlaybackEventsRenderer() = default;

    void render(const Ms::EngravingItem* item, const mpe::dynamic_level_t nominalDynamicLevel, const mpe::ArticulationsProfilePtr profile,
                mpe::PlaybackEventList& result) const;

private:
    void renderNoteEvents(const Ms::Chord* chord, const mpe::dynamic_level_t nominalDynamicLevel,
                          const mpe::ArticulationsProfilePtr profile, mpe::PlaybackEventList& result) const;

    void renderRestEvents(const Ms::Rest* rest, mpe::PlaybackEventList& result) const;

    void renderArticulations(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::PlaybackEventList& result) const;
    bool renderChordArticulations(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::PlaybackEventList& result) const;
    void renderNoteArticulations(const Ms::Chord* chord, const PlaybackContext& ctx, mpe::PlaybackEventList& result) const;
};
}

#endif // MU_ENGRAVING_PLAYBACKEVENTSRENDERER_H
