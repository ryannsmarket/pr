//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "mscore.h"
#include "spatium.h"
#include "fraction.h"
#include "scoreElement.h"
#include "shape.h"

class QPainter;

namespace Ms {

/**
 \file
 Definition of classes Element, ElementList, StaffLines.
*/


class Xml;
class Measure;
class Staff;
class Part;
class Score;
class Sym;
class MuseScoreView;
class Segment;
class TextStyle;
class Element;
class Annotation;
class BarLine;
class Articulation;
class Marker;
class Clef;
class KeySig;
class TimeSig;
class TempoText;
class Breath;
class Box;
class HBox;
class VBox;
class TBox;
class FBox;
class ChordRest;
class Slur;
class Tie;
class Glissando;
class SystemDivider;
class RehearsalMark;
class Harmony;
class Volta;
class Jump;
class StaffText;
class Ottava;
class Note;
class Chord;
class Rest;
class LayoutBreak;
class Tremolo;
class System;
class Lyrics;
class Stem;
class SlurSegment;
class OttavaSegment;
class Beam;
class Hook;
class StemSlash;
class Spacer;
class StaffLines;
class Ambitus;
class Bracket;
class InstrumentChange;
class Text;
class Hairpin;
class HairpinSegment;
class Bend;
class RepeatMeasure;
class Tuplet;
class NoteDot;
class Dynamic;
class InstrumentName;
class DurationElement;
class Accidental;
class TextLine;
class Pedal;

enum class SymId;

//---------------------------------------------------------
//   Grip
//---------------------------------------------------------

enum class Grip : signed char {
      NO_GRIP = -1,
      START = 0, END = 1,                         // arpeggio etc.
          MIDDLE = 2, APERTURE = 3,               // Line
      /*START, END , */
          BEZIER1 = 2, SHOULDER = 3, BEZIER2 = 4, DRAG = 5, // Slur
      GRIPS = 6                     // number of grips for slur
      };

//---------------------------------------------------------
//   ElementFlag
//---------------------------------------------------------

enum class ElementFlag : char {
      DROP_TARGET  = 0x2,
      SELECTABLE   = 0x4,
      MOVABLE      = 0x8,
      SEGMENT      = 0x10,
      HAS_TAG      = 0x20,
      ON_STAFF     = 0x40   // parent is Segment() type
      };

typedef QFlags<ElementFlag> ElementFlags;
Q_DECLARE_OPERATORS_FOR_FLAGS(ElementFlags);

//---------------------------------------------------------
//   DropData
//---------------------------------------------------------

struct DropData {
      MuseScoreView* view;
      QPointF pos;
      QPointF dragOffset;
      Element* element;
      Qt::KeyboardModifiers modifiers;
      Fraction duration;

