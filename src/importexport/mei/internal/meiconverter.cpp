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

#include "meiconverter.h"

#include "libmscore/accidental.h"
#include "libmscore/breath.h"
#include "libmscore/dynamic.h"
#include "libmscore/jump.h"
#include "libmscore/fermata.h"
#include "libmscore/harmony.h"
#include "libmscore/marker.h"
#include "libmscore/measure.h"
#include "libmscore/note.h"
#include "libmscore/part.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/staff.h"
#include "libmscore/stafftype.h"
#include "libmscore/utils.h"
#include "libmscore/tempotext.h"
#include "libmscore/text.h"
#include "libmscore/textbase.h"
#include "libmscore/tie.h"
#include "libmscore/tuplet.h"
#include "engraving/types/typesconv.h"
#include "types/symnames.h"

#include <iostream>
#include <string>
#include <sstream>
#include <valarray>

using namespace mu::iex::mei;
using namespace mu;

StringList Convert::logs;

engraving::ElementType Convert::elementTypeForDir(const libmei::Element& meiElement)
{
    engraving::ElementType dirType = engraving::ElementType::EXPRESSION;
    const libmei::AttTyped* typedAtt = dynamic_cast<const libmei::AttTyped*>(&meiElement);
    if (typedAtt) {
        if (Convert::hasTypeValue(typedAtt->GetType(), std::string(DIR_TYPE) + "playtech-annotation")) {
            dirType = engraving::ElementType::PLAYTECH_ANNOTATION;
        } else if (Convert::hasTypeValue(typedAtt->GetType(), std::string(DIR_TYPE) + "staff-text")) {
            dirType = engraving::ElementType::STAFF_TEXT;
        }
    }
    return dirType;
}

engraving::ElementType Convert::elementTypeFor(const libmei::RepeatMark& meiRepeatMark)
{
    switch (meiRepeatMark.GetFunc()) {
    case (libmei::repeatMarkLog_FUNC_daCapo):
    case (libmei::repeatMarkLog_FUNC_dalSegno):
        return engraving::ElementType::JUMP;
    default:
        return engraving::ElementType::MARKER;
    }
}

engraving::AccidentalType Convert::accidFromMEI(const libmei::data_ACCIDENTAL_WRITTEN meiAccid, bool& warning)
{
    warning = false;
    switch (meiAccid) {
    case (libmei::ACCIDENTAL_WRITTEN_NONE): return engraving::AccidentalType::NONE;
    case (libmei::ACCIDENTAL_WRITTEN_n): return engraving::AccidentalType::NATURAL;
    case (libmei::ACCIDENTAL_WRITTEN_f): return engraving::AccidentalType::FLAT;
    case (libmei::ACCIDENTAL_WRITTEN_ff): return engraving::AccidentalType::FLAT2;
    case (libmei::ACCIDENTAL_WRITTEN_tf): return engraving::AccidentalType::FLAT3;
    case (libmei::ACCIDENTAL_WRITTEN_s): return engraving::AccidentalType::SHARP;
    case (libmei::ACCIDENTAL_WRITTEN_x): return engraving::AccidentalType::SHARP2;
    case (libmei::ACCIDENTAL_WRITTEN_ts): return engraving::AccidentalType::SHARP3;
    case (libmei::ACCIDENTAL_WRITTEN_nf): return engraving::AccidentalType::NATURAL_FLAT;
    case (libmei::ACCIDENTAL_WRITTEN_ns): return engraving::AccidentalType::NATURAL_SHARP;
    case (libmei::ACCIDENTAL_WRITTEN_ss): return engraving::AccidentalType::SHARP_SHARP;
    default:
        warning = true;
        return engraving::AccidentalType::NATURAL;
    }
}

libmei::data_ACCIDENTAL_WRITTEN Convert::accidToMEI(const engraving::AccidentalType accid)
{
    switch (accid) {
    case (engraving::AccidentalType::NONE): return libmei::ACCIDENTAL_WRITTEN_NONE;
    case (engraving::AccidentalType::NATURAL): return libmei::ACCIDENTAL_WRITTEN_n;
    case (engraving::AccidentalType::FLAT): return libmei::ACCIDENTAL_WRITTEN_f;
    case (engraving::AccidentalType::FLAT2): return libmei::ACCIDENTAL_WRITTEN_ff;
    case (engraving::AccidentalType::FLAT3): return libmei::ACCIDENTAL_WRITTEN_tf;
    case (engraving::AccidentalType::SHARP): return libmei::ACCIDENTAL_WRITTEN_s;
    case (engraving::AccidentalType::SHARP2): return libmei::ACCIDENTAL_WRITTEN_x;
    case (engraving::AccidentalType::SHARP3): return libmei::ACCIDENTAL_WRITTEN_ts;
    case (engraving::AccidentalType::NATURAL_FLAT): return libmei::ACCIDENTAL_WRITTEN_nf;
    case (engraving::AccidentalType::NATURAL_SHARP): return libmei::ACCIDENTAL_WRITTEN_ns;
    case (engraving::AccidentalType::SHARP_SHARP): return libmei::ACCIDENTAL_WRITTEN_ss;
    default:
        return libmei::ACCIDENTAL_WRITTEN_n;
    }
}

engraving::AccidentalVal Convert::accidGesFromMEI(const libmei::data_ACCIDENTAL_GESTURAL meiAccid, bool& warning)
{
    warning = false;
    switch (meiAccid) {
    case (libmei::ACCIDENTAL_GESTURAL_NONE): return engraving::AccidentalVal::NATURAL;
    case (libmei::ACCIDENTAL_GESTURAL_n): return engraving::AccidentalVal::NATURAL;
    case (libmei::ACCIDENTAL_GESTURAL_f): return engraving::AccidentalVal::FLAT;
    case (libmei::ACCIDENTAL_GESTURAL_ff): return engraving::AccidentalVal::FLAT2;
    case (libmei::ACCIDENTAL_GESTURAL_tf): return engraving::AccidentalVal::FLAT3;
    case (libmei::ACCIDENTAL_GESTURAL_s): return engraving::AccidentalVal::SHARP;
    case (libmei::ACCIDENTAL_GESTURAL_ss): return engraving::AccidentalVal::SHARP2;
    case (libmei::ACCIDENTAL_GESTURAL_ts): return engraving::AccidentalVal::SHARP3;
    default:
        warning = true;
        return engraving::AccidentalVal::NATURAL;
    }
}

libmei::data_ACCIDENTAL_GESTURAL Convert::accidGesToMEI(const engraving::AccidentalVal accid)
{
    switch (accid) {
    case (engraving::AccidentalVal::NATURAL): return libmei::ACCIDENTAL_GESTURAL_n;
    case (engraving::AccidentalVal::FLAT): return libmei::ACCIDENTAL_GESTURAL_f;
    case (engraving::AccidentalVal::FLAT2): return libmei::ACCIDENTAL_GESTURAL_ff;
    case (engraving::AccidentalVal::FLAT3): return libmei::ACCIDENTAL_GESTURAL_tf;
    case (engraving::AccidentalVal::SHARP): return libmei::ACCIDENTAL_GESTURAL_s;
    case (engraving::AccidentalVal::SHARP2): return libmei::ACCIDENTAL_GESTURAL_ss;
    case (engraving::AccidentalVal::SHARP3): return libmei::ACCIDENTAL_GESTURAL_ts;
    default:
        return libmei::ACCIDENTAL_GESTURAL_n;
    }
}

engraving::BarLineType Convert::barlineFromMEI(const libmei::data_BARRENDITION meiBarline, bool& warning)
{
    warning = false;
    switch (meiBarline) {
    case (libmei::BARRENDITION_single): return engraving::BarLineType::NORMAL;
    case (libmei::BARRENDITION_dbl): return engraving::BarLineType::DOUBLE;
    case (libmei::BARRENDITION_rptstart): return engraving::BarLineType::START_REPEAT;
    case (libmei::BARRENDITION_rptend): return engraving::BarLineType::END_REPEAT;
    case (libmei::BARRENDITION_dashed): return engraving::BarLineType::BROKEN;
    case (libmei::BARRENDITION_end): return engraving::BarLineType::END;
    case (libmei::BARRENDITION_rptboth): return engraving::BarLineType::END_START_REPEAT;
    case (libmei::BARRENDITION_dotted): return engraving::BarLineType::DOTTED;
    case (libmei::BARRENDITION_heavy): return engraving::BarLineType::HEAVY;
    case (libmei::BARRENDITION_dblheavy): return engraving::BarLineType::DOUBLE_HEAVY;
    case (libmei::BARRENDITION_dbldashed):
    case (libmei::BARRENDITION_dbldotted):
        warning = true;
        return engraving::BarLineType::DOUBLE;
    case (libmei::BARRENDITION_dblsegno):
        warning = true;
        return engraving::BarLineType::DOUBLE;
    case (libmei::BARRENDITION_segno):
    case (libmei::BARRENDITION_invis):
        warning = true;
        return engraving::BarLineType::SINGLE;
    default:
        return engraving::BarLineType::SINGLE;
    }
}

