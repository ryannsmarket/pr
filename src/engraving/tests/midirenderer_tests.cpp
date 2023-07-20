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

#include <gtest/gtest.h>

#include "utils/scorerw.h"
#include "engraving/compat/midi/midirender.h"
#include "engraving/infrastructure/localfileinfoprovider.h"
#include "engraving/rw/mscloader.h"
#include "engraving/libmscore/noteevent.h"

using namespace mu;
using namespace mu::engraving;

class MidiRenderer_Tests : public ::testing::Test
{
};

static const String MIDIRENDERER_TESTS_DIR = u"midirenderer_data/";
static constexpr int DEFAULT_CHANNEL = 0;
static constexpr int NOTE_OFF_VOLUME = 0;

static NPlayEvent noteEvent(int pitch, int volume, int channel)
{
    return NPlayEvent(EventType::ME_NOTEON, channel, pitch, volume);
}

static int getEventsCount(EventMap& events)
{
    int eventsCount = 0;
    for (size_t i = 0; i < events.size(); ++i) {
        for (auto& _ : events[i]) {
            ++eventsCount;
        }
    }
    return eventsCount;
}

static void checkEventInterval(EventMap& events, int tickStart, int tickEnd, int pitch, int volume, int channel = DEFAULT_CHANNEL)
{
    auto it = events[channel].find(tickStart);
    EXPECT_TRUE(it != events[channel].end());
    if (it == events[channel].end()) {
        return;
    }

    EXPECT_EQ(it->second.pitch(), pitch);
    EXPECT_EQ(it->second.velo(), volume);
    EXPECT_EQ(it->second.channel(), channel);

    events[channel].erase(it);

    it = events[channel].find(tickEnd);
    EXPECT_TRUE(it != events[channel].end());
    if (it == events[channel].end()) {
        return;
    }

    EXPECT_EQ(it->second.pitch(), pitch);
    EXPECT_EQ(it->second.velo(), NOTE_OFF_VOLUME);
    EXPECT_EQ(it->second.channel(), channel);

    events[channel].erase(it);
}

static EventMap renderMidiEvents(const String& fileName, bool eachStringHasChannel = false, bool instrumentsHaveEffects = false)
{
    MasterScore* score = ScoreRW::readScore(MIDIRENDERER_TESTS_DIR + fileName);
    EXPECT_TRUE(score);

    MidiRenderer render(score);
    EventMap events;
    MidiRenderer::Context ctx;

    ctx.synthState = mu::engraving::SynthesizerState();
    ctx.metronome = false;
    ctx.eachStringHasChannel = eachStringHasChannel;
    ctx.instrumentsHaveEffects = instrumentsHaveEffects;
    score->renderMidi(events, ctx, true);

    return events;
}

static EventMap getNoteOnEvents(EventMap& events)
{
    EventMap filteredEventMap;
    for (size_t i = 0; i < events.size(); ++i) {
        for (auto ev : events[i]) {
            if (ev.second.type() != EventType::ME_NOTEON) {
                continue;
            }
            filteredEventMap[i].insert({ ev.first, ev.second });
        }
    }

    return filteredEventMap;
}

/*****************************************************************************

    ENABLED TESTS BELOW

*****************************************************************************/

TEST_F(MidiRenderer_Tests, mergePitchWheelEvents)
{
    PitchWheelSpecs wheelSpec;
    PitchWheelRenderer render(wheelSpec);

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = 0;
    func.mEndTick = 100;
    //! y = ax + b
    int a = 1;
    int b = 0;

    auto linearFunc = [ startTick = func.mStartTick, a, b] (uint32_t tick) {
        float x = (float)(tick - startTick);
        float y = a * x + b;
        return (int)y;
    };
    func.func = linearFunc;
    render.addPitchWheelFunction(func, 0, 0, MidiInstrumentEffect::NONE);

    EventMap pitchWheelEvents = render.renderPitchWheel();
    EventMap noteEvents;
    NPlayEvent note1_ON{ 144, 0, 59, 96 };
    NPlayEvent note1_OFF{ 144, 0, 59, 0 };
    NPlayEvent note2_ON{ 144, 0, 61, 96 };
    NPlayEvent note2_OFF{ 144, 0, 61, 0 };
    noteEvents[DEFAULT_CHANNEL].insert(std::make_pair(0, note1_ON));
    noteEvents[DEFAULT_CHANNEL].insert(std::make_pair(90, note1_OFF));
    noteEvents[DEFAULT_CHANNEL].insert(std::make_pair(200, note2_ON));
    noteEvents[DEFAULT_CHANNEL].insert(std::make_pair(300, note2_OFF));
    noteEvents.mergePitchWheelEvents(pitchWheelEvents);
    EXPECT_NE(noteEvents[DEFAULT_CHANNEL].find(199), noteEvents[DEFAULT_CHANNEL].end());
}

