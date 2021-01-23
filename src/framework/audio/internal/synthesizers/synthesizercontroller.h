//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_AUDIO_SYNTHESIZERCONTROLLER_H
#define MU_AUDIO_SYNTHESIZERCONTROLLER_H

#include "async/asyncable.h"
#include "isynthesizersregister.h"
#include "isoundfontsprovider.h"

namespace mu::audio::synth {
class SynthesizerController : public async::Asyncable
{
public:
    SynthesizerController(const ISynthesizersRegisterPtr& reg, const ISoundFontsProviderPtr& prov);

    void init(uint sampleRate);

private:
    void reloadSoundFonts(ISynthesizerPtr synth);

    ISynthesizersRegisterPtr m_synthRegister;
    ISoundFontsProviderPtr m_soundFontProvider;
};
}

#endif // MU_AUDIO_SYNTHESIZERCONTROLLER_H
