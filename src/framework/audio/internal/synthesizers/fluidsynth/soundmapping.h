/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This midi::Program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This midi::Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this midi::Program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MU_AUDIO_SOUNDMAPPING_H
#define MU_AUDIO_SOUNDMAPPING_H

#include "mpe/events.h"
#include "midi/miditypes.h"

namespace mu::audio::synth {
struct SoundMappingKey {
    mpe::SoundId id = mpe::SoundId::Undefined;
    mpe::SoundSubCategories subCategories;

    bool operator<(const SoundMappingKey& other) const
    {
        if (id < other.id) {
            return true;
        }

        if (id > other.id) {
            return false;
        }

        return subCategories < other.subCategories;
    }
};

using ArticulationMapping = std::map<mpe::ArticulationType, midi::Program>;

static const auto& mappingByCategory(const mpe::SoundCategory category)
{
    static const std::map<SoundMappingKey, midi::Programs> KEYBOARDS_MAPPINGS = {
        { { mpe::SoundId::Celesta, {} }, { midi::Program(0, 8) } },
        { { mpe::SoundId::Clavichord, { mpe::SoundSubCategory::Baroque } }, { midi::Program(0, 7) } },
        { { mpe::SoundId::Clavichord,  { mpe::SoundSubCategory::Electric } }, { midi::Program(0, 7) } },
        { { mpe::SoundId::OndesMartenot,  { mpe::SoundSubCategory::Electric } }, { midi::Program(0, 79) } },
        { { mpe::SoundId::Harpsichord,  {} }, { midi::Program(0, 6) } },
        { { mpe::SoundId::Virginal,  { mpe::SoundSubCategory::Baroque } }, { midi::Program(0, 6) } },
        { { mpe::SoundId::Piano,  { mpe::SoundSubCategory::Electric } }, { midi::Program(0, 4) } },
        { { mpe::SoundId::Piano,  { mpe::SoundSubCategory::Grand } }, { midi::Program(0, 0) } },
        { { mpe::SoundId::Piano,  { mpe::SoundSubCategory::HonkyTonk } }, { midi::Program(0, 3) } },
        { { mpe::SoundId::Piano,  {} }, { midi::Program(0, 0) } },
        { { mpe::SoundId::Piano,  { mpe::SoundSubCategory::Toy } }, { midi::Program(0, 0) } },
        { { mpe::SoundId::Piano,  { mpe::SoundSubCategory::Upright } }, { midi::Program(0, 0) } },

        { { mpe::SoundId::Organ,  { mpe::SoundSubCategory::Hammond } }, { midi::Program(0, 16) } },
        { { mpe::SoundId::Organ,  {} }, { midi::Program(0, 19) } },
        { { mpe::SoundId::Organ,  { mpe::SoundSubCategory::Percussive } }, { midi::Program(0, 17) } },
        { { mpe::SoundId::Organ,  { mpe::SoundSubCategory::Piped } }, { midi::Program(0, 19) } },
        { { mpe::SoundId::Organ,  { mpe::SoundSubCategory::Rotary } }, { midi::Program(0, 18) } },
        { { mpe::SoundId::Organ,  { mpe::SoundSubCategory::Reed } }, { midi::Program(0, 20) } },

        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric } }, { midi::Program(0, 80) } },
        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric,
                                          mpe::SoundSubCategory::FX_Atmosphere } }, { midi::Program(0, 99) } },
        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric,
                                          mpe::SoundSubCategory::FX_Brightness } }, { midi::Program(0, 100) } },
        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric,
                                          mpe::SoundSubCategory::FX_Crystal } }, { midi::Program(0, 98) } },
        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric,
                                          mpe::SoundSubCategory::FX_Echoes } }, { midi::Program(0, 102) } },
        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric,
                                          mpe::SoundSubCategory::FX_Goblins } }, { midi::Program(0, 101) } },
        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric, mpe::SoundSubCategory::FX_Rain } }, { midi::Program(0, 96) } },
        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric, mpe::SoundSubCategory::FX_Atmosphere,
                                          mpe::SoundSubCategory::FX_Brightness, mpe::SoundSubCategory::FX_Crystal,
                                          mpe::SoundSubCategory::FX_Echoes, mpe::SoundSubCategory::FX_Goblins,
                                          mpe::SoundSubCategory::FX_Rain } }, { midi::Program(0, 80), midi::Program(0, 99), midi::Program(0,
                                                                                                                                          100),
                                                                                midi::Program(0, 98), midi::Program(0, 102), midi::Program(
                                                                                    0, 101),
                                                                                midi::Program(0, 96) } },
    };

    static const std::map<SoundMappingKey, midi::Programs> STRINGS_MAPPINGS = {
        { { mpe::SoundId::Harp,  {} }, { midi::Program(0, 46) } },

        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Acoustic,
                                     mpe::SoundSubCategory::Nylon,
                                     mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 24) } },
        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Acoustic,
                                     mpe::SoundSubCategory::Nylon,
                                     mpe::SoundSubCategory::Alto } }, { midi::Program(0, 24) } },
        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Acoustic,
                                     mpe::SoundSubCategory::Nylon,
                                     mpe::SoundSubCategory::Baritone } }, { midi::Program(0, 24) } },
        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Acoustic,
                                     mpe::SoundSubCategory::Nylon,
                                     mpe::SoundSubCategory::Contra } }, { midi::Program(0, 24) } },

        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Electric } }, { midi::Program(0, 34) } },

        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Acoustic,
                                     mpe::SoundSubCategory::Steel } }, { midi::Program(0, 25) } },

        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Acoustic,
                                     mpe::SoundSubCategory::Pedal,
                                     mpe::SoundSubCategory::Steel } }, { midi::Program(0, 25) } },

        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Acoustic } }, { midi::Program(0, 24) } },

        { { mpe::SoundId::Guitar,  { mpe::SoundSubCategory::Acoustic,
                                     mpe::SoundSubCategory::Nylon } }, { midi::Program(0, 24) } },

        { { mpe::SoundId::BassGuitar,  { mpe::SoundSubCategory::Electric,
                                         mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 33) } },
        { { mpe::SoundId::BassGuitar,  { mpe::SoundSubCategory::Electric } }, { midi::Program(0, 34) } },
        { { mpe::SoundId::BassGuitar,  { mpe::SoundSubCategory::Acoustic } }, { midi::Program(0, 32) } },

        { { mpe::SoundId::Banjo,  {} }, { midi::Program(0, 105) } },
        { { mpe::SoundId::Banjo,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 105) } },
        { { mpe::SoundId::Banjo,  { mpe::SoundSubCategory::Irish,
                                    mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 105) } },

        { { mpe::SoundId::Ukulele,  {} }, { midi::Program(8, 24) } },
        { { mpe::SoundId::Ukulele,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(8, 24) } },
        { { mpe::SoundId::Ukulele,  { mpe::SoundSubCategory::Baritone } }, { midi::Program(8, 24) } },

        { { mpe::SoundId::Mandolin,  {} }, { midi::Program(16, 25) } },
        { { mpe::SoundId::Mandolin,  { mpe::SoundSubCategory::Alto } }, { midi::Program(16, 25) } },
        { { mpe::SoundId::Mandolin,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(16, 25) } },
        { { mpe::SoundId::Mandolin,  { mpe::SoundSubCategory::Octave } }, { midi::Program(16, 25) } },

        { { mpe::SoundId::MtnDulcimer,  {} }, { midi::Program(0, 25) } },
        { { mpe::SoundId::MtnDulcimer,  { mpe::SoundSubCategory::Baritone } }, { midi::Program(16, 25) } },
        { { mpe::SoundId::MtnDulcimer,  { mpe::SoundSubCategory::Bass } }, { midi::Program(16, 25) } },

        { { mpe::SoundId::Lute,  {} }, { midi::Program(8, 25) } },
        { { mpe::SoundId::Lute,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(8, 25) } },
        { { mpe::SoundId::Lute,  { mpe::SoundSubCategory::Baroque } }, { midi::Program(8, 25) } },
        { { mpe::SoundId::Lute,  { } }, { midi::Program(8, 25) } },
        { { mpe::SoundId::Lute,  { mpe::SoundSubCategory::Greek } }, { midi::Program(8, 25) } },

        { { mpe::SoundId::Balalaika,  { mpe::SoundSubCategory::Prima } }, { midi::Program(0, 107) } },
        { { mpe::SoundId::Balalaika,  { mpe::SoundSubCategory::Prima,
                                        mpe::SoundSubCategory::Alto } }, { midi::Program(0, 107) } },
        { { mpe::SoundId::Balalaika,  { mpe::SoundSubCategory::Prima,
                                        mpe::SoundSubCategory::Bass } }, { midi::Program(0, 107) } },
        { { mpe::SoundId::Balalaika,  { mpe::SoundSubCategory::Prima,
                                        mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 107) } },
        { { mpe::SoundId::Balalaika,  { mpe::SoundSubCategory::Secunda } }, { midi::Program(0, 107) } },

        { { mpe::SoundId::Koto,  { mpe::SoundSubCategory::Japanese } }, { midi::Program(0, 107) } },
        { { mpe::SoundId::Shamisen,  { mpe::SoundSubCategory::Japanese } }, { midi::Program(0, 106) } },
        { { mpe::SoundId::Sitar,  { mpe::SoundSubCategory::Indian } }, { midi::Program(0, 104) } },
        { { mpe::SoundId::Oud,  { mpe::SoundSubCategory::African } }, { midi::Program(8, 25) } },
        { { mpe::SoundId::Prim,  { } }, { midi::Program(0, 24) } },
        { { mpe::SoundId::Brac,  { } }, { midi::Program(0, 24) } },
        { { mpe::SoundId::Bugarija,  { } }, { midi::Program(0, 24) } },
        { { mpe::SoundId::Berda,  { } }, { midi::Program(0, 32) } },
        { { mpe::SoundId::Celo,  { } }, { midi::Program(0, 24) } },
        { { mpe::SoundId::Bandurria,  { mpe::SoundSubCategory::Spanish } }, { midi::Program(0, 24) } },
        { { mpe::SoundId::Laud,  { mpe::SoundSubCategory::Spanish } }, { midi::Program(0, 24) } },

        { { mpe::SoundId::StringsGroup,  { } }, { midi::Program(0, 48) } },
        { { mpe::SoundId::Contrabass,  { mpe::SoundSubCategory::Orchestral } }, { midi::Program(0, 43) } },
        { { mpe::SoundId::ContrabasseSection,  { mpe::SoundSubCategory::Orchestral } }, { midi::Program(50, 48) } },
        { { mpe::SoundId::Violin,  { mpe::SoundSubCategory::Orchestral } }, { midi::Program(0, 40) } },
        { { mpe::SoundId::ViolinSection,  { mpe::SoundSubCategory::Orchestral } }, { midi::Program(20, 48) } },
        { { mpe::SoundId::Viola,  { mpe::SoundSubCategory::Orchestral } }, { midi::Program(0, 41) } },
        { { mpe::SoundId::ViolaSection,  { mpe::SoundSubCategory::Orchestral } }, { midi::Program(30, 48) } },
        { { mpe::SoundId::Violoncello,  { mpe::SoundSubCategory::Orchestral } }, { midi::Program(0, 42) } },
        { { mpe::SoundId::VioloncelloSection,  { mpe::SoundSubCategory::Orchestral } }, { midi::Program(40, 48) } },

        { { mpe::SoundId::Viol,  { } }, { midi::Program(0, 40) } },
        { { mpe::SoundId::Viol,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 41) } },
        { { mpe::SoundId::PardessusViol,  {} }, { midi::Program(0, 40) } },
        { { mpe::SoundId::Viol,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 41) } },
        { { mpe::SoundId::Viol,  { mpe::SoundSubCategory::Baritone } }, { midi::Program(0, 42) } },
        { { mpe::SoundId::ViolaDaGamba,  { } }, { midi::Program(0, 42) } },
        { { mpe::SoundId::Violone,  { } }, { midi::Program(0, 43) } },

        { { mpe::SoundId::Octobass,  { } }, { midi::Program(0, 43) } },
        { { mpe::SoundId::Erhu,  { mpe::SoundSubCategory::Chinese } }, { midi::Program(0, 110) } },
        { { mpe::SoundId::Nyckelharpa,  { mpe::SoundSubCategory::Swedish } }, { midi::Program(0, 41) } },

        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric,
                                          mpe::SoundSubCategory::Bass } }, { midi::Program(0, 38) } },
        { { mpe::SoundId::Synthesizer,  { mpe::SoundSubCategory::Electric } }, { midi::Program(0, 92) } },
    };

    static const std::map<SoundMappingKey, midi::Programs> WINDS_MAPPINGS = {
        { { mpe::SoundId::WindsGroup,  {} }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Piccolo,  {} }, { midi::Program(0, 72) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Treble } }, { midi::Program(0, 72) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Contra_Alto } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Sub_Contra_Alto } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Double_Contra_Bass } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Hyper_Bass } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { mpe::SoundSubCategory::Irish } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Flute,  { } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Traverso,  { mpe::SoundSubCategory::Baroque } }, { midi::Program(0, 73) } },
        { { mpe::SoundId::Danso,  {} }, { midi::Program(0, 76) } },

        { { mpe::SoundId::Dizi,  { mpe::SoundSubCategory::Chinese } }, { midi::Program(0, 72) } },

        { { mpe::SoundId::Shakuhachi,  { mpe::SoundSubCategory::Japanese } }, { midi::Program(0, 77) } },
        { { mpe::SoundId::Fife,  { } }, { midi::Program(0, 72) } },

        { { mpe::SoundId::Whistle,  { mpe::SoundSubCategory::Tin } }, { midi::Program(0, 78) } },
        { { mpe::SoundId::Whistle,  { mpe::SoundSubCategory::Slide } }, { midi::Program(0, 79) } },

        { { mpe::SoundId::Flageolet,  {} }, { midi::Program(0, 78) } },

        { { mpe::SoundId::Recorder,  { mpe::SoundSubCategory::Garklein } }, { midi::Program(0, 74) } },
        { { mpe::SoundId::Recorder,  { mpe::SoundSubCategory::Sopranino } }, { midi::Program(0, 74) } },
        { { mpe::SoundId::Recorder,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 74) } },
        { { mpe::SoundId::Recorder,  {} }, { midi::Program(0, 74) } },
        { { mpe::SoundId::Recorder,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 74) } },
        { { mpe::SoundId::Recorder,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 74) } },
        { { mpe::SoundId::Recorder,  { mpe::SoundSubCategory::Great_Bass } }, { midi::Program(0, 74) } },
        { { mpe::SoundId::Recorder,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 74) } },

        { { mpe::SoundId::Ocarina,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 79) } },
        { { mpe::SoundId::Ocarina,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 79) } },
        { { mpe::SoundId::Ocarina,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 79) } },

        { { mpe::SoundId::Gemshorn,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 79) } },
        { { mpe::SoundId::Gemshorn,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 79) } },
        { { mpe::SoundId::Gemshorn,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 79) } },
        { { mpe::SoundId::Gemshorn,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 79) } },

        { { mpe::SoundId::PanFlute,  {} }, { midi::Program(0, 75) } },

        { { mpe::SoundId::Quena,  {} }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Heckelphone,  {} }, { midi::Program(0, 68) } },
        { { mpe::SoundId::Oboe,  { mpe::SoundSubCategory::Baroque } }, { midi::Program(0, 69) } },
        { { mpe::SoundId::Oboe,  {} }, { midi::Program(0, 68) } },
        { { mpe::SoundId::Oboe,  { mpe::SoundSubCategory::English } }, { midi::Program(0, 69) } },
        { { mpe::SoundId::Oboe,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 68) } },
        { { mpe::SoundId::Lupophone,  {} }, { midi::Program(0, 68) } },

        { { mpe::SoundId::Shawm,  { mpe::SoundSubCategory::Sopranino } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Shawm,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Shawm,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Shawm,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Shawm,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Shawm,  { mpe::SoundSubCategory::Great_Bass } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Cromorne,  { mpe::SoundSubCategory::French,
                                       mpe::SoundSubCategory::Baroque,
                                       mpe::SoundSubCategory::Reed } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Crumhorn,  {} }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Crumhorn,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Crumhorn,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Crumhorn,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Crumhorn,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Crumhorn,  { mpe::SoundSubCategory::Great_Bass } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Cornamuse,  {} }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Cornamuse,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Cornamuse,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Cornamuse,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Cornamuse,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Kelhorn,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Kelhorn,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Kelhorn,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Kelhorn,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Kelhorn,  { mpe::SoundSubCategory::Great_Bass } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Rauschpfeife,  { mpe::SoundSubCategory::Sopranino } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Rauschpfeife,  {} }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Rauschpfeife,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Duduk,  { mpe::SoundSubCategory::Armenian } }, { midi::Program(0, 71) } },
        { { mpe::SoundId::Duduk,  { mpe::SoundSubCategory::Bass, mpe::SoundSubCategory::Armenian } }, { midi::Program(0, 71) } },

        { { mpe::SoundId::Shenai,  { mpe::SoundSubCategory::Indian } }, { midi::Program(0, 111) } },

        { { mpe::SoundId::Clarinet,  {} }, { midi::Program(0, 71) } },
        { { mpe::SoundId::Clarinet,  { mpe::SoundSubCategory::Sopranino } }, { midi::Program(0, 71) } },
        { { mpe::SoundId::Clarinet,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 71) } },
        { { mpe::SoundId::Clarinet,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 71) } },
        { { mpe::SoundId::Clarinet,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 71) } },
        { { mpe::SoundId::Clarinet,  { mpe::SoundSubCategory::Contra_Alto } }, { midi::Program(0, 71) } },
        { { mpe::SoundId::Clarinet,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 71) } },

        { { mpe::SoundId::Chalumeau,  {} }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Chalumeau,  { mpe::SoundSubCategory::Sopranino } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Chalumeau,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Chalumeau,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Chalumeau,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Chalumeau,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Xaphoon,  {} }, { midi::Program(0, 71) } },

        { { mpe::SoundId::Tarogato,  { mpe::SoundSubCategory::Hungarian, mpe::SoundSubCategory::Romanian } }, { midi::Program(0, 69) } },

        { { mpe::SoundId::Octavin,  {} }, { midi::Program(0, 69) } },

        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Sopranissimo } }, { midi::Program(0, 64) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Sopranino } }, { midi::Program(0, 64) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 64) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Mezzo_Soprano } }, { midi::Program(0, 65) } },
        { { mpe::SoundId::HeckelphoneClarinet,  {} }, { midi::Program(0, 68) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 65) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Melody } }, { midi::Program(0, 66) } },
        { { mpe::SoundId::Saxophone,  {} }, { midi::Program(0, 66) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 66) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Baritone } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Saxophone,  { mpe::SoundSubCategory::Sub_Contra_Bass } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Bassoon,  {} }, { midi::Program(0, 70) } },
        { { mpe::SoundId::Contrabassoon,  {} }, { midi::Program(0, 70) } },
        { { mpe::SoundId::Contrabassoon,  { mpe::SoundSubCategory::Reed } }, { midi::Program(0, 70) } },
        { { mpe::SoundId::Dulcian,  {} }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Rackett,  {} }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Sarrusophone,  { mpe::SoundSubCategory::Sopranino } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Sarrusophone,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Sarrusophone,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Sarrusophone,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Sarrusophone,  { mpe::SoundSubCategory::Baritone } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Sarrusophone,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 67) } },
        { { mpe::SoundId::Sarrusophone,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 67) } },

        { { mpe::SoundId::Bagpipe,  {} }, { midi::Program(0, 109) } },
        { { mpe::SoundId::Accordion,  {} }, { midi::Program(0, 21) } },
        { { mpe::SoundId::Bandoneon,  {} }, { midi::Program(0, 23) } },
        { { mpe::SoundId::Concertina,  {} }, { midi::Program(0, 21) } },

        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Diatonic } }, { midi::Program(0, 22) } },
        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Chromatic } }, { midi::Program(0, 22) } },
        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Chromatic, mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 22) } },
        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Chromatic } }, { midi::Program(0, 22) } },
        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Huang } }, { midi::Program(0, 22) } },
        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Bass, mpe::SoundSubCategory::Hohner } }, { midi::Program(0, 22) } },
        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Bass, mpe::SoundSubCategory::Huang } }, { midi::Program(0, 22) } },
        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 22) } },

        { { mpe::SoundId::Melodica,  {} }, { midi::Program(0, 22) } },

        { { mpe::SoundId::Sheng,  {} }, { midi::Program(0, 20) } },
        { { mpe::SoundId::Sheng,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 20) } },
        { { mpe::SoundId::Sheng,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 20) } },
        { { mpe::SoundId::Sheng,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 20) } },
        { { mpe::SoundId::Sheng,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 20) } },

        { { mpe::SoundId::BrassGroup,  {} }, { midi::Program(0, 61) } },

        { { mpe::SoundId::Horn,  {} }, { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Horn,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Horn,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Horn,  { mpe::SoundSubCategory::Vienna } }, { midi::Program(0, 60), midi::Program(0, 59) } },

        { { mpe::SoundId::Tuba,  { mpe::SoundSubCategory::Wagner } }, { midi::Program(0, 56) } },

        { { mpe::SoundId::Cornet,  {} }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Cornet,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 56), midi::Program(0, 59) } },

        { { mpe::SoundId::Saxhorn,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Horn,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Horn,  { mpe::SoundSubCategory::Baritone } }, { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Horn,  { mpe::SoundSubCategory::Baritone, mpe::SoundSubCategory::Treble } },
            { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Horn,  { mpe::SoundSubCategory::CentralEuropean,
                                   mpe::SoundSubCategory::Baritone,
                                   mpe::SoundSubCategory::Treble } }, { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Horn,  { mpe::SoundSubCategory::Baritone,
                                   mpe::SoundSubCategory::CentralEuropean } }, { midi::Program(0, 60), midi::Program(0, 59) } },
        { { mpe::SoundId::Posthorn,  {} }, { midi::Program(0, 60), midi::Program(0, 59) } },

        { { mpe::SoundId::Trumpet,  {} }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trumpet,  { mpe::SoundSubCategory::Piccolo } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trumpet,  { mpe::SoundSubCategory::Pocket } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trumpet,  { mpe::SoundSubCategory::Slide } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trumpet,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trumpet,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trumpet,  { mpe::SoundSubCategory::Baroque } }, { midi::Program(0, 56), midi::Program(0, 59) } },

        { { mpe::SoundId::MellophoneBugle,  {} }, { midi::Program(0, 58) } },
        { { mpe::SoundId::EuphoniumBugle,  {} }, { midi::Program(0, 58) } },
        { { mpe::SoundId::MellophoneBugle,  {} }, { midi::Program(0, 58) } },

        { { mpe::SoundId::Bugle,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Bugle,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Bugle,  { mpe::SoundSubCategory::Baritone } }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Bugle,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 58) } },

        { { mpe::SoundId::Fiscorn,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 56), midi::Program(0, 59) } },

        { { mpe::SoundId::Flugelhorn,  {} }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Kuhlohorn,  {} }, { midi::Program(0, 56), midi::Program(0, 59) } },

        { { mpe::SoundId::Ophicleide,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 56) } },
        { { mpe::SoundId::Ophicleide,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 56) } },
        { { mpe::SoundId::Ophicleide,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 56) } },

        { { mpe::SoundId::Cornettino,  {} }, { midi::Program(0, 56) } },

        { { mpe::SoundId::Cornett,  {} }, { midi::Program(0, 56) } },
        { { mpe::SoundId::Cornett,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 56) } },
        { { mpe::SoundId::Cornett,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 56) } },
        { { mpe::SoundId::Cornett,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 56) } },

        { { mpe::SoundId::Serpent,  {} }, { midi::Program(0, 56) } },

        { { mpe::SoundId::Trombone,  {} }, { midi::Program(0, 57), midi::Program(0, 59) } },
        { { mpe::SoundId::Trombone,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trombone,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trombone,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trombone,  { mpe::SoundSubCategory::Treble } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trombone,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Trombone,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 56), midi::Program(0, 59) } },

        { { mpe::SoundId::Cimbasso,  {} }, { midi::Program(0, 57) } },

        { { mpe::SoundId::Sackbut,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 57) } },
        { { mpe::SoundId::Sackbut,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 57) } },
        { { mpe::SoundId::Sackbut,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 57) } },

        { { mpe::SoundId::Euphonium,  {} }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Euphonium,  { mpe::SoundSubCategory::Treble } }, { midi::Program(0, 58) } },

        { { mpe::SoundId::Tuba,  {} }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Tuba,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Tuba,  { mpe::SoundSubCategory::Treble,
                                   mpe::SoundSubCategory::Bass } }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Tuba,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Tuba,  { mpe::SoundSubCategory::Treble } }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Tuba,  { mpe::SoundSubCategory::Sub_Contra_Bass } }, { midi::Program(0, 58) } },

        { { mpe::SoundId::Sousaphone,  {} }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Sousaphone,  { mpe::SoundSubCategory::Treble } }, { midi::Program(0, 58) } },
        { { mpe::SoundId::Helicon,  {} }, { midi::Program(0, 58) } },

        { { mpe::SoundId::Conch,  {} }, { midi::Program(0, 60) } },

        { { mpe::SoundId::Horagai,  { mpe::SoundSubCategory::Japanese } }, { midi::Program(0, 60) } },

        { { mpe::SoundId::Alphorn,  { mpe::SoundSubCategory::Alpine } }, { midi::Program(0, 60) } },
        { { mpe::SoundId::RagDung,  { mpe::SoundSubCategory::Tibetian } }, { midi::Program(0, 56), midi::Program(0, 59) } },
        { { mpe::SoundId::Didgeridoo,  { mpe::SoundSubCategory::Australian } }, { midi::Program(0, 57) } },
        { { mpe::SoundId::Shofar,  {} }, { midi::Program(0, 60) } },
        { { mpe::SoundId::Vuvuzela,  {} }, { midi::Program(0, 56) } },
        { { mpe::SoundId::KlaxonHorns,  {} }, { midi::Program(0, 84) } },
        { { mpe::SoundId::Kazoo,  {} }, { midi::Program(0, 85) } }
    };

    static const std::map<SoundMappingKey, midi::Programs> PERCUSSION_MAPPINGS = {
        { { mpe::SoundId::Timpani,  {} }, { midi::Program(0, 47) } },
        { { mpe::SoundId::RotoToms,  {} }, { midi::Program(0, 117) } },
        { { mpe::SoundId::Tubaphone,  { mpe::SoundSubCategory::Metal } }, { midi::Program(0, 47) } },

        { { mpe::SoundId::SteelDrums,  { mpe::SoundSubCategory::Metal,
                                         mpe::SoundSubCategory::Steel,
                                         mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 114) } },
        { { mpe::SoundId::SteelDrums,  { mpe::SoundSubCategory::Metal,
                                         mpe::SoundSubCategory::Steel,
                                         mpe::SoundSubCategory::Alto } }, { midi::Program(0, 114) } },
        { { mpe::SoundId::SteelDrums,  { mpe::SoundSubCategory::Metal,
                                         mpe::SoundSubCategory::Steel } }, { midi::Program(0, 114) } },
        { { mpe::SoundId::SteelDrums,  { mpe::SoundSubCategory::Metal,
                                         mpe::SoundSubCategory::Steel,
                                         mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 114) } },
        { { mpe::SoundId::SteelDrums,  { mpe::SoundSubCategory::Metal,
                                         mpe::SoundSubCategory::Steel,
                                         mpe::SoundSubCategory::Bass } }, { midi::Program(0, 114) } },

        { { mpe::SoundId::Vibraphone,  {} }, { midi::Program(0, 11) } },
        { { mpe::SoundId::Dulcimer,  {} }, { midi::Program(0, 15) } },
        { { mpe::SoundId::Cimbalom,  {} }, { midi::Program(0, 15) } },

        { { mpe::SoundId::Xylomarimba,  {} }, { midi::Program(0, 12) } },
        { { mpe::SoundId::Marimba,  {} }, { midi::Program(0, 12) } },
        { { mpe::SoundId::Marimba,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 12) } },
        { { mpe::SoundId::Marimba,  { mpe::SoundSubCategory::Contra_Bass } }, { midi::Program(0, 12) } },

        { { mpe::SoundId::Crotales,  { mpe::SoundSubCategory::Metal } }, { midi::Program(0, 10) } },

        { { mpe::SoundId::Carillon,  { mpe::SoundSubCategory::Metal } }, { midi::Program(0, 14) } },
        { { mpe::SoundId::Gong,  { mpe::SoundSubCategory::Metal } }, { midi::Program(0, 14) } },

        { { mpe::SoundId::Glockenspiel,  {} }, { midi::Program(0, 9) } },
        { { mpe::SoundId::Glockenspiel,  { mpe::SoundSubCategory::Soprano,
                                           mpe::SoundSubCategory::Orff } }, { midi::Program(0, 9) } },
        { { mpe::SoundId::Glockenspiel,  { mpe::SoundSubCategory::Alto,
                                           mpe::SoundSubCategory::Orff } }, { midi::Program(0, 9) } },

        { { mpe::SoundId::Metallophone,  { mpe::SoundSubCategory::Orff } }, { midi::Program(0, 11) } },
        { { mpe::SoundId::Metallophone,  { mpe::SoundSubCategory::Soprano,
                                           mpe::SoundSubCategory::Orff } }, { midi::Program(0, 11) } },
        { { mpe::SoundId::Metallophone,  { mpe::SoundSubCategory::Alto,
                                           mpe::SoundSubCategory::Orff } }, { midi::Program(0, 11) } },
        { { mpe::SoundId::Metallophone,  { mpe::SoundSubCategory::Bass,
                                           mpe::SoundSubCategory::Orff } }, { midi::Program(0, 11) } },

        { { mpe::SoundId::Xylophone,  {} }, { midi::Program(0, 13) } },
        { { mpe::SoundId::Xylophone,  { mpe::SoundSubCategory::Soprano,
                                        mpe::SoundSubCategory::Orff } }, { midi::Program(0, 13) } },
        { { mpe::SoundId::Xylophone,  { mpe::SoundSubCategory::Alto,
                                        mpe::SoundSubCategory::Orff } }, { midi::Program(0, 13) } },
        { { mpe::SoundId::Xylophone,  { mpe::SoundSubCategory::Bass,
                                        mpe::SoundSubCategory::Orff } }, { midi::Program(0, 13) } },

        { { mpe::SoundId::Flexatone,  { mpe::SoundSubCategory::Metal } }, { midi::Program(0, 14) } },

        { { mpe::SoundId::MusicalSaw,  { mpe::SoundSubCategory::Metal } }, { midi::Program(0, 92) } },

        { { mpe::SoundId::MusicalGlasses,  { mpe::SoundSubCategory::Glass } }, { midi::Program(0, 93) } },
        { { mpe::SoundId::Harmonica,  { mpe::SoundSubCategory::Glass } }, { midi::Program(0, 92) } },

        { { mpe::SoundId::Kalimba,  {} }, { midi::Program(0, 108) } },
        { { mpe::SoundId::Kalimba,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 108) } },
        { { mpe::SoundId::Kalimba,  { mpe::SoundSubCategory::Treble } }, { midi::Program(0, 108) } },

        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Metal,
                                   mpe::SoundSubCategory::Brake } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Bongos,  {} }, { midi::Program(128, 48) } },
        { { mpe::SoundId::TomToms,  { mpe::SoundSubCategory::Chinese } }, { midi::Program(128, 48) } },
        { { mpe::SoundId::TomToms,  {} }, { midi::Program(128, 48) } },

        { { mpe::SoundId::Conga,  {} }, { midi::Program(128, 48) } },
        { { mpe::SoundId::Cuica,  {} }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Drumset,  {} }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Bass } }, { midi::Program(128, 48) } },
        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Snare } }, { midi::Program(128, 48) } },
        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Millitary } }, { midi::Program(128, 48) } },
        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Frame } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Snare,
                                   mpe::SoundSubCategory::Piccolo } }, { midi::Program(128, 48) } },
        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Slit } }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Tablas,  { mpe::SoundSubCategory::Indian } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Timbales,  {} }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Anvil,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 48) } },
        { { mpe::SoundId::Gong,  { mpe::SoundSubCategory::Metal,
                                   mpe::SoundSubCategory::Bowl } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Chain,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Bell,  { mpe::SoundSubCategory::Metal,
                                   mpe::SoundSubCategory::Hand } }, { midi::Program(0, 112) } },
        { { mpe::SoundId::Bell,  { mpe::SoundSubCategory::Plate,
                                   mpe::SoundSubCategory::Metal } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Bell,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Bell,  { mpe::SoundSubCategory::Metal,
                                   mpe::SoundSubCategory::Cow } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Bell,  { mpe::SoundSubCategory::Metal,
                                   mpe::SoundSubCategory::Sleigh } }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Cymbal,  { mpe::SoundSubCategory::Metal,
                                     mpe::SoundSubCategory::Crash } }, { midi::Program(128, 48) } },
        { { mpe::SoundId::Cymbal,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 48) } },
        { { mpe::SoundId::Cymbal,  { mpe::SoundSubCategory::Metal,
                                     mpe::SoundSubCategory::Finger } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Cymbal,  { mpe::SoundSubCategory::Metal,
                                     mpe::SoundSubCategory::Chinese } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Cymbal,  { mpe::SoundSubCategory::Metal,
                                     mpe::SoundSubCategory::Splash } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Cymbal,  { mpe::SoundSubCategory::Metal,
                                     mpe::SoundSubCategory::Ride } }, { midi::Program(128, 0) } },

        { { mpe::SoundId::HiHat,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Pipe,  { mpe::SoundSubCategory::Metal,
                                   mpe::SoundSubCategory::Iron } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Castanet,  { mpe::SoundSubCategory::Wooden } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Castanet,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Chimes,  {} }, { midi::Program(0, 14) } },
        { { mpe::SoundId::Chimes,  { mpe::SoundSubCategory::Metal,
                                     mpe::SoundSubCategory::Wind } }, { midi::Program(0, 14) } },
        { { mpe::SoundId::Chimes,  { mpe::SoundSubCategory::Wooden,
                                     mpe::SoundSubCategory::Wind } }, { midi::Program(0, 14) } },
        { { mpe::SoundId::Chimes,  { mpe::SoundSubCategory::Glass,
                                     mpe::SoundSubCategory::Wind } }, { midi::Program(0, 14) } },
        { { mpe::SoundId::Chimes,  { mpe::SoundSubCategory::Shell,
                                     mpe::SoundSubCategory::Wind } }, { midi::Program(0, 14) } },

        { { mpe::SoundId::TamTam,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Thundersheet,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Triangle,  { mpe::SoundSubCategory::Metal } }, { midi::Program(128, 48) } },

        { { mpe::SoundId::Claves,  { mpe::SoundSubCategory::Wooden } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Guiro,  { mpe::SoundSubCategory::Wooden } }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Block,  { mpe::SoundSubCategory::Wooden,
                                    mpe::SoundSubCategory::Temple } }, { midi::Program(0, 115) } },
        { { mpe::SoundId::Block,  { mpe::SoundSubCategory::Wooden } }, { midi::Program(0, 115) } },
        { { mpe::SoundId::Block,  { mpe::SoundSubCategory::Sandpaper } }, { midi::Program(0, 115) } },

        { { mpe::SoundId::Cabasa,  {} }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Maraca,  {} }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Quijada,  {} }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Ratchet,  {} }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Shaker,  {} }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Stones,  {} }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Tambourine,  {} }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Tubo,  {} }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Vibraslap,  {} }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Whip,  {} }, { midi::Program(128, 0) } },

        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Marching,
                                   mpe::SoundSubCategory::Snare } }, { midi::Program(128, 56) } },
        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Marching,
                                   mpe::SoundSubCategory::Snare,
                                   mpe::SoundSubCategory::Tenor } }, { midi::Program(128, 96) } },
        { { mpe::SoundId::Drum,  { mpe::SoundSubCategory::Marching,
                                   mpe::SoundSubCategory::Snare,
                                   mpe::SoundSubCategory::Bass } }, { midi::Program(128, 59) } },
        { { mpe::SoundId::Cymbal,  { mpe::SoundSubCategory::Marching,
                                     mpe::SoundSubCategory::Metal,
                                     mpe::SoundSubCategory::Crash } }, { midi::Program(128, 58) } },

        { { mpe::SoundId::Snap,  { mpe::SoundSubCategory::Finger } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Clap,  { mpe::SoundSubCategory::Hand } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Slap,  { mpe::SoundSubCategory::Hand } }, { midi::Program(128, 0) } },
        { { mpe::SoundId::Stamp,  { mpe::SoundSubCategory::Foot } }, { midi::Program(128, 0) } }
    };

    static const std::map<SoundMappingKey, midi::Programs> VOICE_MAPPINGS = {
        { { mpe::SoundId::Choir,  {} }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Soprano, mpe::SoundSubCategory::Boy } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Soprano } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Mezzo_Soprano } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Counter_Tenor } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Alto } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Contra_Alto } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Tenor } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Baritone } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Bass } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Female } }, { midi::Program(0, 52) } },
        { { mpe::SoundId::Choir,  { mpe::SoundSubCategory::Male } }, { midi::Program(0, 52) } }
    };

    switch (category) {
    case mpe::SoundCategory::Keyboards:
        return KEYBOARDS_MAPPINGS;
    case mpe::SoundCategory::Strings:
        return STRINGS_MAPPINGS;
    case mpe::SoundCategory::Winds:
        return WINDS_MAPPINGS;
    case mpe::SoundCategory::Percussions:
        return PERCUSSION_MAPPINGS;
    case mpe::SoundCategory::Voices:
        return VOICE_MAPPINGS;
    default:
        static std::map<SoundMappingKey, midi::Programs> empty;
        return empty;
    }
}