libmei::data_BARRENDITION Convert::barlineToMEI(const engraving::BarLineType barline)
{
    switch (barline) {
    case (engraving::BarLineType::NORMAL): return libmei::BARRENDITION_NONE;
    case (engraving::BarLineType::DOUBLE): return libmei::BARRENDITION_dbl;
    case (engraving::BarLineType::START_REPEAT): return libmei::BARRENDITION_rptstart;
    case (engraving::BarLineType::END_REPEAT): return libmei::BARRENDITION_rptend;
    case (engraving::BarLineType::BROKEN): return libmei::BARRENDITION_dashed;
    case (engraving::BarLineType::END): return libmei::BARRENDITION_end;
    case (engraving::BarLineType::END_START_REPEAT):
        return libmei::BARRENDITION_rptboth;
    case (engraving::BarLineType::DOTTED): return libmei::BARRENDITION_dotted;
    case (engraving::BarLineType::HEAVY): return libmei::BARRENDITION_heavy;
    case (engraving::BarLineType::DOUBLE_HEAVY): return libmei::BARRENDITION_dblheavy;
    case (engraving::BarLineType::REVERSE_END):
        LOGD() << "Unsupported engraving::BarLineType::REVERSE_END";
        return libmei::BARRENDITION_dbl;
    default:
        return libmei::BARRENDITION_single;
    }
}

engraving::BeamMode Convert::beamFromMEI(const std::string& typeAtt, const std::string& prefix, bool& warning)
{
    warning = false;

    engraving::BeamMode beamMode = engraving::BeamMode::AUTO;
    if (Convert::hasTypeValue(typeAtt, prefix + "-begin")) {
        beamMode = engraving::BeamMode::BEGIN;
    } else if (Convert::hasTypeValue(typeAtt, prefix + "-mid")) {
        beamMode = engraving::BeamMode::MID;
    } else if (Convert::hasTypeValue(typeAtt, prefix + "-none")) {
        beamMode = engraving::BeamMode::NONE;
    }

    return beamMode;
}

std::string Convert::beamToMEI(engraving::BeamMode beamMode, const std::string& prefix)
{
    std::string beamType;

    switch (beamMode) {
    case (engraving::BeamMode::BEGIN):
        beamType = prefix + "-begin";
        break;
    case (engraving::BeamMode::MID):
        beamType = prefix + "-mid";
        break;
    case (engraving::BeamMode::NONE):
        beamType = prefix + "-none";
        break;
    default: break;
    }

    return beamType;
}

engraving::BeamMode Convert::breaksecFromMEI(int breaksec, bool& warning)
{
    warning = false;

    switch (breaksec) {
    case (1): return engraving::BeamMode::BEGIN16;
    case (2): return engraving::BeamMode::BEGIN32;
    default:
        warning = true;
        break;
    }

    return engraving::BeamMode::AUTO;
}

int Convert::breaksecToMEI(engraving::BeamMode beamMode)
{
    int breaksec = MEI_UNSET;

    switch (beamMode) {
    case (engraving::BeamMode::BEGIN16): breaksec = 1;
        break;
    case (engraving::BeamMode::BEGIN32): breaksec = 2;
        break;
    default: break;
    }

    return breaksec;
}

engraving::ClefType Convert::clefFromMEI(const libmei::Clef& meiClef, bool& warning)
{
    warning = false;
    if (meiClef.GetShape() == libmei::CLEFSHAPE_G) {
        if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_below) {
            switch (meiClef.GetDis()) {
            case (libmei::OCTAVE_DIS_8): return engraving::ClefType::G8_VB;
            case (libmei::OCTAVE_DIS_15): return engraving::ClefType::G15_MB;
            default:
                break;
            }
        } else if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_above) {
            switch (meiClef.GetDis()) {
            case (libmei::OCTAVE_DIS_8): return engraving::ClefType::G8_VA;
            case (libmei::OCTAVE_DIS_15): return engraving::ClefType::G15_MA;
            default:
                break;
            }
        }
        if (meiClef.GetLine() == 2) {
            return engraving::ClefType::G;
        } else if (meiClef.GetLine() == 1) {
            return engraving::ClefType::G_1;
        }
    } else if (meiClef.GetShape() == libmei::CLEFSHAPE_C) {
        switch (meiClef.GetLine()) {
        case (1): return engraving::ClefType::C1;
        case (2): return engraving::ClefType::C2;
        case (3): return engraving::ClefType::C3;
        case (4): return engraving::ClefType::C4;
        case (5): return engraving::ClefType::C5;
        default:
            break;
        }
    } else if (meiClef.GetShape() == libmei::CLEFSHAPE_F) {
        if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_below) {
            switch (meiClef.GetDis()) {
            case (libmei::OCTAVE_DIS_8): return engraving::ClefType::F8_VB;
            case (libmei::OCTAVE_DIS_15): return engraving::ClefType::F15_MB;
            default:
                break;
            }
        } else if (meiClef.GetDisPlace() == libmei::STAFFREL_basic_above) {
            switch (meiClef.GetDis()) {
            case (libmei::OCTAVE_DIS_8): return engraving::ClefType::F_8VA;
            case (libmei::OCTAVE_DIS_15): return engraving::ClefType::F_15MA;
            default:
                break;
            }
        }
        switch (meiClef.GetLine()) {
        case (3): return engraving::ClefType::F_B;
        case (4): return engraving::ClefType::F;
        case (5): return engraving::ClefType::F_C;
        default:
            break;
        }
    } else if (meiClef.GetShape() == libmei::CLEFSHAPE_GG && meiClef.GetLine() == 2) {
        return engraving::ClefType::G8_VB_O;
    }
    warning = true;
    return engraving::ClefType::G;
}

Convert::BracketStruct Convert::bracketFromMEI(const libmei::StaffGrp& meiStaffGrp)
{
    Convert::BracketStruct bracketSt;

    switch (meiStaffGrp.GetSymbol()) {
    case (libmei::staffGroupingSym_SYMBOL_bracket): bracketSt.bracketType = engraving::BracketType::NORMAL;
        break;
    case (libmei::staffGroupingSym_SYMBOL_brace): bracketSt.bracketType = engraving::BracketType::BRACE;
        break;
    case (libmei::staffGroupingSym_SYMBOL_bracketsq): bracketSt.bracketType = engraving::BracketType::SQUARE;
        break;
    case (libmei::staffGroupingSym_SYMBOL_line): bracketSt.bracketType = engraving::BracketType::LINE;
        break;
    case (libmei::staffGroupingSym_SYMBOL_none): bracketSt.bracketType = engraving::BracketType::NO_BRACKET;
        break;
    default: break;
    }

    if (meiStaffGrp.HasBarThru() && meiStaffGrp.GetBarThru() == libmei::BOOLEAN_true) {
        bracketSt.barLineSpan = 1;
    }

    return bracketSt;
}

libmei::StaffGrp Convert::bracketToMEI(const engraving::BracketType bracket, int barLineSpan)
{
    libmei::StaffGrp meiStaffGrp;
    // @symbol
    switch (bracket) {
    case (engraving::BracketType::NORMAL): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_bracket);
        break;
    case (engraving::BracketType::BRACE): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_brace);
        break;
    case (engraving::BracketType::SQUARE): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_bracketsq);
        break;
    case (engraving::BracketType::LINE): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_line);
        break;
    case (engraving::BracketType::NO_BRACKET): meiStaffGrp.SetSymbol(libmei::staffGroupingSym_SYMBOL_none);
        break;
    default: break;
    }
    // @bar.thru
    if (barLineSpan > 0) {
        meiStaffGrp.SetBarThru(libmei::BOOLEAN_true);
    }

    return meiStaffGrp;
}

void Convert::breathFromMEI(engraving::Breath* breath, const libmei::Breath& meiBreath, bool& warning)
{
    warning = false;

    // @glyhp.name
    bool smufl = (meiBreath.HasGlyphAuth() && (meiBreath.GetGlyphAuth() == "smufl"));

    engraving::SymId symId = engraving::SymId::breathMarkComma;

    if (smufl && meiBreath.HasGlyphName()) {
        symId = engraving::SymNames::symIdByName(String(meiBreath.GetGlyphName().c_str()));
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiBreath.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiBreath.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }

    breath->setSymId(symId);
}

libmei::Breath Convert::breathToMEI(const engraving::Breath* breath)
{
    libmei::Breath meiBreath;

    bool smufl = false;
    // @glyph.name
    switch (breath->symId()) {
    case (engraving::SymId::breathMarkTick):
    case (engraving::SymId::breathMarkSalzedo):
    case (engraving::SymId::breathMarkUpbow): smufl = true;
    default:
        break;
    }
    // @glyph.name
    if (smufl) {
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(breath->symId());
        meiBreath.SetGlyphName(glyphName.ascii());
        meiBreath.SetGlyphAuth("smufl");
    }

    return meiBreath;
}

void Convert::caesuraFromMEI(engraving::Breath* breath, const libmei::Caesura& meiCeasura, bool& warning)
{
    warning = false;

    // @glyhp.name
    bool smufl = (meiCeasura.HasGlyphAuth() && (meiCeasura.GetGlyphAuth() == "smufl"));

    engraving::SymId symId = engraving::SymId::caesura;

    if (smufl && meiCeasura.HasGlyphName()) {
        symId = engraving::SymNames::symIdByName(String(meiCeasura.GetGlyphName().c_str()));
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiCeasura.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiCeasura.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    }

    breath->setSymId(symId);
}

libmei::Caesura Convert::caesuraToMEI(const engraving::Breath* breath)
{
    libmei::Caesura meiCaesura;

    bool smufl = false;
    // @glyph.name
    switch (breath->symId()) {
    case (engraving::SymId::caesuraCurved):
    case (engraving::SymId::caesuraShort):
    case (engraving::SymId::caesuraThick):
    case (engraving::SymId::chantCaesura):
    case (engraving::SymId::caesuraSingleStroke): smufl = true;
    default:
        break;
    }
    // @glyph.name
    if (smufl) {
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(breath->symId());
        meiCaesura.SetGlyphName(glyphName.ascii());
        meiCaesura.SetGlyphAuth("smufl");
    }

    return meiCaesura;
}

