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
#include "soundfontrepositorystub.h"

using namespace muse;
using namespace muse::audio;

const synth::SoundFontPaths& SoundFontRepositoryStub::soundFontPaths() const
{
    static const synth::SoundFontPaths paths;
    return paths;
}

const synth::SoundFontsMap& SoundFontRepositoryStub::soundFonts() const
{
    static const synth::SoundFontsMap map;
    return map;
}

async::Notification SoundFontRepositoryStub::soundFontsChanged() const
{
    return async::Notification();
}

Ret SoundFontRepositoryStub::addSoundFont(const synth::SoundFontPath&)
{
    return make_ret(Ret::Code::NotSupported);
}
