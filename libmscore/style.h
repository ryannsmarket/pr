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

#ifndef __STYLE_H__
#define __STYLE_H__

#include "chordlist.h"

namespace Ms {

enum class Pid : int;
class XmlWriter;
struct ChordDescription;
class Element;
class Score;

//---------------------------------------------------------
//   Sid
//
//    Keep in sync with styleTypes[] in style.cpp
//---------------------------------------------------------

enum class Sid {
      NOSTYLE = -1,

      pageWidth,
      pageHeight,
      pagePrintableWidth,
      pageEvenLeftMargin,
      pageOddLeftMargin,
      pageEvenTopMargin,
      pageEvenBottomMargin,
      pageOddTopMargin,
      pageOddBottomMargin,
      pageTwosided,

      staffUpperBorder,
      staffLowerBorder,
      staffDistance,
      akkoladeDistance,

      minSystemDistance,
      maxSystemDistance,

      lyricsPlacement,
      lyricsPosAbove,
      lyricsPosBelow,
      lyricsMinTopDistance,
      lyricsMinBottomDistance,
      lyricsLineHeight,
      lyricsDashMinLength,
      lyricsDashMaxLength,
      lyricsDashMaxDistance,
      lyricsDashForce,
      lyricsAlignVerseNumber,
      lyricsLineThickness,
      lyricsMelismaAlign,
      lyricsMelismaPad,
      lyricsDashPad,
      lyricsDashLineThickness,
      lyricsDashYposRatio,

      lyricsOddFontFace,
      lyricsOddFontSize,
      lyricsOddFontStyle,
      lyricsOddAlign,
      lyricsOddFrameType,
      lyricsOddFramePadding,
      lyricsOddFrameWidth,
      lyricsOddFrameRound,
      lyricsOddFrameFgColor,
      lyricsOddFrameBgColor,

      lyricsEvenFontFace,
      lyricsEvenFontSize,
      lyricsEvenFontStyle,
      lyricsEvenAlign,
      lyricsEvenFrameType,
      lyricsEvenFramePadding,
      lyricsEvenFrameWidth,
      lyricsEvenFrameRound,
      lyricsEvenFrameFgColor,
      lyricsEvenFrameBgColor,

      figuredBassFontFamily,
//      figuredBassFontSize,
      figuredBassYOffset,
      figuredBassLineHeight,
      figuredBassAlignment,
      figuredBassStyle,
      systemFrameDistance,
      frameSystemDistance,
      minMeasureWidth,

      barWidth,
      doubleBarWidth,
      endBarWidth,
      doubleBarDistance,
      endBarDistance,
      repeatBarlineDotSeparation,
      repeatBarTips,
      startBarlineSingle,
      startBarlineMultiple,

      bracketWidth,
      bracketDistance,
      akkoladeWidth,
      akkoladeBarDistance,
      dividerLeft,
      dividerLeftSym,
      dividerLeftX,
      dividerLeftY,
      dividerRight,
      dividerRightSym,
      dividerRightX,
      dividerRightY,

      clefLeftMargin,
      keysigLeftMargin,
      ambitusMargin,
      timesigLeftMargin,
      timesigScale,

      clefKeyRightMargin,
      clefKeyDistance,
      clefTimesigDistance,
      keyTimesigDistance,
      keyBarlineDistance,
      systemHeaderDistance,
      systemHeaderTimeSigDistance,

      clefBarlineDistance,
      timesigBarlineDistance,

      stemWidth,
      shortenStem,
      shortStemProgression,
      shortestStem,
      beginRepeatLeftMargin,
      minNoteDistance,
      barNoteDistance,
      barAccidentalDistance,
      multiMeasureRestMargin,
      noteBarDistance,

      measureSpacing,
      staffLineWidth,
      ledgerLineWidth,
      ledgerLineLength,
      accidentalDistance,
      accidentalNoteDistance,
      beamWidth,
      beamDistance,
      beamMinLen,
      beamNoSlope,

      dotMag,
      dotNoteDistance,
      dotRestDistance,
      dotDotDistance,
      propertyDistanceHead,
      propertyDistanceStem,
      propertyDistance,
      articulationMag,
      articulationPosAbove,
      lastSystemFillLimit,

