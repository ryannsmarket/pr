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
#ifndef MU_MIDI_WINMIDIOUTPORT_H
#define MU_MIDI_WINMIDIOUTPORT_H

#include "midi/imidioutport.h"

namespace mu {
namespace midi {
class WinMidiOutPort : public IMidiOutPort
{
public:
    WinMidiOutPort();
    ~WinMidiOutPort();

    std::vector<Device> devices() const override;

    bool connect(const std::string& deviceID) override;
    void disconnect() override;

    void sendEvent(const Event& e) override;

private:

    uint32_t message(const Event& e) const;

    struct Win;
    Win* m_win = nullptr;
    bool m_isConnected = false;
};
}
}

#endif // MU_MIDI_WINMIDIOUTPORT_H