libmei::Clef Convert::clefToMEI(engraving::ClefType clef)
{
    libmei::Clef meiClef;
    // @shape
    switch (clef) {
    case (engraving::ClefType::G):
    case (engraving::ClefType::G15_MB):
    case (engraving::ClefType::G8_VB):
    case (engraving::ClefType::G8_VA):
    case (engraving::ClefType::G15_MA):
    case (engraving::ClefType::G_1):
        meiClef.SetLine(2);
        meiClef.SetShape(libmei::CLEFSHAPE_G);
        break;
    case (engraving::ClefType::C1):
    case (engraving::ClefType::C2):
    case (engraving::ClefType::C3):
    case (engraving::ClefType::C4):
    case (engraving::ClefType::C5):
        meiClef.SetLine(3);
        meiClef.SetShape(libmei::CLEFSHAPE_C);
        break;
    case (engraving::ClefType::F):
    case (engraving::ClefType::F15_MB):
    case (engraving::ClefType::F8_VB):
    case (engraving::ClefType::F_8VA):
    case (engraving::ClefType::F_15MA):
    case (engraving::ClefType::F_B):
    case (engraving::ClefType::F_C):
        meiClef.SetLine(4);
        meiClef.SetShape(libmei::CLEFSHAPE_F);
        break;
    case (engraving::ClefType::G8_VB_O):
        meiClef.SetLine(2);
        meiClef.SetShape(libmei::CLEFSHAPE_GG);
        break;
    default:
        LOGD() << "Unsupported clef shape";
        meiClef.SetShape(libmei::CLEFSHAPE_F);
    }

    // @line
    switch (clef) {
    // G
    case (engraving::ClefType::G_1): meiClef.SetLine(1);
        break;
    // C
    case (engraving::ClefType::C1): meiClef.SetLine(1);
        break;
    case (engraving::ClefType::C2): meiClef.SetLine(2);
        break;
    case (engraving::ClefType::C4): meiClef.SetLine(4);
        break;
    case (engraving::ClefType::C5): meiClef.SetLine(5);
        break;
    // F
    case (engraving::ClefType::F_B): meiClef.SetLine(3);
        break;
    case (engraving::ClefType::F_C): meiClef.SetLine(5);
        break;
    default:
        break;
    }

    // @dis and @dis.place
    switch (clef) {
    case (engraving::ClefType::G8_VB):
    case (engraving::ClefType::G8_VA):
    case (engraving::ClefType::F8_VB):
    case (engraving::ClefType::F_8VA):
        meiClef.SetDis(libmei::OCTAVE_DIS_8);
        break;
    case (engraving::ClefType::G15_MB):
    case (engraving::ClefType::G15_MA):
    case (engraving::ClefType::F15_MB):
    case (engraving::ClefType::F_15MA):
        meiClef.SetDis(libmei::OCTAVE_DIS_15);
        break;
    default:
        break;
    }

    // @dis.place
    switch (clef) {
    case (engraving::ClefType::G8_VB):
    case (engraving::ClefType::F8_VB):
    case (engraving::ClefType::G15_MB):
    case (engraving::ClefType::F15_MB):
        meiClef.SetDisPlace(libmei::STAFFREL_basic_below);
        break;
    case (engraving::ClefType::G8_VA):
    case (engraving::ClefType::F_8VA):
    case (engraving::ClefType::G15_MA):
    case (engraving::ClefType::F_15MA):
        meiClef.SetDisPlace(libmei::STAFFREL_basic_above);
        break;
    default:
        break;
    }

    return meiClef;
}

engraving::ClefType Convert::clefFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning)
{
    libmei::Clef meiClef;
    meiClef.SetLine(meiStaffDef.GetClefLine());
    meiClef.SetShape(meiStaffDef.GetClefShape());
    meiClef.SetDis(meiStaffDef.GetClefDis());
    meiClef.SetDisPlace(meiStaffDef.GetClefDisPlace());
    return Convert::clefFromMEI(meiClef, warning);
}

engraving::DirectionV Convert::curvedirFromMEI(const libmei::curvature_CURVEDIR meiCurvedir, bool& warning)
{
    warning = false;
    switch (meiCurvedir) {
    case (libmei::curvature_CURVEDIR_above): return engraving::DirectionV::UP;
    case (libmei::curvature_CURVEDIR_below): return engraving::DirectionV::DOWN;
    default:
        return engraving::DirectionV::AUTO;
    }
}

libmei::curvature_CURVEDIR Convert::curvedirToMEI(engraving::DirectionV curvedir)
{
    switch (curvedir) {
    case (engraving::DirectionV::UP): return libmei::curvature_CURVEDIR_above;
    case (engraving::DirectionV::DOWN): return libmei::curvature_CURVEDIR_below;
    default:
        return libmei::curvature_CURVEDIR_NONE;
    }
}