      hairpinPlacement,
      hairpinPosAbove,
      hairpinPosBelow,
      hairpinHeight,
      hairpinContHeight,
      hairpinLineWidth,
      hairpinFontFace,
      hairpinFontSize,
      hairpinFontStyle,
      hairpinTextAlign,
      hairpinFrameType,
      hairpinFramePadding,
      hairpinFrameWidth,
      hairpinFrameRound,
      hairpinFrameFgColor,
      hairpinFrameBgColor,

      pedalPlacement,
      pedalPosAbove,
      pedalPosBelow,
      pedalLineWidth,
      pedalLineStyle,
      pedalBeginTextOffset,
      pedalHookHeight,
      pedalFontFace,
      pedalFontSize,
      pedalFontStyle,
      pedalTextAlign,
      pedalFrameType,
      pedalFramePadding,
      pedalFrameWidth,
      pedalFrameRound,
      pedalFrameFgColor,
      pedalFrameBgColor,

      trillPlacement,
      trillPosAbove,
      trillPosBelow,

      vibratoPlacement,
      vibratoPosAbove,
      vibratoPosBelow,

      harmonyFretDist,
      minHarmonyDistance,
      maxHarmonyBarDistance,
      harmonyPlacement,

      chordSymbolPosAbove,
      chordSymbolPosBelow,

      chordSymbolAFontFace,
      chordSymbolAFontSize,
      chordSymbolAFontStyle,
      chordSymbolAAlign,
      chordSymbolAFrameType,
      chordSymbolAFramePadding,
      chordSymbolAFrameWidth,
      chordSymbolAFrameRound,
      chordSymbolAFrameFgColor,
      chordSymbolAFrameBgColor,

      chordSymbolBFontFace,
      chordSymbolBFontSize,
      chordSymbolBFontStyle,
      chordSymbolBAlign,
      chordSymbolBFrameType,
      chordSymbolBFramePadding,
      chordSymbolBFrameWidth,
      chordSymbolBFrameRound,
      chordSymbolBFrameFgColor,
      chordSymbolBFrameBgColor,

      capoPosition,
      fretNumMag,
      fretNumPos,
      fretY,
      fretMinDistance,
      fretMag,
      fretPlacement,
      fretStrings,
      fretFrets,
      fretOffset,
      fretBarre,

      showPageNumber,
      showPageNumberOne,
      pageNumberOddEven,
      showMeasureNumber,
      showMeasureNumberOne,
      measureNumberInterval,
      measureNumberSystem,

      measureNumberAllStaffs,
      smallNoteMag,
      graceNoteMag,
      smallStaffMag,
      smallClefMag,
      genClef,
      genKeysig,
      genCourtesyTimesig,
      genCourtesyKeysig,
      genCourtesyClef,

      swingRatio,
      swingUnit,

      useStandardNoteNames,
      useGermanNoteNames,
      useFullGermanNoteNames,
      useSolfeggioNoteNames,
      useFrenchNoteNames,
      automaticCapitalization,
      lowerCaseMinorChords,
      lowerCaseBassNotes,
      allCapsNoteNames,
      chordStyle,
      chordsXmlFile,
      chordDescriptionFile,
      concertPitch,
      createMultiMeasureRests,
      minEmptyMeasures,
      minMMRestWidth,
      hideEmptyStaves,
      dontHideStavesInFirstSystem,
      hideInstrumentNameIfOneInstrument,
      gateTime,
      tenutoGateTime,
      staccatoGateTime,
      slurGateTime,

      ArpeggioNoteDistance,
      ArpeggioLineWidth,
      ArpeggioHookLen,
      ArpeggioHiddenInStdIfTab,

      SlurEndWidth,
      SlurMidWidth,
      SlurDottedWidth,
      MinTieLength,
      SlurMinDistance,

      SectionPause,
      MusicalSymbolFont,
      MusicalTextFont,

      showHeader,
      headerFirstPage,
      headerOddEven,
      evenHeaderL,
      evenHeaderC,
      evenHeaderR,
      oddHeaderL,
      oddHeaderC,
      oddHeaderR,

      showFooter,
      footerFirstPage,
      footerOddEven,
      evenFooterL,
      evenFooterC,
      evenFooterR,
      oddFooterL,
      oddFooterC,
      oddFooterR,