      bool control() const { return modifiers & Qt::ControlModifier; }
      DropData();
      };

//---------------------------------------------------------
//   EditData
//    used in editDrag
//---------------------------------------------------------

struct EditData {
      MuseScoreView* view;
      Grip curGrip;
      QPointF startMove;
      QPointF pos;
      QPointF lastPos;
      QPointF delta;
      bool hRaster;
      bool vRaster;
      };

//---------------------------------------------------------
//   ElementName
//---------------------------------------------------------

struct ElementName {
      const char* name;
      const char* userName;
      ElementName(const char* _name, const char* _userName) : name(_name), userName(_userName) {}
      };

//-------------------------------------------------------------------
//    @@ Element
///     \brief Base class of score layout elements
///
///     The Element class is the virtual base class of all
///     score layout elements.
//
//    @P bbox       rect                  bounding box relative to pos and userOff (read only)
//    @P color      color                 element drawing color
//    @P generated  bool                  true if the element has been generated by layout
//    @P pagePos    point                 position in page coordinated (read only)
//    @P parent     Element               the parent element in drawing hierarchy
//    @P placement  enum (Element.ABOVE, Element.BELOW)
//    @P pos        point                 position relative to parent
//    @P selected   bool                  true if the element is currently selected
//    @P track      int                   the track the elment belongs to
//    @P type       enum (Element.ACCIDENTAL, .AMBITUS, .ANNOTATION, .ARPEGGIO, .BAGPIPE_EMBELLISHMENT, .BAR_LINE, .BEAM, .BEND, .BRACKET, .BREATH, .CHORD, .CHORDLINE, .CLEF, .COMPOUND, .DYNAMIC, .ELEMENT, .ELEMENT_LIST, .FBOX, .FIGURED_BASS, .FINGERING, .FRET_DIAGRAM, .FSYMBOL, .GLISSANDO, .GLISSANDO_SEGMENT, .HAIRPIN, .HAIRPIN_SEGMENT, .HARMONY, .HBOX, .HOOK, .ICON, .IMAGE, .INSTRUMENT_CHANGE, .INSTRUMENT_NAME, .JUMP, .KEYSIG, .LASSO, .LAYOUT_BREAK, .LEDGER_LINE, .LINE, .LYRICS, .LYRICSLINE, .LYRICSLINE_SEGMENT, .MARKER, .MEASURE, .MEASURE_LIST, .NOTE, .NOTEDOT, .NOTEHEAD, .NOTELINE, .OSSIA, .OTTAVA, .OTTAVA_SEGMENT, .PAGE, .PEDAL, .PEDAL_SEGMENT, .REHEARSAL_MARK, .REPEAT_MEASURE, .REST, .SEGMENT, .SELECTION, .SHADOW_NOTE, .SLUR, .SLUR_SEGMENT, .SPACER, .STAFF_LINES, .STAFF_LIST, .STAFF_STATE, .STAFF_TEXT, .STEM, .STEM_SLASH, .SYMBOL, .SYSTEM, .TAB_DURATION_SYMBOL, .TBOX, .TEMPO_TEXT, .TEXT, .TEXTLINE, .TEXTLINE_SEGMENT, .TIE, .TIMESIG, .TREMOLO, .TREMOLOBAR, .TRILL, .TRILL_SEGMENT, .TUPLET, .VBOX, .VOLTA, .VOLTA_SEGMENT) (read only)
//    @P userOff    point                 manual offset to position determined by layout
//    @P visible    bool
//-------------------------------------------------------------------

class Element : public QObject, public ScoreElement {
      Q_OBJECT
      Q_ENUMS(Type)
      Q_ENUMS(Placement)

      Q_PROPERTY(QRectF                   bbox        READ scriptBbox )
      Q_PROPERTY(QColor                   color       READ color        WRITE undoSetColor)
      Q_PROPERTY(bool                     generated   READ generated    WRITE setGenerated)
      Q_PROPERTY(QPointF                  pagePos     READ scriptPagePos)
      Q_PROPERTY(Ms::Element*             parent      READ parent       WRITE setParent)
      Q_PROPERTY(Ms::Element::Placement   placement   READ placement    WRITE undoSetPlacement)
      Q_PROPERTY(QPointF                  pos         READ scriptPos    WRITE scriptSetPos)
      Q_PROPERTY(bool                     selected    READ selected     WRITE setSelected)
      Q_PROPERTY(qreal                    spatium     READ spatium)
      Q_PROPERTY(int                      track       READ track        WRITE setTrack)
      Q_PROPERTY(Ms::Element::Type        type        READ type)
      Q_PROPERTY(QPointF                  userOff     READ scriptUserOff WRITE scriptSetUserOff)
      Q_PROPERTY(bool                     visible     READ visible      WRITE undoSetVisible)

      Element* _parent { 0 };

      bool _generated;            ///< automatically generated Element

  protected:
      bool _selected;             ///< set if element is selected
      bool _visible;              ///< visibility attribute
      QColor _color;              ///< element color attribute

  public:
      //-------------------------------------------------------------------
      //    The value of this enum determines the "stacking order"
      //    of elements on the canvas.
      //   Note: keep in sync with array elementNames[] in element.cpp
      //-------------------------------------------------------------------
      enum class Type : char {
            INVALID = 0,
            SYMBOL,
            TEXT,
            INSTRUMENT_NAME,
            SLUR_SEGMENT,
            STAFF_LINES,
            BAR_LINE,
            SYSTEM_DIVIDER,
            STEM_SLASH,
            LINE,
            ARPEGGIO,
            ACCIDENTAL,
            ANNOTATION,

