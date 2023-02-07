﻿/*
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

/**
 \file
 render score into event list
*/

#include "midirender.h"

#include <set>
#include <cmath>

#include "compat/midi/event.h"
#include "style/style.h"
#include "types/constants.h"

#include "libmscore/arpeggio.h"
#include "libmscore/articulation.h"
#include "libmscore/bend.h"
#include "libmscore/changeMap.h"
#include "libmscore/chord.h"
#include "libmscore/durationtype.h"
#include "libmscore/dynamic.h"
#include "libmscore/easeInOut.h"
#include "libmscore/glissando.h"
#include "libmscore/hairpin.h"
#include "libmscore/instrument.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/note.h"
#include "libmscore/noteevent.h"
#include "libmscore/palmmute.h"
#include "libmscore/part.h"
#include "libmscore/repeatlist.h"
#include "libmscore/score.h"
#include "libmscore/segment.h"
#include "libmscore/sig.h"
#include "libmscore/slur.h"
#include "libmscore/staff.h"
#include "libmscore/stafftextbase.h"
#include "libmscore/stretchedbend.h"
#include "libmscore/swing.h"
#include "libmscore/synthesizerstate.h"
#include "libmscore/tempo.h"
#include "libmscore/tie.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"
#include "libmscore/vibrato.h"
#include "libmscore/volta.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
static PitchWheelSpecs wheelSpec;

static bool graceNotesMerged(Chord* chord);

//---------------------------------------------------------
//   Converts midi time (noteoff - noteon) to milliseconds
//---------------------------------------------------------
int toMilliseconds(float tempo, float midiTime)
{
    float ticksPerSecond = (float)Constants::division * tempo;
    int time = (int)((midiTime / ticksPerSecond) * 1000.0f);
    if (time > 0x7fff) { //maximum possible value
        time = 0x7fff;
    }
    return time;
}

//---------------------------------------------------------
//   Detects if a note is a start of a glissando
//---------------------------------------------------------
bool isGlissandoFor(const Note* note)
{
    for (Spanner* spanner : note->spannerFor()) {
        if (spanner->type() == ElementType::GLISSANDO) {
            return true;
        }
    }
    return false;
}

static void collectGlissando(int channel,
                             int onTime, int offTime,
                             int pitchDelta,
                             PitchWheelRenderer& pitchWheelRenderer)
{
    const float scale = (float)wheelSpec.mLimit / wheelSpec.mAmplitude;

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = onTime;
    func.mEndTick = offTime;

    auto linearFunc = [startTick = onTime, endTick = offTime, pitchDelta, scale] (uint32_t tick) {
        float x = (float)(tick - startTick) / (endTick - startTick);
        return pitchDelta * x * scale;
    };
    func.func = linearFunc;

    pitchWheelRenderer.addPitchWheelFunction(func, channel);
}

static Fraction getPlayTicksForBend(const Note* note)
{
    Tie* tie = note->tieFor();
    if (!tie) {
        return note->playTicksFraction();
    }

    Fraction stick = note->chord()->tick();
    Note* nextNote = tie->endNote();
    while (tie) {
        nextNote = tie->endNote();
        for (EngravingItem* e : nextNote->el()) {
            if (e && (e->type() == ElementType::BEND || e->type() == ElementType::STRETCHED_BEND)) {
                return nextNote->chord()->tick() - stick;
            }
        }

        tie = nextNote->tieFor();
    }

    return nextNote->chord()->tick() + nextNote->chord()->actualTicks() - stick;
}

//---------------------------------------------------------
//   playNote
//---------------------------------------------------------
static void playNote(EventMap* events, const Note* note, int channel, int pitch,
                     int velo, int onTime, int offTime, int staffIdx, PitchWheelRenderer& pitchWheelRenderer)
{
    if (!note->play()) {
        return;
    }

    if (note->userVelocity() != 0) {
        velo = note->customizeVelocity(velo);
    }

    NPlayEvent ev(ME_NOTEON, channel, pitch, velo);
    ev.setOriginatingStaff(staffIdx);
    ev.setTuning(note->tuning());
    ev.setNote(note);
    if (offTime < onTime) {
        offTime = onTime;
    }
    events->insert(std::pair<int, NPlayEvent>(onTime, ev));
    // adds portamento for continuous glissando
    for (Spanner* spanner : note->spannerFor()) {
        if (spanner->type() == ElementType::GLISSANDO) {
            Glissando* glissando = toGlissando(spanner);
            if (glissando->glissandoStyle() == GlissandoStyle::PORTAMENTO) {
                Note* nextNote = toNote(spanner->endElement());
                double pitchDelta = nextNote->ppitch() - pitch;
                int timeDelta = offTime - onTime;
                if (pitchDelta != 0 && timeDelta != 0) {
                    collectGlissando(channel, onTime, offTime, pitchDelta, pitchWheelRenderer);
                }
            }
        }
    }

    ev.setVelo(0);
    if (!note->part()->instrument(note->tick())->useDrumset()) {
        events->insert(std::pair<int, NPlayEvent>(offTime, ev));
    }
}

