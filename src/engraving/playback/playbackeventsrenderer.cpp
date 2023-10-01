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

#include "playbackeventsrenderer.h"

#include "log.h"

#include "dom/chord.h"
#include "dom/harmony.h"
#include "dom/note.h"
#include "dom/accidental.h"
#include "dom/rest.h"
#include "dom/sig.h"
#include "dom/tempo.h"
#include "dom/staff.h"

#include "utils/arrangementutils.h"
#include "metaparsers/chordarticulationsparser.h"
#include "metaparsers/notearticulationsparser.h"

#include "renderers/bendsrenderer.h"
#include "renderers/gracechordsrenderer.h"
#include "renderers/chordarticulationsrenderer.h"
#include "filters/chordfilter.h"

using namespace mu::engraving;
using namespace mu::mpe;

namespace mu {
bool g_tuneMap_loaded = false;
bool g_tuning_additive = false;
bool g_tuning_write    = false;
std::map<int, double> g_tunePitchMap;
std::map<int, double> g_tuneAccidentalMap;

double tuneMapGet(const mu::engraving::Note* note, int pitch, double curTuning)
{
    if (!g_tuneMap_loaded) {
        return curTuning;
    }
    double tuning = 0;
    if (g_tuning_additive) {
        tuning += curTuning;
    }
    Accidental* acc = note->accidental();
    if (acc) {
        AccidentalType at = acc->accidentalType();
        int sid = static_cast<int>(acc->symId());
        const char* str = acc->subtype2name(at).ascii();
        if (g_tuneAccidentalMap.find(sid) != g_tuneAccidentalMap.end()) {
            LOGI("accidental-tune-map accidental acc-sid=%i acc-name=%s", sid, str);
            return g_tuneAccidentalMap[sid] + tuning;
        }
    }

    if (g_tunePitchMap.find(pitch) != g_tunePitchMap.end()) {
        LOGI("tunemap %i, %f + %f", pitch, g_tunePitchMap[pitch], tuning);
        return g_tunePitchMap[pitch] + tuning;
    }
    return tuning;
}
}

static ArticulationMap makeStandardArticulationMap(const ArticulationsProfilePtr profile, timestamp_t timestamp, duration_t duration)
{
    ArticulationMeta meta(ArticulationType::Standard,
                          profile->pattern(ArticulationType::Standard),
                          timestamp,
                          duration,
                          0,
                          0);

    ArticulationMap articulations;
    articulations.emplace(ArticulationType::Standard, mu::mpe::ArticulationAppliedData(std::move(meta), 0, mu::mpe::HUNDRED_PERCENT));
    articulations.preCalculateAverageData();

    return articulations;
}

void PlaybackEventsRenderer::render(const EngravingItem* item, const dynamic_level_t nominalDynamicLevel,
                                    const ArticulationType persistentArticulationApplied,
                                    const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    render(item, 0, nominalDynamicLevel, persistentArticulationApplied, profile, result);
}

void PlaybackEventsRenderer::render(const EngravingItem* item, const int tickPositionOffset,
                                    const dynamic_level_t nominalDynamicLevel,
                                    const ArticulationType persistentArticulationApplied, const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(item->isChordRest()) {
        return;
    }

    if (item->type() == ElementType::CHORD) {
        renderNoteEvents(toChord(item), tickPositionOffset, nominalDynamicLevel, persistentArticulationApplied, profile, result);
    } else if (item->type() == ElementType::REST) {
        renderRestEvents(toRest(item), tickPositionOffset, result);
    }
}

void PlaybackEventsRenderer::render(const EngravingItem* item, const mpe::timestamp_t actualTimestamp,
                                    const mpe::duration_t actualDuration, const mpe::dynamic_level_t actualDynamicLevel,
                                    const ArticulationType persistentArticulationApplied, const ArticulationsProfilePtr profile,
                                    PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(item->isChordRest() || item->isNote()) {
        return;
    }

    ElementType type = item->type();

    if (type == ElementType::CHORD) {
        const Chord* chord = toChord(item);
        mpe::PlaybackEventList& events = result[actualTimestamp];

        for (const Note* note : chord->notes()) {
            renderFixedNoteEvent(note, actualTimestamp, actualDuration,
                                 actualDynamicLevel, persistentArticulationApplied, profile, events);
        }
    } else if (type == ElementType::NOTE) {
        renderFixedNoteEvent(toNote(item), actualTimestamp, actualDuration,
                             actualDynamicLevel, persistentArticulationApplied, profile, result[actualTimestamp]);
    } else if (type == ElementType::REST) {
        renderRestEvents(toRest(item), 0, result);
    }
}