            STEM,             // list STEM before NOTE: notes in TAB might 'break' stems
            NOTE,             // and this requires stems to be drawn before notes
            CLEF,             // elements from CLEF to TIMESIG need to be in the order
            KEYSIG,           // in which they appear in a measure
            AMBITUS,
            TIMESIG,
            REST,
            BREATH,

            REPEAT_MEASURE,
            IMAGE,
            TIE,
            ARTICULATION,
            CHORDLINE,
            DYNAMIC,
            BEAM,
            HOOK,
            LYRICS,
            FIGURED_BASS,

            MARKER,
            JUMP,
            FINGERING,
            TUPLET,
            TEMPO_TEXT,
            STAFF_TEXT,
            REHEARSAL_MARK,
            INSTRUMENT_CHANGE,
            HARMONY,
            FRET_DIAGRAM,
            BEND,
            TREMOLOBAR,
            VOLTA,
            HAIRPIN_SEGMENT,
            OTTAVA_SEGMENT,
            TRILL_SEGMENT,
            TEXTLINE_SEGMENT,
            VOLTA_SEGMENT,
            PEDAL_SEGMENT,
            LYRICSLINE_SEGMENT,
            GLISSANDO_SEGMENT,
            LAYOUT_BREAK,
            SPACER,
            STAFF_STATE,
            LEDGER_LINE,
            NOTEHEAD,
            NOTEDOT,
            TREMOLO,
            MEASURE,
            SELECTION,
            LASSO,
            SHADOW_NOTE,
            TAB_DURATION_SYMBOL,
            FSYMBOL,
            PAGE,
            HAIRPIN,
            OTTAVA,
            PEDAL,
            TRILL,
            TEXTLINE,
            NOTELINE,
            LYRICSLINE,
            GLISSANDO,
            BRACKET,

            SEGMENT,
            SYSTEM,
            COMPOUND,
            CHORD,
            SLUR,
            ELEMENT,
            ELEMENT_LIST,
            STAFF_LIST,
            MEASURE_LIST,
            HBOX,
            VBOX,
            TBOX,
            FBOX,
            ICON,
            OSSIA,
            BAGPIPE_EMBELLISHMENT,

            MAXTYPE
            };

      enum class Placement : char {
            ABOVE, BELOW
            };

  private:
      Placement _placement;

      mutable ElementFlags _flags;
      bool _autoplace;

      int _track;                 ///< staffIdx * VOICES + voice
      qreal _mag;                 ///< standard magnification (derived value)

      QPointF _pos;               ///< Reference position, relative to _parent.
      QPointF _userOff;           ///< offset from normal layout position:
                                  ///< user dragged object this amount.
      QPointF _readPos;

      mutable QRectF _bbox;       ///< Bounding box relative to _pos + _userOff
                                  ///< valid after call to layout()
      uint _tag;                  ///< tag bitmask

   protected:
      QPointF _startDragPosition;   ///< used during drag

   public:
      Element(Score* s = 0);
      Element(const Element&);
      virtual ~Element() {}

      Element &operator=(const Element&) = delete;
      //@ create a copy of the element
      Q_INVOKABLE virtual Ms::Element* clone() const = 0;
      virtual Element* linkedClone();

      Element* parent() const                 { return _parent;     }
      void setParent(Element* e)              { _parent = e;        }
      Element* findMeasure();

      qreal spatium() const;

      bool selected() const                   { return _selected;   }
      virtual void setSelected(bool f)        { _selected = f;      }

      bool visible() const                    { return _visible;    }
      virtual void setVisible(bool f)         { _visible = f;       }

      Placement placement() const             { return _placement;  }
      void setPlacement(Placement val)        { _placement = val;   }
      void undoSetPlacement(Placement val);

      bool generated() const                  { return _generated;  }
      void setGenerated(bool val)             { _generated = val;   }

      const QPointF& ipos() const             { return _pos;                    }
      virtual const QPointF pos() const       { return _pos + _userOff;         }
      virtual qreal x() const                 { return _pos.x() + _userOff.x(); }
      virtual qreal y() const                 { return _pos.y() + _userOff.y(); }
      void setPos(qreal x, qreal y)           { _pos.rx() = x, _pos.ry() = y;   }
      void setPos(const QPointF& p)           { _pos = p;                }
      qreal& rxpos()                          { return _pos.rx();        }
      qreal& rypos()                          { return _pos.ry();        }
      virtual void move(const QPointF& s)     { _pos += s;               }

