/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#ifndef MU_AUDIO_LINUXAUDIODRIVER_H
#define MU_AUDIO_LINUXAUDIODRIVER_H

#include "iaudiodriver.h"

namespace mu::audio {
class LinuxAudioDriver : public IAudioDriver
{
public:
    LinuxAudioDriver();

    std::string name() const override;
    bool open(const Spec& spec, Spec* activeSpec) override;
    void close() override;
    bool isOpened() const override;

    std::string outputDevice() const override;
    bool selectOutputDevice(const std::string& name) override;
    std::vector<std::string> availableOutputDevices() const override;
    async::Notification availableOutputDevicesChanged() const override;
    void resume() override;
    void suspend() override;
};
}

#endif // MU_AUDIO_LINUXAUDIODRIVER_H