void PlaybackEventsRenderer::renderChordSymbol(const Harmony* chordSymbol,
                                               const int ticksPositionOffset,
                                               const mpe::ArticulationsProfilePtr profile,
                                               mpe::PlaybackEventsMap& result) const
{
    if (!chordSymbol->isRealizable()) {
        return;
    }

    const RealizedHarmony& realized = chordSymbol->getRealizedHarmony();
    const RealizedHarmony::PitchMap& notes = realized.notes();

    const Score* score = chordSymbol->score();
    int positionTick = chordSymbol->tick().ticks();

    timestamp_t eventTimestamp = timestampFromTicks(score, positionTick + ticksPositionOffset);
    PlaybackEventList& events = result[eventTimestamp];

    int durationTicks = realized.getActualDuration(positionTick + ticksPositionOffset).ticks();
    duration_t duration = timestampFromTicks(score, positionTick + ticksPositionOffset + durationTicks) - eventTimestamp;

    voice_layer_idx_t voiceIdx = static_cast<voice_layer_idx_t>(chordSymbol->voice());
    staff_layer_idx_t staffIdx = static_cast<staff_layer_idx_t>(chordSymbol->staffIdx());
    Key key = chordSymbol->staff()->key(chordSymbol->tick());

    ArticulationMap articulations = makeStandardArticulationMap(profile, eventTimestamp, duration);

    double bps = score->tempomap()->tempo(positionTick).val;

    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        int pitch = it->first;
        //if (g_tuning_write) {
        //    it->setTuning(tuneMapGet(pitch, it->tuning()));
        //}
        int tpc = pitch2tpc(pitch, key, Prefer::NEAREST);
        int octave = playingOctave(pitch, tpc);
        pitch_level_t pitchLevel = notePitchLevel(tpc, octave);

        events.emplace_back(mpe::NoteEvent(eventTimestamp,
                                           duration,
                                           voiceIdx,
                                           staffIdx,
                                           pitchLevel,
                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                           articulations,
                                           bps));
    }
}

void PlaybackEventsRenderer::renderChordSymbol(const Harmony* chordSymbol, const mpe::timestamp_t actualTimestamp,
                                               const mpe::duration_t actualDuration, const ArticulationsProfilePtr profile,
                                               mpe::PlaybackEventsMap& result) const
{
    if (!chordSymbol->isRealizable()) {
        return;
    }

    const RealizedHarmony& realized = chordSymbol->getRealizedHarmony();
    const RealizedHarmony::PitchMap& notes = realized.notes();

    PlaybackEventList& events = result[actualTimestamp];

    voice_layer_idx_t voiceIdx = static_cast<voice_layer_idx_t>(chordSymbol->voice());
    staff_layer_idx_t staffIdx = static_cast<staff_layer_idx_t>(chordSymbol->staffIdx());

    Key key = chordSymbol->staff()->key(chordSymbol->tick());

    ArticulationMap articulations = makeStandardArticulationMap(profile, actualTimestamp, actualDuration);

    for (auto it = notes.cbegin(); it != notes.cend(); ++it) {
        int pitch = it->first;
        //if (g_tuning_write) {
        //    it->setTuning(tuneMapGet(pitch, it->tuning()));
        //}
        int tpc = pitch2tpc(pitch, key, Prefer::NEAREST);
        int octave = playingOctave(pitch, tpc);
        pitch_level_t pitchLevel = notePitchLevel(tpc, octave);

        events.emplace_back(mpe::NoteEvent(actualTimestamp,
                                           actualDuration,
                                           voiceIdx,
                                           staffIdx,
                                           pitchLevel,
                                           dynamicLevelFromType(mpe::DynamicType::Natural),
                                           articulations,
                                           Constants::DEFAULT_TEMPO.val));
    }
}

void PlaybackEventsRenderer::renderMetronome(const Score* score, const int measureStartTick, const int measureEndTick,
                                             const int ticksPositionOffset, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(measureStartTick).timesig();
    BeatsPerSecond bps = score->tempomap()->tempo(measureStartTick);

    int step = timeSignatureFraction.isBeatedCompound(bps.val)
               ? timeSignatureFraction.beatTicks() : timeSignatureFraction.dUnitTicks();

    for (int tick = measureStartTick; tick < measureEndTick; tick += step) {
        timestamp_t eventTimestamp = timestampFromTicks(score, tick + ticksPositionOffset);

        renderMetronome(score, tick, eventTimestamp, result);
    }
}