void Convert::dirFromMEI(engraving::TextBase* textBase, const StringList& meiLines, const libmei::Dir& meiDir, bool& warning)
{
    IF_ASSERT_FAILED(textBase) {
        return;
    }

    warning = false;

    // @place
    if (meiDir.HasPlace()) {
        textBase->setPlacement(meiDir.GetPlace()
                               == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        textBase->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @type
    // already process in Convert::elementTypeFor called for determining the factory to call in MeiImporter

    // text
    textBase->setXmlText(meiLines.join(u"\n"));

    return;
}

libmei::Dir Convert::dirToMEI(const engraving::TextBase* textBase, StringList& meiLines)
{
    libmei::Dir meiDir;

    // @place
    if (textBase->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiDir.SetPlace(Convert::placeToMEI(textBase->placement()));
    }

    // @type
    if (textBase->type() != engraving::ElementType::EXPRESSION) {
        std::string dirType = DIR_TYPE;
        switch (textBase->type()) {
        case (engraving::ElementType::PLAYTECH_ANNOTATION):
            dirType = std::string(HARMONY_TYPE) + "playtech-annotation";
            break;
        case (engraving::ElementType::STAFF_TEXT):
            dirType = std::string(HARMONY_TYPE) + "staff-text";
            break;
        default: break;
        }
        meiDir.SetType(dirType);
    }

    // text content - only split lines
    meiLines = String(textBase->plainText()).split(u"\n");

    return meiDir;
}

engraving::DurationType Convert::durFromMEI(const libmei::data_DURATION meiDuration, bool& warning)
{
    warning = false;
    switch (meiDuration) {
    case (libmei::DURATION_long): return engraving::DurationType::V_LONG;
    case (libmei::DURATION_breve): return engraving::DurationType::V_BREVE;
    case (libmei::DURATION_1): return engraving::DurationType::V_WHOLE;
    case (libmei::DURATION_2): return engraving::DurationType::V_HALF;
    case (libmei::DURATION_4): return engraving::DurationType::V_QUARTER;
    case (libmei::DURATION_8): return engraving::DurationType::V_EIGHTH;
    case (libmei::DURATION_16): return engraving::DurationType::V_16TH;
    case (libmei::DURATION_32): return engraving::DurationType::V_32ND;
    case (libmei::DURATION_64): return engraving::DurationType::V_64TH;
    case (libmei::DURATION_128): return engraving::DurationType::V_128TH;
    case (libmei::DURATION_256): return engraving::DurationType::V_256TH;
    case (libmei::DURATION_512): return engraving::DurationType::V_512TH;
    default:
        warning = true;
        return engraving::DurationType::V_QUARTER;
    }
}

libmei::data_DURATION Convert::durToMEI(const engraving::DurationType duration)
{
    switch (duration) {
    case (engraving::DurationType::V_LONG): return libmei::DURATION_long;
    case (engraving::DurationType::V_BREVE): return libmei::DURATION_breve;
    case (engraving::DurationType::V_WHOLE): return libmei::DURATION_1;
    case (engraving::DurationType::V_HALF): return libmei::DURATION_2;
    case (engraving::DurationType::V_QUARTER): return libmei::DURATION_4;
    case (engraving::DurationType::V_EIGHTH): return libmei::DURATION_8;
    case (engraving::DurationType::V_16TH): return libmei::DURATION_16;
    case (engraving::DurationType::V_32ND): return libmei::DURATION_32;
    case (engraving::DurationType::V_64TH): return libmei::DURATION_64;
    case (engraving::DurationType::V_128TH): return libmei::DURATION_128;
    case (engraving::DurationType::V_256TH): return libmei::DURATION_256;
    case (engraving::DurationType::V_512TH): return libmei::DURATION_512;
    default:
        return libmei::DURATION_4;
    }
}

void Convert::dynamFromMEI(engraving::Dynamic* dynamic, const StringList& meiLines, const libmei::Dynam& meiDynam, bool& warning)
{
    // The letters in the MEI to be mapped to SMuFL symbols in the xmlText
    static const std::map<Char, engraving::SymId> DYN_MAP = {
        { 'p', engraving::SymId::dynamicPiano },
        { 'm', engraving::SymId::dynamicMezzo },
        { 'f', engraving::SymId::dynamicForte },
        { 'r', engraving::SymId::dynamicRinforzando },
        { 's', engraving::SymId::dynamicSforzando },
        { 'z', engraving::SymId::dynamicZ },
        { 'n', engraving::SymId::dynamicNiente },
    };

    IF_ASSERT_FAILED(dynamic) {
        return;
    }

    warning = false;

    // @place
    if (meiDynam.HasPlace()) {
        dynamic->setPlacement(meiDynam.GetPlace() == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        dynamic->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @label
    if (meiDynam.HasLabel()) {
        dynamic->setDynamicType(String::fromStdString(meiDynam.GetLabel()));
    }
    // If no @label, try to use the text
    else if (meiLines.size() > 0 && !meiLines.at(0).contains(' ')) {
        dynamic->setDynamicType(meiLines.at(0));
    }

    // text content
    StringList lines;
    // For each line in the dynamic text
    for (const String& meiline : meiLines) {
        String line;
        StringList words = meiline.split(u" ");
        bool isFirst = true;
        // For each word in the line
        for (const String& word : words) {
            if (!isFirst) {
                line += u" ";
            }
            // If the word is only dynamic letters, convert them to SMuFL symbols one by one
            if (word.toStdString().find_first_not_of("fpmrszn") == std::string::npos) {
                for (size_t i = 0; i < word.size(); i++) {
                    line += u"<sym>" + String::fromAscii(engraving::SymNames::nameForSymId(DYN_MAP.at(word.at(i))).ascii()) + u"</sym>";
                }
            }
            // Otherwise keep it as is
            else {
                line += word;
            }
            isFirst = false;
        }
        lines.push_back(line);
    }
    dynamic->setXmlText(lines.join(u"\n"));

    return;
}

libmei::Dynam Convert::dynamToMEI(const engraving::Dynamic* dynamic, StringList& meiLines)
{
    // The SMuFL unicode points in the plainText to be mapped to letters in the MEI
    static const std::map<char16_t, Char> DYN_MAP = {
        { u'\uE520', 'p' },
        { u'\uE521', 'm' },
        { u'\uE522', 'f' },
        { u'\uE523', 'z' },
        { u'\uE524', 's' },
        { u'\uE525', 'z' },
        { u'\uE526', 'n' }
    };

    libmei::Dynam meiDynam;

    // @place
    if (dynamic->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiDynam.SetPlace(Convert::placeToMEI(dynamic->placement()));
    }

    // @label
    if (dynamic->dynamicType() != engraving::DynamicType::OTHER) {
        meiDynam.SetLabel(engraving::TConv::toXml(dynamic->dynamicType()).ascii());
    }

    // text content
    String meiText;
    String plainText = dynamic->plainText();
    for (size_t i = 0; i < plainText.size(); i++) {
        char16_t c = plainText.at(i).unicode();
        if (c < u'\uE000' || c > u'\uF8FF' || !DYN_MAP.count(c)) {
            meiText += c;
            continue;
        }
        meiText += DYN_MAP.at(c);

        /*
        char16_t c = plainText.at(i).unicode();
        if (c < u'\uE000' || c > u'\uF8FF') {
            meiText += c;
            continue;
        }
        SymId symId = m_score->engravingFont()->fromCode(c);
        if (symId == SymId::noSym) continue;
        String letter = u"<sym>" + String::fromAscii(SymNames::nameForSymId(symId).ascii()) + u"</sym>";
        for (auto dyn : Dynamic::dynamicList()) {
            if ((dyn.text == letter) && (dyn.type != DynamicType::OTHER)) {
                meiText += String::fromAscii(TConv::toXml(dyn.type).ascii());
                break;
            }
        }
        */
    }
    meiLines = String(meiText).split(u"\n");

    return meiDynam;
}

void Convert::endingFromMEI(engraving::Volta* volta, const libmei::Ending& meiEnding, bool& warning)
{
    IF_ASSERT_FAILED(volta) {
        return;
    }

    warning = false;
    // @type used for storing endings
    std::list<std::string> endings = Convert::getTypeValuesWithPrefix(meiEnding.GetType(), ENDING_REPEAT_TYPE);
    for (std::string ending : endings) {
        volta->endings().push_back(String(ending.c_str()).toInt());
    }

    // @label
    if (meiEnding.HasLabel()) {
        volta->setText(String(meiEnding.GetLabel().c_str()));
    }

    // @lform
    if (meiEnding.HasLform() && meiEnding.GetLform() != libmei::LINEFORM_solid) {
        bool lineWarning;
        engraving::LineType lineType = Convert::lineFromMEI(meiEnding.GetLform(), lineWarning);
        if (!lineWarning) {
            volta->setLineStyle(lineType);
            volta->setPropertyFlags(engraving::Pid::LINE_STYLE, engraving::PropertyFlags::UNSTYLED);
        }
        warning = (warning || lineWarning);
    }

    // @lendsym
    if (meiEnding.HasLendsym() && (meiEnding.GetLendsym() == libmei::LINESTARTENDSYMBOL_none)) {
        volta->setVoltaType(engraving::Volta::Type::OPEN);
        volta->setPropertyFlags(engraving::Pid::VOLTA_ENDING, engraving::PropertyFlags::UNSTYLED);
    } else {
        volta->setVoltaType(engraving::Volta::Type::CLOSED);
    }

    return;
}

libmei::Ending Convert::endingToMEI(const engraving::Volta* volta)
{
    libmei::Ending meiEnding;

    // @type used for storing endings
    StringList endings;
    for (int ending : volta->endings()) {
        endings << String("%1%2").arg(String(ENDING_REPEAT_TYPE)).arg(ending);
    }
    meiEnding.SetType(endings.join(u" ").toStdString());

    // @label used for text
    meiEnding.SetLabel(volta->text().toStdString());

    // @lform
    if (volta->lineStyle() != engraving::LineType::SOLID) {
        meiEnding.SetLform(Convert::lineToMEI(volta->lineStyle()));
    }

    // @lendsym
    if (volta->voltaType() == engraving::Volta::Type::OPEN) {
        meiEnding.SetLendsym(libmei::LINESTARTENDSYMBOL_none);
    }

    return meiEnding;
}

void Convert::fermataFromMEI(engraving::Fermata* fermata, const libmei::Fermata& meiFermata, bool& warning)
{
    warning = false;

    // @place
    bool below = (meiFermata.HasPlace() && (meiFermata.GetPlace() == libmei::STAFFREL_below));

    // @glyhp.name
    bool smufl = (meiFermata.HasGlyphAuth() && (meiFermata.GetGlyphAuth() == "smufl"));

    engraving::SymId symId = engraving::SymId::fermataAbove;

    if (smufl && meiFermata.HasGlyphName()) {
        symId = engraving::SymNames::symIdByName(String(meiFermata.GetGlyphName().c_str()));
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    } else if (meiFermata.HasShape()) {
        if (meiFermata.GetShape() == libmei::fermataVis_SHAPE_square) {
            symId = (below) ? engraving::SymId::fermataLongBelow : engraving::SymId::fermataLongAbove;
        } else if (meiFermata.GetShape() == libmei::fermataVis_SHAPE_angular) {
            symId = (below) ? engraving::SymId::fermataShortBelow : engraving::SymId::fermataShortAbove;
        } else if (meiFermata.GetShape() == libmei::fermataVis_SHAPE_curved) {
            symId = (below) ? engraving::SymId::fermataBelow : engraving::SymId::fermataAbove;
        }
    }
    // This is for loading MEI files not written by MuseScore and that use @glyph.num instead of @glyph.name
    else if (smufl && meiFermata.HasGlyphNum()) {
        symId = engravingFonts()->fallbackFont()->fromCode(meiFermata.GetGlyphNum());
        if (symId == engraving::SymId::noSym) {
            warning = true;
        }
    } else if (below) {
        symId = engraving::SymId::fermataBelow;
    }

    fermata->setSymId(symId);
    if (below) {
        fermata->setPlacement(engraving::PlacementV::BELOW);
    }
}

libmei::Fermata Convert::fermataToMEI(const engraving::Fermata* fermata)
{
    libmei::Fermata meiFermata;

    bool below = false;
    bool smufl = false;
    // @shape and @glyph.name
    switch (fermata->symId()) {
    case (engraving::SymId::fermataAbove): break;
    case (engraving::SymId::fermataBelow): below = true;
        break;
    case (engraving::SymId::fermataLongAbove): meiFermata.SetShape(libmei::fermataVis_SHAPE_square);
        break;
    case (engraving::SymId::fermataLongBelow): meiFermata.SetShape(libmei::fermataVis_SHAPE_square);
        below = true;
        break;
    case (engraving::SymId::fermataShortAbove): meiFermata.SetShape(libmei::fermataVis_SHAPE_angular);
        break;
    case (engraving::SymId::fermataShortBelow): meiFermata.SetShape(libmei::fermataVis_SHAPE_angular);
        below = true;
    case (engraving::SymId::fermataLongHenzeAbove):
    case (engraving::SymId::fermataVeryLongAbove):
    case (engraving::SymId::fermataShortHenzeAbove):
    case (engraving::SymId::fermataVeryShortAbove): smufl = true;
        break;
    case (engraving::SymId::fermataLongHenzeBelow):
    case (engraving::SymId::fermataVeryLongBelow):
    case (engraving::SymId::fermataShortHenzeBelow):
    case (engraving::SymId::fermataVeryShortBelow): smufl = true;
        below = true;
        break;
    default:
        break;
    }
    // @place
    if (below) {
        meiFermata.SetPlace(libmei::STAFFREL_below);
    }
    // @glyph.name
    if (smufl) {
        AsciiStringView glyphName = engraving::SymNames::nameForSymId(fermata->symId());
        meiFermata.SetGlyphName(glyphName.ascii());
        meiFermata.SetGlyphAuth("smufl");
    }

    return meiFermata;
}

std::pair<bool, engraving::NoteType> Convert::gracegrpFromMEI(const libmei::graceGrpLog_ATTACH meiAttach, const libmei::data_GRACE meiGrace,
                                                              bool& warning)
{
    warning = false;
    bool isAfter = (meiAttach == libmei::graceGrpLog_ATTACH_pre);
    engraving::NoteType noteType = engraving::NoteType::APPOGGIATURA;

    if (isAfter) {
        noteType = engraving::NoteType::GRACE8_AFTER;
    } else if (meiGrace == libmei::GRACE_unacc) {
        noteType = engraving::NoteType::ACCIACCATURA;
    }

    return { isAfter, noteType };
}

std::pair<libmei::graceGrpLog_ATTACH, libmei::data_GRACE> Convert::gracegrpToMEI(bool isAfter, engraving::NoteType noteType)
{
    libmei::graceGrpLog_ATTACH meiAttach = (isAfter) ? libmei::graceGrpLog_ATTACH_pre : libmei::graceGrpLog_ATTACH_NONE;
    libmei::data_GRACE meiGrace = libmei::GRACE_acc;

    if (isAfter) {
        meiGrace = libmei::GRACE_unknown;
    } else if (noteType == engraving::NoteType::ACCIACCATURA) {
        meiGrace = libmei::GRACE_unacc;
    }

    return { meiAttach, meiGrace };
}

void Convert::harmFromMEI(engraving::Harmony* harmony, const StringList& meiLines, const libmei::Harm& meiHarm, bool& warning)
{
    IF_ASSERT_FAILED(harmony) {
        return;
    }

    warning = false;

    // @place
    /*
    if (meiHarm.HasPlace()) {
        harmony->setPlacement(meiHarm.GetPlace() == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        harmony->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }
    */

    // @type
    engraving::HarmonyType harmonyType = engraving::HarmonyType::STANDARD;
    if (Convert::hasTypeValue(meiHarm.GetType(), std::string(HARMONY_TYPE) + "roman")) {
        harmonyType = engraving::HarmonyType::ROMAN;
    }

    // text content
    harmony->setHarmonyType(harmonyType);
    harmony->setHarmony(meiLines.join(u"\n"));
    harmony->setPlainText(harmony->harmonyName());

    return;
}

libmei::Harm Convert::harmToMEI(const engraving::Harmony* harmony, StringList& meiLines)
{
    libmei::Harm meiHarm;

    // @place
    if (harmony->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        //meiHarm.SetPlace(Convert::placeToMEI(harmony->placement()));
    }

    // @type
    if (harmony->harmonyType() != engraving::HarmonyType::STANDARD) {
        std::string harmonyType = HARMONY_TYPE;

        switch (harmony->harmonyType()) {
        case (engraving::HarmonyType::ROMAN):
            harmonyType = std::string(HARMONY_TYPE) + "roman";
            break;
        default: break;
        }
        meiHarm.SetType(harmonyType);
    }

    // content
    String plainText = harmony->plainText();
    meiLines = plainText.split(u"\n");

    return meiHarm;
}

void Convert::jumpFromMEI(engraving::Jump* jump, const libmei::RepeatMark& meiRepeatMark, bool& warning)
{
    warning = false;

    engraving::JumpType jumpType;

    // @func
    switch (meiRepeatMark.GetFunc()) {
    case (libmei::repeatMarkLog_FUNC_daCapo): jumpType = engraving::JumpType::DC;
        break;
    case (libmei::repeatMarkLog_FUNC_dalSegno): jumpType = engraving::JumpType::DS;
        break;
    default:
        jumpType = engraving::JumpType::DC;
    }

    // @type
    if (meiRepeatMark.HasType()) {
        std::list<std::string> jumpTypes = Convert::getTypeValuesWithPrefix(meiRepeatMark.GetType(), JUMP_TYPE);
        if (jumpTypes.size() > 0) {
            std::string value = jumpTypes.front();
            auto result = std::find_if(Convert::s_jumpTypes.begin(), Convert::s_jumpTypes.end(),
                                       [value](const auto& entry) { return entry.second == value; });

            if (result != Convert::s_jumpTypes.end()) {
                jumpType = result->first;
            } else {
                warning = true;
            }
        }
    }

    jump->setJumpType(jumpType);

    return;
}

libmei::RepeatMark Convert::jumpToMEI(const engraving::Jump* jump, String& text)
{
    libmei::RepeatMark meiRepeatMark;

    // @func
    switch (jump->jumpType()) {
    case (engraving::JumpType::DC):
    case (engraving::JumpType::DC_AL_FINE):
    case (engraving::JumpType::DC_AL_CODA): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_daCapo);
        break;
    case (engraving::JumpType::DS_AL_CODA):
    case (engraving::JumpType::DS_AL_FINE):
    case (engraving::JumpType::DS): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_dalSegno);
        break;
    case (engraving::JumpType::DC_AL_DBLCODA): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_daCapo);
        break;
    case (engraving::JumpType::DS_AL_DBLCODA):
    case (engraving::JumpType::DSS):
    case (engraving::JumpType::DSS_AL_CODA):
    case (engraving::JumpType::DSS_AL_DBLCODA):
    case (engraving::JumpType::DSS_AL_FINE): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_dalSegno);
        break;
    default:
        meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_daCapo);
    }

    // @type
    if (Convert::s_jumpTypes.count(jump->jumpType())) {
        meiRepeatMark.SetType(JUMP_TYPE + Convert::s_jumpTypes.at(jump->jumpType()));
    }

    switch (jump->jumpType()) {
    // No text for the default symbols
    case (engraving::JumpType::DC):
    case (engraving::JumpType::DS):
        break;
    default:
        text = jump->plainText();
    }

    return meiRepeatMark;
}