      virtual QPointF pagePos() const;          ///< position in page coordinates
      virtual QPointF canvasPos() const;        ///< position in canvas coordinates
      qreal pageX() const;
      qreal canvasX() const;

      const QPointF& userOff() const             { return _userOff;  }
      virtual void setUserOff(const QPointF& o)  { _userOff = o;     }
      void setUserXoffset(qreal v)               { _userOff.setX(v); }
      void setUserYoffset(qreal v)               { _userOff.setY(v); }

      qreal& rUserXoffset()                   { return _userOff.rx(); }
      qreal& rUserYoffset()                   { return _userOff.ry(); }

      // function versions for scripts: use coords in spatium units rather than raster
      // and route pos changes to userOff
      QRectF scriptBbox() const;
      virtual QPointF scriptPagePos() const;
      virtual QPointF scriptPos() const;
      void scriptSetPos(const QPointF& p);
      QPointF scriptUserOff() const;
      void scriptSetUserOff(const QPointF& o);

      bool isNudged() const                   { return !(_readPos.isNull() && _userOff.isNull()); }
      const QPointF& readPos() const          { return _readPos;   }
      void setReadPos(const QPointF& p)       { _readPos = p;      }
      virtual void adjustReadPos();

      virtual const QRectF& bbox() const      { return _bbox;              }
      virtual QRectF& bbox()                  { return _bbox;              }
      virtual qreal height() const            { return bbox().height();    }
      virtual void setHeight(qreal v)         { _bbox.setHeight(v);        }
      virtual qreal width() const             { return bbox().width();     }
      virtual void setWidth(qreal v)          { _bbox.setWidth(v);         }
      QRectF abbox() const                        { return bbox().translated(pagePos());   }
      QRectF pageBoundingRect() const             { return bbox().translated(pagePos());   }
      QRectF canvasBoundingRect() const           { return bbox().translated(canvasPos()); }
      virtual void setbbox(const QRectF& r) const { _bbox = r;           }
      virtual void addbbox(const QRectF& r) const { _bbox |= r;          }
      virtual bool contains(const QPointF& p) const;
      bool intersects(const QRectF& r) const;
      virtual QPainterPath outline() const;
      virtual Shape shape() const;
      virtual qreal baseLine() const          { return -height();       }

      virtual Element::Type type() const = 0;
      virtual int subtype() const   { return -1; }  // for select gui

      virtual void draw(QPainter*) const {}
      void drawAt(QPainter*p, const QPointF& pt) const { p->translate(pt); draw(p); p->translate(-pt);}

      virtual void writeProperties(Xml& xml) const;
      virtual bool readProperties(XmlReader&);

      virtual void write(Xml&) const;
      virtual void read(XmlReader&);

      virtual QRectF drag(EditData*);
      virtual void endDrag()                  {}
      virtual QLineF dragAnchor() const       { return QLineF(); }

      virtual bool isEditable() const         { return !_generated; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, Grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void editDrag(const EditData&);
      virtual void endEditDrag()                               {}
      virtual void endEdit()                                   {}
      virtual void updateGrips(Grip*, QVector<QRectF>&) const      { }
      virtual bool nextGrip(Grip*) const;
      virtual int grips() const                { return 0; }
      virtual bool prevGrip(Grip*) const;
      virtual QPointF gripAnchor(Grip) const   { return QPointF(); }
      virtual void setGrip(Grip, const QPointF&);
      virtual QPointF getGrip(Grip) const;

      int track() const                       { return _track; }
      virtual void setTrack(int val)          { _track = val;  }

      virtual int z() const                   { return int(type()) * 100; }  // stacking order

      int staffIdx() const                    { return _track >> 2;        }
      virtual int vStaffIdx() const           { return staffIdx();         }
      int voice() const                       { return _track & 3;         }
      void setVoice(int v)                    { _track = (_track / VOICES) * VOICES + v; }
      Staff* staff() const;
      Part* part() const;

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);

      virtual void layout() {}
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);
      virtual void localSpatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

