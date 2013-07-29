//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <unordered_map>
#include "mscore.h"
#include "style.h"
#include "style_p.h"
#include "xml.h"
#include "score.h"
#include "articulation.h"
#include "harmony.h"
#include "chordlist.h"
#include "page.h"
#include "mscore.h"

namespace Ms {

MStyle* style;

//  20 points        font design size
//  72 points/inch   point size
// 120 dpi           screen resolution
//  spatium = 20/4 points

//---------------------------------------------------------
//   styleTypes
//---------------------------------------------------------

struct StyleTypes2 {
      StyleIdx idx;
      StyleType val;
      };

static const StyleTypes2 styleTypes2[] = {
      { ST_staffUpperBorder,            StyleType("staffUpperBorder",        ST_SPATIUM) },
      { ST_staffLowerBorder,            StyleType("staffLowerBorder",        ST_SPATIUM) },
      { ST_staffDistance,               StyleType("staffDistance",           ST_SPATIUM) },
      { ST_akkoladeDistance,            StyleType("akkoladeDistance",        ST_SPATIUM) },
      { ST_minSystemDistance,           StyleType("minSystemDistance",       ST_SPATIUM) },
      { ST_maxSystemDistance,           StyleType("maxSystemDistance",       ST_SPATIUM) },
      { ST_lyricsDistance,              StyleType("lyricsDistance",          ST_SPATIUM) },
      { ST_lyricsMinBottomDistance,     StyleType("lyricsMinBottomDistance", ST_SPATIUM) },
      { ST_lyricsLineHeight,            StyleType("lyricsLineHeight",        ST_DOUBLE) },      // in % of normal height (default: 1.0)
      { ST_figuredBassFontFamily,       StyleType("figuredBassFontFamily",   ST_STRING) },
      { ST_figuredBassFontSize,         StyleType("figuredBassFontSize",     ST_DOUBLE) },      // in pt
      { ST_figuredBassYOffset,          StyleType("figuredBassYOffset",      ST_DOUBLE) },      // in sp
      { ST_figuredBassLineHeight,       StyleType("figuredBassLineHeight",   ST_DOUBLE) },      // in % of normal height
      { ST_figuredBassAlignment,        StyleType("figuredBassAlignment",    ST_INT) },         // 0 = top, 1 = bottom
      { ST_figuredBassStyle,            StyleType("figuredBassStyle" ,       ST_INT) },         // 0=modern, 1=historic
      { ST_systemFrameDistance,         StyleType("systemFrameDistance",     ST_SPATIUM) },     // dist. between staff and vertical box
      { ST_frameSystemDistance,         StyleType("frameSystemDistance",     ST_SPATIUM) },     // dist. between vertical box and next system
      { ST_minMeasureWidth,             StyleType("minMeasureWidth",         ST_SPATIUM) },
      { ST_barWidth,                    StyleType("barWidth",                ST_SPATIUM) },
      { ST_doubleBarWidth,              StyleType("doubleBarWidth",          ST_SPATIUM) },
      { ST_endBarWidth,                 StyleType("endBarWidth",             ST_SPATIUM) },
      { ST_doubleBarDistance,           StyleType("doubleBarDistance",       ST_SPATIUM) },
      { ST_endBarDistance,              StyleType("endBarDistance",          ST_SPATIUM) },
      { ST_repeatBarTips,               StyleType("repeatBarTips",           ST_BOOL) },
      { ST_startBarlineSingle,          StyleType("startBarlineSingle",      ST_BOOL) },
      { ST_startBarlineMultiple,        StyleType("startBarlineMultiple",    ST_BOOL) },
      { ST_bracketWidth,                StyleType("bracketWidth",            ST_SPATIUM) },     // system bracket line width
      { ST_bracketDistance,             StyleType("bracketDistance",         ST_SPATIUM) },     // system bracket distance
      { ST_akkoladeWidth,               StyleType("akkoladeWidth",           ST_SPATIUM) },
      { ST_akkoladeBarDistance,         StyleType("akkoladeBarDistance",     ST_SPATIUM) },
      { ST_clefLeftMargin,              StyleType("clefLeftMargin",          ST_SPATIUM) },
      { ST_keysigLeftMargin,            StyleType("keysigLeftMargin",        ST_SPATIUM) },
      { ST_timesigLeftMargin,           StyleType("timesigLeftMargin",       ST_SPATIUM) },
      { ST_clefKeyRightMargin,          StyleType("clefKeyRightMargin",      ST_SPATIUM) },
      { ST_clefBarlineDistance,         StyleType("clefBarlineDistance",     ST_SPATIUM) },
      { ST_stemWidth,                   StyleType("stemWidth",               ST_SPATIUM) },
      { ST_shortenStem,                 StyleType("shortenStem",             ST_BOOL) },        // ST_shortenStem,
      { ST_shortStemProgression,        StyleType("shortStemProgression",    ST_SPATIUM) },     // ST_shortStemProgression,
      { ST_shortestStem,                StyleType("shortestStem",            ST_SPATIUM) },
      { ST_beginRepeatLeftMargin,       StyleType("beginRepeatLeftMargin",   ST_SPATIUM) },
      { ST_minNoteDistance,             StyleType("minNoteDistance",         ST_SPATIUM) },
      { ST_barNoteDistance,             StyleType("barNoteDistance",         ST_SPATIUM) },
      { ST_barAccidentalDistance,       StyleType("barAccidentalDistance",   ST_SPATIUM) },
      { ST_multiMeasureRestMargin,      StyleType("multiMeasureRestMargin",  ST_SPATIUM) },
      { ST_noteBarDistance,             StyleType("noteBarDistance",         ST_SPATIUM) },
      { ST_measureSpacing,              StyleType("measureSpacing",          ST_DOUBLE) },
      { ST_staffLineWidth,              StyleType("staffLineWidth",          ST_SPATIUM) },
      { ST_ledgerLineWidth,             StyleType("ledgerLineWidth",         ST_SPATIUM) },
      { ST_ledgerLineLength,            StyleType("ledgerLineLength",        ST_SPATIUM) },
      { ST_accidentalDistance,          StyleType("accidentalDistance",      ST_SPATIUM) },
      { ST_accidentalNoteDistance,      StyleType("accidentalNoteDistance",  ST_SPATIUM) },
      { ST_beamWidth,                   StyleType("beamWidth",               ST_SPATIUM) },
      { ST_beamDistance,                StyleType("beamDistance",            ST_DOUBLE) },      // in beamWidth units
      { ST_beamMinLen,                  StyleType("beamMinLen",              ST_SPATIUM) },     // len for broken beams
      { ST_beamMinSlope,                StyleType("beamMinSlope",            ST_DOUBLE) },
      { ST_beamMaxSlope,                StyleType("beamMaxSlope",            ST_DOUBLE) },
      { ST_maxBeamTicks,                StyleType("maxBeamTicks",            ST_INT) },
      { ST_dotMag,                      StyleType("dotMag",                  ST_DOUBLE) },
      { ST_dotNoteDistance,             StyleType("dotNoteDistance",         ST_SPATIUM) },
      { ST_dotRestDistance,             StyleType("dotRestDistance",         ST_SPATIUM) },
      { ST_dotDotDistance,              StyleType("dotDotDistance",          ST_SPATIUM) },
      { ST_propertyDistanceHead,        StyleType("propertyDistanceHead",    ST_SPATIUM) },     // note property to note head
      { ST_propertyDistanceStem,        StyleType("propertyDistanceStem",    ST_SPATIUM) },     // note property to note stem
      { ST_propertyDistance,            StyleType("propertyDistance",        ST_SPATIUM) },     // note property to note property
      { ST_lastSystemFillLimit,         StyleType("lastSystemFillLimit",     ST_DOUBLE) },
      { ST_hairpinY,                    StyleType("hairpinY",                ST_SPATIUM) },
      { ST_hairpinHeight,               StyleType("hairpinHeight",           ST_SPATIUM) },
      { ST_hairpinContHeight,           StyleType("hairpinContHeight",       ST_SPATIUM) },
      { ST_hairpinWidth,                StyleType("hairpinWidth",            ST_SPATIUM) },
      { ST_pedalY,                      StyleType("pedalY",                  ST_SPATIUM) },
      { ST_trillY,                      StyleType("trillY",                  ST_SPATIUM) },
      { ST_harmonyY,                    StyleType("harmonyY",                ST_SPATIUM) },
      { ST_harmonyFretDist,             StyleType("harmonyFretDist",         ST_SPATIUM) },
      { ST_minHarmonyDistance,          StyleType("minHarmonyDistance",      ST_SPATIUM) },
      { ST_showPageNumber,              StyleType("showPageNumber",          ST_BOOL) },
      { ST_showPageNumberOne,           StyleType("showPageNumberOne",       ST_BOOL) },
      { ST_pageNumberOddEven,           StyleType("pageNumberOddEven",       ST_BOOL) },
      { ST_showMeasureNumber,           StyleType("showMeasureNumber",       ST_BOOL) },
      { ST_showMeasureNumberOne,        StyleType("showMeasureNumberOne",    ST_BOOL) },
      { ST_measureNumberInterval,       StyleType("measureNumberInterval",   ST_INT) },
      { ST_measureNumberSystem,         StyleType("measureNumberSystem",     ST_BOOL) },
      { ST_measureNumberAllStaffs,      StyleType("measureNumberAllStaffs",  ST_BOOL) },
      { ST_smallNoteMag,                StyleType("smallNoteMag",            ST_DOUBLE) },
      { ST_graceNoteMag,                StyleType("graceNoteMag",            ST_DOUBLE) },
      { ST_smallStaffMag,               StyleType("smallStaffMag",           ST_DOUBLE) },
      { ST_smallClefMag,                StyleType("smallClefMag",            ST_DOUBLE) },
      { ST_genClef,                     StyleType("genClef",                 ST_BOOL) },           // create clef for all systems, not only for first
      { ST_genKeysig,                   StyleType("genKeysig",               ST_BOOL) },         // create key signature for all systems
      { ST_genTimesig,                  StyleType("genTimesig",              ST_BOOL) },
      { ST_genCourtesyTimesig,          StyleType("genCourtesyTimesig",      ST_BOOL) },
      { ST_genCourtesyKeysig,           StyleType("genCourtesyKeysig",       ST_BOOL) },
      { ST_genCourtesyClef,             StyleType("genCourtesyClef",         ST_BOOL) },
      { ST_useStandardNoteNames,        StyleType("useStandardNoteNames",    ST_BOOL) },
      { ST_useGermanNoteNames,          StyleType("useGermanNoteNames",      ST_BOOL) },
      { ST_useSolfeggioNoteNames,       StyleType("useSolfeggioNoteNames",   ST_BOOL) },
      { ST_lowerCaseMinorChords,        StyleType("lowerCaseMinorChords",    ST_BOOL) },
      { ST_chordStyle,                  StyleType("chordStyle",              ST_STRING) },
      { ST_chordsXmlFile,               StyleType("chordsXmlFile",           ST_BOOL) },
      { ST_chordDescriptionFile,        StyleType("chordDescriptionFile",    ST_STRING) },
      { ST_concertPitch,                StyleType("concertPitch",            ST_BOOL) },            // display transposing instruments in concert pitch
      { ST_createMultiMeasureRests,     StyleType("createMultiMeasureRests", ST_BOOL) },
      { ST_minEmptyMeasures,            StyleType("minEmptyMeasures",        ST_INT) },         // minimum number of empty measures for multi measure rest
      { ST_minMMRestWidth,              StyleType("minMMRestWidth",          ST_SPATIUM) },       // minimum width of multi measure rest
      { ST_hideEmptyStaves,             StyleType("hideEmptyStaves",         ST_BOOL) },
      { ST_dontHideStavesInFirstSystem, StyleType("dontHidStavesInFirstSystm", ST_BOOL) },
      { ST_stemDir1,                    StyleType("stemDir1",                ST_DIRECTION) },
      { ST_stemDir2,                    StyleType("stemDir2",                ST_DIRECTION) },
      { ST_stemDir3,                    StyleType("stemDir3",                ST_DIRECTION) },
      { ST_stemDir4,                    StyleType("stemDir4",                ST_DIRECTION) },
      { ST_gateTime,                    StyleType("gateTime",                ST_INT) },           // 0-100%
      { ST_tenutoGateTime,              StyleType("tenutoGateTime",          ST_INT) },
      { ST_staccatoGateTime,            StyleType("staccatoGateTime",        ST_INT) },
      { ST_slurGateTime,                StyleType("slurGateTime",            ST_INT) },
      { ST_ArpeggioNoteDistance,        StyleType("ArpeggioNoteDistance",    ST_SPATIUM) },
      { ST_ArpeggioLineWidth,           StyleType("ArpeggioLineWidth",       ST_SPATIUM) },
      { ST_ArpeggioHookLen,             StyleType("ArpeggioHookLen",         ST_SPATIUM) },
      { ST_FixMeasureNumbers,           StyleType("FixMeasureNumbers",       ST_INT) },
      { ST_FixMeasureWidth,             StyleType("FixMeasureWidth",         ST_BOOL) },
      { ST_SlurEndWidth,                StyleType("slurEndWidth",            ST_SPATIUM) },
      { ST_SlurMidWidth,                StyleType("slurMidWidth",            ST_SPATIUM) },
      { ST_SlurDottedWidth,             StyleType("slurDottedWidth",         ST_SPATIUM) },
      { ST_SlurBow,                     StyleType("slurBow",                 ST_SPATIUM) },
      { ST_SectionPause,                StyleType("sectionPause",            ST_DOUBLE) },
      { ST_MusicalSymbolFont,           StyleType("musicalSymbolFont",       ST_STRING) },
      { ST_showHeader,                  StyleType("showHeader",              ST_BOOL) },
      { ST_headerStyled,                StyleType("headerStyled",            ST_BOOL) },
      { ST_headerFirstPage,             StyleType("headerFirstPage",         ST_BOOL) },
      { ST_headerOddEven,               StyleType("headerOddEven",           ST_BOOL) },
      { ST_evenHeaderL,                 StyleType("evenHeaderL",             ST_STRING) },
      { ST_evenHeaderC,                 StyleType("evenHeaderC",             ST_STRING) },
      { ST_evenHeaderR,                 StyleType("evenHeaderR",             ST_STRING) },
      { ST_oddHeaderL,                  StyleType("oddHeaderL",              ST_STRING) },
      { ST_oddHeaderC,                  StyleType("oddHeaderC",              ST_STRING) },
      { ST_oddHeaderR,                  StyleType("oddHeaderR",              ST_STRING) },
      { ST_showFooter,                  StyleType("showFooter",              ST_BOOL) },
      { ST_footerStyled,                StyleType("footerStyled",            ST_BOOL) },
      { ST_footerFirstPage,             StyleType("footerFirstPage",         ST_BOOL) },
      { ST_footerOddEven,               StyleType("footerOddEven",           ST_BOOL) },
      { ST_evenFooterL,                 StyleType("evenFooterL",             ST_STRING) },
      { ST_evenFooterC,                 StyleType("evenFooterC",             ST_STRING) },
      { ST_evenFooterR,                 StyleType("evenFooterR",             ST_STRING) },
      { ST_oddFooterL,                  StyleType("oddFooterL",              ST_STRING) },
      { ST_oddFooterC,                  StyleType("oddFooterC",              ST_STRING) },
      { ST_oddFooterR,                  StyleType("oddFooterR",              ST_STRING) },
      { ST_voltaY,                      StyleType("voltaY",                  ST_SPATIUM) },
      { ST_voltaHook,                   StyleType("voltaHook",               ST_SPATIUM) },
      { ST_voltaLineWidth,              StyleType("voltaLineWidth",          ST_SPATIUM) },
      { ST_ottavaY,                     StyleType("ottavaY",                 ST_SPATIUM) },
      { ST_ottavaHook,                  StyleType("ottavaHook",              ST_SPATIUM) },
      { ST_ottavaLineWidth,             StyleType("ottavaLineWidth",         ST_SPATIUM) },
      { ST_tabClef,                     StyleType("tabClef",                 ST_INT) },
      { ST_tremoloWidth,                StyleType("tremoloWidth",            ST_SPATIUM) },
      { ST_tremoloBoxHeight,            StyleType("tremoloBoxHeight",        ST_SPATIUM) },
      { ST_tremoloStrokeWidth,          StyleType("tremoloLineWidth",        ST_SPATIUM) },
      { ST_tremoloDistance,             StyleType("tremoloDistance",         ST_SPATIUM) },
      { ST_linearStretch,               StyleType("linearStretch",           ST_DOUBLE) },
      { ST_crossMeasureValues,          StyleType("crossMeasureValues",      ST_BOOL) },
      { ST_keySigNaturals,              StyleType("keySigNaturals",          ST_INT) }
      };

class StyleTypes {
      StyleType st[ST_STYLES];

