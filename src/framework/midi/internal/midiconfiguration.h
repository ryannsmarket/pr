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
#ifndef MU_MIDI_MIDICONFIGURATION_H
#define MU_MIDI_MIDICONFIGURATION_H

#include "modularity/ioc.h"
#include "iglobalconfiguration.h"
#include "../imidiconfiguration.h"

namespace mu::midi {
class MidiConfiguration : public IMidiConfiguration
{
    INJECT(midi, framework::IGlobalConfiguration, globalConfiguration)

public:
    void init();

    io::path midiMappingsPath() const override;

    bool useRemoteControl() const override;
    void setUseRemoteControl(bool value) override;

    MidiDeviceID midiInputDeviceId() const override;
    void setMidiInputDeviceId(const MidiDeviceID& deviceId) override;

    MidiDeviceID midiOutputDeviceId() const override;
    void setMidiOutputDeviceId(const MidiDeviceID& deviceId) override;
};
}

#endif // MU_MIDI_MIDICONFIGURATION_H