static void collectVibrato(int channel,
                           int onTime, int offTime,
                           int lowPitch, int highPitch,
                           PitchWheelRenderer& pitchWheelRenderer)
{
    const uint16_t vibratoPeriod = Constants::division / 2;
    const uint32_t duration = offTime - onTime;
    const float scale = 2 * (float)wheelSpec.mLimit / wheelSpec.mAmplitude / 100;

    if (duration < vibratoPeriod) {
        return;
    }

    const int pillarAmplitude = (highPitch - lowPitch);

    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = onTime;
    func.mEndTick = offTime - duration % vibratoPeriod;//removed last points to make more smooth of the end

    auto vibratoFunc = [startTick = onTime, pillarAmplitude, vibratoPeriod, lowPitch, scale] (uint32_t tick) {
        float x = (float)(tick - startTick) / vibratoPeriod;
        return (pillarAmplitude * 2 / M_PI * asin(sin(2 * M_PI * x)) + lowPitch) * scale;
    };
    func.func = vibratoFunc;

    pitchWheelRenderer.addPitchWheelFunction(func, channel);
}

static void collectBend(const Bend* bend,
                        int channel,
                        int onTime, int offTime,
                        PitchWheelRenderer& pitchWheelRenderer)
{
    const PitchValues& points = bend->points();
    size_t pitchSize = points.size();

    const float scale = 2 * (float)wheelSpec.mLimit / wheelSpec.mAmplitude / PitchValue::PITCH_FOR_SEMITONE;
    uint32_t duration = offTime - onTime;

    for (size_t i = 0; i < pitchSize - 1; i++) {
        PitchValue curValue = points[i];
        PitchValue nextValue = points[i + 1];

        //! y = a x^2 + b - curve
        float curTick = (float)curValue.time * duration / PitchValue::MAX_TIME;
        float nextTick = (float)nextValue.time * duration / PitchValue::MAX_TIME;

        float a = (float)(nextValue.pitch - curValue.pitch) / ((curTick - nextTick) * (curTick - nextTick));
        float b = curValue.pitch;

        uint32_t x0 = curValue.time * duration / PitchValue::MAX_TIME;

        PitchWheelRenderer::PitchWheelFunction func;
        func.mStartTick = onTime + x0;
        uint32_t startTimeNextPoint = nextValue.time * duration / PitchValue::MAX_TIME;
        func.mEndTick = onTime + startTimeNextPoint;

        auto bendFunc = [ startTick = func.mStartTick, scale,
                          a, b] (uint32_t tick) {
            float x = (float)(tick - startTick);

            float y = a * x * x + b;

            return y * scale;
        };
        func.func = bendFunc;
        pitchWheelRenderer.addPitchWheelFunction(func, channel);
    }
    PitchWheelRenderer::PitchWheelFunction func;
    func.mStartTick = onTime + points[pitchSize - 1].time * duration / PitchValue::MAX_TIME;
    func.mEndTick = offTime;

    if (func.mEndTick == func.mStartTick) {
        return;
    }

    //! y = releaseValue linear curve
    uint32_t releaseValue = points[pitchSize - 1].pitch * scale;
    auto bendFunc = [releaseValue] (uint32_t tick) {
        UNUSED(tick)
        return releaseValue;
    };
    func.func = bendFunc;
    pitchWheelRenderer.addPitchWheelFunction(func, channel);
}

//---------------------------------------------------------
//   collectNote
//---------------------------------------------------------