      // debug functions
      virtual void dump() const;
      virtual const char* name() const override;
      virtual Q_INVOKABLE QString subtypeName() const;
      //@ Returns the human-readable name of the element type
      virtual Q_INVOKABLE QString userName() const;
      //@ Returns the name of the element type
      virtual Q_INVOKABLE QString _name() const { return QString(name()); }
      void dumpQPointF(const char*) const;

      virtual QColor color() const             { return _color; }
      QColor curColor() const;
      QColor curColor(const Element* proxy) const;
      virtual void setColor(const QColor& c)     { _color = c;    }
      void undoSetColor(const QColor& c);
      void undoSetVisible(bool v);

      static Element::Type readType(XmlReader& node, QPointF*, Fraction*);

      QByteArray mimeData(const QPointF&) const;
/**
 Return true if this element accepts a drop at canvas relative \a pos
 of given element \a type and \a subtype.

 Reimplemented by elements that accept drops. Used to change cursor shape while
 dragging to indicate drop targets.
*/
      virtual bool acceptDrop(const DropData&) const { return false; }

/**
 Handle a dropped element at canvas relative \a pos of given element
 \a type and \a subtype. Returns dropped element if any.
 The ownership of element in DropData is transfered to the called
 element (if not used, element has to be deleted).
 The returned element will be selected if not in note edit mode.

 Reimplemented by elements that accept drops.
*/
      virtual Element* drop(const DropData&) { return 0;}

/**
 delivers mouseEvent to element in edit mode
 returns true if mouse event is accepted by element
 */
      virtual bool mousePress(const QPointF&, QMouseEvent*) { return false; }

      mutable bool itemDiscovered;     ///< helper flag for bsp

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      virtual void reset();         // reset all properties & position to default

      virtual qreal mag() const        { return _mag;   }
      void setMag(qreal val)           { _mag = val;    }
      qreal magS() const;

      bool isText() const;
      bool isPrintable() const;
      virtual bool isSpanner() const           { return false; }
      virtual bool isSpannerSegment() const    { return false; }

      qreal point(const Spatium sp) const { return sp.val() * spatium(); }

      virtual int tick() const;       // utility, searches for segment / segment parent
      virtual int rtick() const;      // utility, searches for segment / segment parent

      //
      // check element for consistency; return false if element
      // is not valid
      //
      virtual bool check() const { return true; }

      QPointF startDragPosition() const           { return _startDragPosition; }
      void setStartDragPosition(const QPointF& v) { _startDragPosition = v; }

      static const char* name(Element::Type type);
      static Ms::Element* create(Ms::Element::Type type, Score*);
      static Element::Type name2type(const QStringRef&);
      static Element* name2Element(const QStringRef&, Score*);

      void setFlag(ElementFlag f, bool v)  {
            if (v)
                  _flags |= f;
            else
                  _flags &= ~ElementFlags(f);
            }
      bool flag(ElementFlag f) const   { return _flags & f; }
      void setFlags(ElementFlags f)    { _flags = f;    }
      ElementFlags flags() const       { return _flags; }
      virtual bool systemFlag() const  { return false;  }
      bool selectable() const          { return flag(ElementFlag::SELECTABLE);  }
      void setSelectable(bool val)     { setFlag(ElementFlag::SELECTABLE, val); }
      bool dropTarget() const          { return flag(ElementFlag::DROP_TARGET); }
      void setDropTarget(bool v) const {
            if (v)
                  _flags |= ElementFlag::DROP_TARGET;
            else
                  _flags &= ~ElementFlags(ElementFlag::DROP_TARGET);
            }
      virtual bool isMovable() const   { return flag(ElementFlag::MOVABLE);     }
      bool isSegmentFlag() const           { return flag(ElementFlag::SEGMENT);     }
      uint tag() const                 { return _tag;                      }
      void setTag(uint val)            { _tag = val;                       }
      bool autoplace() const           { return _autoplace; }
      void setAutoplace(bool v)        { _autoplace = v; }

      virtual QVariant getProperty(P_ID) const override;
      virtual bool setProperty(P_ID, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID) const override;
      void undoChangeProperty(P_ID, const QVariant&, PropertyStyle ps = PropertyStyle::NOSTYLE);
      void resetProperty(P_ID);
      void undoResetProperty(P_ID);
      bool custom(P_ID) const;
      void readProperty(XmlReader&, P_ID);
      virtual bool isUserModified() const;