      voltaPosAbove,
      voltaHook,
      voltaLineWidth,
      voltaLineStyle,
      voltaFontFace,
      voltaFontSize,
      voltaFontStyle,
      voltaAlign,
      voltaOffset,
      voltaFrameType,
      voltaFramePadding,
      voltaFrameWidth,
      voltaFrameRound,
      voltaFrameFgColor,
      voltaFrameBgColor,

      ottava8VAPlacement,
      ottava8VBPlacement,
      ottava15MAPlacement,
      ottava15MBPlacement,
      ottava22MAPlacement,
      ottava22MBPlacement,

      ottava8VAText,
      ottava8VBText,
      ottava15MAText,
      ottava15MBText,
      ottava22MAText,
      ottava22MBText,

      ottava8VAnoText,
      ottava8VBnoText,
      ottava15MAnoText,
      ottava15MBnoText,
      ottava22MAnoText,
      ottava22MBnoText,

      ottavaPosAbove,
      ottavaPosBelow,
      ottavaHookAbove,
      ottavaHookBelow,
      ottavaLineWidth,
      ottavaLineStyle,
      ottavaNumbersOnly,
      ottavaFontFace,
      ottavaFontSize,
      ottavaFontStyle,
      ottavaTextAlign,
      ottavaFrameType,
      ottavaFramePadding,
      ottavaFrameWidth,
      ottavaFrameRound,
      ottavaFrameFgColor,
      ottavaFrameBgColor,

      tabClef,

      tremoloWidth,
      tremoloBoxHeight,
      tremoloStrokeWidth,
      tremoloDistance,
      // TODO tremoloBeamLengthMultiplier,
      // TODO tremoloMaxBeamLength,

      linearStretch,
      crossMeasureValues,
      keySigNaturals,

      tupletMaxSlope,
      tupletOufOfStaff,
      tupletVHeadDistance,
      tupletVStemDistance,
      tupletStemLeftDistance,
      tupletStemRightDistance,
      tupletNoteLeftDistance,
      tupletNoteRightDistance,
      tupletBracketWidth,
      tupletDirection,
      tupletNumberType,
      tupletBracketType,
      tupletFontFace,
      tupletFontSize,
      tupletFontStyle,
      tupletAlign,
      tupletBracketHookHeight,
      tupletOffset,
      tupletFrameType,
      tupletFramePadding,
      tupletFrameWidth,
      tupletFrameRound,
      tupletFrameFgColor,
      tupletFrameBgColor,

      barreLineWidth,
      scaleBarlines,
      barGraceDistance,

      minVerticalDistance,
      ornamentStyle,
      spatium,

      autoplaceHairpinDynamicsDistance,

      dynamicsPlacement,
      dynamicsPosAbove,
      dynamicsPosBelow,

      dynamicsMinDistance,
      autoplaceVerticalAlignRange,

      textLinePlacement,
      textLinePosAbove,
      textLinePosBelow,
      textLineFrameType,
      textLineFramePadding,
      textLineFrameWidth,
      textLineFrameRound,
      textLineFrameFgColor,
      textLineFrameBgColor,

      tremoloBarLineWidth,
      jumpPosAbove,
      markerPosAbove,

      defaultFontFace,
      defaultFontSize,
      defaultFontSpatiumDependent,
      defaultFontStyle,
      defaultAlign,
      defaultFrameType,
      defaultFramePadding,
      defaultFrameWidth,
      defaultFrameRound,
      defaultFrameFgColor,
      defaultFrameBgColor,
      defaultOffset,
      defaultOffsetType,
      defaultSystemFlag,
      defaultText,

      titleFontFace,
      titleFontSize,
      titleFontSpatiumDependent,
      titleFontStyle,
      titleAlign,
      titleOffset,
      titleOffsetType,
      titleFrameType,
      titleFramePadding,
      titleFrameWidth,
      titleFrameRound,
      titleFrameFgColor,
      titleFrameBgColor,

      subTitleFontFace,
      subTitleFontSize,
      subTitleFontSpatiumDependent,
      subTitleFontStyle,
      subTitleAlign,
      subTitleOffset,
      subTitleOffsetType,
      subTitleFrameType,
      subTitleFramePadding,
      subTitleFrameWidth,
      subTitleFrameRound,
      subTitleFrameFgColor,
      subTitleFrameBgColor,

