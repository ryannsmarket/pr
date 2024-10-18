/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#pragma once

#include "mpe/events.h"

namespace mu::engraving {
class Note;
struct NominalNoteCtx;
class GlissandosRenderer
{
public:
    static muse::mpe::duration_t discreteGlissandoStepDuration(const Note* note, const muse::mpe::duration_t noteDuration);

    static void renderDiscreteGlissando(const Note* note, NominalNoteCtx& ctx, muse::mpe::PlaybackEventList& result);
    static void renderContinuousGlissando(const Note* note, NominalNoteCtx& ctx, muse::mpe::PlaybackEventList& result);
};
}