static void collectNote(EventMap* events, int channel, const Note* note, double velocityMultiplier, int tickOffset, Staff* staff,
                        PitchWheelRenderer& pitchWheelRenderer, int graceOffsetOn = 0, int graceOffsetOff = 0)
{
    if (!note->play() || note->hidden()) {      // do not play overlapping notes
        return;
    }
    Chord* chord = note->chord();

    int staffIdx = static_cast<int>(staff->idx());
    int ticks;
    int tieLen = 0;
    if (chord->isGrace()) {
        assert(!graceNotesMerged(chord));      // this function should not be called on a grace note if grace notes are merged
        chord = toChord(chord->explicitParent());
    }

    ticks = chord->actualTicks().ticks();   // ticks of the actual note
    // calculate additional length due to ties forward
    // taking NoteEvent length adjustments into account
    // but stopping at any note with multiple NoteEvents
    // and processing those notes recursively
    if (note->tieFor()) {
        Note* n = note->tieFor()->endNote();
        while (n) {
            NoteEventList nel = n->playEvents();
            if (nel.size() == 1 && !isGlissandoFor(n)) {
                // add value of this note to main note
                // if we wish to suppress first note of ornament,
                // then do this regardless of number of NoteEvents
                tieLen += (n->chord()->actualTicks().ticks() * (nel[0].len())) / 1000;
            } else {
                // recurse
                collectNote(events, channel, n, velocityMultiplier, tickOffset, staff, pitchWheelRenderer);
                break;
            }
            if (n->tieFor() && n != n->tieFor()->endNote()) {
                n = n->tieFor()->endNote();
            } else {
                break;
            }
        }
    }

    int tick1    = chord->tick().ticks() + tickOffset;
    bool tieFor  = note->tieFor();
    bool tieBack = note->tieBack();

    NoteEventList nel = note->playEvents();
    size_t nels = nel.size();
    for (int i = 0, pitch = note->ppitch(); i < static_cast<int>(nels); ++i) {
        const NoteEvent& e = nel[i];     // we make an explicit const ref, not a const copy.  no need to copy as we won't change the original object.

        // skip if note has a tie into it and only one NoteEvent
        // its length was already added to previous note
        // if we wish to suppress first note of ornament
        // then change "nels == 1" to "i == 0", and change "break" to "continue"
        if (tieBack && nels == 1 && !isGlissandoFor(note)) {
            break;
        }
        int p = pitch + e.pitch();
        if (p < 0) {
            p = 0;
        } else if (p > 127) {
            p = 127;
        }
        int on  = tick1 + (ticks * e.ontime()) / 1000;
        int off = on + (ticks * e.len()) / 1000 - 1;
        if (tieFor && i == static_cast<int>(nels) - 1) {
            off += tieLen;
        }

        // Get the velocity used for this note from the staff
        // This allows correct playback of tremolos even without SND enabled.
        int velo;
        Fraction nonUnwoundTick = Fraction::fromTicks(on - tickOffset);
        velo = staff->velocities().val(nonUnwoundTick);

        velo *= velocityMultiplier;
        velo *= e.velocityMultiplier();
        playNote(events, note, channel, p, std::clamp(velo, 1, 127), std::max(0, on - graceOffsetOn), std::max(0,
                                                                                                               off - graceOffsetOff),
                 staffIdx, pitchWheelRenderer);
    }

    // Bends
    for (EngravingItem* e : note->el()) {
        if (e == 0 || (e->type() != ElementType::BEND && e->type() != ElementType::STRETCHED_BEND)) {
            continue;
        }
        Bend* bend = toBend(e);
        if (!bend->playBend()) {
            break;
        }
        collectBend(bend, channel, tick1, tick1 + getPlayTicksForBend(note).ticks(), pitchWheelRenderer);
    }
}

//---------------------------------------------------------
//   aeolusSetStop
//---------------------------------------------------------

static void aeolusSetStop(int tick, int channel, int i, int k, bool val, EventMap* events)
{
    NPlayEvent event;
    event.setType(ME_CONTROLLER);
    event.setController(98);
    if (val) {
        event.setValue(0x40 + 0x20 + i);
    } else {
        event.setValue(0x40 + 0x10 + i);
    }

    event.setChannel(static_cast<uint8_t>(channel));
    events->insert(std::pair<int, NPlayEvent>(tick, event));

    event.setValue(k);
    events->insert(std::pair<int, NPlayEvent>(tick, event));
//      event.setValue(0x40 + i);
//      events->insert(std::pair<int,NPlayEvent>(tick, event));
}

//---------------------------------------------------------
//   collectProgramChanges
//---------------------------------------------------------

