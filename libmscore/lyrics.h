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

#ifndef __LYRICS_H__
#define __LYRICS_H__

#include "line.h"
#include "text.h"

// uncomment the following line to use the actual metrics of
// the font used by each lyrics rather than conventional values
//
// NOTE: CURRENTLY DOES NOT WORK (Font::tightBoundingBox() returns unusable values for glyphs not on base line)
//
//#define USE_FONT_DASH_METRIC

#if defined(USE_FONT_DASH_METRIC)
// the following line is used to turn the single font dash thickness value on or off
// when the other font dash parameters are on;
// the rationale is that the dash thickness is the most unreliable of the dash parameters
// retrievable from font metrics and it may make sense to use the other values but ignore this one.
//   #define USE_FONT_DASH_TICKNESS
#endif

//class QPainter;

namespace Ms {

//---------------------------------------------------------
//   @@ Lyrics
//   @P syllabic  Ms::Lyrics::Syllabic  (SINGLE, BEGIN, END, MIDDLE)
//---------------------------------------------------------

class LyricsLine;

class Lyrics : public Text {
      Q_OBJECT
      Q_PROPERTY(Ms::Lyrics::Syllabic syllabic READ syllabic WRITE setSyllabic)
      Q_ENUMS(Syllabic)

   public:
      enum class Syllabic : char { SINGLE, BEGIN, END, MIDDLE };

   private:
      int _ticks;             ///< if > 0 then draw an underline to tick() + _ticks
                              ///< (melisma)
      Syllabic _syllabic;
      LyricsLine* _separator;

   protected:
      int _no;                ///< row index
#if defined(USE_FONT_DASH_METRIC)
      qreal _dashY;           // dash dimensions for lyrics line dashes
      qreal _dashLength;
   #if defined (USE_FONT_DASH_TICKNESS)
      qreal _dashThickness;
   #endif
#endif

   public:
      Lyrics(Score* = 0);
      Lyrics(const Lyrics&);
      ~Lyrics();
      virtual Lyrics* clone() const override          { return new Lyrics(*this); }
      virtual Element::Type type() const override     { return Element::Type::LYRICS; }
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;

      Segment* segment() const                        { return (Segment*)parent()->parent(); }
      Measure* measure() const                        { return (Measure*)parent()->parent()->parent(); }
      ChordRest* chordRest() const                    { return (ChordRest*)parent(); }

      virtual void layout() override;
      virtual void layout1() override;

      virtual void write(Xml& xml) const override;
      virtual void read(XmlReader&) override;
      virtual int subtype() const override            { return _no; }
      virtual QString subtypeName() const override    { return tr("Verse %1").arg(_no + 1); }
      void setNo(int n);
      int no() const                                  { return _no; }
      void setSyllabic(Syllabic s)                    { _syllabic = s; }
      Syllabic syllabic() const                       { return _syllabic; }
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
//      virtual void draw(QPainter*) const override;
      virtual void endEdit() override;

      int ticks() const                               { return _ticks;    }
      void setTicks(int tick)                         { _ticks = tick;    }
      int endTick() const;
      bool isMelisma() const;
      void removeFromScore();

#if defined(USE_FONT_DASH_METRIC)
      qreal dashLength() const                        { return _dashLength;         }
      qreal dashY() const                             { return _dashY;              }
   #if defined (USE_FONT_DASH_TICKNESS)
      qreal dashThickness() const                     { return _dashThickness;      }
   #endif
#endif

      using Text::paste;
      void paste(MuseScoreView * scoreview);

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID id) const override;
      };

//---------------------------------------------------------
//   LyricsLine
//---------------------------------------------------------

class LyricsLine : public SLine {
      Q_OBJECT

   protected:
      Lyrics*     _nextLyrics;

   public:
      LyricsLine(Score* s);
      LyricsLine(const LyricsLine&);

      virtual LyricsLine* clone() const override     { return new LyricsLine(*this); }
      virtual Element::Type type() const override     { return Element::Type::LYRICSLINE; }
      virtual void layout();
      virtual LineSegment* createLineSegment() override;

      Lyrics*     lyrics() const                      { return (Lyrics*)parent();   }
      Lyrics*     nextLyrics() const                  { return _nextLyrics;         }
      void        unchain();
      };

//---------------------------------------------------------
//   LyricsLineSegment
//---------------------------------------------------------

class LyricsLineSegment : public LineSegment {
      Q_OBJECT

   protected:
      int         _numOfDashes;
      qreal       _dashLength;

public:
      LyricsLineSegment(Score* s);

      virtual LyricsLineSegment* clone() const override     { return new LyricsLineSegment(*this); }
      virtual Element::Type type() const override           { return Element::Type::LYRICSLINE_SEGMENT; }
      LyricsLine* lyricsLine() const                        { return (LyricsLine*)spanner(); }
      virtual void draw(QPainter*) const override;

      virtual void layout() override;
      };

}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Lyrics::Syllabic);

#endif