static const midi::Programs& findPrograms(const mpe::PlaybackSetupData& setupData)
{
    const std::map<SoundMappingKey, midi::Programs>& mapping = mappingByCategory(setupData.category);

    auto search = mapping.find({ setupData.id, setupData.subCategorySet });

    if (search != mapping.cend()) {
        return search->second;
    }

    static midi::Programs empty;
    return empty;
}

static const ArticulationMapping& articulationSounds(const mpe::PlaybackSetupData& setupData)
{
    static ArticulationMapping ELECTRIC_GUITAR = {
        { mpe::ArticulationType::Mute, midi::Program(0, 28) },
        { mpe::ArticulationType::Harmonic, midi::Program(0, 31) },
        { mpe::ArticulationType::Distortion, midi::Program(0, 30) },
        { mpe::ArticulationType::Overdrive, midi::Program(0, 29) }
    };

    static ArticulationMapping ACOUSTIC_GUITAR = {
        { mpe::ArticulationType::Mute, midi::Program(0, 28) },
        { mpe::ArticulationType::Harmonic, midi::Program(8, 31) }
    };

    static ArticulationMapping ELECTRIC_BASS_GUITAR = {
        { mpe::ArticulationType::Slap, midi::Program(0, 36) },
        { mpe::ArticulationType::Pop, midi::Program(0, 37) },
        { mpe::ArticulationType::Mute, midi::Program(0, 28) },
        { mpe::ArticulationType::Tremolo8th, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo16th, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo32nd, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo64th, midi::Program(0, 44) },
    };

    static ArticulationMapping ACOUSTIC_BASS_GUITAR = {
        { mpe::ArticulationType::Slap, midi::Program(0, 36) },
        { mpe::ArticulationType::Pop, midi::Program(0, 37) },
        { mpe::ArticulationType::Pizzicato, midi::Program(0, 32) },
        { mpe::ArticulationType::Mute, midi::Program(0, 32) },
        { mpe::ArticulationType::Tremolo8th, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo16th, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo32nd, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo64th, midi::Program(0, 44) },
    };

    static ArticulationMapping BASIC_STRING_SECTION = {
        { mpe::ArticulationType::SnapPizzicato, midi::Program(0, 45) },
        { mpe::ArticulationType::Pizzicato, midi::Program(0, 45) },
        { mpe::ArticulationType::Mute, midi::Program(0, 45) },
        { mpe::ArticulationType::Tremolo8th, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo16th, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo32nd, midi::Program(0, 44) },
        { mpe::ArticulationType::Tremolo64th, midi::Program(0, 44) },
    };

    static ArticulationMapping VIOLIN_SECTION = {
        { mpe::ArticulationType::SnapPizzicato, midi::Program(20, 45) },
        { mpe::ArticulationType::Pizzicato, midi::Program(20, 45) },
        { mpe::ArticulationType::Mute, midi::Program(20, 45) },
        { mpe::ArticulationType::Tremolo8th, midi::Program(20, 44) },
        { mpe::ArticulationType::Tremolo16th, midi::Program(20, 44) },
        { mpe::ArticulationType::Tremolo32nd, midi::Program(20, 44) },
        { mpe::ArticulationType::Tremolo64th, midi::Program(20, 44) },
    };

    static ArticulationMapping VIOLA_SECTION = {
        { mpe::ArticulationType::SnapPizzicato, midi::Program(30, 45) },
        { mpe::ArticulationType::Pizzicato, midi::Program(30, 45) },
        { mpe::ArticulationType::Mute, midi::Program(30, 45) },
        { mpe::ArticulationType::Tremolo8th, midi::Program(30, 44) },
        { mpe::ArticulationType::Tremolo16th, midi::Program(30, 44) },
        { mpe::ArticulationType::Tremolo32nd, midi::Program(30, 44) },
        { mpe::ArticulationType::Tremolo64th, midi::Program(30, 44) },
    };

    static ArticulationMapping VIOLONCELLO_SECTION = {
        { mpe::ArticulationType::SnapPizzicato, midi::Program(40, 45) },
        { mpe::ArticulationType::Pizzicato, midi::Program(40, 45) },
        { mpe::ArticulationType::Mute, midi::Program(40, 45) },
        { mpe::ArticulationType::Tremolo8th, midi::Program(40, 44) },
        { mpe::ArticulationType::Tremolo16th, midi::Program(40, 44) },
        { mpe::ArticulationType::Tremolo32nd, midi::Program(40, 44) },
        { mpe::ArticulationType::Tremolo64th, midi::Program(40, 44) },
    };

    static ArticulationMapping CONTRABASS_SECTION = {
        { mpe::ArticulationType::SnapPizzicato, midi::Program(50, 45) },
        { mpe::ArticulationType::Pizzicato, midi::Program(50, 45) },
        { mpe::ArticulationType::Mute, midi::Program(50, 45) },
        { mpe::ArticulationType::Tremolo8th, midi::Program(50, 44) },
        { mpe::ArticulationType::Tremolo16th, midi::Program(50, 44) },
        { mpe::ArticulationType::Tremolo32nd, midi::Program(50, 44) },
        { mpe::ArticulationType::Tremolo64th, midi::Program(50, 44) },
    };

    static ArticulationMapping WINDS = {
        { mpe::ArticulationType::Mute, midi::Program(0, 59) }
    };

    if (setupData.id == mpe::SoundId::Guitar) {
        if (setupData.contains(mpe::SoundSubCategory::Acoustic)) {
            return ACOUSTIC_GUITAR;
        }

        if (setupData.contains(mpe::SoundSubCategory::Electric)) {
            return ELECTRIC_GUITAR;
        }
    }

    if (setupData.id == mpe::SoundId::BassGuitar) {
        if (setupData.contains(mpe::SoundSubCategory::Acoustic)) {
            return ACOUSTIC_BASS_GUITAR;
        }

        if (setupData.contains(mpe::SoundSubCategory::Electric)) {
            return ELECTRIC_BASS_GUITAR;
        }
    }

    if (setupData.id == mpe::SoundId::ViolinSection) {
        return VIOLIN_SECTION;
    }

    if (setupData.id == mpe::SoundId::ViolaSection) {
        return VIOLA_SECTION;
    }

    if (setupData.id == mpe::SoundId::VioloncelloSection
        || setupData.id == mpe::SoundId::Violoncello) {
        return VIOLONCELLO_SECTION;
    }

    if (setupData.id == mpe::SoundId::ContrabasseSection
        || setupData.id == mpe::SoundId::Contrabass) {
        return CONTRABASS_SECTION;
    }

    if (setupData.category == mpe::SoundCategory::Strings) {
        return BASIC_STRING_SECTION;
    }

    if (setupData.category == mpe::SoundCategory::Winds) {
        return WINDS;
    }

    static ArticulationMapping empty;
    return empty;
}