      composerFontFace,
      composerFontSize,
      composerFontSpatiumDependent,
      composerFontStyle,
      composerAlign,
      composerOffset,
      composerOffsetType,
      composerFrameType,
      composerFramePadding,
      composerFrameWidth,
      composerFrameRound,
      composerFrameFgColor,
      composerFrameBgColor,

      lyricistFontFace,
      lyricistFontSize,
      lyricistFontSpatiumDependent,
      lyricistFontStyle,
      lyricistAlign,
      lyricistOffset,
      lyricistOffsetType,
      lyricistFrameType,
      lyricistFramePadding,
      lyricistFrameWidth,
      lyricistFrameRound,
      lyricistFrameFgColor,
      lyricistFrameBgColor,

      fingeringFontFace,
      fingeringFontSize,
      fingeringFontStyle,
      fingeringAlign,
      fingeringFrameType,
      fingeringFramePadding,
      fingeringFrameWidth,
      fingeringFrameRound,
      fingeringFrameFgColor,
      fingeringFrameBgColor,
      fingeringOffset,

      lhGuitarFingeringFontFace,
      lhGuitarFingeringFontSize,
      lhGuitarFingeringFontStyle,
      lhGuitarFingeringAlign,
      lhGuitarFingeringFrameType,
      lhGuitarFingeringFramePadding,
      lhGuitarFingeringFrameWidth,
      lhGuitarFingeringFrameRound,
      lhGuitarFingeringFrameFgColor,
      lhGuitarFingeringFrameBgColor,
      lhGuitarFingeringOffset,

      rhGuitarFingeringFontFace,
      rhGuitarFingeringFontSize,
      rhGuitarFingeringFontStyle,
      rhGuitarFingeringAlign,
      rhGuitarFingeringFrameType,
      rhGuitarFingeringFramePadding,
      rhGuitarFingeringFrameWidth,
      rhGuitarFingeringFrameRound,
      rhGuitarFingeringFrameFgColor,
      rhGuitarFingeringFrameBgColor,
      rhGuitarFingeringOffset,

      stringNumberFontFace,
      stringNumberFontSize,
      stringNumberFontStyle,
      stringNumberAlign,
      stringNumberFrameType,
      stringNumberFramePadding,
      stringNumberFrameWidth,
      stringNumberFrameRound,
      stringNumberFrameFgColor,
      stringNumberFrameBgColor,
      stringNumberOffset,

      longInstrumentFontFace,
      longInstrumentFontSize,
      longInstrumentFontStyle,
      longInstrumentAlign,
      longInstrumentOffset,
      longInstrumentFrameType,
      longInstrumentFramePadding,
      longInstrumentFrameWidth,
      longInstrumentFrameRound,
      longInstrumentFrameFgColor,
      longInstrumentFrameBgColor,

      shortInstrumentFontFace,
      shortInstrumentFontSize,
      shortInstrumentFontStyle,
      shortInstrumentAlign,
      shortInstrumentOffset,
      shortInstrumentFrameType,
      shortInstrumentFramePadding,
      shortInstrumentFrameWidth,
      shortInstrumentFrameRound,
      shortInstrumentFrameFgColor,
      shortInstrumentFrameBgColor,

      partInstrumentFontFace,
      partInstrumentFontSize,
      partInstrumentFontStyle,
      partInstrumentAlign,
      partInstrumentOffset,
      partInstrumentFrameType,
      partInstrumentFramePadding,
      partInstrumentFrameWidth,
      partInstrumentFrameRound,
      partInstrumentFrameFgColor,
      partInstrumentFrameBgColor,

      dynamicsFontFace,
      dynamicsFontSize,
      dynamicsFontStyle,
      dynamicsAlign,
      dynamicsFrameType,
      dynamicsFramePadding,
      dynamicsFrameWidth,
      dynamicsFrameRound,
      dynamicsFrameFgColor,
      dynamicsFrameBgColor,

