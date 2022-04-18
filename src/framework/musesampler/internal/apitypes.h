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

#ifndef MU_MUSESAMPLER_APITYPES_H
#define MU_MUSESAMPLER_APITYPES_H

#include <stdint.h>
#include <dlfcn.h>
#include <memory>

#include "log.h"

typedef void* MuseSamplerLib;

typedef enum ms_Result
{
    ms_Result_OK = 0,
    ms_Result_Error = -1,
    ms_Result_TimeoutErr = -2
} ms_Result;

// Functions to return the major + minor version of the core code. Note that this is independent of the UI version.
typedef const char*(* ms_get_version_string)();
typedef int (* ms_get_version_major)();
typedef int (* ms_get_version_minor)();
typedef int (* ms_get_version_revision)();

/*\\\ TYPES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

typedef void* ms_MuseSampler;
typedef void* ms_InstrumentList;
typedef void* ms_InstrumentInfo;
typedef void* ms_Track;

typedef struct ms_OutputBuffer
{
    float** _channels;
    int _num_data_pts;
    int _num_channels; // For sanity checking! Shouldn't change from init!
} ms_OutputBuffer;

typedef struct ms_DynamicsEvent
{
    // TODO
} ms_DynamicsEvent;

enum ms_NoteArticulation : uint64_t
{
    ms_NoteArticulation_None = 0,
    ms_NoteArticulation_Staccato = 1LL << 0,
    ms_NoteArticulation_Staccatissimo = 1LL << 1,
    ms_NoteArticulation_Accent = 1LL << 2,
    ms_NoteArticulation_Tenuto = 1LL << 3,
    ms_NoteArticulation_Marcato = 1LL << 4,
    ms_NoteArticulation_Harmonics = 1LL << 5,
    ms_NoteArticulation_Mute = 1LL << 6,
    ms_NoteArticulation_HalfTrill = 1LL << 7,
    ms_NoteArticulation_WholeTrill = 1LL << 8,
    ms_NoteArticulation_MordentSemi = 1LL << 9,
    ms_NoteArticulation_MordentWhole = 1LL << 10,
    ms_NoteArticulation_MordentInvertedSemi = 1LL << 11,
    ms_NoteArticulation_MordentInvertedWhole = 1LL << 12,
    ms_NoteArticulation_TurnSemiWhole = 1LL << 13,
    ms_NoteArticulation_TurnSemiSemi = 1LL << 14,
    ms_NoteArticulation_TurnWholeWhole = 1LL << 15,
    ms_NoteArticulation_TurnWholeSemi = 1LL << 16,
    ms_NoteArticulation_TurnInvertedSemiWhole = 1LL << 17,
    ms_NoteArticulation_TurnInvertedSemiSemi = 1LL << 18,
    ms_NoteArticulation_TurnInvertedWholeWhole = 1LL << 19,
    ms_NoteArticulation_TurnInvertedWholeSemi = 1LL << 20,
    ms_NoteArticulation_ArpeggioUp = 1LL << 21,
    ms_NoteArticulation_ArpeggioDown = 1LL << 22,
    ms_NoteArticulation_Tremolo1 = 1LL << 23,
    ms_NoteArticulation_Tremolo2 = 1LL << 24,
    ms_NoteArticulation_Tremolo3 = 1LL << 25,
    ms_NoteArticulation_Scoop = 1LL << 26,
    ms_NoteArticulation_Plop = 1LL << 27,
    ms_NoteArticulation_Doit = 1LL << 28,
    ms_NoteArticulation_Fall = 1LL << 29,
    ms_NoteArticulation_Appoggiatura = 1LL << 30, // Duration is ignored
    ms_NoteArticulation_Acciaccatura = 1LL << 31, // Duration is ignored
    ms_NoteArticulation_XNote = 1LL << 32,
    ms_NoteArticulation_Ghost = 1LL << 33,
    ms_NoteArticulation_Circle = 1LL << 34,
    ms_NoteArticulation_Triangle = 1LL << 35,
    ms_NoteArticulation_Diamond = 1LL << 36,
};

enum ms_TwoNoteArticulation : uint64_t
{
    ms_TwoNoteArticulation_None = 0,
    ms_TwoNoteArticulation_TwoNoteTremolo = 1LL << 1,
    ms_TwoNoteArticulation_Glissando = 1LL << 2,
    ms_TwoNoteArticulation_Portamento = 1LL << 3,
};

enum ms_RangeArticulation : uint64_t
{
    ms_RangeArticulation_None = 0,
    ms_RangeArticulation_Slur = 1LL << 1,
    ms_RangeArticulation_Pedal = 1LL << 2,
};

enum ms_EventType
{
    ms_EventTypeNote = 0,
    ms_EventTypeSlur = 1
};

typedef struct ms_NoteEvent
{
    int _voice; // 0-3
    long _location_ms;
    long _duration_ms;
    int _pitch; // MIDI pitch
    ms_NoteArticulation _articulation;
} ms_NoteEvent;

typedef struct ms_TwoNoteArticulationEvent
{
    int _voice; // 0-3
    long _second_note_location_ms;
    ms_TwoNoteArticulation _articulation;
} ms_TwoNoteArticulationEvent;

typedef struct ms_RangeArticulationEvent
{
    int _voice; // 0-3
    long _start_location_ms;
    long _end_location_ms;
    ms_RangeArticulation _articulation;
} ms_RangeArticulationEvent;

typedef struct ms_Event
{
    ms_EventType _event_type;
    union
    {
        ms_NoteEvent _note;
        // Notes must both already exist in the given voice
        ms_TwoNoteArticulationEvent _two_note_articulation;
        // Range of notes must already exist, and notes must exist starting at
        // start and ending at end of marked range.
        ms_RangeArticulationEvent _range_articulation;
    } _event;
} ms_Event;

typedef ms_Result (* ms_init)();
typedef ms_InstrumentList (* ms_get_instrument_list)();
typedef ms_InstrumentInfo (* ms_InstrumentList_get_next)(ms_InstrumentList instrument_list);

typedef int (* ms_Instrument_get_id)(ms_InstrumentInfo instrument);
typedef const char*(* ms_Instrument_get_name)(ms_InstrumentInfo);
typedef const char*(* ms_Instrument_get_musicxml_sound)(ms_InstrumentInfo);

typedef ms_MuseSampler (* ms_MuseSampler_create)();
typedef void (* ms_MuseSampler_destroy)(ms_MuseSampler);
typedef ms_Result (* ms_MuseSampler_init)(ms_MuseSampler ms, double sample_rate, int block_size, int channel_count);

typedef ms_Result (* ms_MuseSampler_set_demo_score)(ms_MuseSampler ms);
typedef ms_Result (* ms_MuseSampler_clear_score)(ms_MuseSampler ms);
typedef ms_Result (* ms_MuseSampler_finalize_score)(ms_MuseSampler ms);
typedef ms_Track (* ms_MuseSampler_add_track)(ms_MuseSampler ms, int instrument_id);
typedef ms_Result (* ms_MuseSampler_clear_track)(ms_MuseSampler ms, ms_Track track);

typedef ms_Result (* ms_MuseSampler_add_dynamics_event)(ms_MuseSampler ms, ms_DynamicsEvent evt);
typedef ms_Result (* ms_MuseSampler_add_track_event)(ms_MuseSampler ms, ms_Track track, ms_Event evt);
typedef ms_Result (* ms_MuseSampler_process)(ms_MuseSampler, ms_OutputBuffer, long long micros);

#endif // MU_MUSESAMPLER_APITYPES_H