engraving::Key Convert::keyFromMEI(const libmei::data_KEYSIGNATURE& meiKeysig, bool& warning)
{
    warning = false;
    if (meiKeysig.second == libmei::ACCIDENTAL_WRITTEN_s) {
        return engraving::Key(meiKeysig.first);
    } else if (meiKeysig.second == libmei::ACCIDENTAL_WRITTEN_f) {
        return engraving::Key(-meiKeysig.first);
    } else if (meiKeysig.second == libmei::ACCIDENTAL_WRITTEN_n) {
        return engraving::Key(0);
    }
    warning = true;
    return engraving::Key(0);
}

libmei::data_KEYSIGNATURE Convert::keyToMEI(const engraving::Key key)
{
    if (key > 0) {
        return std::make_pair(std::min(static_cast<int>(key), 7), libmei::ACCIDENTAL_WRITTEN_s);
    } else if (key < 0) {
        return std::make_pair(std::abs(std::max(static_cast<int>(key), -7)), libmei::ACCIDENTAL_WRITTEN_f);
    } else {
        return std::make_pair(0, libmei::ACCIDENTAL_WRITTEN_n);
    }
}

engraving::LineType Convert::lineFromMEI(const libmei::data_LINEFORM meiLine, bool& warning)
{
    warning = false;
    switch (meiLine) {
    case (libmei::LINEFORM_solid): return engraving::LineType::SOLID;
    case (libmei::LINEFORM_dashed): return engraving::LineType::DASHED;
    case (libmei::LINEFORM_dotted): return engraving::LineType::DOTTED;
    case (libmei::LINEFORM_wavy):
        warning = true;
        return engraving::LineType::SOLID;
    default:
        return engraving::LineType::SOLID;
    }
}

libmei::data_LINEFORM Convert::lineToMEI(engraving::LineType line)
{
    switch (line) {
    case (engraving::LineType::SOLID): return libmei::LINEFORM_solid;
    case (engraving::LineType::DASHED): return libmei::LINEFORM_dashed;
    case (engraving::LineType::DOTTED): return libmei::LINEFORM_dotted;
    default:
        return libmei::LINEFORM_NONE;
    }
}

void Convert::markerFromMEI(engraving::Marker* marker, const libmei::RepeatMark& meiRepeatMark, bool& warning)
{
    warning = false;

    engraving::MarkerType markerType;

    // @func
    switch (meiRepeatMark.GetFunc()) {
    case (libmei::repeatMarkLog_FUNC_segno): markerType = engraving::MarkerType::SEGNO;
        break;
    case (libmei::repeatMarkLog_FUNC_coda): markerType = engraving::MarkerType::CODA;
        break;
    case (libmei::repeatMarkLog_FUNC_fine): markerType = engraving::MarkerType::FINE;
        break;
    default:
        markerType = engraving::MarkerType::SEGNO;
    }

    // @type
    if (meiRepeatMark.HasType()) {
        std::list<std::string> markerTypes = Convert::getTypeValuesWithPrefix(meiRepeatMark.GetType(), MARKER_TYPE);
        if (markerTypes.size() > 0) {
            std::string value = markerTypes.front();
            auto result = std::find_if(Convert::s_markerTypes.begin(), Convert::s_markerTypes.end(),
                                       [value](const auto& entry) { return entry.second == value; });

            if (result != Convert::s_markerTypes.end()) {
                markerType = result->first;
            } else {
                warning = true;
            }
        }
    }

    marker->setMarkerType(markerType);

    return;
}

libmei::RepeatMark Convert::markerToMEI(const engraving::Marker* marker, String& text)
{
    libmei::RepeatMark meiRepeatMark;

    // @func
    switch (marker->markerType()) {
    case (engraving::MarkerType::SEGNO):
    case (engraving::MarkerType::VARSEGNO): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_segno);
        break;
    case (engraving::MarkerType::CODA):
    case (engraving::MarkerType::VARCODA):
    case (engraving::MarkerType::CODETTA): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_coda);
        break;
    case (engraving::MarkerType::FINE): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_fine);
        break;
    case (engraving::MarkerType::TOCODA):
    case (engraving::MarkerType::TOCODASYM):
    case (engraving::MarkerType::DA_CODA):
    case (engraving::MarkerType::DA_DBLCODA): meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_coda);
        break;
    default:
        meiRepeatMark.SetFunc(libmei::repeatMarkLog_FUNC_segno);
    }

    // @type
    if (Convert::s_markerTypes.count(marker->markerType())) {
        meiRepeatMark.SetType(MARKER_TYPE + Convert::s_markerTypes.at(marker->markerType()));
    }

    switch (marker->markerType()) {
    // No text for the default symbols
    case (engraving::MarkerType::SEGNO):
    case (engraving::MarkerType::CODA):
    case (engraving::MarkerType::CODETTA):
    case (engraving::MarkerType::FINE):
        break;
    case (engraving::MarkerType::VARCODA):
    case (engraving::MarkerType::VARSEGNO):
        // Here we could as @glpyh.auth and @glyph.name but there as not in MEI-Basic
        break;
    case (engraving::MarkerType::TOCODASYM):
        text = "To 𝄌";
        break;
    default:
        text = marker->plainText();
    }

    return meiRepeatMark;
}