static void collectProgramChanges(EventMap* events, Measure const* m, const Staff* staff, int tickOffset)
{
    int firstStaffIdx = static_cast<int>(staff->idx());
    int nextStaffIdx  = firstStaffIdx + 1;

    //
    // collect program changes and controller
    //
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        for (EngravingItem* e : s->annotations()) {
            if (!e->isStaffTextBase() || static_cast<int>(e->staffIdx()) < firstStaffIdx
                || static_cast<int>(e->staffIdx()) >= nextStaffIdx) {
                continue;
            }
            const StaffTextBase* st1 = toStaffTextBase(e);
            Fraction tick = s->tick() + Fraction::fromTicks(tickOffset);

            Instrument* instr = e->part()->instrument(tick);
            for (const ChannelActions& ca : st1->channelActions()) {
                int channel = instr->channel().at(ca.channel)->channel();
                for (const String& ma : ca.midiActionNames) {
                    NamedEventList* nel = instr->midiAction(ma, ca.channel);
                    if (!nel) {
                        continue;
                    }
                    for (MidiCoreEvent event : nel->events) {
                        event.setChannel(channel);
                        NPlayEvent e1(event);
                        e1.setOriginatingStaff(firstStaffIdx);
                        if (e1.dataA() == CTRL_PROGRAM) {
                            events->insert(std::pair<int, NPlayEvent>(tick.ticks() - 1, e1));
                        } else {
                            events->insert(std::pair<int, NPlayEvent>(tick.ticks(), e1));
                        }
                    }
                }
            }
            if (st1->setAeolusStops()) {
                Staff* s1 = st1->staff();
                int voice   = 0;
                int channel = s1->channel(tick, voice);

                for (int i = 0; i < 4; ++i) {
                    static int num[4] = { 12, 13, 16, 16 };
                    for (int k = 0; k < num[i]; ++k) {
                        aeolusSetStop(tick.ticks(), channel, i, k, st1->getAeolusStop(i, k), events);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//    renderHarmony
///    renders chord symbols
//---------------------------------------------------------
static void renderHarmony(EventMap* events, Measure const* m, Harmony* h, int tickOffset)
{
    if (!h->isRealizable()) {
        return;
    }
    Staff* staff = m->score()->staff(h->track() / VOICES);
    const InstrChannel* channel = staff->part()->harmonyChannel();
    IF_ASSERT_FAILED(channel) {
        return;
    }

    events->registerChannel(channel->channel());
    if (!staff->isPrimaryStaff()) {
        return;
    }

    int staffIdx = static_cast<int>(staff->idx());
    int velocity = staff->velocities().val(h->tick());

    RealizedHarmony r = h->getRealizedHarmony();
    std::vector<int> pitches = r.pitches();

    NPlayEvent ev(ME_NOTEON, static_cast<uint8_t>(channel->channel()), 0, velocity);
    ev.setHarmony(h);
    Fraction duration = r.getActualDuration(h->tick().ticks() + tickOffset);

    int onTime = h->tick().ticks() + tickOffset;
    int offTime = onTime + duration.ticks();

    ev.setOriginatingStaff(staffIdx);
    ev.setTuning(0.0);

    //add play events
    for (int p : pitches) {
        ev.setPitch(p);
        ev.setVelo(velocity);
        events->insert(std::pair<int, NPlayEvent>(onTime, ev));
        ev.setVelo(0);
        events->insert(std::pair<int, NPlayEvent>(offTime, ev));
    }
}

void MidiRenderer::collectGraceBeforeChordEvents(Chord* chord, EventMap* events, double veloMultiplier,
                                                 Staff* st, int tickOffset, PitchWheelRenderer& pitchWheelRenderer)
{
    // calculate offset for grace notes here
    const auto& grChords = chord->graceNotesBefore();
    std::vector<Chord*> graceNotesBeforeBar;
    std::copy_if(grChords.begin(), grChords.end(), std::back_inserter(graceNotesBeforeBar), [](Chord* ch) {
        return ch->noteType() == NoteType::ACCIACCATURA;
    });

    int graceTickSum = 0;
    int graceTickOffset = 0;

    size_t acciacaturaGraceSize = graceNotesBeforeBar.size();
    if (acciacaturaGraceSize > 0) {
        graceTickSum = graceNotesBeforeBar[0]->ticks().ticks();
        graceTickOffset = graceTickSum / static_cast<int>(acciacaturaGraceSize);
    }

    if (!graceNotesMerged(chord)) {
        int currentBeaforeBeatNote = 0;
        for (Chord* c : chord->graceNotesBefore()) {
            for (const Note* note : c->notes()) {
                int channel = getChannel(chord->part()->instrument(chord->tick()), note);
                if (note->noteType() == NoteType::ACCIACCATURA) {
                    collectNote(events, channel, note, veloMultiplier, tickOffset, st,
                                pitchWheelRenderer,
                                graceTickSum - graceTickOffset * currentBeaforeBeatNote,
                                graceTickSum - graceTickOffset * (currentBeaforeBeatNote + 1) + 1);
                    currentBeaforeBeatNote++;
                } else {
                    collectNote(events, channel, note, veloMultiplier, tickOffset, st, pitchWheelRenderer);
                }
            }
        }
    }
}

//---------------------------------------------------------
//   collectMeasureEventsDefault
//---------------------------------------------------------

void MidiRenderer::doCollectMeasureEvents(EventMap* events, Measure const* m, const Staff* staff, int tickOffset,
                                          PitchWheelRenderer& pitchWheelRenderer)
{
    staff_idx_t firstStaffIdx = staff->idx();
    staff_idx_t nextStaffIdx  = firstStaffIdx + 1;

    SegmentType st = SegmentType::ChordRest;
    track_idx_t strack = firstStaffIdx * VOICES;
    track_idx_t etrack = nextStaffIdx * VOICES;
    for (Segment* seg = m->first(st); seg; seg = seg->next(st)) {
        Fraction tick = seg->tick();

        //render harmony
        for (EngravingItem* e : seg->annotations()) {
            if (!e || (e->track() < strack) || (e->track() >= etrack)) {
                continue;
            }
            Harmony* h = nullptr;
            if (e->isHarmony()) {
                h = toHarmony(e);
            } else if (e->isFretDiagram()) {
                h = toFretDiagram(e)->harmony();
            }
            if (!h || !h->play()) {
                continue;
            }
            renderHarmony(events, m, h, tickOffset);
        }

        for (track_idx_t track = strack; track < etrack; ++track) {
            // Skip linked staves, except primary
            Staff* st1 = m->score()->staff(track / VOICES);
            if (!st1->isPrimaryStaff()) {
                track += VOICES - 1;
                continue;
            }

            EngravingItem* cr = seg->element(track);
            if (!cr) {
                continue;
            }

            if (!cr->isChord()) {
                continue;
            }

            Chord* chord = toChord(cr);

            Instrument* instr = st1->part()->instrument(tick);

            // Get a velocity multiplier
            double veloMultiplier = 1;
            for (Articulation* a : chord->articulations()) {
                if (a->playArticulation()) {
                    veloMultiplier *= instr->getVelocityMultiplier(a->articulationName());
                }
            }

            //
            // Add normal note events
            //
            collectGraceBeforeChordEvents(chord, events, veloMultiplier, st1, tickOffset, pitchWheelRenderer);

            for (const Note* note : chord->notes()) {
                int channel = getChannel(instr, note);
                collectNote(events, channel, note, veloMultiplier, tickOffset, st1, pitchWheelRenderer);
            }

            if (!graceNotesMerged(chord)) {
                for (Chord* c : chord->graceNotesAfter()) {
                    for (const Note* note : c->notes()) {
                        int channel = getChannel(instr, note);
                        collectNote(events, channel, note, veloMultiplier, tickOffset, st1, pitchWheelRenderer);
                    }
                }
            }
        }
    }
}

MidiRenderer::MidiRenderer(Score* s)
    : score(s)
{
}

//---------------------------------------------------------
//   collectMeasureEvents
//    redirects to the correct function based on the passed method
//---------------------------------------------------------

void MidiRenderer::collectMeasureEvents(EventMap* events, Measure const* m, const Staff* staff, int tickOffset,
                                        PitchWheelRenderer& pitchWheelRenderer)
{
    doCollectMeasureEvents(events, m, staff, tickOffset, pitchWheelRenderer);

    collectProgramChanges(events, m, staff, tickOffset);
}

//---------------------------------------------------------
//   renderStaffChunk
//---------------------------------------------------------

void MidiRenderer::renderStaff(EventMap* events, const Staff* staff, PitchWheelRenderer& pitchWheelRenderer)
{
    Measure const* lastMeasure = nullptr;

    const RepeatList& repeatList = score->repeatList();

    for (const RepeatSegment* rs : repeatList) {
        const int tickOffset = rs->utick - rs->tick;

        Measure const* const start = rs->firstMeasure();
        Measure const* const end = rs->lastMeasure()->nextMeasure();

        for (Measure const* m = start; m; m = m->nextMeasure()) {
            staff_idx_t staffIdx = staff->idx();
            if (m->isMeasureRepeatGroup(staffIdx)) {
                MeasureRepeat* mr = m->measureRepeatElement(staffIdx);
                Measure const* playMeasure = lastMeasure;
                if (!playMeasure || !mr) {
                    continue;
                }

                for (int i = m->measureRepeatCount(staffIdx); i < mr->numMeasures() && playMeasure->prevMeasure(); ++i) {
                    playMeasure = playMeasure->prevMeasure();
                }

                int offset = (m->tick() - playMeasure->tick()).ticks();
                collectMeasureEvents(events, playMeasure, staff, tickOffset + offset, pitchWheelRenderer);
            } else {
                lastMeasure = m;
                collectMeasureEvents(events, lastMeasure, staff, tickOffset, pitchWheelRenderer);
            }
        }
    }
}

//---------------------------------------------------------
//   renderSpanners
//---------------------------------------------------------

void MidiRenderer::renderSpanners(EventMap* events, PitchWheelRenderer& pitchWheelRenderer)
{
    for (const auto& sp : score->spannerMap().map()) {
        Spanner* s = sp.second;

        int staff = static_cast<int>(s->staffIdx());
        int idx = s->staff()->channel(s->tick(), 0);
        int channel = s->part()->instrument(s->tick())->channel(idx)->channel();
        const auto& channels = _context.channels->channelsMap[channel];
        if (channels.empty()) {
            doRenderSpanners(events, s, channel, pitchWheelRenderer);
        } else {
            for (const auto& channel : channels) {
                doRenderSpanners(events, s, channel.second, pitchWheelRenderer);
            }
        }
    }
}

void MidiRenderer::doRenderSpanners(EventMap* events, Spanner* s, uint32_t channel, PitchWheelRenderer& pitchWheelRenderer)
{
    std::vector<std::pair<int, std::pair<bool, int> > > pedalEventList;

    int staff = static_cast<int>(s->staffIdx());

    if (s->isPedal()) {
        std::pair<int, std::pair<bool, int> > lastEvent;

        if (!pedalEventList.empty()) {
            lastEvent = pedalEventList.back();
        } else {
            lastEvent = std::pair<int, std::pair<bool, int> >(0, std::pair<bool, int>(true, staff));
        }

        int st = s->tick().ticks();

        if (lastEvent.second.first == false && lastEvent.first >= (st + 2)) {
            pedalEventList.pop_back();
            pedalEventList.push_back(std::pair<int,
                                               std::pair<bool,
                                                         int> >(st + (2 - MScore::pedalEventsMinTicks),
                                                                std::pair<bool, int>(false, staff)));
        }
        int a = st + 2;
        pedalEventList.push_back(std::pair<int, std::pair<bool, int> >(a, std::pair<bool, int>(true, staff)));

        int t = s->tick2().ticks() + (2 - MScore::pedalEventsMinTicks);
        const RepeatSegment& lastRepeat = *score->repeatList().back();
        if (t > lastRepeat.utick + lastRepeat.len()) {
            t = lastRepeat.utick + lastRepeat.len();
        }
        pedalEventList.push_back(std::pair<int, std::pair<bool, int> >(t, std::pair<bool, int>(false, staff)));
    } else if (s->isVibrato()) {
        int stick = s->tick().ticks();
        int etick = s->tick2().ticks();

        // from start to end of trill, send bend events at regular interval
        Vibrato* t = toVibrato(s);
        // guitar vibrato, up only
        int spitch = 0;       // 1/8 (100 is a semitone)
        int epitch = 12;
        if (t->vibratoType() == VibratoType::GUITAR_VIBRATO_WIDE) {
            spitch = 0;         // 1/4
            epitch = 25;
        }
        // vibrato with whammy bar up and down
        else if (t->vibratoType() == VibratoType::VIBRATO_SAWTOOTH_WIDE) {
            spitch = -25;         // 1/16
            epitch = 25;
        } else if (t->vibratoType() == VibratoType::VIBRATO_SAWTOOTH) {
            spitch = -12;
            epitch = 12;
        }

        collectVibrato(channel, stick, etick, spitch, epitch, pitchWheelRenderer);
    }

    for (const auto& pe : pedalEventList) {
        NPlayEvent event;
        if (pe.second.first == true) {
            event = NPlayEvent(ME_CONTROLLER, static_cast<uint8_t>(channel), CTRL_SUSTAIN, 127);
        } else {
            event = NPlayEvent(ME_CONTROLLER, static_cast<uint8_t>(channel), CTRL_SUSTAIN, 0);
        }
        event.setOriginatingStaff(pe.second.second);
        events->insert(std::pair<int, NPlayEvent>(pe.first, event));
    }
}

// This struct specifies how to render an articulation.
//   atype - the articulation type to implement, such as SymId::ornamentTurn
//   ostyles - the actual ornament has a property called ornamentStyle whose value is
//             a value of type OrnamentStyle.  This ostyles field indicates the
//             the set of ornamentStyles which apply to this rendition.
//   duration - the default duration for each note in the rendition, the final duration
//            rendered might be less than this if an articulation is attached to a note of
//            short duration.
//   prefix - vector of integers. indicating which notes to play at the beginning of rendering the
//            articulation.  0 represents the principle note, 1==> the note diatonically 1 above
//            -1 ==> the note diatonically 1 below.  E.g., in the key of G, if a turn articulation
//            occurs above the note F#, then 0==>F#, 1==>G, -1==>E.
//            These integers indicate which notes actual notes to play when rendering the ornamented
//            note.   However, if the same integer appears several times adjacently such as {0,0,0,1}
//            That means play the notes tied.  e.g., F# followed by G, but the duration of F# is 3x the
//            duration of the G.
//    body   - notes to play comprising the body of the rendered ornament.
//            The body differs from the prefix and suffix in several ways.
//            * body does not support tied notes: {0,0,0,1} means play 4 distinct notes (not tied).
//            * if there is sufficient duration in the principle note, AND repeatp is true, then body
//               will be rendered multiple times, as the duration allows.
//            * to avoid a time gap (or rest) in rendering the articulation, if sustainp is true,
//               then the final note of the body will be sustained to fill the left-over time.
//    suffix - similar to prefix but played once at the end of the rendered ornament.
//    repeatp  - whether the body is repeatable in its entirety.
//    sustainp - whether the final note of the body should be sustained to fill the remaining duration.

struct OrnamentExcursion {
    SymId atype;
    std::set<OrnamentStyle> ostyles;
    int duration;
    std::vector<int> prefix;
    std::vector<int> body;
    bool repeatp;
    bool sustainp;
    std::vector<int> suffix;
};

std::set<OrnamentStyle> baroque  = { OrnamentStyle::BAROQUE };
std::set<OrnamentStyle> defstyle = { OrnamentStyle::DEFAULT };
std::set<OrnamentStyle> any; // empty set has the special meaning of any-style, rather than no-styles.
int _16th = Constants::division / 4;
int _32nd = _16th / 2;

std::vector<OrnamentExcursion> excursions = {
    //  articulation type            set of  duration       body         repeatp      suffix
    //                               styles          prefix                    sustainp
    { SymId::ornamentTurn,                any, _32nd, {},    { 1, 0, -1, 0 },   false, true, {} },
    { SymId::ornamentTurnInverted,        any, _32nd, {},    { -1, 0, 1, 0 },   false, true, {} },
    { SymId::ornamentTurnSlash,           any, _32nd, {},    { -1, 0, 1, 0 },   false, true, {} },
    { SymId::ornamentTrill,           baroque, _32nd, { 1, 0 }, { 1, 0 },        true,  true, {} },
    { SymId::ornamentTrill,          defstyle, _32nd, { 0, 1 }, { 0, 1 },        true,  true, {} },
    { SymId::brassMuteClosed,         baroque, _32nd, { 0, -1 }, { 0, -1 },      true,  true, {} },
    { SymId::ornamentMordent,             any, _32nd, {},    { 0, -1, 0 },     false, true, {} },
    { SymId::ornamentShortTrill,     defstyle, _32nd, {},    { 0, 1, 0 },      false, true, {} },// inverted mordent
    { SymId::ornamentShortTrill,      baroque, _32nd, { 1, 0, 1 }, { 0 },         false, true, {} },// short trill
    { SymId::ornamentTremblement,         any, _32nd, { 1, 0 }, { 1, 0 },        false, true, {} },
    { SymId::brassMuteClosed,        defstyle, _32nd, {},    { 0 },             false, true, {} },// regular hand-stopped brass
    { SymId::ornamentPrallMordent,        any, _32nd, {},    { 1, 0, -1, 0 },   false, true, {} },
    { SymId::ornamentLinePrall,           any, _32nd, { 2, 2, 2 }, { 1, 0 },       true,  true, {} },
    { SymId::ornamentUpPrall,             any, _16th, { -1, 0 }, { 1, 0 },        true,  true, { 1, 0 } },// p 144 Ex 152 [1]
    { SymId::ornamentUpMordent,           any, _16th, { -1, 0 }, { 1, 0 },        true,  true, { -1, 0 } },// p 144 Ex 152 [1]
    { SymId::ornamentPrecompMordentUpperPrefix, any, _16th, { 1, 1, 1, 0 }, { 1, 0 },    true,  true, {} },// p136 Cadence Appuyee [1] [2]
    { SymId::ornamentDownMordent,         any, _16th, { 1, 1, 1, 0 }, { 1, 0 },    true,  true, { -1, 0 } },// p136 Cadence Appuyee + mordent [1] [2]
    { SymId::ornamentPrallUp,             any, _16th, { 1, 0 }, { 1, 0 },        true,  true, { -1, 0 } },// p136 Double Cadence [1]
    { SymId::ornamentPrallDown,           any, _16th, { 1, 0 }, { 1, 0 },        true,  true, { -1, 0, 0, 0 } },// p144 ex 153 [1]
    { SymId::ornamentPrecompSlide,        any, _32nd, {},    { 0 },          false, true, {} }

    // [1] Some of the articulations/ornaments in the excursions table above come from
    // Baroque Music, Style and Performance A Handbook, by Robert Donington,(c) 1982
    // ISBN 0-393-30052-8, W. W. Norton & Company, Inc.

    // [2] In some cases, the example from [1] does not preserve the timing.
    // For example, illustrates 2+1/4 counts per half note.
};

//---------------------------------------------------------
// findFirstTrill
//  search the spanners in the score, finding the first one
//  which overlaps this chord and is of type ElementType::TRILL
//---------------------------------------------------------

static Trill* findFirstTrill(Chord* chord)
{
    auto spanners = chord->score()->spannerMap().findOverlapping(1 + chord->tick().ticks(),
                                                                 chord->tick().ticks() + chord->actualTicks().ticks() - 1);
    for (auto i : spanners) {
        if (i.value->type() != ElementType::TRILL) {
            continue;
        }
        if (i.value->track() != chord->track()) {
            continue;
        }
        Trill* trill = toTrill(i.value);
        if (trill->playArticulation() == false) {
            continue;
        }
        return trill;
    }
    return nullptr;
}

// In the case that graceNotesBefore or graceNotesAfter are attached to a note
// with an articulation such as a trill, then the grace notes are/will-be/have-been
// already merged into the articulation.
// So this predicate, graceNotesMerged, checks for this condition to avoid calling
// functions which would re-emit the grace notes by a different algorithm.

static bool graceNotesMerged(Chord* chord)
{
    if (findFirstTrill(chord)) {
        return true;
    }
    for (Articulation* a : chord->articulations()) {
        for (auto& oe : excursions) {
            if (oe.atype == a->symId()) {
                return true;
            }
        }
    }
    return false;
}

//---------------------------------------------------------
//   renderMetronome
///   add metronome tick events
//---------------------------------------------------------

void MidiRenderer::renderMetronome(EventMap* events)
{
    Measure const* const start = score->firstMeasure();
    Measure const* const end = score->lastMeasure();

    for (Measure const* m = start; m != end; m = m->nextMeasure()) {
        renderMetronome(events, m);
    }
}

//---------------------------------------------------------
//   renderMetronome
///   add metronome tick events
//---------------------------------------------------------

void MidiRenderer::renderMetronome(EventMap* events, Measure const* m)
{
    int msrTick         = m->tick().ticks();
    BeatsPerSecond tempo = score->tempomap()->tempo(msrTick);
    TimeSigFrac timeSig = score->sigmap()->timesig(msrTick).nominal();

    int clickTicks      = timeSig.isBeatedCompound(tempo.val) ? timeSig.beatTicks() : timeSig.dUnitTicks();
    int endTick         = m->endTick().ticks();

    int rtick;

    if (m->isAnacrusis()) {
        int rem = m->ticks().ticks() % clickTicks;
        msrTick += rem;
        rtick = rem + timeSig.ticksPerMeasure() - m->ticks().ticks();
    } else {
        rtick = 0;
    }

    for (int tick = msrTick; tick < endTick; tick += clickTicks, rtick += clickTicks) {
        events->insert(std::pair<int, NPlayEvent>(tick, NPlayEvent(timeSig.rtick2beatType(rtick))));
    }
}

void MidiRenderer::renderScore(EventMap* events, const Context& ctx)
{
    _context = ctx;
    PitchWheelRenderer pitchWheelRender(wheelSpec);

    score->updateSwing();
    score->updateCapo();

    score->createPlayEvents(score->firstMeasure(), nullptr);

    score->updateChannel();
    score->updateVelo();

    // create note & other events
    for (const Staff* st : score->staves()) {
        renderStaff(events, st, pitchWheelRender);
    }
    events->fixupMIDI();

    // create sustain pedal events
    renderSpanners(events, pitchWheelRender);

    EventMap pitchWheelEvents = pitchWheelRender.renderPitchWheel();
    events->merge(pitchWheelEvents);

    if (ctx.metronome) {
        renderMetronome(events);
    }

    // NOTE:JT this is a temporary fix for duplicate events until polyphonic aftertouch support
    // can be implemented. This removes duplicate SND events.
    int lastChannel = -1;
    int lastController = -1;
    int lastValue = -1;
    for (auto i = events->begin(); i != events->end();) {
        if (i->second.type() == ME_CONTROLLER) {
            auto& event = i->second;
            if (event.channel() == lastChannel
                && event.controller() == lastController
                && event.value() == lastValue) {
                i = events->erase(i);
            } else {
                lastChannel = event.channel();
                lastController = event.controller();
                lastValue = event.value();
                i++;
            }
        } else {
            i++;
        }
    }
}

uint32_t MidiRenderer::getChannel(const Instrument* instr, const Note* note)
{
    int subchannel = note->subchannel();
    int channel = instr->channel(subchannel)->channel();

    if (!_context.eachStringHasChannel || !instr->hasStrings()) {
        return channel;
    }

    return _context.channels->getChannel(channel, note->string());
}

uint32_t MidiRenderer::ChannelLookup::getChannel(uint32_t instrumentChannel, int32_t string)
{
    auto& channelsForString = channelsMap[instrumentChannel];

    if (string == -1) {
        auto channelIt = channelsForString.find(string);
        if (channelIt != channelsForString.end()) {
            return channelIt->second;
        } else {
            channelsForString[string] = maxChannel;
            return maxChannel++;
        }
    }

    auto channelIt = channelsForString.find(string);
    if (channelIt != channelsForString.end()) {
        return channelIt->second;
    }

    channelsForString.insert({ string, maxChannel });
    return maxChannel++;
}
}