      expressionFontFace,
      expressionFontSize,
      expressionFontStyle,
      expressionAlign,
      expressionPlacement,
      expressionOffset,
      expressionFrameType,
      expressionFramePadding,
      expressionFrameWidth,
      expressionFrameRound,
      expressionFrameFgColor,
      expressionFrameBgColor,

      tempoFontFace,
      tempoFontSize,
      tempoFontStyle,
      tempoAlign,
      tempoSystemFlag,
      tempoPlacement,
      tempoPosAbove,
      tempoPosBelow,
      tempoMinDistance,
      tempoFrameType,
      tempoFramePadding,
      tempoFrameWidth,
      tempoFrameRound,
      tempoFrameFgColor,
      tempoFrameBgColor,

      metronomeFontFace,
      metronomeFontSize,
      metronomeFontStyle,
      metronomePlacement,
      metronomeAlign,
      metronomeOffset,
      metronomeFrameType,
      metronomeFramePadding,
      metronomeFrameWidth,
      metronomeFrameRound,
      metronomeFrameFgColor,
      metronomeFrameBgColor,

      measureNumberFontFace,
      measureNumberFontSize,
      measureNumberFontStyle,
      measureNumberOffset,
      measureNumberOffsetType,
      measureNumberAlign,
      measureNumberFrameType,
      measureNumberFramePadding,
      measureNumberFrameWidth,
      measureNumberFrameRound,
      measureNumberFrameFgColor,
      measureNumberFrameBgColor,

      translatorFontFace,
      translatorFontSize,
      translatorFontStyle,
      translatorAlign,
      translatorOffset,
      translatorFrameType,
      translatorFramePadding,
      translatorFrameWidth,
      translatorFrameRound,
      translatorFrameFgColor,
      translatorFrameBgColor,

      systemTextFontFace,
      systemTextFontSize,
      systemTextFontStyle,
      systemTextAlign,
      systemTextOffsetType,
      systemTextPlacement,
      systemTextPosAbove,
      systemTextPosBelow,
      systemTextMinDistance,
      systemTextFrameType,
      systemTextFramePadding,
      systemTextFrameWidth,
      systemTextFrameRound,
      systemTextFrameFgColor,
      systemTextFrameBgColor,

      staffTextFontFace,
      staffTextFontSize,
      staffTextFontStyle,
      staffTextAlign,
      staffTextOffsetType,
      staffTextPlacement,
      staffTextPosAbove,
      staffTextPosBelow,
      staffTextMinDistance,
      staffTextFrameType,
      staffTextFramePadding,
      staffTextFrameWidth,
      staffTextFrameRound,
      staffTextFrameFgColor,
      staffTextFrameBgColor,

      rehearsalMarkFontFace,
      rehearsalMarkFontSize,
      rehearsalMarkFontStyle,
      rehearsalMarkAlign,
      rehearsalMarkFrameType,
      rehearsalMarkFramePadding,
      rehearsalMarkFrameWidth,
      rehearsalMarkFrameRound,
      rehearsalMarkFrameFgColor,
      rehearsalMarkFrameBgColor,
      rehearsalMarkPlacement,
      rehearsalMarkPosAbove,
      rehearsalMarkPosBelow,
      rehearsalMarkMinDistance,

      repeatLeftFontFace,
      repeatLeftFontSize,
      repeatLeftFontStyle,
      repeatLeftAlign,
      repeatLeftPlacement,
      repeatLeftFrameType,
      repeatLeftFramePadding,
      repeatLeftFrameWidth,
      repeatLeftFrameRound,
      repeatLeftFrameFgColor,
      repeatLeftFrameBgColor,

      repeatRightFontFace,
      repeatRightFontSize,
      repeatRightFontStyle,
      repeatRightAlign,
      repeatRightPlacement,
      repeatRightFrameType,
      repeatRightFramePadding,
      repeatRightFrameWidth,
      repeatRightFrameRound,
      repeatRightFrameFgColor,
      repeatRightFrameBgColor,

      frameFontFace,
      frameFontSize,
      frameFontStyle,
      frameAlign,
      frameOffset,
      frameFrameType,
      frameFramePadding,
      frameFrameWidth,
      frameFrameRound,
      frameFrameFgColor,
      frameFrameBgColor,

      textLineFontFace,
      textLineFontSize,
      textLineFontStyle,
      textLineTextAlign,