Convert::MeasureStruct Convert::measureFromMEI(const libmei::Measure& meiMeasure, bool& warning)
{
    warning = false;
    MeasureStruct measureSt;

    // for now only rely on the presence of @n
    measureSt.irregular = (!meiMeasure.HasN());

    if (meiMeasure.HasN()) {
        measureSt.n = std::stoi(meiMeasure.GetN()) - 1;
        // Make sure we have no measure number below 0;
        measureSt.n = std::max(0, measureSt.n);
    }

    if (meiMeasure.HasLeft()) {
        bool barlineWarning = false;
        engraving::BarLineType leftBarlineType = Convert::barlineFromMEI(meiMeasure.GetLeft(), barlineWarning);
        measureSt.repeatStart = (leftBarlineType == engraving::BarLineType::START_REPEAT);
        warning = (warning || barlineWarning);
    }

    if (meiMeasure.HasRight()) {
        bool barlineWarning = false;
        measureSt.endBarLineType = Convert::barlineFromMEI(meiMeasure.GetRight(), barlineWarning);
        warning = (warning || barlineWarning);
        // the measure will be flagged as repeatEnd and the BarLineType will not have to be set
        measureSt.repeatEnd
            = (std::set<engraving::BarLineType> { engraving::BarLineType::END_REPEAT, engraving::BarLineType::END_START_REPEAT }).count(
                  measureSt.endBarLineType) > 0;
    }

    if (meiMeasure.HasType()) {
        std::list<std::string> repeats = Convert::getTypeValuesWithPrefix(meiMeasure.GetType(), MEASURE_REPEAT_TYPE);
        if (repeats.size() > 0) {
            measureSt.repeatCount = String(repeats.front().c_str()).toInt();
            // make sure it is not smaller than 0
            measureSt.repeatCount = std::max(0, measureSt.repeatCount);
        }
    }

    return measureSt;
}

libmei::Measure Convert::measureToMEI(const engraving::Measure* measure, int& measureN, bool& wasPreviousIrregular)
{
    libmei::Measure meiMeasure;

    // @metcon
    if (measure->ticks() != measure->timesig()) {
        meiMeasure.SetMetcon(libmei::BOOLEAN_false);
    }
    // @n
    if (!measure->irregular()) {
        measureN++;
        meiMeasure.SetN(String("%1").arg(measure->no() + 1).toStdString());
    }
    // @left
    if (measure->repeatStart()) {
        meiMeasure.SetLeft(libmei::BARRENDITION_rptstart);
    }

    // @type used for storing measure repeat count
    if (measure->repeatEnd() && (measure->repeatCount() != 0)) {
        meiMeasure.SetType(String(u"%1%2").arg(String(MEASURE_REPEAT_TYPE)).arg(measure->repeatCount()).toStdString());
    }

    // @right
    meiMeasure.SetRight(Convert::barlineToMEI(measure->endBarLineType()));

    // update the flag for the next measure
    wasPreviousIrregular = (measure->irregular());

    return meiMeasure;
}

std::pair<engraving::Fraction, engraving::TimeSigType> Convert::meterFromMEI(const libmei::ScoreDef& meiScoreDef, bool& warning)
{
    libmei::StaffDef meiStaffDef;
    meiStaffDef.SetMeterSym(meiScoreDef.GetMeterSym());
    meiStaffDef.SetMeterUnit(meiScoreDef.GetMeterUnit());
    meiStaffDef.SetMeterCount(meiScoreDef.GetMeterCount());
    return meterFromMEI(meiStaffDef, warning);
}

std::pair<engraving::Fraction, engraving::TimeSigType> Convert::meterFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning)
{
    warning = false;
    engraving::Fraction fraction;
    engraving::TimeSigType tsType = engraving::TimeSigType::NORMAL;
    if (meiStaffDef.HasMeterCount() && meiStaffDef.HasMeterUnit()) {
        auto [counts, sign] = meiStaffDef.GetMeterCount();
        int numerator = !counts.empty() ? counts.front() : (meiStaffDef.GetMeterSym() == libmei::METERSIGN_common) ? 4 : 2;
        fraction.set(numerator, meiStaffDef.GetMeterUnit());
    }
    if (meiStaffDef.HasMeterSym()) {
        if (meiStaffDef.GetMeterSym() == libmei::METERSIGN_common) {
            tsType = engraving::TimeSigType::FOUR_FOUR;
            fraction.set(4, 4);
        } else {
            tsType = engraving::TimeSigType::ALLA_BREVE;
            fraction.set(2, 2);
        }
    }
    if (fraction.isZero()) {
        fraction.set(4, 4);
        warning = true;
    }
    return { fraction, tsType };
}

libmei::StaffDef Convert::meterToMEI(const engraving::Fraction& fraction, engraving::TimeSigType tsType)
{
    libmei::StaffDef meiStaffDef;
    if (tsType == engraving::TimeSigType::FOUR_FOUR) {
        meiStaffDef.SetMeterSym(libmei::METERSIGN_common);
    } else if (tsType == engraving::TimeSigType::ALLA_BREVE) {
        meiStaffDef.SetMeterSym(libmei::METERSIGN_cut);
    } else {
        meiStaffDef.SetMeterCount({ { fraction.numerator() }, libmei::MeterCountSign::None });
        meiStaffDef.SetMeterUnit(fraction.denominator());
    }
    return meiStaffDef;
}

Convert::PitchStruct Convert::pitchFromMEI(const libmei::Note& meiNote, const libmei::Accid& meiAccid, const engraving::Interval& interval,
                                           bool& warning)
{
    // The mapping from pitch name to step
    static int pitchMap[7]  = { 0, 2, 4, 5, 7, 9, 11 };
    //                          c  d  e  f  g  a   b

    warning = false;
    PitchStruct pitchSt;

    int step = meiNote.HasPname() ? meiNote.GetPname() - 1 : 0;
    // It should never be necessay, but just in case
    step = std::clamp(step, 0, 6);

    int oct = meiNote.HasOct() ? meiNote.GetOct() : 3;

    engraving::AccidentalVal alter = engraving::AccidentalVal::NATURAL;
    if (meiAccid.HasAccid() || meiAccid.HasAccidGes()) {
        bool accidWarning = false;
        libmei::data_ACCIDENTAL_GESTURAL meiAccidGes
            = (meiAccid.HasAccidGes()) ? meiAccid.GetAccidGes() : libmei::Att::AccidentalWrittenToGestural(meiAccid.GetAccid());
        alter = Convert::accidGesFromMEI(meiAccidGes, accidWarning);
        warning = (warning || accidWarning);
    }
    int alterInt = static_cast<int>(alter);

    /* This is currently not available in MEI Basic
    if (accid.HasEnclose()) {
        pitch.accidBracket = (accid.GetEnclose() == paren) ? engraving::AccidentalBracket::PARENTHESIS : engraving::AccidentalBracket::BRACKET;
    }
    */

    bool accidWarning = false;
    if (meiAccid.HasAccid()) {
        pitchSt.accidRole = engraving::AccidentalRole::USER;
    }
    pitchSt.accidType = Convert::accidFromMEI(meiAccid.GetAccid(), accidWarning);
    warning = (warning || accidWarning);
    pitchSt.pitch = pitchMap[step] + alterInt + (oct + 1) * 12;
    pitchSt.tpc2 = engraving::step2tpc(step, alter);
    // The pitch retrieved from the MEI is the written pitch and we need to transpose it
    pitchSt.pitch += interval.chromatic;

    return pitchSt;
}

std::pair<libmei::Note, libmei::Accid> Convert::pitchToMEI(const engraving::Note* note, const engraving::Accidental* accid,
                                                           const engraving::Interval& interval)
{
    libmei::Note meiNote;
    libmei::Accid meiAccid;

    IF_ASSERT_FAILED(note) {
        return { meiNote, meiAccid };
    }

    Convert::PitchStruct pitch;
    pitch.pitch = note->pitch();
    pitch.tpc2 = note->tpc2();
    if (accid) {
        pitch.accidType = accid->accidentalType();
        // Not available in MEI Basic
        // pitch.accidBracket = accid->bracket();
        // Not needed because relying on accidType
        // pitch.accidRole = accid->role();
    }

    meiNote.SetPname(static_cast<libmei::data_PITCHNAME>(engraving::tpc2step(pitch.tpc2) + 1));

    int writtenAlterInt = static_cast<int>(engraving::Accidental::subtype2value(pitch.accidType));
    int alterInt  = tpc2alterByKey(pitch.tpc2, engraving::Key::C);

    // We need to ajusted the pitch to its transpossed value for the octave calculation
    int oct = ((pitch.pitch - interval.chromatic - alterInt) / 12) - 1;
    meiNote.SetOct(oct);

    if (pitch.accidType != engraving::AccidentalType::NONE) {
        meiAccid.SetAccid(Convert::accidToMEI(pitch.accidType));
    }
    if (alterInt && (alterInt != writtenAlterInt)) {
        meiAccid.SetAccidGes(Convert::accidGesToMEI(static_cast<engraving::AccidentalVal>(alterInt)));
    }

    return { meiNote, meiAccid };
}