      virtual void styleChanged() {}

      void drawSymbol(SymId id, QPainter* p, const QPointF& o = QPointF()) const;
      void drawSymbol(SymId id, QPainter* p, const QPointF& o, int n) const;
      void drawSymbols(const std::vector<SymId>&, QPainter* p, const QPointF& o = QPointF()) const;
      qreal symHeight(SymId id) const;
      qreal symWidth(SymId id) const;
      qreal symWidth(const std::vector<SymId>&) const;
      QRectF symBbox(SymId id) const;
      QRectF symBbox(const std::vector<SymId>&) const;
      QPointF symStemDownNW(SymId id) const;
      QPointF symStemUpSE(SymId id) const;
      QPointF symCutOutNE(SymId id) const;
      QPointF symCutOutNW(SymId id) const;
      QPointF symCutOutSE(SymId id) const;
      QPointF symCutOutSW(SymId id) const;
      qreal symAdvance(SymId id) const;
      std::vector<SymId> toTimeSigString(const QString& s) const;
      bool symIsValid(SymId id) const;

      bool concertPitch() const;

      virtual Element* nextElement();  //< Used for navigation
      virtual Element* prevElement();  //< next-element and prev-element command

      virtual QString accessibleInfo() const;         //< used to populate the status bar
      virtual QString screenReaderInfo() const  {     //< by default returns accessibleInfo, but can be overriden
            return accessibleInfo();
            }
                                                       //  if the screen-reader needs a special string (see note for example)
      virtual QString accessibleExtraInfo() const {    // used to return info that will be appended to accessibleInfo
            return QString();                          // and passed only to the screen-reader
            }

      virtual void triggerLayout() const;

      //---------------------------------------------------
      // check type
      //
      // Example for ChordRest:
      //
      //    bool             isChordRest()
      //---------------------------------------------------
      // DEBUG: check to catch old (now renamed) ambitious Segment->isChordRest() calls
      //    (which check the subtype)

      bool isChordRest() const { Q_ASSERT(type() != Element::Type::SEGMENT); return type() == Element::Type::REST || type() == Element::Type::CHORD
            || type() == Element::Type::REPEAT_MEASURE; }
      bool isChordRest1() const { return type() == Element::Type::REST || type() == Element::Type::CHORD
            || type() == Element::Type::REPEAT_MEASURE; }
      bool isDurationElement() const { return isChordRest() || (type() == Element::Type::TUPLET); }
      bool isSLine() const;
      bool isSLineSegment() const;

#define CONVERT(a,b) \
      bool is##a() const { return type() == Element::Type::b; }

      CONVERT(Note,          NOTE)
      CONVERT(Rest,          REST)
      CONVERT(Chord,         CHORD)
      CONVERT(BarLine,       BAR_LINE)
      CONVERT(Articulation,  ARTICULATION)
      CONVERT(Marker,        MARKER)
      CONVERT(Clef,          CLEF)
      CONVERT(KeySig,        KEYSIG)
      CONVERT(TimeSig,       TIMESIG)
      CONVERT(Measure,       MEASURE)
      CONVERT(TempoText,     TEMPO_TEXT)
      CONVERT(Breath,        BREATH)
      CONVERT(HBox,          HBOX)
      CONVERT(VBox,          VBOX)
      CONVERT(TBox,          TBOX)
      CONVERT(FBox,          FBOX)
      CONVERT(Tie,           TIE)
      CONVERT(Slur,          SLUR)
      CONVERT(Glissando,     GLISSANDO)
      CONVERT(SystemDivider, SYSTEM_DIVIDER)
      CONVERT(RehearsalMark, REHEARSAL_MARK)
      CONVERT(Harmony,       HARMONY)
      CONVERT(Volta,         VOLTA)
      CONVERT(Jump,          JUMP)
      CONVERT(StaffText,     STAFF_TEXT)
      CONVERT(Ottava,        OTTAVA)
      CONVERT(LayoutBreak,   LAYOUT_BREAK)
      CONVERT(Segment,       SEGMENT)
      CONVERT(Tremolo,       TREMOLO)
      CONVERT(System,        SYSTEM)
      CONVERT(Lyrics,        LYRICS)
      CONVERT(Stem,          STEM)
      CONVERT(Beam,          BEAM)
      CONVERT(Hook,          HOOK)
      CONVERT(StemSlash,     STEM_SLASH)
      CONVERT(SlurSegment,   SLUR_SEGMENT)
      CONVERT(Spacer,        SPACER)
      CONVERT(StaffLines,    STAFF_LINES)
      CONVERT(Ambitus,       AMBITUS)
      CONVERT(Bracket,       BRACKET)
      CONVERT(InstrumentChange, INSTRUMENT_CHANGE)
      CONVERT(Hairpin,       HAIRPIN)
      CONVERT(HairpinSegment,HAIRPIN_SEGMENT)
      CONVERT(Bend,          BEND)
      CONVERT(RepeatMeasure, REPEAT_MEASURE)
      CONVERT(Tuplet,        TUPLET)
      CONVERT(NoteDot,       NOTEDOT)
      CONVERT(Dynamic,       DYNAMIC)
      CONVERT(InstrumentName, INSTRUMENT_NAME)
      CONVERT(Accidental,    ACCIDENTAL)
      CONVERT(TextLine,      TEXTLINE)
      CONVERT(Pedal,         PEDAL)
      CONVERT(OttavaSegment, OTTAVA_SEGMENT)
#undef CONVERT
      };

      //---------------------------------------------------
      // safe casting of Element
      //
      // Example for ChordRest:
      //
      //    ChordRest* toChordRest(Element* e)
      //---------------------------------------------------

