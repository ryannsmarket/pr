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
#include "playbackactions.h"

#include "ui/view/iconcodes.h"

using namespace mu::playback;
using namespace mu::actions;
using namespace mu::shortcuts;
using namespace mu::ui;

const mu::actions::ActionList PlaybackActions::m_actions = {
    ActionItem("play",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Play"),
               QT_TRANSLATE_NOOP("action", "Start or stop playback"),
               IconCode::Code::PLAY
               ),
    ActionItem("rewind",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Rewind"),
               QT_TRANSLATE_NOOP("action", "Rewind to start position"),
               IconCode::Code::REWIND
               ),
    ActionItem("loop",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Loop Playback"),
               QT_TRANSLATE_NOOP("action", "Toggle 'Loop Playback'"),
               IconCode::Code::LOOP
               ),
    ActionItem("repeat",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Play Repeats"),
               QT_TRANSLATE_NOOP("action", "Play repeats"),
               IconCode::Code::PLAY_REPEATS
               ),
    ActionItem("pan",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Pan Score"),
               QT_TRANSLATE_NOOP("action", "Pan score automatically"),
               IconCode::Code::PAN_SCORE
               ),
    ActionItem("metronome",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Metronome"),
               QT_TRANSLATE_NOOP("action", "Play metronome during playback"),
               IconCode::Code::METRONOME
               ),
    ActionItem("midi-on",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "MIDI Input"),
               QT_TRANSLATE_NOOP("action", "Toggle 'MIDI Input'"),
               IconCode::Code::MIDI_INPUT
               ),
    ActionItem("countin",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Count-In"),
               QT_TRANSLATE_NOOP("action", "Play count-in at playback start"),
               IconCode::Code::CLOSE_X_ROUNDED
               ),
    ActionItem("loop-in",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Loop In"),
               QT_TRANSLATE_NOOP("action", "Set loop in position"),
               IconCode::Code::ARROW_LEFT
               ),
    ActionItem("loop-out",
               ShortcutContext::Any,
               QT_TRANSLATE_NOOP("action", "Loop Out"),
               QT_TRANSLATE_NOOP("action", "Set loop out position"),
               IconCode::Code::ARROW_RIGHT
               ),
};

const ActionItem& PlaybackActions::action(const ActionCode& actionCode) const
{
    for (const ActionItem& action : m_actions) {
        if (action.code == actionCode) {
            return action;
        }
    }

    static ActionItem null;
    return null;
}