TEST_F(MidiRenderer_Tests, subscriptOperator)
{
    EventMap events;
    events[0];
    EXPECT_EQ(events.size(), 16);
    events[1];
    EXPECT_EQ(events.size(), 16);
    events[3];
    EXPECT_EQ(events.size(), 16);
    events[50];
    EXPECT_EQ(events.size(), 51);
    EventMap events2;
    events2[10];
    EXPECT_EQ(events2.size(), 16);
    events2[192];
    EXPECT_EQ(events2.size(), 193);
}

TEST_F(MidiRenderer_Tests, oneGuitarNote)
{
    constexpr int defVol = 96; // f

    EventMap events = renderMidiEvents(u"one_guitar_note.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 2);

    checkEventInterval(events, 0, 479, 59, defVol);
}

TEST_F(MidiRenderer_Tests, onePercussionNote)
{
    constexpr int defVol = 80; // mf

    EventMap events = renderMidiEvents(u"one_percussion_note.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 1);

    EXPECT_EQ(events[DEFAULT_CHANNEL].find(0)->second, noteEvent(41, defVol, DEFAULT_CHANNEL));
}

TEST_F(MidiRenderer_Tests, graceBeforeBeat)
{
    constexpr int defVol = 96; // f

    EventMap events = renderMidiEvents(u"grace_before_beat.mscx");

    EXPECT_EQ(events[0].size(), 6);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 55, defVol);
    checkEventInterval(events, 480, 959, 57, defVol);
}

TEST_F(MidiRenderer_Tests, graceOnBeat)
{
    constexpr int defVol = 96; // f

    EventMap events = renderMidiEvents(u"grace_on_beat.mscx");

    EXPECT_EQ(events[0].size(), 6);

    checkEventInterval(events, 0, 479, 59, defVol);
    checkEventInterval(events, 480, 719, 55, defVol);
    checkEventInterval(events, 720, 959, 57, defVol);
}

TEST_F(MidiRenderer_Tests, ghostNote)
{
    constexpr int defVol = 96; // f
    constexpr int ghostVol = defVol * NoteEvent::GHOST_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"ghost_note.mscx");

    EXPECT_EQ(events[0].size(), 4);

    checkEventInterval(events, 0, 479, 59, defVol);
    checkEventInterval(events, 480, 959, 57, ghostVol);
}

TEST_F(MidiRenderer_Tests, simpleTremolo)
{
    constexpr int defVol = 96; // f

    EventMap events = renderMidiEvents(u"simple_tremolo.mscx");

    EXPECT_EQ(events[0].size(), 8);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 59, defVol);
    checkEventInterval(events, 480, 719, 59, defVol);
    checkEventInterval(events, 720, 959, 59, defVol);
}

TEST_F(MidiRenderer_Tests, simpleGlissando)
{
    constexpr int defVol = 96; // forte
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"simple_glissando.mscx");

    EXPECT_EQ(events[0].size(), 10);

    checkEventInterval(events, 0, 599, 59, defVol);
    checkEventInterval(events, 600, 719, 58, glissVol);
    checkEventInterval(events, 720, 839, 57, glissVol);
    checkEventInterval(events, 840, 959, 56, glissVol);
    checkEventInterval(events, 960, 1439, 55, defVol);
}

TEST_F(MidiRenderer_Tests, sameStringNoEffects)
{
    constexpr int defVol = 80; // mf

    auto midiEvents = renderMidiEvents(u"channels.mscx", false, false);
    EventMap events = getNoteOnEvents(midiEvents);

    checkEventInterval(events, 0, 959, 60, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 960, 1919, 60, defVol, DEFAULT_CHANNEL);
}

TEST_F(MidiRenderer_Tests, sameStringWithEffects)
{
    constexpr int defVol = 80; // mf

    auto midiEvents = renderMidiEvents(u"channels.mscx", false, true);
    EventMap events = getNoteOnEvents(midiEvents);

    checkEventInterval(events, 0, 959, 60, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 960, 1919, 60, defVol, DEFAULT_CHANNEL + 1);
}

TEST_F(MidiRenderer_Tests, diffStringNoEffects)
{
    constexpr int defVol = 80; // mf

    auto midievents = renderMidiEvents(u"channels.mscx", true, false);
    EventMap events = getNoteOnEvents(midievents);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size() + events[DEFAULT_CHANNEL + 1].size(), 6);

    checkEventInterval(events, 0, 959, 60, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 960, 1919, 60, defVol, DEFAULT_CHANNEL);
}

TEST_F(MidiRenderer_Tests, diffStringWithEffects)

{
    constexpr int defVol = 80; // mf

    auto midiEvents = renderMidiEvents(u"channels.mscx", true, true);
    EventMap events = getNoteOnEvents(midiEvents);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 2);
    int eventsCount =  getEventsCount(events);
    EXPECT_EQ(eventsCount, 6);

    checkEventInterval(events, 0, 959, 60, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 480, 1439, 64, defVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 960, 1919, 60, defVol, DEFAULT_CHANNEL + 2);
}

TEST_F(MidiRenderer_Tests, tremoloAndGlissando)
{
    constexpr int defVol = 96; // f
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"tremolo_and_glissando.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 14);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 59, defVol);
    checkEventInterval(events, 480, 599, 59, defVol);
    checkEventInterval(events, 600, 719, 58, glissVol);
    checkEventInterval(events, 720, 839, 57, glissVol);
    checkEventInterval(events, 840, 959, 56, glissVol);
    checkEventInterval(events, 960, 1439, 55, defVol);
}

