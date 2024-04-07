/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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

#include "fluidsequencer.h"

#include "global/interpolation.h"

using namespace muse;
using namespace muse::audio;
using namespace muse::midi;
using namespace muse::mpe;

static constexpr mpe::pitch_level_t MIN_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(PitchClass::C, 0);
static constexpr note_idx_t MIN_SUPPORTED_NOTE = 12; // MIDI equivalent for C0
static constexpr mpe::pitch_level_t MAX_SUPPORTED_PITCH_LEVEL = mpe::pitchLevel(PitchClass::C, 8);
static constexpr note_idx_t MAX_SUPPORTED_NOTE = 108; // MIDI equivalent for C8

void FluidSequencer::init(const PlaybackSetupData& setupData, const std::optional<midi::Program>& programOverride)
{
    m_channels.init(setupData, programOverride);
}

int FluidSequencer::currentExpressionLevel() const
{
    return expressionLevel(dynamicLevel(m_playbackPosition));
}

void FluidSequencer::updateOffStreamEvents(const mpe::PlaybackEventsMap& events, const PlaybackParamMap&)
{
    m_offStreamEvents.clear();

    if (m_onOffStreamFlushed) {
        m_onOffStreamFlushed();
    }

    updatePlaybackEvents(m_offStreamEvents, events);
    updateOffSequenceIterator();
}

void FluidSequencer::updateMainStreamEvents(const mpe::PlaybackEventsMap& events, const mpe::DynamicLevelMap& dynamics,
                                            const mpe::PlaybackParamMap&)
{
    m_dynamicLevelMap = dynamics;

    m_mainStreamEvents.clear();
    m_dynamicEvents.clear();

    if (m_onMainStreamFlushed) {
        m_onMainStreamFlushed();
    }

    updatePlaybackEvents(m_mainStreamEvents, events);
    updateMainSequenceIterator();

    updateDynamicEvents(m_dynamicEvents, dynamics);
    updateDynamicChangesIterator();
}

muse::async::Channel<channel_t, Program> FluidSequencer::channelAdded() const
{
    return m_channels.channelAdded;
}

const ChannelMap& FluidSequencer::channels() const
{
    return m_channels;
}

void FluidSequencer::updatePlaybackEvents(EventSequenceMap& destination, const mpe::PlaybackEventsMap& changes)
{
    for (const auto& pair : changes) {
        for (const mpe::PlaybackEvent& event : pair.second) {
            if (!std::holds_alternative<mpe::NoteEvent>(event)) {
                continue;
            }

            const mpe::NoteEvent& noteEvent = std::get<mpe::NoteEvent>(event);

            timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;
            timestamp_t timestampTo = timestampFrom + noteEvent.arrangementCtx().actualDuration;

            channel_t channelIdx = channel(noteEvent);
            note_idx_t noteIdx = noteIndex(noteEvent.pitchCtx().nominalPitchLevel);
            velocity_t velocity = noteVelocity(noteEvent);
            tuning_t tuning = noteTuning(noteEvent, noteIdx);

            midi::Event noteOn(Event::Opcode::NoteOn, Event::MessageType::ChannelVoice20);
            noteOn.setChannel(channelIdx);
            noteOn.setNote(noteIdx);
            noteOn.setVelocity(velocity);
            noteOn.setPitchNote(noteIdx, tuning);

            destination[timestampFrom].emplace(std::move(noteOn));

            midi::Event noteOff(Event::Opcode::NoteOff, Event::MessageType::ChannelVoice20);
            noteOff.setChannel(channelIdx);
            noteOff.setNote(noteIdx);
            noteOff.setPitchNote(noteIdx, tuning);

            destination[timestampTo].emplace(std::move(noteOff));

            appendControlSwitch(destination, noteEvent, PEDAL_CC_SUPPORTED_TYPES, 64);
            appendPitchBend(destination, noteEvent, BEND_SUPPORTED_TYPES, channelIdx);
        }
    }
}