engraving::PlacementV Convert::placeFromMEI(const libmei::data_STAFFREL meiPlace, bool& warning)
{
    warning = false;
    switch (meiPlace) {
    case (libmei::STAFFREL_above): return engraving::PlacementV::ABOVE;
    case (libmei::STAFFREL_below): return engraving::PlacementV::BELOW;
    default:
        return engraving::PlacementV::ABOVE;
    }
}

libmei::data_STAFFREL Convert::placeToMEI(engraving::PlacementV place)
{
    switch (place) {
    case (engraving::PlacementV::ABOVE): return libmei::STAFFREL_above;
    case (engraving::PlacementV::BELOW): return libmei::STAFFREL_below;
    default:
        return libmei::STAFFREL_above;
    }
}

void Convert::slurFromMEI(engraving::SlurTie* slur, const libmei::Slur& meiSlur, bool& warning)
{
    warning = false;

    // @place
    if (meiSlur.HasCurvedir()) {
        slur->setSlurDirection(Convert::curvedirFromMEI(meiSlur.GetCurvedir(), warning));
        //slur->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @lform
    if (meiSlur.HasLform()) {
        bool typeWarning = false;
        slur->setStyleType(Convert::slurstyleFromMEI(meiSlur.GetLform(), typeWarning));
        warning = (warning || typeWarning);
    }
}

libmei::Slur Convert::slurToMEI(const engraving::SlurTie* slur)
{
    libmei::Slur meiSlur;

    // @place
    if (slur->slurDirection() != engraving::DirectionV::AUTO) {
        meiSlur.SetCurvedir(Convert::curvedirToMEI(slur->slurDirection()));
    }

    // @lform
    if (slur->styleType() != engraving::SlurStyleType::Solid) {
        meiSlur.SetLform(Convert::slurstyleToMEI(slur->styleType()));
    }

    return meiSlur;
}

engraving::SlurStyleType Convert::slurstyleFromMEI(const libmei::data_LINEFORM meiLine, bool& warning)
{
    warning = false;
    switch (meiLine) {
    case (libmei::LINEFORM_solid): return engraving::SlurStyleType::Solid;
    case (libmei::LINEFORM_dashed): return engraving::SlurStyleType::Dashed;
    case (libmei::LINEFORM_dotted): return engraving::SlurStyleType::Dotted;
    case (libmei::LINEFORM_wavy):
        warning = true;
        return engraving::SlurStyleType::Solid;
    default:
        return engraving::SlurStyleType::Solid;
    }
}

libmei::data_LINEFORM Convert::slurstyleToMEI(engraving::SlurStyleType slurstyle)
{
    switch (slurstyle) {
    case (engraving::SlurStyleType::Solid): return libmei::LINEFORM_solid;
    case (engraving::SlurStyleType::Dashed): return libmei::LINEFORM_dashed;
    case (engraving::SlurStyleType::WideDashed): return libmei::LINEFORM_dashed;
    case (engraving::SlurStyleType::Dotted): return libmei::LINEFORM_dotted;
    default:
        return libmei::LINEFORM_NONE;
    }
}

Convert::StaffStruct Convert::staffFromMEI(const libmei::StaffDef& meiStaffDef, bool& warning)
{
    warning = false;
    StaffStruct staffSt;

    if (meiStaffDef.HasLines()) {
        staffSt.lines = meiStaffDef.GetLines();
    }

    // Set it only if both are given
    if (meiStaffDef.HasTransDiat() && meiStaffDef.HasTransSemi()) {
        staffSt.interval.diatonic = meiStaffDef.GetTransDiat();
        staffSt.interval.chromatic = meiStaffDef.GetTransSemi();
    }

    return staffSt;
}

libmei::StaffDef Convert::staffToMEI(const engraving::Staff* staff)
{
    libmei::StaffDef meiStaffDef;

    IF_ASSERT_FAILED(staff) {
        return meiStaffDef;
    }

    // @n
    meiStaffDef.SetN(static_cast<int>(staff->idx() + 1));
    // @trans.*
    const engraving::Interval& interval = staff->part()->instrument()->transpose();
    if (!interval.isZero()) {
        meiStaffDef.SetTransDiat(interval.diatonic);
        meiStaffDef.SetTransSemi(interval.chromatic);
    }
    // @lines
    const engraving::StaffType* staffType = staff->staffType(engraving::Fraction(0, 1));
    if (staffType) {
        meiStaffDef.SetLines(staffType->lines());
    }
    return meiStaffDef;
}

std::pair<engraving::DirectionV, bool> Convert::stemFromMEI(const libmei::AttStems& meiStemsAtt, bool& warning)
{
    warning = false;

    engraving::DirectionV direction = engraving::DirectionV::AUTO;
    bool noStem = false;

    switch (meiStemsAtt.GetStemDir()) {
    case (libmei::STEMDIRECTION_up): direction = engraving::DirectionV::UP;
        break;
    case (libmei::STEMDIRECTION_down): direction = engraving::DirectionV::DOWN;
        break;
    default:
        break;
    }
    if (meiStemsAtt.GetStemLen() == 0.0) {
        noStem = true;
    }

    return { direction, noStem };
}

std::pair<libmei::data_STEMDIRECTION, double> Convert::stemToMEI(const engraving::DirectionV direction, bool noStem)
{
    libmei::data_STEMDIRECTION meiStemDir = libmei::STEMDIRECTION_NONE;
    double meiStemLen = -1.0;

    switch (direction) {
    case (engraving::DirectionV::UP): meiStemDir = libmei::STEMDIRECTION_up;
        break;
    case (engraving::DirectionV::DOWN): meiStemDir = libmei::STEMDIRECTION_down;
        break;
    default:
        break;
    }
    if (noStem) {
        meiStemLen = 0.0;
    }

    return { meiStemDir, meiStemLen };
}

void Convert::tempoFromMEI(engraving::TempoText* tempoText, const StringList& meiLines, const libmei::Tempo& meiTempo, bool& warning)
{
    IF_ASSERT_FAILED(tempoText) {
        return;
    }

    warning = false;

    // @place
    if (meiTempo.HasPlace()) {
        tempoText->setPlacement(meiTempo.GetPlace()
                                == libmei::STAFFREL_above ? engraving::PlacementV::ABOVE : engraving::PlacementV::BELOW);
        tempoText->setPropertyFlags(engraving::Pid::PLACEMENT, engraving::PropertyFlags::UNSTYLED);
    }

    // @bmp.midi
    if (meiTempo.HasMidiBpm()) {
        double tempoValue = meiTempo.GetMidiBpm() / 60;
        tempoText->setTempo(tempoValue);
    }

    // @type
    if (meiTempo.HasType() && Convert::hasTypeValue(meiTempo.GetType(), TEMPO_INFER_FROM_TEXT)) {
        tempoText->setFollowText(true);
    }

    // text
    tempoText->setXmlText(meiLines.join(u"\n"));

    return;
}

libmei::Tempo Convert::tempoToMEI(const engraving::TempoText* tempoText, StringList& meiLines)
{
    libmei::Tempo meiTempo;

    // @midi.bpm
    engraving::BeatsPerMinute bpm = tempoText->tempo().toBPM();
    double bpmRounded = round(bpm.val * 100) / 100;
    meiTempo.SetMidiBpm(bpmRounded);

    // @place
    if (tempoText->propertyFlags(engraving::Pid::PLACEMENT) == engraving::PropertyFlags::UNSTYLED) {
        meiTempo.SetPlace(Convert::placeToMEI(tempoText->placement()));
    }

    // @type
    if (tempoText->followText()) {
        meiTempo.SetType(TEMPO_INFER_FROM_TEXT);
    }

    // text content - only split lines
    meiLines = String(tempoText->plainText()).split(u"\n");

    return meiTempo;
}

engraving::TextStyleType Convert::textFromMEI(const libmei::Rend& meiRend, bool& warning)
{
    warning = false;

    AsciiStringView rendType(meiRend.GetType());
    engraving::TextStyleType textStyle = engraving::TextStyleType::DEFAULT;
    if (rendType.size() > 0) {
        textStyle = engraving::TConv::fromXml(rendType, engraving::TextStyleType::DEFAULT);
    }

    if (textStyle == engraving::TextStyleType::DEFAULT) {
        warning = true;
        textStyle = engraving::TextStyleType::TITLE;
    }

    return textStyle;
}

std::tuple<libmei::Rend, TextCell, String> Convert::textToMEI(const engraving::Text* text)
{
    libmei::Rend meiRend;
    TextCell cell = MiddleCenter;
    String string = text->plainText();

    libmei::data_FONTSIZE fontsize;

    /**
     Use default style placements.
     We could use at the style actual values eventually, even though that would be more complicated to get back in
     */
    switch (text->textStyleType()) {
    case (engraving::TextStyleType::TITLE):
        cell = TopCenter;
        fontsize.SetTerm(libmei::FONTSIZETERM_x_large);
        break;
    case (engraving::TextStyleType::SUBTITLE):
        cell = TopCenter;
        fontsize.SetTerm(libmei::FONTSIZETERM_large);
        break;
    case (engraving::TextStyleType::COMPOSER):
        cell = BottomRight;
        break;
    case (engraving::TextStyleType::POET):
        cell = BottomLeft;
        break;
    case (engraving::TextStyleType::INSTRUMENT_EXCERPT):
        cell = TopLeft;
        break;
    default: cell = MiddleCenter;
    }

    meiRend.SetFontsize(fontsize);

    // @label (@lang because not available in MEI basic
    // This is what allows for reading pgHead back as VBox
    AsciiStringView rendType = engraving::TConv::toXml(text->textStyleType());
    meiRend.SetType(rendType.ascii());

    return { meiRend, cell, string };
}

/**
 * Convert the segmented text block into MuseScore xmlText (with <sym>)
 * The text block are {bool, string} pairs with true for smufl and false for plain text
 */

void Convert::textFromMEI(String& text, const textWithSmufl& textBlocks)
{
    text.clear();

    for (auto& block : textBlocks) {
        if (!block.first) {
            text += block.second;
        } else {
            for (size_t i = 0; i < block.second.size(); i++) {
                char16_t c = block.second.at(i).unicode();
                if (c < u'\uE000' || c > u'\uF8FF') {
                    continue; // this should not happen because the char should be smufl
                }
                engraving::SymId symId = engravingFonts()->fallbackFont()->fromCode(c);
                if (symId == engraving::SymId::noSym) {
                    continue; // smufl code not found
                }
                text += u"<sym>" + String::fromAscii(engraving::SymNames::nameForSymId(symId).ascii()) + u"</sym>";
            }
        }
    }
}

/**
 * Convert a MuseScore plainText (without <sym>) into text blocks
 * The text block are {bool, string} pairs with true for smufl and false for plain text
 */

void Convert::textToMEI(textWithSmufl& textBlocks, const String& text)
{
    // Status of what is being parsed
    bool isSmufl = false;

    String smuflBlock;
    String textBlock;

    // Go throught the text char by char and build blocks of plain text / smufl text
    for (size_t i = 0; i < text.size(); i++) {
        char16_t c = text.at(i).unicode();
        // Not SMuFL
        if (c < u'\uE000' || c > u'\uF8FF') {
            // Changing to plain text, add the current smufl block if something in it
            if (isSmufl && smuflBlock.size() > 0) {
                textBlocks.push_back(std::make_pair(true, smuflBlock));
                smuflBlock.clear();
            }
            textBlock += c;
            isSmufl = false;
        }
        // SMuFL
        else {
            // Changing to smufl, add the current plain text block if something in in
            if (!isSmufl && textBlock.size() > 0) {
                textBlocks.push_back(std::make_pair(false, textBlock));
                textBlock.clear();
            }
            smuflBlock += c;
            isSmufl = true;
        }
    }
    // Add the last block
    if (textBlock.size() > 0) {
        textBlocks.push_back(std::make_pair(false, textBlock));
    }
    if (smuflBlock.size() > 0) {
        textBlocks.push_back(std::make_pair(true, smuflBlock));
    }
}

void Convert::tieFromMEI(engraving::SlurTie* tie, const libmei::Tie& meiTie, bool& warning)
{
    libmei::Slur meiSlur;
    meiSlur.SetCurvedir(meiTie.GetCurvedir());
    meiSlur.SetLform(meiTie.GetLform());
    Convert::slurFromMEI(tie, meiSlur, warning);
}

libmei::Tie Convert::tieToMEI(const engraving::SlurTie* tie)
{
    libmei::Slur meiSlur = Convert::slurToMEI(tie);
    libmei::Tie meiTie;
    meiTie.SetCurvedir(meiSlur.GetCurvedir());
    meiTie.SetLform(meiSlur.GetLform());
    return meiTie;
}

void Convert::tupletFromMEI(engraving::Tuplet* tuplet, const libmei::Tuplet& meiTuplet, bool& warning)
{
    IF_ASSERT_FAILED(tuplet) {
        return;
    }

    warning = false;
    if (!meiTuplet.HasNum() || !meiTuplet.HasNumbase()) {
        warning = true;
    } else {
        tuplet->setRatio(engraving::Fraction(meiTuplet.GetNum(), meiTuplet.GetNumbase()));
    }

    if (meiTuplet.GetNumFormat() == libmei::tupletVis_NUMFORMAT_ratio) {
        tuplet->setNumberType(engraving::TupletNumberType::SHOW_RELATION);
        tuplet->setPropertyFlags(engraving::Pid::NUMBER_TYPE, engraving::PropertyFlags::UNSTYLED);
    } else if (meiTuplet.GetNumVisible() == libmei::BOOLEAN_false) {
        tuplet->setNumberType(engraving::TupletNumberType::NO_TEXT);
        tuplet->setPropertyFlags(engraving::Pid::NUMBER_TYPE, engraving::PropertyFlags::UNSTYLED);
    }

    if (meiTuplet.GetBracketVisible() == libmei::BOOLEAN_true) {
        tuplet->setBracketType(engraving::TupletBracketType::SHOW_BRACKET);
        tuplet->setPropertyFlags(engraving::Pid::BRACKET_TYPE, engraving::PropertyFlags::UNSTYLED);
    } else if (meiTuplet.GetBracketVisible() == libmei::BOOLEAN_false) {
        tuplet->setBracketType(engraving::TupletBracketType::SHOW_NO_BRACKET);
        tuplet->setPropertyFlags(engraving::Pid::BRACKET_TYPE, engraving::PropertyFlags::UNSTYLED);
    }

    libmei::data_STAFFREL_basic bracketPlace = meiTuplet.GetBracketPlace();
    libmei::data_STAFFREL_basic numPlace = meiTuplet.GetNumPlace();
    if ((bracketPlace == libmei::STAFFREL_basic_above) || (numPlace == libmei::STAFFREL_basic_above)) {
        tuplet->setDirection(engraving::DirectionV::UP);
        tuplet->setPropertyFlags(engraving::Pid::DIRECTION, engraving::PropertyFlags::UNSTYLED);
    } else if ((bracketPlace == libmei::STAFFREL_basic_below) || (numPlace == libmei::STAFFREL_basic_below)) {
        tuplet->setDirection(engraving::DirectionV::DOWN);
        tuplet->setPropertyFlags(engraving::Pid::DIRECTION, engraving::PropertyFlags::UNSTYLED);
    }

    return;
}

libmei::Tuplet Convert::tupletToMEI(const engraving::Tuplet* tuplet)
{
    libmei::Tuplet meiTuplet;

    meiTuplet.SetNum(tuplet->ratio().numerator());
    meiTuplet.SetNumbase(tuplet->ratio().denominator());

    if (tuplet->numberType() == engraving::TupletNumberType::SHOW_RELATION) {
        meiTuplet.SetNumFormat(libmei::tupletVis_NUMFORMAT_ratio);
    } else if (tuplet->numberType() == engraving::TupletNumberType::NO_TEXT) {
        meiTuplet.SetNumVisible(libmei::BOOLEAN_false);
    }

    if (tuplet->bracketType() == engraving::TupletBracketType::SHOW_NO_BRACKET) {
        meiTuplet.SetBracketVisible(libmei::BOOLEAN_false);
    } else if (tuplet->bracketType() == engraving::TupletBracketType::SHOW_BRACKET) {
        meiTuplet.SetBracketVisible(libmei::BOOLEAN_true);
    }

    if (tuplet->direction() == engraving::DirectionV::UP) {
        meiTuplet.SetBracketPlace(libmei::STAFFREL_basic_above);
        meiTuplet.SetNumPlace(libmei::STAFFREL_basic_above);
    } else if (tuplet->direction() == engraving::DirectionV::DOWN) {
        meiTuplet.SetBracketPlace(libmei::STAFFREL_basic_below);
        meiTuplet.SetNumPlace(libmei::STAFFREL_basic_below);
    }

    return meiTuplet;
}

bool Convert::hasTypeValue(const std::string& typeStr, const std::string& value)
{
    std::istringstream iss(typeStr);

    std::string token;
    while (std::getline(iss, token, ' ')) {
        if (token == value) {
            return true;  // value found
        }
    }

    return false;
}

/**
 * Extract the list of values in a @type attribute having a prefix string
 * For example, "mscore-ending-1 primary mscore-ending-3" return ["1", "3"] with prefix "mscore-ending-"
 */

std::list<std::string> Convert::getTypeValuesWithPrefix(const std::string& typeStr, const std::string& prefix)
{
    std::istringstream iss(typeStr);
    std::list<std::string> values;

    std::string token;
    while (std::getline(iss, token, ' ')) {
        if ((token.rfind(prefix, 0) == 0) && (prefix.size() < token.size())) {
            values.push_back(token.erase(0, prefix.length()));
        }
    }

    return values;
}

double Convert::tstampFromFraction(const engraving::Fraction& fraction, const engraving::Fraction& timesig)
{
    return (double)fraction.numerator() / fraction.denominator() * timesig.denominator() + 1.0;
}

/**
 * Approximate a fraction for an MEI timestamp (double) values
 * From https://stackoverflow.com/questions/26643695/
 */

engraving::Fraction Convert::tstampToFraction(double tstamp, const engraving::Fraction& timesig)
{
    // Value is -1.0 because MEI time stamps are one-based
    // However, make sure it is not smaller than 0.0, which means that a tstamp 0.0 is moved to 1.0 (the first note)
    tstamp = std::max(tstamp - 1.0, 0.0);

    const int cycles = 10;
    const double precision = 0.01;

    int sign  = tstamp > 0 ? 1 : -1;
    tstamp = tstamp * sign; //abs(number);
    double new_number, whole_part;
    double decimal_part =  tstamp - (int)tstamp;
    int counter = 0;

    std::valarray<double> vec_1{ double((int)tstamp), 1 }, vec_2{ 1, 0 }, temporary;

    while (decimal_part > precision && counter < cycles) {
        new_number = 1 / decimal_part;
        whole_part = (int)new_number;

        temporary = vec_1;
        vec_1 = whole_part * vec_1 + vec_2;
        vec_2 = temporary;

        decimal_part = new_number - whole_part;
        counter += 1;
    }

    return engraving::Fraction(sign * vec_1[0], vec_1[1]) / timesig.denominator();
}