TEST_F(MidiRenderer_Tests, slideInFromBelow)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_in_from_below.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 239, 60, defVol);
    checkEventInterval(events, 240, 318, 57, glissVol);
    checkEventInterval(events, 320, 398, 58, glissVol);
    checkEventInterval(events, 400, 478, 59, glissVol);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, slideInFromAbove)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_in_from_above.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 239, 60, defVol);
    checkEventInterval(events, 240, 318, 63, glissVol);
    checkEventInterval(events, 320, 398, 62, glissVol);
    checkEventInterval(events, 400, 478, 61, glissVol);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, slideOutFromAbove)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_out_from_above.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 239, 60, defVol);
    checkEventInterval(events, 240, 318, 59, glissVol);
    checkEventInterval(events, 319, 397, 58, glissVol);
    checkEventInterval(events, 399, 477, 57, glissVol);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, slideOutFromBelow)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_out_from_below.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 10);

    checkEventInterval(events, 0, 239, 60, defVol);
    checkEventInterval(events, 240, 318, 61, glissVol);
    checkEventInterval(events, 319, 397, 62, glissVol);
    checkEventInterval(events, 399, 477, 63, glissVol);
    checkEventInterval(events, 480, 959, 60, defVol);
}

TEST_F(MidiRenderer_Tests, tremoloSlideIn)
{
    constexpr int defVol = 96; // f
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"tremolo_and_slide_in.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 16);

    checkEventInterval(events, 0, 119, 59, defVol);
    checkEventInterval(events, 120, 239, 59, defVol);
    checkEventInterval(events, 240, 359, 59, defVol);
    checkEventInterval(events, 360, 479, 59, defVol);
    checkEventInterval(events, 480, 638, 56, glissVol);
    checkEventInterval(events, 640, 798, 57, glissVol);
    checkEventInterval(events, 800, 958, 58, glissVol);
    checkEventInterval(events, 960, 1439, 59, defVol);
}

TEST_F(MidiRenderer_Tests, tremoloSlideOut)
{
    constexpr int defVol = 96; // f
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"tremolo_and_slide_out.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 14);

    checkEventInterval(events, 0, 239, 59, defVol);
    checkEventInterval(events, 240, 479, 59, defVol);
    checkEventInterval(events, 480, 719, 59, defVol);
    checkEventInterval(events, 720, 959, 59, defVol);
    checkEventInterval(events, 960, 1277, 58, glissVol);
    checkEventInterval(events, 1278, 1595, 57, glissVol);
    checkEventInterval(events, 1597, 1914, 56, glissVol);
}

TEST_F(MidiRenderer_Tests, slideInAndOut)
{
    constexpr int defVol = 80; // mf
    constexpr int glissVol = defVol * NoteEvent::GLISSANDO_VELOCITY_MULTIPLIER;

    EventMap events = renderMidiEvents(u"slide_in_and_out.mscx");

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 14);

    checkEventInterval(events, 240, 318, 57, glissVol);
    checkEventInterval(events, 320, 398, 58, glissVol);
    checkEventInterval(events, 400, 478, 59, glissVol);
    checkEventInterval(events, 480, 719, 60, defVol);
    checkEventInterval(events, 720, 798, 61, glissVol);
    checkEventInterval(events, 799, 877, 62, glissVol);
    checkEventInterval(events, 879, 957, 63, glissVol);
}

TEST_F(MidiRenderer_Tests, sameStringDifferentStaves)
{
    constexpr int defVol = 80; // mf

    auto midiEvents = renderMidiEvents(u"same_string_diff_staves.mscx", true);
    EventMap events = getNoteOnEvents(midiEvents);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 4);

    checkEventInterval(events, 0, 239, 62, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 240, 479, 62, defVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 0, 1919, 35, defVol, DEFAULT_CHANNEL + 1);
}

TEST_F(MidiRenderer_Tests, trillOnHiddenStaff)
{
    constexpr int mfVol = 80;
    constexpr int fVol = 96;

    auto midiEvents = renderMidiEvents(u"trill_on_hidden_staff.mscx");
    EventMap events = getNoteOnEvents(midiEvents);

    EXPECT_EQ(events[DEFAULT_CHANNEL].size(), 2);
    int eventsCount = getEventsCount(events);
    EXPECT_EQ(eventsCount, 18);

    checkEventInterval(events, 0, 1919, 60, mfVol, DEFAULT_CHANNEL);
    checkEventInterval(events, 1920, 1979, 79, fVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 1980, 2039, 81, fVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2040, 2099, 79, fVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2100, 2159, 81, fVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2160, 2219, 79, fVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2220, 2279, 81, fVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2280, 2339, 79, fVol, DEFAULT_CHANNEL + 1);
    checkEventInterval(events, 2340, 2399, 81, fVol, DEFAULT_CHANNEL + 1);
}

/*****************************************************************************

    DISABLED TESTS BELOW

*****************************************************************************/