static const mpe::ArticulationTypeSet PORTAMENTO_CC_SUPPORTED_TYPES = {
    mpe::ArticulationType::DiscreteGlissando, mpe::ArticulationType::ContinuousGlissando,
    mpe::ArticulationType::Fall, mpe::ArticulationType::QuickFall, mpe::ArticulationType::Doit,
    mpe::ArticulationType::Plop, mpe::ArticulationType::Scoop, mpe::ArticulationType::SlideOutDown,
    mpe::ArticulationType::SlideInAbove, mpe::ArticulationType::SlideInBelow, mpe::ArticulationType::SlideOutUp
};

static const mpe::ArticulationTypeSet LEGATO_CC_SUPPORTED_TYPES = {
    mpe::ArticulationType::Legato, mpe::ArticulationType::Acciaccatura,
    mpe::ArticulationType::PreAppoggiatura, mpe::ArticulationType::PostAppoggiatura,
    mpe::ArticulationType::Arpeggio, mpe::ArticulationType::ArpeggioDown,
    mpe::ArticulationType::ArpeggioStraightDown, mpe::ArticulationType::ArpeggioStraightUp,
    mpe::ArticulationType::ArpeggioUp, mpe::ArticulationType::Trill, mpe::ArticulationType::TrillBaroque,
    mpe::ArticulationType::Tremolo8th, mpe::ArticulationType::Tremolo16th,
    mpe::ArticulationType::Tremolo32nd, mpe::ArticulationType::Tremolo64th
};

static const mpe::ArticulationTypeSet PEDAL_CC_SUPPORTED_TYPES = {
    mpe::ArticulationType::Pedal
};

static const mpe::ArticulationTypeSet BEND_SUPPORTED_TYPES = {
    mpe::ArticulationType::Bend
};

static const mpe::ArticulationTypeSet AFTERTOUCH_SUPPORTED_TYPES = {
    mpe::ArticulationType::Vibrato, mpe::ArticulationType::MoltoVibrato,
    mpe::ArticulationType::SenzaVibrato, mpe::ArticulationType::WideVibrato
};
}

#endif // MU_AUDIO_SOUNDMAPPING_H
