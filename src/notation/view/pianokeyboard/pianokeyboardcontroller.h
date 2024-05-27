/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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
#ifndef MU_NOTATION_PIANOKEYBOARDCONTROLLER_H
#define MU_NOTATION_PIANOKEYBOARDCONTROLLER_H

#include <QObject>

#include "async/asyncable.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

#include "pianokeyboardtypes.h"

namespace mu::notation {
class PianoKeyboardController : public muse::async::Asyncable, public muse::actions::Actionable
{
    INJECT(INotationConfiguration, configuration)
    INJECT(context::IGlobalContext, context)

public:
    PianoKeyboardController() = default;

    void init();

    std::optional<piano_key_t> pressedKey() const;
    void setPressedKey(std::optional<piano_key_t> key);

    KeyState keyState(piano_key_t key) const;
    muse::async::Notification keyStatesChanged() const;

    bool isFromMidi() const;

    bool pianoKeyboardPitchState() const;
    void setPianoKeyboardPitchState(bool useNotatedPitch);

private:
    INotationPtr currentNotation() const;

    void onNotationChanged();
    void updateNotesKeys(const std::vector<const Note*>& receivedNotes);

    void sendNoteOn(piano_key_t key);
    void sendNoteOff(piano_key_t key);

    std::optional<piano_key_t> m_pressedKey = std::nullopt;
    std::unordered_set<piano_key_t> m_keys;
    std::unordered_set<piano_key_t> m_otherNotesInChord;

    bool m_isFromMidi = false;

    muse::async::Notification m_keyStatesChanged;
};
}

#endif // MU_NOTATION_PIANOKEYBOARDCONTROLLER_H