      glissandoFontFace,
      glissandoFontSize,
      glissandoFontStyle,
      glissandoAlign,
      glissandoOffset,
      glissandoFrameType,
      glissandoFramePadding,
      glissandoFrameWidth,
      glissandoFrameRound,
      glissandoFrameFgColor,
      glissandoFrameBgColor,
      glissandoLineWidth,
      glissandoText,

      bendFontFace,
      bendFontSize,
      bendFontStyle,
      bendAlign,
      bendOffset,
      bendFrameType,
      bendFramePadding,
      bendFrameWidth,
      bendFrameRound,
      bendFrameFgColor,
      bendFrameBgColor,
      bendLineWidth,
      bendArrowWidth,

      headerFontFace,
      headerFontSize,
      headerFontStyle,
      headerAlign,
      headerOffset,
      headerFrameType,
      headerFramePadding,
      headerFrameWidth,
      headerFrameRound,
      headerFrameFgColor,
      headerFrameBgColor,

      footerFontFace,
      footerFontSize,
      footerFontStyle,
      footerAlign,
      footerOffset,
      footerFrameType,
      footerFramePadding,
      footerFrameWidth,
      footerFrameRound,
      footerFrameFgColor,
      footerFrameBgColor,

      instrumentChangeFontFace,
      instrumentChangeFontSize,
      instrumentChangeFontStyle,
      instrumentChangeAlign,
      instrumentChangeOffset,
      instrumentChangePlacement,
      instrumentChangePosAbove,
      instrumentChangePosBelow,
      instrumentChangeMinDistance,
      instrumentChangeFrameType,
      instrumentChangeFramePadding,
      instrumentChangeFrameWidth,
      instrumentChangeFrameRound,
      instrumentChangeFrameFgColor,
      instrumentChangeFrameBgColor,

      figuredBassFontFace,
      figuredBassFontSize,
      figuredBassFontStyle,

      user1FontFace,
      user1FontSize,
      user1FontStyle,
      user1Align,
      user1Offset,
      user1OffsetType,
      user1FrameType,
      user1FramePadding,
      user1FrameWidth,
      user1FrameRound,
      user1FrameFgColor,
      user1FrameBgColor,

      user2FontFace,
      user2FontSize,
      user2FontStyle,
      user2Align,
      user2Offset,
      user2OffsetType,
      user2FrameType,
      user2FramePadding,
      user2FrameWidth,
      user2FrameRound,
      user2FrameFgColor,
      user2FrameBgColor,

      user3FontFace,
      user3FontSize,
      user3FontStyle,
      user3Align,
      user3Offset,
      user3OffsetType,
      user3FrameType,
      user3FramePadding,
      user3FrameWidth,
      user3FrameRound,
      user3FrameFgColor,
      user3FrameBgColor,

      user4FontFace,
      user4FontSize,
      user4FontStyle,
      user4Align,
      user4Offset,
      user4OffsetType,
      user4FrameType,
      user4FramePadding,
      user4FrameWidth,
      user4FrameRound,
      user4FrameFgColor,
      user4FrameBgColor,

      user5FontFace,
      user5FontSize,
      user5FontStyle,
      user5Align,
      user5Offset,
      user5OffsetType,
      user5FrameType,
      user5FramePadding,
      user5FrameWidth,
      user5FrameRound,
      user5FrameFgColor,
      user5FrameBgColor,

      user6FontFace,
      user6FontSize,
      user6FontStyle,
      user6Align,
      user6Offset,
      user6OffsetType,
      user6FrameType,
      user6FramePadding,
      user6FrameWidth,
      user6FrameRound,
      user6FrameFgColor,
      user6FrameBgColor,

      letRingFontFace,
      letRingFontSize,
      letRingFontStyle,
      letRingTextAlign,
      letRingHookHeight,
      letRingPlacement,
      letRingPosAbove,
      letRingPosBelow,
      letRingLineWidth,
      letRingLineStyle,
      letRingBeginTextOffset,
      letRingText,
      letRingFrameType,
      letRingFramePadding,
      letRingFrameWidth,
      letRingFrameRound,
      letRingFrameFgColor,
      letRingFrameBgColor,