void PlaybackEventsRenderer::renderMetronome(const Score* score, const int tick, const mpe::timestamp_t actualTimestamp,
                                             mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(score) {
        return;
    }

    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(tick).timesig();
    int ticksPerBeat = timeSignatureFraction.ticks() / timeSignatureFraction.numerator();

    duration_t duration = timestampFromTicks(score, tick + ticksPerBeat) - actualTimestamp;

    BeatType beatType = score->tick2beatType(Fraction::fromTicks(tick));
    pitch_level_t eventPitchLevel = beatType == BeatType::DOWNBEAT
                                    ? pitchLevel(PitchClass::E, 5) // high wood block
                                    : pitchLevel(PitchClass::F, 5); // low wood block

    static const ArticulationMap emptyArticulations;

    BeatsPerSecond bps = score->tempomap()->tempo(tick);

    result[actualTimestamp].emplace_back(mpe::NoteEvent(actualTimestamp,
                                                        duration,
                                                        0,
                                                        0,
                                                        eventPitchLevel,
                                                        dynamicLevelFromType(mpe::DynamicType::Natural),
                                                        emptyArticulations,
                                                        bps.val));
}

void PlaybackEventsRenderer::renderNoteEvents(const Chord* chord, const int tickPositionOffset,
                                              const mpe::dynamic_level_t nominalDynamicLevel,
                                              const ArticulationType persistentArticulationApplied,
                                              const mpe::ArticulationsProfilePtr profile, PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(chord) {
        return;
    }

    int chordPosTick = chord->tick().ticks();
    int chordDurationTicks = chord->actualTicks().ticks();

    const Score* score = chord->score();

    auto chordTnD = timestampAndDurationFromStartAndDurationTicks(score, chordPosTick + tickPositionOffset, chordDurationTicks);

    BeatsPerSecond bps = score->tempomap()->tempo(chordPosTick);
    TimeSigFrac timeSignatureFraction = score->sigmap()->timesig(chordPosTick).timesig();

    static ArticulationMap articulations;

    RenderingContext ctx(chordTnD.timestamp,
                         chordTnD.duration,
                         nominalDynamicLevel,
                         chord->tick().ticks(),
                         tickPositionOffset,
                         chordDurationTicks,
                         bps,
                         timeSignatureFraction,
                         persistentArticulationApplied,
                         articulations,
                         profile);

    if (!ChordFilter::isItemPlayable(chord, ctx)) {
        return;
    }

    ChordArticulationsParser::buildChordArticulationMap(chord, ctx, ctx.commonArticulations);

    renderArticulations(chord, ctx, result[ctx.nominalTimestamp]);
}

void PlaybackEventsRenderer::renderFixedNoteEvent(const Note* note, const mpe::timestamp_t actualTimestamp,
                                                  const mpe::duration_t actualDuration,
                                                  const mpe::dynamic_level_t actualDynamicLevel,
                                                  const mpe::ArticulationType persistentArticulationApplied,
                                                  const mpe::ArticulationsProfilePtr profile, mpe::PlaybackEventList& result) const
{
    static const ArticulationMap articulations;

    RenderingContext ctx(actualTimestamp,
                         actualDuration,
                         actualDynamicLevel,
                         0,
                         0,
                         ticksFromTempoAndDuration(Constants::DEFAULT_TEMPO.val, actualDuration),
                         Constants::DEFAULT_TEMPO,
                         TimeSigMap::DEFAULT_TIME_SIGNATURE,
                         persistentArticulationApplied,
                         articulations,
                         profile);

    NoteArticulationsParser::buildNoteArticulationMap(note, ctx, ctx.commonArticulations);
    NominalNoteCtx noteCtx(note, ctx);

    result.emplace_back(buildNoteEvent(std::move(noteCtx)));
}

void PlaybackEventsRenderer::renderRestEvents(const Rest* rest, const int tickPositionOffset, mpe::PlaybackEventsMap& result) const
{
    IF_ASSERT_FAILED(rest) {
        return;
    }

    int positionTick = rest->tick().ticks();
    int durationTicks = rest->ticks().ticks();

    auto nominalTnD
        = timestampAndDurationFromStartAndDurationTicks(rest->score(), positionTick + tickPositionOffset, durationTicks);

    result[nominalTnD.timestamp].emplace_back(mpe::RestEvent(nominalTnD.timestamp, nominalTnD.duration,
                                                             static_cast<voice_layer_idx_t>(rest->voice())));
}

void PlaybackEventsRenderer::renderArticulations(const Chord* chord, const RenderingContext& ctx, mpe::PlaybackEventList& result) const
{
    if (ctx.commonArticulations.contains(mpe::ArticulationType::Multibend)) {
        BendsRenderer::render(chord, mpe::ArticulationType::Last, ctx, result);
        return;
    }

    for (const auto& type : ctx.commonArticulations) {
        if (GraceChordsRenderer::isAbleToRender(type.first)) {
            GraceChordsRenderer::render(chord, type.first, ctx, result);
            return;
        }
    }

    ChordArticulationsRenderer::render(chord, ArticulationType::Last, ctx, result);
}