void FluidSequencer::updateDynamicEvents(EventSequenceMap& destination, const mpe::DynamicLevelMap& changes)
{
    for (const auto& pair : changes) {
        muse::midi::Event event(muse::midi::Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        event.setIndex(muse::midi::EXPRESSION_CONTROLLER);
        event.setData(expressionLevel(pair.second));

        destination[pair.first].emplace(std::move(event));
    }
}

void FluidSequencer::appendControlSwitch(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                         const mpe::ArticulationTypeSet& appliableTypes, const int midiControlIdx)
{
    mpe::ArticulationType currentType = mpe::ArticulationType::Undefined;

    for (const mpe::ArticulationType type : appliableTypes) {
        if (noteEvent.expressionCtx().articulations.contains(type)) {
            currentType = type;
            break;
        }
    }

    if (currentType != mpe::ArticulationType::Undefined) {
        const ArticulationAppliedData& articulationData = noteEvent.expressionCtx().articulations.at(currentType);
        const ArticulationMeta& articulationMeta = articulationData.meta;

        midi::Event start(Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        start.setIndex(midiControlIdx);
        start.setData(127);

        destination[noteEvent.arrangementCtx().actualTimestamp].emplace(std::move(start));

        midi::Event end(Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        end.setIndex(midiControlIdx);
        end.setData(0);

        destination[articulationMeta.timestamp + articulationMeta.overallDuration].emplace(std::move(end));
    } else {
        midi::Event cc(Event::Opcode::ControlChange, Event::MessageType::ChannelVoice10);
        cc.setIndex(midiControlIdx);
        cc.setData(0);

        destination[noteEvent.arrangementCtx().actualTimestamp].emplace(std::move(cc));
    }
}

void FluidSequencer::appendPitchBend(EventSequenceMap& destination, const mpe::NoteEvent& noteEvent,
                                     const mpe::ArticulationTypeSet& appliableTypes, const channel_t channelIdx)
{
    mpe::ArticulationType currentType = mpe::ArticulationType::Undefined;

    for (const mpe::ArticulationType type : appliableTypes) {
        if (noteEvent.expressionCtx().articulations.contains(type)) {
            currentType = type;
            break;
        }
    }

    timestamp_t timestampFrom = noteEvent.arrangementCtx().actualTimestamp;
    midi::Event event(Event::Opcode::PitchBend, Event::MessageType::ChannelVoice10);
    event.setChannel(channelIdx);

    if (currentType == mpe::ArticulationType::Undefined || noteEvent.pitchCtx().pitchCurve.empty()) {
        event.setData(8192);
        destination[timestampFrom].emplace(std::move(event));
        return;
    }

    mpe::duration_t duration = noteEvent.arrangementCtx().actualDuration;

    auto currIt = noteEvent.pitchCtx().pitchCurve.cbegin();
    auto nextIt = std::next(currIt);
    auto endIt = noteEvent.pitchCtx().pitchCurve.cend();

    if (nextIt == endIt) {
        int bendValue = pitchBendLevel(currIt->second);
        timestamp_t time = timestampFrom + duration * percentageToFactor(currIt->first);
        event.setData(bendValue);
        destination[time].insert(std::move(event));
        return;
    }

    auto makePoint = [](mpe::timestamp_t time, int value) {
        return mu::Interpolation::Point { static_cast<double>(time), static_cast<double>(value) };
    };

    //! NOTE: Increasing this number results in fewer points being interpolated
    const mpe::pitch_level_t POINT_WEIGHT = currentType == mpe::ArticulationType::Multibend
                                            ? mpe::PITCH_LEVEL_STEP / 5
                                            : mpe::PITCH_LEVEL_STEP / 2;

    for (; nextIt != endIt; currIt = nextIt, nextIt = std::next(currIt)) {
        int currBendValue = pitchBendLevel(currIt->second);
        int nextBendValue = pitchBendLevel(nextIt->second);

        timestamp_t currTime = timestampFrom + duration * percentageToFactor(currIt->first);
        timestamp_t nextTime = timestampFrom + duration * percentageToFactor(nextIt->first);

        mu::Interpolation::Point p0 = makePoint(currTime, currBendValue);
        mu::Interpolation::Point p1 = makePoint(nextTime, currBendValue);
        mu::Interpolation::Point p2 = makePoint(nextTime, nextBendValue);

        size_t pointCount = std::abs(nextIt->second - currIt->second) / POINT_WEIGHT;
        pointCount = std::max(pointCount, size_t(1));

        std::vector<mu::Interpolation::Point> points = mu::Interpolation::quadraticBezierCurve(p0, p1, p2, pointCount);

        for (const mu::Interpolation::Point& point : points) {
            timestamp_t time = static_cast<timestamp_t>(std::round(point.x));
            int bendValue = static_cast<int>(std::round(point.y));

            event.setData(bendValue);
            destination[time].insert(event);
        }
    }
}

channel_t FluidSequencer::channel(const mpe::NoteEvent& noteEvent) const
{
    return m_channels.resolveChannelForEvent(noteEvent);
}

note_idx_t FluidSequencer::noteIndex(const mpe::pitch_level_t pitchLevel) const
{
    if (pitchLevel <= MIN_SUPPORTED_PITCH_LEVEL) {
        return MIN_SUPPORTED_NOTE;
    }

    if (pitchLevel >= MAX_SUPPORTED_PITCH_LEVEL) {
        return MAX_SUPPORTED_NOTE;
    }

    float stepCount = MIN_SUPPORTED_NOTE
                      + ((pitchLevel - MIN_SUPPORTED_PITCH_LEVEL)
                         / static_cast<float>(mpe::PITCH_LEVEL_STEP));

    return stepCount;
}

tuning_t FluidSequencer::noteTuning(const mpe::NoteEvent& noteEvent, const int noteIdx) const
{
    int semitonesCount = noteIdx - MIN_SUPPORTED_NOTE;

    mpe::pitch_level_t tuningPitchLevel = noteEvent.pitchCtx().nominalPitchLevel
                                          - (semitonesCount * mpe::PITCH_LEVEL_STEP);

    return tuningPitchLevel / static_cast<float>(mpe::PITCH_LEVEL_STEP);
}

velocity_t FluidSequencer::noteVelocity(const mpe::NoteEvent& noteEvent) const
{
    static constexpr midi::velocity_t MAX_SUPPORTED_VELOCITY = 127;

    velocity_t result = mu::RealRound(noteEvent.expressionCtx().expressionCurve.velocityFraction() * MAX_SUPPORTED_VELOCITY, 0);

    return std::clamp<velocity_t>(result, 0, MAX_SUPPORTED_VELOCITY);
}

int FluidSequencer::expressionLevel(const mpe::dynamic_level_t dynamicLevel) const
{
    static constexpr mpe::dynamic_level_t MIN_SUPPORTED_DYNAMICS_LEVEL = mpe::dynamicLevelFromType(DynamicType::ppp);
    static constexpr mpe::dynamic_level_t MAX_SUPPORTED_DYNAMICS_LEVEL = mpe::dynamicLevelFromType(DynamicType::fff);
    static constexpr int MIN_SUPPORTED_VOLUME = 16; // MIDI equivalent for PPP
    static constexpr int MAX_SUPPORTED_VOLUME = 127; // MIDI equivalent for FFF
    static constexpr int VOLUME_STEP = 16;

    if (dynamicLevel <= MIN_SUPPORTED_DYNAMICS_LEVEL) {
        return MIN_SUPPORTED_VOLUME;
    }

    if (dynamicLevel >= MAX_SUPPORTED_DYNAMICS_LEVEL) {
        return MAX_SUPPORTED_VOLUME;
    }

    float stepCount = ((dynamicLevel - MIN_SUPPORTED_DYNAMICS_LEVEL)
                       / static_cast<float>(mpe::DYNAMIC_LEVEL_STEP));

    if (dynamicLevel == mpe::dynamicLevelFromType(DynamicType::Natural)) {
        stepCount -= 0.5;
    }

    if (dynamicLevel > mpe::dynamicLevelFromType(DynamicType::Natural)) {
        stepCount -= 1;
    }

    dynamic_level_t result = mu::RealRound(MIN_SUPPORTED_VOLUME + (stepCount * VOLUME_STEP), 0);

    return std::min(result, MAX_SUPPORTED_DYNAMICS_LEVEL);
}

int FluidSequencer::pitchBendLevel(const mpe::pitch_level_t pitchLevel) const
{
    static constexpr int PITCH_BEND_SEMITONE_STEP = 4096 / 12;

    float pitchLevelSteps = pitchLevel / static_cast<float>(mpe::PITCH_LEVEL_STEP);

    int offset = pitchLevelSteps * PITCH_BEND_SEMITONE_STEP;

    return std::clamp(8192 + offset, 0, 16383);
}