   public:
      StyleTypes()
            {
            for (auto i : styleTypes2)
                  st[i.idx] = i.val;
            };
      const char* name(StyleIdx i) const { return st[i].name(); }
      StyleValueType valueType(StyleIdx i) const { return st[i].valueType(); }
      };

static const StyleTypes styleTypes;

static const QString ff("FreeSerifMscore");

#define MM(x) ((x)/INCH)
#define OA     OFFSET_ABS
#define OS     OFFSET_SPATIUM
#define TR(x)  QT_TRANSLATE_NOOP("MuseScore", x)
#define AS(x)  s->addTextStyle(x)

//---------------------------------------------------------
//   setDefaultStyle
//    synchronize with TextStyleType
//---------------------------------------------------------

void initStyle(MStyle* s)
      {
      // this is an empty style, no offsets are allowed
      // never show this style
      AS(TextStyle(
         "", ff, 10, false, false, false, ALIGN_LEFT | ALIGN_BASELINE, QPointF(), OS, QPointF(), false,
               Spatium(0.0), Spatium(0.0), 25, QColor(Qt::black), false, false, QColor(Qt::black),
               QColor(255, 255, 255, 0), TextStyle::HIDE_ALWAYS));

      AS(TextStyle(
         TR("Title"), ff, 24, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, QPointF(), OA, QPointF(50.0, .0)));

      AS(TextStyle(
         TR("Subtitle"), ff, 14, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, QPointF(0, MM(10)), OA, QPointF(50.0, .0)));

      AS(TextStyle(
        TR("Composer"), ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_BASELINE, QPointF(MM(-1), MM(-2)), OA, QPointF(100.0, 100.0)));

      AS(TextStyle(
         TR("Lyricist"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(MM(1), MM(-2)), OA, QPointF(0.0, 100.0)));

      AS(TextStyle(
         TR("Lyrics odd lines"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(0, 7), OS, QPointF(), true));

      AS(TextStyle(
         TR("Lyrics even lines"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(0, 7), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Fingering"), ff,  8, false, false, false,
         ALIGN_CENTER, QPointF(), OA, QPointF(), true));

      AS(TextStyle(
         TR( "InstrumentsLong"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, QPointF(), OA, QPointF(), true));

      AS(TextStyle(
         TR( "InstrumentsShort"),   ff, 12, false, false, false,
         ALIGN_RIGHT | ALIGN_VCENTER, QPointF(), OA, QPointF(), true));

      AS(TextStyle(
         TR( "InstrumentsExcerpt"), ff, 18, false, false, false,
         ALIGN_LEFT | ALIGN_TOP, QPointF(), OA, QPointF()));

      AS(TextStyle(
         TR( "Dynamics"), ff, 12, false,
         false,                                 // italic?
         false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0.0, 8.0), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Technik"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0.0, -2.0), OS));

      AS(TextStyle(
         TR( "Tempo"), ff, 12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0, -4.0), OS, QPointF(),
         true, MMSP(.0), MMSP(.0), 0, Qt::black, false, true));

      AS(TextStyle(
         TR( "Metronome"), ff, 12, true, false, false, ALIGN_LEFT));

      AS(TextStyle(
         TR( "Measure Number"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_BOTTOM, QPointF(.0, -2.0), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Translator"), ff, 11, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP, QPointF(0, 6)));

      AS(TextStyle(
         TR( "Tuplets"), ff,  10, false, true, false,
         ALIGN_CENTER, QPointF(), OA, QPointF(), true));

      AS(TextStyle(
         TR( "System"), ff,  10, false, false, false,
         ALIGN_LEFT, QPointF(0, -4.0), OS, QPointF(), true,
         Spatium(0.0), Spatium(0.0), 25, Qt::black, false, true));

      AS(TextStyle(
         TR( "Staff"), ff,  10, false, false, false,
         ALIGN_LEFT, QPointF(0, -4.0), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Chordname"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Rehearsal Mark"), ff,  14, true, false, false,
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(0, -3.0), OS, QPointF(), true,
         Spatium(0.2), Spatium(.5), 20, Qt::black, false, true));

      AS(TextStyle(
         TR( "Repeat Text Left"), "MScore",  20, false, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0, -2.0), OS, QPointF(), true,
         MMSP(0.0), MMSP(0.0), 25, Qt::black, false, true));

      AS(TextStyle(
         TR( "Repeat Text Right"), ff,  12, false, false, false,
         ALIGN_RIGHT | ALIGN_BASELINE, QPointF(0, -2.0), OS, QPointF(100, 0), true,
         MMSP(0.0), MMSP(0.0), 25, Qt::black, false, true));

      AS(TextStyle(
         TR( "Repeat Text"), ff,  12, false, false, false,          // for backward compatibility
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(0, -2.0), OS, QPointF(100, 0), true,
         MMSP(0.0), MMSP(0.0), 25, Qt::black, false, true));

      // y offset may depend on voltaHook style element
      AS(TextStyle(
         TR( "Volta"), ff, 11, true, false, false,
         ALIGN_LEFT | ALIGN_BASELINE, QPointF(0.5, 1.9), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Frame"), ff, 12, false, false, false, ALIGN_LEFT | ALIGN_TOP));

      AS(TextStyle(
         TR( "TextLine"), ff,  12, false, false, false,
         ALIGN_LEFT | ALIGN_VCENTER, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Glissando"), ff, 8, false, true, false,
         ALIGN_HCENTER | ALIGN_BASELINE, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "String Number"), ff,  8, false, false, false,
         ALIGN_CENTER, QPointF(0, -5.0), OS, QPointF(100, 0), true,
         Spatium(0.1), Spatium(0.2), 0, Qt::black, true, false));

      AS(TextStyle(
         TR( "Ottava"), ff, 12, false, true, false,
         ALIGN_LEFT | ALIGN_VCENTER, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Bend"), ff, 8, false, false, false,
         ALIGN_CENTER | ALIGN_BOTTOM, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
         TR( "Header"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_TOP));

      AS(TextStyle(
         TR( "Footer"), ff, 8, false, false, false,
         ALIGN_HCENTER | ALIGN_BOTTOM, QPointF(0.0, MM(5)), OA));

      AS(TextStyle(
         TR( "Instrument Change"), ff,  12, true, false, false,
         ALIGN_LEFT | ALIGN_BOTTOM, QPointF(0, -3.0), OS, QPointF(0, 0), true));

      AS(TextStyle(
         TR("Lyrics Verse"), ff, 11, false, false, false,
         ALIGN_RIGHT | ALIGN_TOP, QPointF(), OS, QPointF(), true));

      AS(TextStyle(
      TR("Figured Bass"), "MScoreBC", 8, false, false, false,
         ALIGN_LEFT | ALIGN_TOP, QPointF(0, 6), OS, QPointF(), true,
         Spatium(0.0), Spatium(0.0), 25, QColor(Qt::black), false,      // default params
         false, QColor(Qt::black), QColor(255, 255, 255, 0),            // default params
         TextStyle::HIDE_IN_EDITOR));                                   // don't show in Style Editor

#undef MM
#undef OA
#undef OS
#undef TR
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

StyleData::StyleData()
   : _values(ST_STYLES)
      {
      _customChordList = false;

      struct StyleVal2 {
            StyleIdx idx;
            StyleVal val;
            };
      static const StyleVal2 values2[] = {
            { ST_staffUpperBorder,            StyleVal(Spatium(7.0)) },
            { ST_staffLowerBorder,            StyleVal(Spatium(7.0)) },
            { ST_staffDistance,               StyleVal(Spatium(6.5)) },
            { ST_akkoladeDistance,            StyleVal(Spatium(6.5)) },
            { ST_minSystemDistance,           StyleVal(Spatium(8.5)) },
            { ST_maxSystemDistance,           StyleVal(Spatium(15.0)) },
            { ST_lyricsDistance,              StyleVal(Spatium(3.5)) },
            { ST_lyricsMinBottomDistance,     StyleVal(Spatium(2)) },
            { ST_lyricsLineHeight,            StyleVal(qreal(1.0)) },
            { ST_figuredBassFontFamily,       StyleVal(QString("MuseScore Figured Bass")) },
            { ST_figuredBassFontSize,         StyleVal(qreal(8.0)) },
            { ST_figuredBassYOffset,          StyleVal(Spatium(6.0)) },
            { ST_figuredBassLineHeight,       StyleVal(qreal(1.0)) },
            { ST_figuredBassAlignment,        StyleVal(0) },
            { ST_figuredBassStyle,            StyleVal(0) },
            { ST_systemFrameDistance,         StyleVal(Spatium(7.0)) },
            { ST_frameSystemDistance,         StyleVal(Spatium(7.0)) },
            { ST_minMeasureWidth,             StyleVal(Spatium(5.0)) },
            { ST_barWidth,                    StyleVal(Spatium(0.16)) },      // 0.1875
            { ST_doubleBarWidth,              StyleVal(Spatium(0.16)) },
            { ST_endBarWidth,                 StyleVal(Spatium(0.3)) },       // 0.5
            { ST_doubleBarDistance,           StyleVal(Spatium(0.30)) },
            { ST_endBarDistance,              StyleVal(Spatium(0.40)) },     // 0.3
            { ST_repeatBarTips,               StyleVal(false) },
            { ST_startBarlineSingle,          StyleVal(false) },
            { ST_startBarlineMultiple,        StyleVal(true) },
            { ST_bracketWidth,                StyleVal(Spatium(0.45)) },
            { ST_bracketDistance,             StyleVal(Spatium(0.8)) },
            { ST_akkoladeWidth,               StyleVal(Spatium(1.6)) },
            { ST_akkoladeBarDistance,         StyleVal(Spatium(.4)) },
            { ST_clefLeftMargin,              StyleVal(Spatium(0.64)) },
            { ST_keysigLeftMargin,            StyleVal(Spatium(0.5)) },
            { ST_timesigLeftMargin,           StyleVal(Spatium(0.5)) },
            { ST_clefKeyRightMargin,          StyleVal(Spatium(1.75)) },
            { ST_clefBarlineDistance,         StyleVal(Spatium(0.18)) },      // was 0.5
            { ST_stemWidth,                   StyleVal(Spatium(0.13)) },      // 0.09375
            { ST_shortenStem,                 StyleVal(true) },
            { ST_shortStemProgression,        StyleVal(Spatium(0.25)) },
            { ST_shortestStem,                StyleVal(Spatium(2.25)) },
            { ST_beginRepeatLeftMargin,       StyleVal(Spatium(1.0)) },
            { ST_minNoteDistance,             StyleVal(Spatium(0.25)) },      // 0.4
            { ST_barNoteDistance,             StyleVal(Spatium(1.2)) },
            { ST_barAccidentalDistance,       StyleVal(Spatium(.3)) },
            { ST_multiMeasureRestMargin,      StyleVal(Spatium(1.2)) },
            { ST_noteBarDistance,             StyleVal(Spatium(1.0)) },
            { ST_measureSpacing,              StyleVal(qreal(1.2)) },
            { ST_staffLineWidth,              StyleVal(Spatium(0.08)) },      // 0.09375
            { ST_ledgerLineWidth,             StyleVal(Spatium(0.12)) },     // 0.1875
            { ST_ledgerLineLength,            StyleVal(Spatium(.6)) },     // note head width + this value
            { ST_accidentalDistance,          StyleVal(Spatium(0.22)) },
            { ST_accidentalNoteDistance,      StyleVal(Spatium(0.22)) },
            { ST_beamWidth,                   StyleVal(Spatium(0.5)) },           // was 0.48
            { ST_beamDistance,                StyleVal(qreal(0.5)) },          // 0.25sp
            { ST_beamMinLen,                  StyleVal(Spatium(1.316178)) },      // exactly note head width
            { ST_beamMinSlope,                StyleVal(qreal(0.05)) },
            { ST_beamMaxSlope,                StyleVal(qreal(0.2)) },
            { ST_maxBeamTicks,                StyleVal(MScore::division) },
            { ST_dotMag,                      StyleVal(qreal(1.0)) },
            { ST_dotNoteDistance,             StyleVal(Spatium(0.35)) },
            { ST_dotRestDistance,             StyleVal(Spatium(0.25)) },
            { ST_dotDotDistance,              StyleVal(Spatium(0.5)) },
            { ST_propertyDistanceHead,        StyleVal(Spatium(1.0)) },
            { ST_propertyDistanceStem,        StyleVal(Spatium(1.8)) },
            { ST_propertyDistance,            StyleVal(Spatium(1.0)) },
            { ST_lastSystemFillLimit,         StyleVal(qreal(0.3)) },
            { ST_hairpinY,                    StyleVal(Spatium(8)) },
            { ST_hairpinHeight,               StyleVal(Spatium(1.2)) },
            { ST_hairpinContHeight,           StyleVal(Spatium(0.5)) },
            { ST_hairpinWidth,                StyleVal(Spatium(0.13)) },
            { ST_pedalY,                      StyleVal(Spatium(8)) },
            { ST_trillY,                      StyleVal(Spatium(-1)) },
            { ST_harmonyY,                    StyleVal(Spatium(-2.5)) },
            { ST_harmonyFretDist,             StyleVal(Spatium(-1.5)) },
            { ST_minHarmonyDistance,          StyleVal(Spatium(0.5)) },
            { ST_showPageNumber,              StyleVal(true) },
            { ST_showPageNumberOne,           StyleVal(false) },
            { ST_pageNumberOddEven,           StyleVal(true) },
            { ST_showMeasureNumber,           StyleVal(true) },
            { ST_showMeasureNumberOne,        StyleVal(false) },
            { ST_measureNumberInterval,       StyleVal(5) },
            { ST_measureNumberSystem,         StyleVal(true) },
            { ST_measureNumberAllStaffs,      StyleVal(false) },
            { ST_smallNoteMag,                StyleVal(qreal(.7)) },
            { ST_graceNoteMag,                StyleVal(qreal(0.7)) },
            { ST_smallStaffMag,               StyleVal(qreal(0.7)) },
            { ST_smallClefMag,                StyleVal(qreal(0.8)) },
            { ST_genClef,                     StyleVal(true) },
            { ST_genKeysig,                   StyleVal(true) },
            { ST_genTimesig,                  StyleVal(true) },
            { ST_genCourtesyTimesig,          StyleVal(true) },
            { ST_genCourtesyKeysig,           StyleVal(true) },
            { ST_genCourtesyClef,             StyleVal(true) },
            { ST_useStandardNoteNames,        StyleVal(true) },
            { ST_useGermanNoteNames,          StyleVal(false) },
            { ST_useSolfeggioNoteNames,       StyleVal(false) },
            { ST_lowerCaseMinorChords,        StyleVal(false) },
            { ST_chordStyle,                  StyleVal(QString("std")) },
            { ST_chordsXmlFile,               StyleVal(false) },
            { ST_chordDescriptionFile,        StyleVal(QString("chords_std.xml")) },
            { ST_concertPitch,                StyleVal(false) },
            { ST_createMultiMeasureRests,     StyleVal(false) },
            { ST_minEmptyMeasures,            StyleVal(2) },
            { ST_minMMRestWidth,              StyleVal(Spatium(4)) },
            { ST_hideEmptyStaves,             StyleVal(false) },
            { ST_dontHideStavesInFirstSystem, StyleVal(true) },
            { ST_stemDir1,                    StyleVal(MScore::UP) },
            { ST_stemDir2,                    StyleVal(MScore::DOWN) },
            { ST_stemDir3,                    StyleVal(MScore::UP) },
            { ST_stemDir4,                    StyleVal(MScore::DOWN) },
            { ST_gateTime,                    StyleVal(100) },
            { ST_tenutoGateTime,              StyleVal(100) },
            { ST_staccatoGateTime,            StyleVal(50) },
            { ST_slurGateTime,                StyleVal(100) },
            { ST_ArpeggioNoteDistance,        StyleVal(Spatium(.5)) },
            { ST_ArpeggioLineWidth,           StyleVal(Spatium(.18)) },
            { ST_ArpeggioHookLen,             StyleVal(Spatium(.8)) },
            { ST_FixMeasureNumbers,           StyleVal(0) },
            { ST_FixMeasureWidth,             StyleVal(false) },
            { ST_SlurEndWidth,                StyleVal(Spatium(.07)) },
            { ST_SlurMidWidth,                StyleVal(Spatium(.15)) },
            { ST_SlurDottedWidth,             StyleVal(Spatium(.1)) },
            { ST_SlurBow,                     StyleVal(Spatium(1.6)) },
            { ST_SectionPause,                StyleVal(qreal(3.0)) },
            { ST_MusicalSymbolFont,           StyleVal(QString("Emmentaler")) },
            { ST_showHeader,                  StyleVal(false) },
            { ST_headerStyled,                StyleVal(true) },
            { ST_headerFirstPage,             StyleVal(false) },
            { ST_headerOddEven,               StyleVal(true) },
            { ST_evenHeaderL,                 StyleVal(QString()) },
            { ST_evenHeaderC,                 StyleVal(QString()) },
            { ST_evenHeaderR,                 StyleVal(QString()) },
            { ST_oddHeaderL,                  StyleVal(QString()) },
            { ST_oddHeaderC,                  StyleVal(QString()) },
            { ST_oddHeaderR,                  StyleVal(QString()) },
            { ST_showFooter,                  StyleVal(true) },
            { ST_footerStyled,                StyleVal(true) },
            { ST_footerFirstPage,             StyleVal(true) },
            { ST_footerOddEven,               StyleVal(true) },
            { ST_evenFooterL,                 StyleVal(QString("$p")) },
            { ST_evenFooterC,                 StyleVal(QString("$:copyright:")) },
            { ST_evenFooterR,                 StyleVal(QString()) },
            { ST_oddFooterL,                  StyleVal(QString()) },
            { ST_oddFooterC,                  StyleVal(QString("$:copyright:")) },
            { ST_oddFooterR,                  StyleVal(QString("$p")) },
            { ST_voltaY,                      StyleVal(Spatium(-3.0)) },
            { ST_voltaHook,                   StyleVal(Spatium(1.9)) },
            { ST_voltaLineWidth,              StyleVal(Spatium(.1)) },
            { ST_ottavaY,                     StyleVal(Spatium(-3.0)) },
            { ST_ottavaHook,                  StyleVal(Spatium(1.9)) },
            { ST_ottavaLineWidth,             StyleVal(Spatium(.1)) },
            { ST_tabClef,                     StyleVal(int(CLEF_TAB2)) },
            { ST_tremoloWidth,                StyleVal(Spatium(1.2)) },  // tremolo stroke width: note head width
            { ST_tremoloBoxHeight,            StyleVal(Spatium(0.65)) },
            { ST_tremoloStrokeWidth,          StyleVal(Spatium(0.35)) },
            { ST_tremoloDistance,             StyleVal(Spatium(0.8)) },
            { ST_linearStretch,               StyleVal(qreal(1.5)) },
            { ST_crossMeasureValues,          StyleVal(false) },
            { ST_keySigNaturals,              StyleVal(NAT_NONE) }
            };
      for (unsigned i = 0; i < sizeof(values2)/sizeof(*values2); ++i)
            _values[values2[i].idx] = values2[i].val;

// _textStyles.append(TextStyle(defaultTextStyles[i]));
      _spatium = SPATIUM20 * MScore::DPI;
      _articulationAnchor[Articulation_Fermata]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_Shortfermata]    = A_TOP_STAFF;
      _articulationAnchor[Articulation_Longfermata]     = A_TOP_STAFF;
      _articulationAnchor[Articulation_Verylongfermata] = A_TOP_STAFF;
      _articulationAnchor[Articulation_Thumb]           = A_CHORD;
      _articulationAnchor[Articulation_Sforzatoaccent]  = A_CHORD;
      _articulationAnchor[Articulation_Espressivo]      = A_CHORD;
      _articulationAnchor[Articulation_Staccato]        = A_CHORD;
      _articulationAnchor[Articulation_Staccatissimo]   = A_CHORD;
      _articulationAnchor[Articulation_Tenuto]          = A_CHORD;
      _articulationAnchor[Articulation_Portato]         = A_CHORD;
      _articulationAnchor[Articulation_Marcato]         = A_CHORD;
      _articulationAnchor[Articulation_Ouvert]          = A_CHORD;
      _articulationAnchor[Articulation_Plusstop]        = A_CHORD;
      _articulationAnchor[Articulation_Upbow]           = A_TOP_STAFF;
      _articulationAnchor[Articulation_Downbow]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_Reverseturn]     = A_TOP_STAFF;
      _articulationAnchor[Articulation_Turn]            = A_TOP_STAFF;
      _articulationAnchor[Articulation_Trill]           = A_TOP_STAFF;
      _articulationAnchor[Articulation_Prall]           = A_TOP_STAFF;
      _articulationAnchor[Articulation_Mordent]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_PrallPrall]      = A_TOP_STAFF;
      _articulationAnchor[Articulation_PrallMordent]    = A_TOP_STAFF;
      _articulationAnchor[Articulation_UpPrall]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_DownPrall]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_UpMordent]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_DownMordent]     = A_TOP_STAFF;
      _articulationAnchor[Articulation_PrallDown]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_PrallUp]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_LinePrall]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_Schleifer]       = A_TOP_STAFF;
      _articulationAnchor[Articulation_Snappizzicato]   = A_TOP_STAFF;
      _articulationAnchor[Articulation_Tapping]         = A_TOP_STAFF;
      _articulationAnchor[Articulation_Slapping]        = A_TOP_STAFF;
      _articulationAnchor[Articulation_Popping]         = A_TOP_STAFF;
      _spatium = SPATIUM20 * MScore::DPI;
      };

