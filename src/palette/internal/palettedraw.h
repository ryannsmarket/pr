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
#ifndef MU_PALETTE_PALETTEDRAW_H
#define MU_PALETTE_PALETTEDRAW_H

#include "libmscore/engravingitem.h"

namespace mu::engraving {
class MStyle;
class IEngravingFont;
class Score;

class Accidental;
class ActionIcon;
class Ambitus;
class Arpeggio;
class Articulation;

class BagpipeEmbellishment;
class BarLine;
class Bend;
class Bracket;
class Breath;

class Capo;
class Chord;
class ChordLine;
class Clef;

class Dynamic;

class Expression;

class Fermata;
class Fingering;
class FretDiagram;
class FSymbol;

class Glissando;
class GlissandoSegment;
class GradualTempoChange;
class GradualTempoChangeSegment;

class Hairpin;
class HairpinSegment;
class HarpPedalDiagram;

class InstrumentChange;

class Jump;

class KeySig;

class LayoutBreak;
class LetRing;
class LetRingSegment;
class SLine;
class LineSegment;

class Marker;
class MeasureNumber;
class MeasureRepeat;

class NoteHead;

class Ornament;
class Ottava;
class OttavaSegment;

class PalmMute;
class PalmMuteSegment;
class Pedal;
class PedalSegment;
class PlayTechAnnotation;

class RehearsalMark;

class Slur;
class Spacer;
class StaffText;
class StaffTypeChange;
class Symbol;
class SystemText;

class TempoText;
class Text;
class TextBase;
class TextLine;
class TextLineSegment;
class TextLineBaseSegment;
class TimeSig;
class Tremolo;
class TremoloBar;
class Trill;
class TrillSegment;

class Vibrato;
class VibratoSegment;
class Volta;
class VoltaSegment;
}

namespace mu::palette {
class PaletteDraw
{
public:

    static void drawItem(const engraving::EngravingItem* item, draw::Painter* p);      // factory

private:
    static void draw(const engraving::Accidental* item, draw::Painter* p);
};
}

#endif // MU_PALETTE_PALETTEDRAW_H