      palmMuteFontFace,
      palmMuteFontSize,
      palmMuteFontStyle,
      palmMuteTextAlign,
      palmMuteHookHeight,
      palmMutePlacement,
      palmMutePosAbove,
      palmMutePosBelow,
      palmMuteLineWidth,
      palmMuteLineStyle,
      palmMuteBeginTextOffset,
      palmMuteText,
      palmMuteFrameType,
      palmMuteFramePadding,
      palmMuteFrameWidth,
      palmMuteFrameRound,
      palmMuteFrameFgColor,
      palmMuteFrameBgColor,

      fermataPosAbove,
      fermataPosBelow,
      fermataMinDistance,

      STYLES
      };

//-------------------------------------------------------------------
//   Tid
//    Enumerate the list of built-in text substyles
//    must be in sync with textStyles array
//-------------------------------------------------------------------

enum class Tid {
      DEFAULT,
      TITLE,
      SUBTITLE,
      COMPOSER,
      POET,
      LYRICS_ODD,
      LYRICS_EVEN,
      FINGERING,
      LH_GUITAR_FINGERING,
      RH_GUITAR_FINGERING,
      STRING_NUMBER,
      INSTRUMENT_LONG,
      INSTRUMENT_SHORT,
      INSTRUMENT_EXCERPT,
      DYNAMICS,
      EXPRESSION,
      TEMPO,
      METRONOME,
      MEASURE_NUMBER,
      TRANSLATOR,
      TUPLET,
      SYSTEM,
      STAFF,
      HARMONY_A,
      HARMONY_B,
      REHEARSAL_MARK,
      REPEAT_LEFT,       // align to start of measure
      REPEAT_RIGHT,      // align to end of measure
      FRAME,
      TEXTLINE,
      GLISSANDO,
      OTTAVA,
      VOLTA,
      PEDAL,
      LET_RING,
      PALM_MUTE,
      HAIRPIN,
      BEND,
      HEADER,
      FOOTER,
      INSTRUMENT_CHANGE,
      USER1,
      USER2,
      USER3,
      USER4,
      USER5,
      USER6,
      TEXT_STYLES
      };

//---------------------------------------------------------
//   MStyle
//    the name "Style" gives problems with some microsoft
//    header files...
//---------------------------------------------------------

class MStyle {
      std::array<QVariant, int(Sid::STYLES)> _values;
      std::array<qreal, int(Sid::STYLES)> _precomputedValues;

      ChordList _chordList;
      bool _customChordList;        // if true, chordlist will be saved as part of score

   public:
      MStyle();

      void precomputeValues();
      QVariant value(Sid idx) const;
      qreal pvalue(Sid idx) const    { return _precomputedValues[int(idx)]; }
      void set(Sid idx, const QVariant& v);

      bool isDefault(Sid idx) const;

      const ChordDescription* chordDescription(int id) const;
      ChordList* chordList()  { return &_chordList; }
      void setChordList(ChordList*, bool custom = true);    // Style gets ownership of ChordList
      void setCustomChordList(bool t) { _customChordList = t; }

      bool load(QFile* qf);
      void load(XmlReader& e);
      void save(XmlWriter& xml, bool optimize);
      bool readProperties(XmlReader&);
      bool readStyleValCompat(XmlReader&);
      bool readTextStyleValCompat(XmlReader&);

      void reset(Score*);

      static const char* valueType(const Sid);
      static const char* valueName(const Sid);
      static Sid styleIdx(const QString& name);
      };

//---------------------------------------------------------
//   StyledProperty
//---------------------------------------------------------

struct StyledProperty {
      Sid sid;
      Pid pid;
      };

typedef std::vector<StyledProperty> ElementStyle;

#define TEXT_STYLE_SIZE 12

typedef std::array<StyledProperty, TEXT_STYLE_SIZE> TextStyle;


const TextStyle* textStyle(Tid);
const TextStyle* textStyle(const char*);

const char* textStyleName(Tid);
QString textStyleUserName(Tid);
Tid textStyleFromName(const QString&);

const std::vector<Tid>& allTextStyles();
const std::vector<Tid>& primaryTextStyles();

#ifndef NDEBUG
extern void checkStyles();
#endif

}     // namespace Ms


#endif