StyleData::StyleData(const StyleData& s)
   : QSharedData(s)
      {
      _values          = s._values;
      _chordList       = s._chordList;
      _customChordList = s._customChordList;
      _textStyles      = s._textStyles;
      _pageFormat.copy(s._pageFormat);
      _spatium         = s._spatium;
      for (int i = 0; i < ARTICULATIONS; ++i)
            _articulationAnchor[i] = s._articulationAnchor[i];
      }

//---------------------------------------------------------
//   Style
//---------------------------------------------------------

StyleData::~StyleData()
      {
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

TextStyle::TextStyle()
      {
      d = new TextStyleData;
      _hidden = HIDE_NEVER;
      }

TextStyle::TextStyle(QString _name, QString _family, qreal _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   const QPointF& _off, OffsetType _ot, const QPointF& _roff,
   bool sd,
   Spatium fw, Spatium pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg, QColor bg, Hidden hidden)
      {
      d = new TextStyleData(_name, _family, _size,
         _bold, _italic, _underline, _align, _off, _ot, _roff,
         sd, fw, pw, fr, co, _circle, _systemFlag, fg, bg);
      _hidden = hidden;
      }

TextStyle::TextStyle(const TextStyle& s)
   : d(s.d)
      {
      _hidden = s._hidden;
      }
TextStyle::~TextStyle()
      {
      }

//---------------------------------------------------------
//   TextStyle::operator=
//---------------------------------------------------------

TextStyle& TextStyle::operator=(const TextStyle& s)
      {
      d = s.d;
//      _hidden = s._hidden;
      return *this;
      }

//---------------------------------------------------------
//   TextStyleData
//---------------------------------------------------------

TextStyleData::TextStyleData()
      {
      family                 = "FreeSerif";
      size                   = 10.0;
      bold                   = false;
      italic                 = false;
      underline              = false;
      hasFrame               = false;
      sizeIsSpatiumDependent = false;
      frameWidth             = Spatium(0);
      paddingWidth           = Spatium(0);
      frameWidthMM           = 0.0;
      paddingWidthMM         = 0.0;
      frameRound             = 25;
      frameColor             = MScore::defaultColor;
      circle                 = false;
      systemFlag             = false;
      foregroundColor        = Qt::black;
      backgroundColor        = QColor(255, 255, 255, 0);
      }

TextStyleData::TextStyleData(
   QString _name, QString _family, qreal _size,
   bool _bold, bool _italic, bool _underline,
   Align _align,
   const QPointF& _off, OffsetType _ot, const QPointF& _roff,
   bool sd,
   Spatium fw, Spatium pw, int fr, QColor co, bool _circle, bool _systemFlag,
   QColor fg, QColor bg)
   :
   ElementLayout(_align, _off, _ot, _roff),
   name(_name), size(_size), bold(_bold),
   italic(_italic), underline(_underline),
   sizeIsSpatiumDependent(sd), frameWidth(fw), paddingWidth(pw),
   frameRound(fr), frameColor(co), circle(_circle), systemFlag(_systemFlag),
   foregroundColor(fg), backgroundColor(bg)
      {
      hasFrame       = (fw.val() != 0.0) || (bg.alpha() != 0);
      family         = _family;
      frameWidthMM   = 0.0;
      paddingWidthMM = 0.0;
      }

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool TextStyleData::operator!=(const TextStyleData& s) const
      {
      return s.name                   != name
          || s.family                 != family
          || s.size                   != size
          || s.bold                   != bold
          || s.italic                 != italic
          || s.underline              != underline
          || s.hasFrame               != hasFrame
          || s.sizeIsSpatiumDependent != sizeIsSpatiumDependent
          || s.frameWidth             != frameWidth
          || s.paddingWidth           != paddingWidth
          || s.frameRound             != frameRound
          || s.frameColor             != frameColor
          || s.circle                 != circle
          || s.systemFlag             != systemFlag
          || s.foregroundColor        != foregroundColor
          || s.backgroundColor        != backgroundColor
          || s.align()                != align()
          || s.offset()               != offset()
          || s.rxoff()                != rxoff()
          || s.ryoff()                != ryoff()
          || s.offsetType()           != offsetType()
          ;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyleData::font(qreal _spatium) const
      {
      qreal m = size;

      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      f.setUnderline(underline);

      if (sizeIsSpatiumDependent)
            m *= _spatium / ( SPATIUM20 * MScore::DPI);

      f.setPointSizeF(m);
      return f;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextStyleData::fontPx(qreal _spatium) const
      {
      qreal m = size * MScore::DPI / PPI;

      QFont f(family);
      f.setBold(bold);
      f.setItalic(italic);
      f.setUnderline(underline);
#ifdef USE_GLYPHS
      f.setHintingPreference(QFont::PreferVerticalHinting);
#endif
      if (sizeIsSpatiumDependent)
            m *= _spatium / (SPATIUM20 * MScore::DPI);

      f.setPixelSize(lrint(m));
      return f;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextStyleData::write(Xml& xml) const
      {
      xml.stag("TextStyle");
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void TextStyleData::writeProperties(Xml& xml) const
      {
      ElementLayout::writeProperties(xml);
      if (!name.isEmpty())
            xml.tag("name", name);
      xml.tag("family", family);
      xml.tag("size", size);
      if (bold)
            xml.tag("bold", bold);
      if (italic)
            xml.tag("italic", italic);
      if (underline)
            xml.tag("underline", underline);
      if (sizeIsSpatiumDependent)
            xml.tag("sizeIsSpatiumDependent", sizeIsSpatiumDependent);
      if (foregroundColor != Qt::black)
            xml.tag("foregroundColor", foregroundColor);
      if (backgroundColor != QColor(255, 255, 255, 0))
            xml.tag("backgroundColor", backgroundColor);

      if (hasFrame) {
            xml.tag("frameWidthS",   frameWidth.val());
            xml.tag("paddingWidthS", paddingWidth.val());
            xml.tag("frameRound",   frameRound);
            xml.tag("frameColor",   frameColor);
            if (circle)
                  xml.tag("circle", circle);
            }
      if (systemFlag)
            xml.tag("systemFlag", systemFlag);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextStyleData::read(XmlReader& e)
      {
      frameWidth = Spatium(0.0);
      name = e.attribute("name");

      while (e.readNextStartElement()) {
            if (!readProperties(e))
                  e.unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextStyleData::readProperties(XmlReader& e)
      {
      const QStringRef& tag(e.name());

      if (tag == "name")
            name = e.readElementText();
      else if (tag == "family")
            family = e.readElementText();
      else if (tag == "size")
            size = e.readDouble();
      else if (tag == "bold")
            bold = e.readInt();
      else if (tag == "italic")
            italic = e.readInt();
      else if (tag == "underline")
            underline = e.readInt();
      else if (tag == "align")
            setAlign(Align(e.readInt()));
      else if (tag == "anchor")     // obsolete
            e.skipCurrentElement();
      else if (ElementLayout::readProperties(e))
            ;
      else if (tag == "sizeIsSpatiumDependent" || tag == "spatiumSizeDependent")
            sizeIsSpatiumDependent = e.readInt();
      else if (tag == "frameWidth") {
            hasFrame = true;
            frameWidthMM = e.readDouble();
            }
      else if (tag == "frameWidthS") {
            hasFrame = true;
            frameWidth = Spatium(e.readDouble());
            }
      else if (tag == "frame")      // obsolete
            hasFrame = e.readInt();
      else if (tag == "paddingWidth")          // obsolete
            paddingWidthMM = e.readDouble();
      else if (tag == "paddingWidthS")
            paddingWidth = Spatium(e.readDouble());
      else if (tag == "frameRound")
            frameRound = e.readInt();
      else if (tag == "frameColor")
            frameColor = e.readColor();
      else if (tag == "foregroundColor")
            foregroundColor = e.readColor();
      else if (tag == "backgroundColor")
            backgroundColor = e.readColor();
      else if (tag == "circle")
            circle = e.readInt();
      else if (tag == "systemFlag")
            systemFlag = e.readInt();
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void StyleData::load(XmlReader& e)
      {
      QString oldChordDescriptionFile = value(ST_chordDescriptionFile).toString();
      bool chordListTag = false;
      while (e.readNextStartElement()) {
            QString tag = e.name().toString();

            if (tag == "TextStyle") {
                  TextStyle s;
                  s.read(e);
                  setTextStyle(s);
                  }
            else if (tag == "Spatium")
                  setSpatium (e.readDouble() * MScore::DPMM);
            else if (tag == "page-layout")
                  _pageFormat.read(e);
            else if (tag == "displayInConcertPitch")
                  set(ST_concertPitch, StyleVal(bool(e.readInt())));
            else if (tag == "ChordList") {
                  _chordList.clear();
                  _chordList.read(e);
                  _customChordList = true;
                  chordListTag = true;
                  }
            else if (tag == "pageFillLimit")   // obsolete
                  e.skipCurrentElement();
            else if (tag == "systemDistance")  // obsolete
                  set(ST_minSystemDistance, StyleVal(Spatium(e.readDouble())));
            else {
                  if (tag == "stemDir") {
                        int voice = e.attribute("voice", "1").toInt() - 1;
                        switch(voice) {
                              case 0: tag = "StemDir1"; break;
                              case 1: tag = "StemDir2"; break;
                              case 2: tag = "StemDir3"; break;
                              case 3: tag = "StemDir4"; break;
                              }
                        }
                  // for compatibility:
                  if (tag == "oddHeader" || tag == "evenHeader"
                     || tag == "oddFooter" || tag == "evenFooter")
                        tag += "C";

                  QString val(e.readElementText());

                  StyleIdx idx;
                  for (int i = 0; i < ST_STYLES; ++i) {
                        idx = static_cast<StyleIdx>(i);
                        if (styleTypes.name(idx) == tag) {
                              switch(styleTypes.valueType(idx)) {
                                    case ST_SPATIUM:   set(idx, StyleVal(Spatium(val.toDouble()))); break;
                                    case ST_DOUBLE:    set(idx, StyleVal(qreal(val.toDouble())));          break;
                                    case ST_BOOL:      set(idx, StyleVal(bool(val.toInt())));       break;
                                    case ST_INT:       set(idx, StyleVal(val.toInt()));             break;
                                    case ST_DIRECTION: set(idx, StyleVal(MScore::Direction(val.toInt())));  break;
                                    case ST_STRING:    set(idx, StyleVal(val));                     break;
                                    }
                              break;
                              }
                        }
                  if (idx >= ST_STYLES) {
                        if (tag == "oddHeader")
                              ;
                        else if (tag == "evenHeader")
                              ;
                        else if (tag == "oddFooter")
                              ;
                        else if (tag == "evenHeader")
                              ;
                        else {
                              int idx2;
                              for (idx2 = 0; idx2 < ARTICULATIONS; ++idx2) {
                                    ArticulationInfo& ai =  Articulation::articulationList[idx2];
                                    if (tag == ai.name + "Anchor"
                                       || (tag == "U" + ai.name + "Anchor")
                                       || (tag == "D" + ai.name + "Anchor")
                                       ) {
                                          _articulationAnchor[idx2] = ArticulationAnchor(val.toInt());
                                          break;
                                          }
                                    }
                              if (idx2 == ARTICULATIONS)
                                    e.unknown();
                              }
                        }
                  }
            }

      // if we just specified a new chord description file
      // and didn't encounter a ChordList tag
      // then load the chord description file
      QString newChordDescriptionFile = value(ST_chordDescriptionFile).toString();
      if (newChordDescriptionFile != oldChordDescriptionFile && !chordListTag) {
            if (!newChordDescriptionFile.startsWith("chords_") && value(ST_chordStyle).toString() == "std") {
                  // should not normally happen,
                  // but treat as "old" (114) score just in case
                  set(ST_chordStyle, StyleVal(QString("custom")));
                  set(ST_chordsXmlFile, StyleVal(true));
                  qDebug("StyleData::load: custom chord description file %s with chordStyle == std", qPrintable(newChordDescriptionFile));
                  }
            if (value(ST_chordStyle).toString() == "custom")
                  _customChordList = true;
            else
                  _customChordList = false;
            _chordList.unload();
            }

      // make sure we have a chordlist
      if (!_chordList.loaded() && !chordListTag) {
            if (value(ST_chordsXmlFile).toBool())
                  _chordList.read("chords.xml");
            _chordList.read(newChordDescriptionFile);
            }

      //
      //  Compatibility with old scores/styles:
      //  translate old frameWidthMM and paddingWidthMM
      //  into spatium units
      //
      int n = _textStyles.size();
      qreal spMM = _spatium / MScore::DPMM;
      for (int i = 0; i < n; ++i) {
            TextStyle* s = &_textStyles[i];
            if (s->frameWidthMM() != 0.0)
                  s->setFrameWidth(Spatium(s->frameWidthMM() / spMM));
            if (s->paddingWidthMM() != 0.0)
                  s->setPaddingWidth(Spatium(s->paddingWidthMM() / spMM));
            }
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool StyleData::isDefault(StyleIdx idx) const
      {
      switch(styleTypes.valueType(idx)) {
            case ST_DOUBLE:
            case ST_SPATIUM:
                  return _values[idx].toDouble() == MScore::baseStyle()->valueD(idx);
            case ST_BOOL:
                  return _values[idx].toBool() == MScore::baseStyle()->valueB(idx);
            case ST_INT:
            case ST_DIRECTION:
                  return _values[idx].toInt() == MScore::baseStyle()->valueI(idx);
            case ST_STRING:
                  return _values[idx].toString() == MScore::baseStyle()->valueSt(idx);
            }
      return false;
      }

//---------------------------------------------------------
//   save
//    if optimize is true, save only if different to default
//    style
//---------------------------------------------------------

void StyleData::save(Xml& xml, bool optimize) const
      {
      xml.stag("Style");

      for (int i = 0; i < ST_STYLES; ++i) {
            StyleIdx idx = StyleIdx(i);
            if (optimize && isDefault(idx))
                  continue;
            switch(styleTypes.valueType(idx)) {
                  case ST_SPATIUM:
                  case ST_DOUBLE:    xml.tag(styleTypes.name(idx), value(idx).toDouble()); break;
                  case ST_BOOL:      xml.tag(styleTypes.name(idx), value(idx).toBool()); break;
                  case ST_INT:       xml.tag(styleTypes.name(idx), value(idx).toInt()); break;
                  case ST_DIRECTION: xml.tag(styleTypes.name(idx), int(value(idx).toDirection())); break;
                  case ST_STRING:    xml.tag(styleTypes.name(idx), value(idx).toString()); break;
                  }
            }
      for (int i = 0; i < TEXT_STYLES; ++i) {
            if (!optimize || _textStyles[i] != MScore::defaultStyle()->textStyle(i))
                  _textStyles[i].write(xml);
            }
      for (int i = TEXT_STYLES; i < _textStyles.size(); ++i)
            _textStyles[i].write(xml);
      if (_customChordList && !_chordList.isEmpty()) {
            xml.stag("ChordList");
            _chordList.write(xml);
            xml.etag();
            }
      for (int i = 0; i < ARTICULATIONS; ++i) {
            if (optimize && _articulationAnchor[i] == MScore::defaultStyle()->articulationAnchor(i))
                  continue;
            const ArticulationInfo& ai = Articulation::articulationList[i];
            xml.tag(ai.name + "Anchor", int(_articulationAnchor[i]));
            }
      _pageFormat.write(xml);
      xml.tag("Spatium", _spatium / MScore::DPMM);
      xml.etag();
      }

//---------------------------------------------------------
//   chordDescription
//---------------------------------------------------------

const ChordDescription* StyleData::chordDescription(int id) const
      {
      return _chordList.value(id);
      }

//---------------------------------------------------------
//   chordList
//---------------------------------------------------------

ChordList* StyleData::chordList()
      {
      return &_chordList;
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void StyleData::setChordList(ChordList* cl, bool custom)
      {
      _chordList = *cl;
      _customChordList = custom;
      }

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle& StyleData::textStyle(int idx) const
      {
      Q_ASSERT(idx >= 0 && idx < _textStyles.count());
      return _textStyles[idx];
      }

//---------------------------------------------------------
//   StyleVal
//---------------------------------------------------------

StyleVal::StyleVal(Spatium val)
      {
      v.dbl = val.val();
      }

StyleVal::StyleVal(qreal val)
      {
      v.dbl = val;
      }

StyleVal::StyleVal(bool val)
      {
      v.b   = val;
      }

StyleVal::StyleVal(int val)
      {
      v.i   = val;
      }

StyleVal::StyleVal(MScore::Direction val)
      {
      v.d   = val;
      }

StyleVal::StyleVal(const QString& val)
      {
      s    = val;
      }

StyleVal::StyleVal(const StyleVal& val)
      {
      s   = val.s;
      v   = val.v;
      }

StyleVal& StyleVal::operator=(const StyleVal& val)
      {
      s   = val.s;
      v   = val.v;
      return *this;
      }

StyleVal::StyleVal(const QString& name, const QString& val)
      {
      for (int i = 0; i < ST_STYLES; ++i) {
            StyleIdx idx = StyleIdx(i);
            if (styleTypes.name(idx) != name)
                  continue;
            switch(styleTypes.valueType(idx)) {
                  case ST_DOUBLE:
                  case ST_SPATIUM:
                        v.dbl = val.toDouble();
                        break;
                  case ST_BOOL:
                        v.b  = val.toInt();
                        break;
                  case ST_INT:
                        v.i = val.toInt();
                        break;
                  case ST_DIRECTION:
                        v.d = MScore::Direction(val.toInt());
                        break;
                  case ST_STRING:
                        s = val;
                        break;
                  }
            break;
            }
      }

//---------------------------------------------------------
//   value
//---------------------------------------------------------

StyleVal MStyle::value(StyleIdx idx) const
      {
      return d->_values[idx];
      }

//---------------------------------------------------------
//   isDefault
//---------------------------------------------------------

bool MStyle::isDefault(StyleIdx idx) const
      {
      return d->isDefault(idx);
      }

//---------------------------------------------------------
//   chordDescription
//---------------------------------------------------------

const ChordDescription* MStyle::chordDescription(int id) const
      {
      return d->chordDescription(id);
      }

//---------------------------------------------------------
//   chordList
//---------------------------------------------------------

ChordList* MStyle::chordList()
      {
      return d->chordList();
      }

//---------------------------------------------------------
//   setChordList
//---------------------------------------------------------

void MStyle::setChordList(ChordList* cl, bool custom)
      {
      d->setChordList(cl, custom);
      }

//---------------------------------------------------------
//   textStyle
//---------------------------------------------------------

const TextStyle& StyleData::textStyle(const QString& name) const
      {
      foreach(const TextStyle& s, _textStyles) {
            if (s.name() == name)
                  return s;
            }
      qDebug("TextStyle <%s> not found", qPrintable(name));
      abort();
      return _textStyles[0];
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

int StyleData::textStyleType(const QString& name) const
      {
      for (int i = 0; i < _textStyles.size(); ++i) {
            if (_textStyles[i].name() == name)
                  return i;
            }
      if (name == "Dynamics2")
            return TEXT_STYLE_DYNAMICS;
      qDebug("TextStyleType <%s> not found", qPrintable(name));
      return TEXT_STYLE_UNKNOWN;
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void StyleData::setTextStyle(const TextStyle& ts)
      {
      for (int i = 0; i < _textStyles.size(); ++i) {
            if (_textStyles[i].name() == ts.name()) {
                  _textStyles[i] = ts;
                  return;
                  }
            }
      _textStyles.append(ts);
      }

//---------------------------------------------------------
//   TextStyle method wrappers
//---------------------------------------------------------

QString TextStyle::name() const                          {  return d->name;    }
QString TextStyle::family() const                        {  return d->family;  }
qreal TextStyle::size() const                            {  return d->size;    }
bool TextStyle::bold() const                             {  return d->bold;    }
bool TextStyle::italic() const                           { return d->italic; }
bool TextStyle::underline() const                        { return d->underline; }
bool TextStyle::hasFrame() const                         { return d->hasFrame; }
Align TextStyle::align() const                           { return d->align(); }
const QPointF& TextStyle::offset() const                 { return d->offset(); }
QPointF TextStyle::offset(qreal spatium) const           { return d->offset(spatium); }
OffsetType TextStyle::offsetType() const                 { return d->offsetType(); }
bool TextStyle::sizeIsSpatiumDependent() const           { return d->sizeIsSpatiumDependent; }

Spatium TextStyle::frameWidth()  const                   { return d->frameWidth; }
Spatium TextStyle::paddingWidth() const                  { return d->paddingWidth; }
qreal TextStyle::frameWidthMM()  const                   { return d->frameWidthMM; }
qreal TextStyle::paddingWidthMM() const                  { return d->paddingWidthMM; }
void TextStyle::setFrameWidth(Spatium v)                 { d->frameWidth = v; }
void TextStyle::setPaddingWidth(Spatium v)               { d->paddingWidth = v; }

int TextStyle::frameRound() const                        { return d->frameRound; }
QColor TextStyle::frameColor() const                     { return d->frameColor; }
bool TextStyle::circle() const                           { return d->circle;     }
bool TextStyle::systemFlag() const                       { return d->systemFlag; }
QColor TextStyle::foregroundColor() const                { return d->foregroundColor; }
QColor TextStyle::backgroundColor() const                { return d->backgroundColor; }
void TextStyle::setName(const QString& s)                { d->name = s; }
void TextStyle::setFamily(const QString& s)              { d->family = s; }
void TextStyle::setSize(qreal v)                         { d->size = v; }
void TextStyle::setBold(bool v)                          { d->bold = v; }
void TextStyle::setItalic(bool v)                        { d->italic = v; }
void TextStyle::setUnderline(bool v)                     { d->underline = v; }
void TextStyle::setHasFrame(bool v)                      { d->hasFrame = v; }
void TextStyle::setAlign(Align v)                        { d->setAlign(v); }
void TextStyle::setXoff(qreal v)                         { d->setXoff(v); }
void TextStyle::setYoff(qreal v)                         { d->setYoff(v); }
void TextStyle::setOffsetType(OffsetType v)              { d->setOffsetType(v); }
void TextStyle::setRxoff(qreal v)                        { d->setRxoff(v); }
void TextStyle::setRyoff(qreal v)                        { d->setRyoff(v); }
void TextStyle::setSizeIsSpatiumDependent(bool v)        { d->sizeIsSpatiumDependent = v; }
void TextStyle::setFrameRound(int v)                     { d->frameRound = v; }
void TextStyle::setFrameColor(const QColor& v)           { d->frameColor = v; }
void TextStyle::setCircle(bool v)                        { d->circle = v;     }
void TextStyle::setSystemFlag(bool v)                    { d->systemFlag = v; }
void TextStyle::setForegroundColor(const QColor& v)      { d->foregroundColor = v; }
void TextStyle::setBackgroundColor(const QColor& v)      { d->backgroundColor = v; }
void TextStyle::write(Xml& xml) const                    { d->write(xml); }
void TextStyle::read(XmlReader& v)               { d->read(v); }
QFont TextStyle::font(qreal space) const                 { return d->font(space); }
QFont TextStyle::fontPx(qreal spatium) const             { return d->fontPx(spatium); }
QRectF TextStyle::bbox(qreal sp, const QString& s) const { return d->bbox(sp, s); }
QFontMetricsF TextStyle::fontMetrics(qreal space) const  { return d->fontMetrics(space); }
bool TextStyle::operator!=(const TextStyle& s) const     { return d->operator!=(*s.d); }
void TextStyle::layout(Element* e) const                 { d->layout(e); }
void TextStyle::writeProperties(Xml& xml) const          { d->writeProperties(xml); }
const QPointF& TextStyle::reloff() const                 { return d->reloff();      }
void TextStyle::setReloff(const QPointF& p)              { setRxoff(p.x()), setRyoff(p.y()); }
bool TextStyle::readProperties(XmlReader& v)     { return d->readProperties(v); }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void TextStyle::setFont(const QFont&)
      {
      //TODOxx
      }

//---------------------------------------------------------
//   MStyle
//---------------------------------------------------------

MStyle::MStyle()
      {
      d = new StyleData;
      }

MStyle::MStyle(const MStyle& s)
   : d(s.d)
      {
      }

MStyle::~MStyle()
      {
      }

MStyle& MStyle::operator=(const MStyle& s)
      {
      d = s.d;
      return *this;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void MStyle::set(StyleIdx id, const StyleVal& v)
      {
      d->_values[id] = v;
      }

//---------------------------------------------------------
//   TextStyle
//---------------------------------------------------------

const TextStyle& MStyle::textStyle(int idx) const
      {
      return d->textStyle(idx);
      }

const TextStyle& MStyle::textStyle(const QString& name) const
      {
      return d->textStyle(name);
      }

//---------------------------------------------------------
//   textStyleType
//---------------------------------------------------------

int MStyle::textStyleType(const QString& name) const
      {
      return d->textStyleType(name);
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void MStyle::setTextStyle(const TextStyle& ts)
      {
      d->setTextStyle(ts);
      }

//---------------------------------------------------------
//   addTextStyle
//---------------------------------------------------------

void MStyle::addTextStyle(const TextStyle& ts)
      {
      d->_textStyles.append(ts);
      }

//---------------------------------------------------------
//   removeTextStyle
//---------------------------------------------------------

void MStyle::removeTextStyle(const TextStyle& /*ts*/)
      {
      // TODO: d->_textStyles.append(ts);
      }

//---------------------------------------------------------
//   textStyles
//---------------------------------------------------------

const QList<TextStyle>& MStyle::textStyles() const
      {
      return d->_textStyles;
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void MStyle::set(StyleIdx t, Spatium val)
      {
      set(t, StyleVal(val));
      }

void MStyle::set(StyleIdx t, const QString& val)
      {
      set(t, StyleVal(val));
      }

void MStyle::set(StyleIdx t, bool val)
      {
      set(t, StyleVal(val));
      }

void MStyle::set(StyleIdx t, qreal val)
      {
      set(t, StyleVal(val));
      }

void MStyle::set(StyleIdx t, int val)
      {
      set(t, StyleVal(val));
      }

void MStyle::set(StyleIdx t, MScore::Direction val)
      {
      set(t, StyleVal(val));
      }

//---------------------------------------------------------
//   valueS
//---------------------------------------------------------

Spatium MStyle::valueS(StyleIdx idx) const
      {
      return value(idx).toSpatium();
      }

//---------------------------------------------------------
//   valueSt
//---------------------------------------------------------

QString MStyle::valueSt(StyleIdx idx) const
      {
      return value(idx).toString();
      }

//---------------------------------------------------------
//   valueB
//---------------------------------------------------------

bool MStyle::valueB(StyleIdx idx) const
      {
      return value(idx).toBool();
      }

//---------------------------------------------------------
//   valueD
//---------------------------------------------------------

qreal MStyle::valueD(StyleIdx idx) const
      {
      return value(idx).toDouble();
      }

//---------------------------------------------------------
//   valueI
//---------------------------------------------------------

int MStyle::valueI(StyleIdx idx) const
      {
      return value(idx).toInt();
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool MStyle::load(QFile* qf)
      {
      return d->load(qf);
      }

void MStyle::load(XmlReader& e)
      {
      d->load(e);
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void MStyle::save(Xml& xml, bool optimize)
      {
      d->save(xml, optimize);
      }

//---------------------------------------------------------
//   load
//    return true on success
//---------------------------------------------------------

bool StyleData::load(QFile* qf)
      {
      XmlReader e(qf);
      while (e.readNextStartElement()) {
            if (e.name() == "museScore") {
                  QString version = e.attribute("version");
                  QStringList sl  = version.split('.');
                  // _mscVersion  = sl[0].toInt() * 100 + sl[1].toInt();
                  while (e.readNextStartElement()) {
                        if (e.name() == "Style")
                              load(e);
                        else
                              e.unknown();
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   pageFormat
//---------------------------------------------------------

const PageFormat* MStyle::pageFormat() const
      {
      return d->pageFormat();
      }

//---------------------------------------------------------
//   setPageFormat
//---------------------------------------------------------

void MStyle::setPageFormat(const PageFormat& pf)
      {
      d->setPageFormat(pf);
      }

void StyleData::setPageFormat(const PageFormat& pf)
      {
      _pageFormat.copy(pf);
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal MStyle::spatium() const
      {
      return d->spatium();
      }

//---------------------------------------------------------
//   setSpatium
//---------------------------------------------------------

void MStyle::setSpatium(qreal v)
      {
      d->setSpatium(v);
      }

//---------------------------------------------------------
//   articulationAnchor
//---------------------------------------------------------

ArticulationAnchor MStyle::articulationAnchor(int id) const
      {
      return d->articulationAnchor(id);
      }

//---------------------------------------------------------
//   setArticulationAnchor
//---------------------------------------------------------

void MStyle::setArticulationAnchor(int id, ArticulationAnchor val)
      {
      return d->setArticulationAnchor(id, val);
      }

}

