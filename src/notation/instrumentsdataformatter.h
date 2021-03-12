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

#ifndef MU_INSTRUMENTS_INSTRUMENTSDATAFORAMTTER_H
#define MU_INSTRUMENTS_INSTRUMENTSDATAFORAMTTER_H

#include "instruments/instrumentstypes.h"

namespace Ms {
class Instrument;
struct NamedEventList;
}

namespace mu::instruments {
class InstrumentsDataFormatter
{
public:
    static Ms::Instrument convertInstrument(const Instrument& instrument);
    static Instrument convertInstrument(const Ms::Instrument& insturment);

    static QString buildInstrumentName(const QString& format, const QString& instrumentName, const QString& transpositionName, int instrumentNumber);

private:
    static MidiActionList convertMidiActions(const QList<Ms::NamedEventList>& midiActions);
    static QList<Ms::NamedEventList> convertMidiActions(const MidiActionList& midiActions);

    static bool needUseDefaultNameFormat(const QString& format);
    static QString buildDefaultInstrumentName(const QString& instrumentName, const QString& transpositionName, int instrumentNumber);
};
}

#endif // MU_INSTRUMENTS_INSTRUMENTSDATAFORAMTTER_H