static inline ChordRest* toChordRest(Element* e) {
      Q_ASSERT(e == 0 || e->type() == Element::Type::CHORD || e->type() == Element::Type::REST
         || e->type() == Element::Type::REPEAT_MEASURE);
      return (ChordRest*)e;
      }
static inline const ChordRest* toChordRest(const Element* e) {
      Q_ASSERT(e == 0 || e->type() == Element::Type::CHORD || e->type() == Element::Type::REST
         || e->type() == Element::Type::REPEAT_MEASURE);
      return (const ChordRest*)e;
      }
static inline const DurationElement* toDurationElement(const Element* e) {
      Q_ASSERT(e == 0 || e->type() == Element::Type::CHORD || e->type() == Element::Type::REST
         || e->type() == Element::Type::REPEAT_MEASURE || e->type() == Element::Type::TUPLET);
      return (const DurationElement*)e;
      }

#define CONVERT(a,b) \
static inline a* to##a(Element* e)             { Q_ASSERT(e == 0 || e->type() == Element::Type::b); return (a*)e; } \
static inline const a* to##a(const Element* e) { Q_ASSERT(e == 0 || e->type() == Element::Type::b); return (const a*)e; }

      CONVERT(Note,          NOTE)
      CONVERT(Rest,          REST)
      CONVERT(Chord,         CHORD)
      CONVERT(BarLine,       BAR_LINE)
      CONVERT(Articulation,  ARTICULATION)
      CONVERT(Marker,        MARKER)
      CONVERT(Clef,          CLEF)
      CONVERT(KeySig,        KEYSIG)
      CONVERT(TimeSig,       TIMESIG)
      CONVERT(Measure,       MEASURE)
      CONVERT(TempoText,     TEMPO_TEXT)
      CONVERT(Breath,        BREATH)
      CONVERT(HBox,          HBOX)
      CONVERT(VBox,          VBOX)
      CONVERT(TBox,          TBOX)
      CONVERT(FBox,          FBOX)
      CONVERT(Tie,           TIE)
      CONVERT(Slur,          SLUR)
      CONVERT(Glissando,     GLISSANDO)
      CONVERT(SystemDivider, SYSTEM_DIVIDER)
      CONVERT(RehearsalMark, REHEARSAL_MARK)
      CONVERT(Harmony,       HARMONY)
      CONVERT(Volta,         VOLTA)
      CONVERT(Jump,          JUMP)
      CONVERT(StaffText,     STAFF_TEXT)
      CONVERT(Ottava,        OTTAVA)
      CONVERT(LayoutBreak,   LAYOUT_BREAK)
      CONVERT(Segment,       SEGMENT)
      CONVERT(Tremolo,       TREMOLO)
      CONVERT(System,        SYSTEM)
      CONVERT(Lyrics,        LYRICS)
      CONVERT(Stem,          STEM)
      CONVERT(Beam,          BEAM)
      CONVERT(Hook,          HOOK)
      CONVERT(StemSlash,     STEM_SLASH)
      CONVERT(SlurSegment,   SLUR_SEGMENT)
      CONVERT(Spacer,        SPACER)
      CONVERT(StaffLines,    STAFF_LINES)
      CONVERT(Ambitus,       AMBITUS)
      CONVERT(Bracket,       BRACKET)
      CONVERT(InstrumentChange, INSTRUMENT_CHANGE)
      CONVERT(Text,          TEXT)
      CONVERT(Hairpin,       HAIRPIN)
      CONVERT(HairpinSegment,HAIRPIN_SEGMENT)
      CONVERT(Bend,          BEND)
      CONVERT(RepeatMeasure, REPEAT_MEASURE)
      CONVERT(Tuplet,        TUPLET)
      CONVERT(NoteDot,       NOTEDOT)
      CONVERT(Dynamic,       DYNAMIC)
      CONVERT(InstrumentName, INSTRUMENT_NAME)
      CONVERT(Accidental,    ACCIDENTAL)
      CONVERT(TextLine,     TEXTLINE)
      CONVERT(Pedal,         PEDAL)
      CONVERT(OttavaSegment, OTTAVA_SEGMENT)
