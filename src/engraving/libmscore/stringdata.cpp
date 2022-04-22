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

#include "stringdata.h"

#include <map>

#include "rw/xml.h"

#include "chord.h"
#include "note.h"
#include "part.h"
#include "score.h"
#include "staff.h"
#include "undo.h"
#include "segment.h"

using namespace mu;

namespace Ms {
//---------------------------------------------------------
//   StringData
//---------------------------------------------------------

bool StringData::bFretting = false;

StringData::StringData(int numFrets, int numStrings, int strings[])
{
    instrString strg = { 0, false, 0 };
    _frets = numFrets;

    for (int i = 0; i < numStrings; i++) {
        strg.pitch = strings[i];
        stringTable.push_back(strg);
    }
}

StringData::StringData(int numFrets, std::vector<instrString>& strings)
{
    _frets = numFrets;

    stringTable.clear();
    for (const instrString& i : strings) {
        stringTable.push_back(i);
    }
}

// called from import (musicxml/guitarpro/...)
void StringData::set(const StringData& src)
{
    *this = src;
    if (isFiveStringBanjo()) {
        configBanjo5thString();
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StringData::read(XmlReader& e)
{
    stringTable.clear();
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "frets") {
            _frets = e.readInt();
        } else if (tag == "string") {
            instrString strg;
            strg.open  = e.intAttribute("open", 0);
            strg.pitch = e.readInt();
            stringTable.push_back(strg);
        } else {
            e.unknown();
        }
    }
    if (isFiveStringBanjo()) {
        configBanjo5thString();
    }
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StringData::write(XmlWriter& xml) const
{
    xml.startObject("StringData");
    xml.tag("frets", _frets);
    foreach (instrString strg, stringTable) {
        if (strg.open) {
            xml.tag("string open=\"1\"", strg.pitch);
        } else {
            xml.tag("string", strg.pitch);
        }
    }
    xml.endObject();
}

//---------------------------------------------------------
//   convertPitch
//   Finds string and fret for a note.
//
//   Fills *string and *fret with suitable values for pitch at given tick of given staff,
//   using the highest possible string.
//   If note cannot be fretted, uses fret 0 on nearest string and returns false
//
//    Note: Strings are stored internally from lowest (0) to highest (strings()-1),
//          but the returned *string value references strings in reversed, 'visual', order:
//          from highest (0) to lowest (strings()-1)
//---------------------------------------------------------

bool StringData::convertPitch(int pitch, Staff* staff, int* string, int* fret) const
{
    return convertPitch(pitch, pitchOffsetAt(staff), string, fret);
}

//---------------------------------------------------------
//   getPitch
//    Returns the pitch corresponding to the string / fret combination
//    at given tick of given staff.
//    Returns INVALID_PITCH if not possible
//    Note: frets above max fret are accepted.
//---------------------------------------------------------

int StringData::getPitch(int string, int fret, Staff* staff) const
{
    return getPitch(string, fret, pitchOffsetAt(staff));
}

//---------------------------------------------------------
//   fret
//    Returns the fret corresponding to the pitch / string combination
//    at given tick of given staff.
//    Returns INVALID_FRET_INDEX if not possible
//---------------------------------------------------------

int StringData::fret(int pitch, int string, Staff* staff) const
{
    return fret(pitch, string, pitchOffsetAt(staff));
}

//---------------------------------------------------------
//   fretChords
//    Assigns fretting to all the notes of each chord in the same segment of chord
//    re-using existing fretting wherever possible
//
//    Minimizes fret conflicts (multiple notes on the same string)
//    but marks as fretConflict notes which cannot be fretted
//    (outside tablature range) or which cannot be assigned
//    a separate string
//---------------------------------------------------------

void StringData::fretChords(Chord* chord) const
{
    int nFret, minFret, maxFret, nNewFret, nTempFret;
    int nString, nNewString, nTempString;

    if (bFretting) {
        return;
    }
    bFretting = true;

    // we need to keep track of string allocation
#if (!defined (_MSCVER) && !defined (_MSC_VER))
    int bUsed[strings()];                      // initially all strings are available
#else
    // MSVC does not support VLA. Replace with std::vector. If profiling determines that the
    //    heap allocation is slow, an optimization might be used.
    std::vector<int> bUsed(strings());
#endif
    for (nString = 0; nString < static_cast<int>(strings()); nString++) {
        bUsed[nString] = 0;
    }
    // we also need the notes sorted in order of string (from highest to lowest) and then pitch
    std::map<int, Note*> sortedNotes;
    int count = 0;
    // store staff pitch offset at this tick, to speed up actual note pitch calculations
    int transp = chord->staff() ? chord->part()->instrument(chord->tick())->transpose().chromatic : 0;
    int pitchOffset = -transp + chord->staff()->pitchOffset(chord->segment()->tick());
    // if chord parent is not a segment, the chord is special (usually a grace chord):
    // fret it by itself, ignoring the segment
    if (chord->explicitParent()->type() != ElementType::SEGMENT) {
        sortChordNotes(sortedNotes, chord, pitchOffset, &count);
    } else {
        // scan each chord of seg from same staff as 'chord', inserting each of its notes in sortedNotes
        Segment* seg = chord->segment();
        track_idx_t trk;
        track_idx_t trkFrom = (chord->track() / VOICES) * VOICES;
        track_idx_t trkTo   = trkFrom + VOICES;
        for (trk = trkFrom; trk < trkTo; ++trk) {
            EngravingItem* ch = seg->elist().at(trk);
            if (ch && ch->type() == ElementType::CHORD) {
                sortChordNotes(sortedNotes, toChord(ch), pitchOffset, &count);
            }
        }
    }
    // determine used range of frets
    minFret = INT32_MAX;
    maxFret = INT32_MIN;
    for (auto& p : sortedNotes) {
        Note* note = p.second;
        if (note->string() != INVALID_STRING_INDEX) {
            bUsed[note->string()]++;
        }
        if (note->fret() != INVALID_FRET_INDEX && note->fret() < minFret) {
            minFret = note->fret();
        }
        if (note->fret() != INVALID_FRET_INDEX && note->fret() > maxFret) {
            maxFret = note->fret();
        }
    }

    // scan chord notes from highest, matching with strings from the highest
    for (auto& p : sortedNotes) {
        Note* note = p.second;
        nString     = nNewString    = note->string();
        nFret       = nNewFret      = note->fret();
        note->setFretConflict(false);           // assume no conflicts on this note
        // if no fretting (any invalid fretting has been erased by sortChordNotes() )
        if (nString == INVALID_STRING_INDEX /*|| nFret == INVALID_FRET_INDEX || getPitch(nString, nFret) != note->pitch()*/) {
            // get a new fretting
            if (!convertPitch(note->pitch(), pitchOffset, &nNewString, &nNewFret)) {
                // no way to fit this note in this tab:
                // mark as fretting conflict
                note->setFretConflict(true);
                // store fretting change without affecting chord context
                if (nFret != nNewFret) {
                    note->undoChangeProperty(Pid::FRET, nNewFret);
                }
                if (nString != nNewString) {
                    note->undoChangeProperty(Pid::STRING, nNewString);
                }
                continue;
            }
            // note can be fretted: use string
            else {
                bUsed[nNewString]++;
            }
        }

        // if the note string (either original or newly assigned) is also used by another note
        if (bUsed[nNewString] > 1) {
            // attempt to find a suitable string, from topmost
            for (nTempString=0; nTempString < static_cast<int>(strings()); nTempString++) {
                if (bUsed[nTempString] < 1
                    && (nTempFret=fret(note->pitch(), nTempString, pitchOffset)) != INVALID_FRET_INDEX) {
                    bUsed[nNewString]--;              // free previous string
                    bUsed[nTempString]++;             // and occupy new string
                    nNewFret   = nTempFret;
                    nNewString = nTempString;
                    break;
                }
            }
        }

        // TODO : try to optimize used fret range, avoiding eccessively open positions

        // if fretting did change, store as a fret change
        if (nFret != nNewFret) {
            note->undoChangeProperty(Pid::FRET, nNewFret);
        }
        if (nString != nNewString) {
            note->undoChangeProperty(Pid::STRING, nNewString);
        }
    }

    // check for any remaining fret conflict
    for (auto& p : sortedNotes) {
        Note* note = p.second;
        if (note->string() == -1 || bUsed[note->string()] > 1) {
            note->setFretConflict(true);
        }
    }

    bFretting = false;
}

//---------------------------------------------------------
//   frettedStrings
//    Returns the number of fretted strings.
//---------------------------------------------------------

int StringData::frettedStrings() const
{
    int num = 0;
    for (auto s : stringTable) {
        if (!s.open) {
            num++;
        }
    }
    return num;
}

//********************
// STATIC METHODS
//********************

//---------------------------------------------------------
//   pitchOffsetAt
//   Computes the pitch offset relevant for string data calculation at the given point.
//
//   For string data calculations, pitch offset may depend on transposition, capos and, possibly, ottavas.
//---------------------------------------------------------

int StringData::pitchOffsetAt(Staff* staff)
{
    return -(staff ? staff->part()->instrument()->transpose().chromatic : 0);
}

//********************
// PRIVATE METHODS
//********************

//---------------------------------------------------------
//   convertPitch
//   Finds string and fret for a note.
//
//   Fills *string and *fret with suitable values for pitch / pitchOffset,
//   using the highest possible string.
//   If note cannot be fretted, uses fret 0 on nearest string and returns false
//
//    Note: Strings are stored internally from lowest (0) to highest (strings()-1),
//          but the returned *string value references strings in reversed, 'visual', order:
//          from highest (0) to lowest (strings()-1)
//---------------------------------------------------------

bool StringData::convertPitch(int pitch, int pitchOffset, int* string, int* fret) const
{
    int strings = static_cast<int>(stringTable.size());
    if (strings < 1) {
        return false;
    }

    pitch += pitchOffset;

    // if above max fret on highest string, fret on first string, but return failure
    if (pitch > stringTable.at(strings - 1).pitch + _frets) {
        *string = 0;
        *fret   = 0;
        return false;
    }

    if (isFiveStringBanjo()) {
        // special case: open banjo 5th string
        if (pitch == stringTable.at(0).pitch) {
            *string = 4;
            *fret = 0;
            return true;
        }
        // test remaining 4 strings from highest to lowest
        for (int i = 4; i > 0; i--) {
            instrString strg = stringTable.at(i);
            if (pitch >= strg.pitch) {
                *string = strings - i - 1;
                *fret = pitch - strg.pitch;
                return true;
            }
        }
    } else {
        // look for a suitable string, starting from the highest
        // NOTE: this assumes there are always enough frets to fill
        // the interval between any fretted string and the next
        for (int i = strings - 1; i >= 0; i--) {
            instrString strg = stringTable.at(i);
            if (pitch >= strg.pitch) {
                if (pitch == strg.pitch || !strg.open) {
                    *string = strings - i - 1;
                }
                *fret = pitch - strg.pitch;
                return true;
            }
        }
    }

    // if no string found, pitch is below lowest string:
    // fret on last string, but return failure
    *string = strings - 1;
    *fret   = 0;
    return false;
}

//---------------------------------------------------------
//   getPitch
//    Returns the pitch corresponding to the string / fret / pitchOffset combination.
//    Returns INVALID_PITCH if not possible
//    Note: frets above max fret are accepted.
//---------------------------------------------------------

int StringData::getPitch(int string, int fret, int pitchOffset) const
{
    size_t strings = stringTable.size();
    if (string < 0 || string >= strings) {
        return INVALID_PITCH;
    }
    instrString strg = stringTable.at(strings - string - 1);
    int pitch = strg.pitch - pitchOffset + (strg.open ? 0 : fret);
    if (strg.startFret > 0 && fret >= strg.startFret) {
        pitch -= strg.startFret; // banjo 5th string adjustment
    }
    return pitch;
}

//---------------------------------------------------------
//   fret
//    Returns the fret corresponding to the pitch / string / pitchOffset combination.
//    returns INVALID_FRET_INDEX if not possible
//---------------------------------------------------------

int StringData::fret(int pitch, int string, int pitchOffset) const
{
    size_t strings = stringTable.size();
    if (strings < 1) {                          // no strings at all!
        return INVALID_FRET_INDEX;
    }

    if (string < 0 || string >= strings) {      // no such a string
        return INVALID_FRET_INDEX;
    }

    pitch += pitchOffset;

    const instrString& strg = stringTable[strings - string - 1];
    int fret = pitch - strg.pitch;
    if (fret > 0 && strg.startFret > 0) {
        fret += strg.startFret;  // banjo 5th string adjustment
    }

    // fret number is invalid or string cannot be fretted
    if (fret < 0 || fret >= _frets || (fret > 0 && strg.open)) {
        return INVALID_FRET_INDEX;
    }
    return fret;
}

//---------------------------------------------------------
//   sortChordNotes
//    Adds to sortedNotes the notes of Chord in string/pitch order
//    Note: notes are sorted first by string (top string being 0),
//          then by negated pitch (higher pitches resulting in lower key),
//          then by order of submission to disambiguate notes with the same pitch.
//          Everything else being equal, this makes notes in higher-numbered voices
//          to be sorted after notes in lower-numbered voices (voice 2 after voice 1 and so on)
//    Notes without a string assigned yet, are sorted according to the lowest string which can accommodate them.
//---------------------------------------------------------

void StringData::sortChordNotes(std::map<int, Note*>& sortedNotes, const Chord* chord, int pitchOffset, int* count) const
{
    int capoFret = chord->staff()->part()->capoFret();

    for (Note* note : chord->notes()) {
        int string = note->string();
        int fret = note->fret();

        // if note not fretted yet or current fretting no longer valid,
        // use most convenient string as key
        if (string <= INVALID_STRING_INDEX || fret <= INVALID_FRET_INDEX
            || getPitch(string, fret + capoFret, pitchOffset) != note->pitch()) {
            note->setString(INVALID_STRING_INDEX);
            note->setFret(INVALID_FRET_INDEX);
            convertPitch(note->pitch(), pitchOffset, &string, &fret);
        }
        int key = string * 100000;
        key += -(note->pitch() + pitchOffset) * 100 + *count;       // disambiguate notes of equal pitch
        sortedNotes.insert({ key, note });
        (*count)++;
    }
}

//---------------------------------------------------------
//   configBanjo5thString
//   Assumes isFiveStringBanjo() has already been called.
//   This method looks at the banjo tuning and sets startFret
//   appropriately.
//---------------------------------------------------------

void StringData::configBanjo5thString()
{
    // banjo 5th string (pitch 67 == G)
    instrString& strg5 = stringTable[0];

    _frets = 24; // not needed after bug #316931 is fixed

    // adjust startFret if using a 5th string capo (6..12)
    if (strg5.pitch > 67 && strg5.pitch < 74) {
        strg5.startFret = strg5.pitch - 62;
    } else {
        strg5.startFret = 5;  // no 5th string capo
    }
}

//---------------------------------------------------------
//   adjustBanjo5thFret
//   Convert 5th string fret number from (0, 1, 2, 3...) to (0, 6, 7, 8...).
//   Called from import (GuitarPro mostly)
//   Returns adjusted fret number
//---------------------------------------------------------

int StringData::adjustBanjo5thFret(int fret) const
{
    return (fret > 0 && isFiveStringBanjo()) ? fret + stringTable[0].startFret : fret;
}

//---------------------------------------------------------
//    isFiveStringBanjo
//    Based only on number of strings and tuning - other info
//    may not be available when this is called. Checks 5th string
//    pitch is higher than 4th (i.e. not a 5 string bass)
//---------------------------------------------------------

bool StringData::isFiveStringBanjo() const
{
    return stringTable.size() == 5 && stringTable[0].pitch > stringTable[1].pitch;
}
}