#undef CONVERT

//---------------------------------------------------------
//   ElementList
//---------------------------------------------------------

class ElementList : public std::vector<Element*> {
   public:
      ElementList() {}
      bool remove(Element*);
      void replace(Element* old, Element* n);
      void write(Xml&) const;
      void write(Xml&, const char* name) const;
      };

//-------------------------------------------------------------------
//   @@ StaffLines
///    The StaffLines class is the graphic representation of a staff,
///    it draws the horizontal staff lines.
//-------------------------------------------------------------------

class StaffLines : public Element {
      Q_OBJECT

      qreal dist;
      qreal lw;
      int lines;

   public:
      StaffLines(Score*);
      virtual StaffLines* clone() const    { return new StaffLines(*this); }
      virtual Element::Type type() const   { return Element::Type::STAFF_LINES; }
      virtual void layout();

      Measure* measure() const             { return (Measure*)parent(); }
      virtual void draw(QPainter*) const;
      virtual QPointF pagePos() const;    ///< position in page coordinates
      virtual QPointF canvasPos() const;  ///< position in page coordinates
      qreal y1() const;
      qreal staffHeight() const { return (lines-1) * dist; }
      };

//---------------------------------------------------------
//   @@ Line
//---------------------------------------------------------

class Line : public Element {
      Q_OBJECT

      Spatium _width;
      Spatium _len;
      int _z;                     ///< stacking order when drawing or selecting;
                                  ///< elements are drawn from high number to low number;
                                  ///< default is type() * 100;

   protected:
      bool vertical;

public:
      Line(Score*);
      Line(Score*, bool vertical);
      Line &operator=(const Line&);

      virtual Line* clone() const        { return new Line(*this); }
      virtual Element::Type type() const { return Element::Type::LINE; }
      virtual void layout();

      virtual void draw(QPainter*) const;
      void writeProperties(Xml& xml) const;
      bool readProperties(XmlReader&);
      void dump() const;

      Spatium len()    const { return _len; }
      Spatium lineWidth()  const { return _width; }
      void setLen(Spatium);
      void setLineWidth(Spatium);
      virtual int z() const               { return _z; }
      void setZ(int val)                  { _z = val;  }
      };

//---------------------------------------------------------
//   @@ Compound
//---------------------------------------------------------

class Compound : public Element {
      Q_OBJECT

      QList<Element*> elements;

   protected:
      const QList<Element*>& getElements() const { return elements; }

   public:
      Compound(Score*);
      Compound(const Compound&);
      virtual Element::Type type() const = 0;

      virtual void draw(QPainter*) const;
      virtual void addElement(Element*, qreal x, qreal y);
      void clear();
      virtual void setSelected(bool f);
      virtual void setVisible(bool);
      virtual void layout();
      };

extern bool elementLessThan(const Element* const, const Element* const);
extern void collectElements(void* data, Element* e);


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Element::Type);
Q_DECLARE_METATYPE(Ms::Element::Placement);

#endif

