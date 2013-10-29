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

#include "xml.h"
#include "mscore.h"
#include "property.h"
#include "spatium.h"

class QPainter;

namespace Ms {

/**
 \file
 Definition of classes Element, ElementList, StaffLines.
*/


class Xml;
class Measure;
class Staff;
class Score;
class Sym;
class MuseScoreView;
class Segment;
class TextStyle;
class Element;

//---------------------------------------------------------
//   ElementFlag
//---------------------------------------------------------

enum ElementFlag {
      ELEMENT_DROP_TARGET  = 0x2,
      ELEMENT_SELECTABLE   = 0x4,
      ELEMENT_MOVABLE      = 0x8,
      ELEMENT_SEGMENT      = 0x10,
      ELEMENT_HAS_TAG      = 0x20,
      ELEMENT_ON_STAFF     = 0x40   // parent is Segment() type
      };

typedef QFlags<ElementFlag> ElementFlags;
Q_DECLARE_OPERATORS_FOR_FLAGS(ElementFlags)

//---------------------------------------------------------
//   PropertyStyle
//---------------------------------------------------------

enum class PropertyStyle : char {
      NOSTYLE, UNSTYLED, STYLED
      };

//---------------------------------------------------------
///   \brief Unit of horizontal measure
//---------------------------------------------------------

class Space {
      qreal _lw;       // space needed to the left
      qreal _rw;       // space needed to the right

   public:
      Space() : _lw(0.0), _rw(0.0)  {}
      Space(qreal a, qreal b) : _lw(a), _rw(b) {}
      qreal lw() const             { return _lw; }
      qreal rw() const             { return _rw; }
      qreal& rLw()                 { return _lw; }
      qreal& rRw()                 { return _rw; }
      qreal width() const          { return _lw + _rw; }
      void setLw(qreal e)          { _lw = e; }
      void setRw(qreal m)          { _rw = m; }
      void max(const Space& s);
      Space& operator+=(const Space&);
      };

//---------------------------------------------------------
//   LinkedElements
//---------------------------------------------------------

class LinkedElements : public QList<Element*> {
      int _lid;         // unique id for every linked list

   public:
      LinkedElements(Score*);
      LinkedElements(Score*, int id);
      void setLid(Score*, int val);
      int lid() const   { return _lid;    }
      };

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

      DropData();
      };

//---------------------------------------------------------
//   EditData
//    used in editDrag
//---------------------------------------------------------

struct EditData {
      MuseScoreView* view;
      int curGrip;
      QPointF startMove;
      QPointF delta;
      QPointF pos;
      bool hRaster;
      bool vRaster;
      };

//-------------------------------------------------------------------
//    @@ Element
///   \brief Base class of score layout elements
///
///   The Element class is the virtual base class of all
///   score layout elements.
//
//      @P type      ElementType element type, read only
//      @P parent    Element*    parent in drawing hierarchy
//      @P track     int
//      @P color     QColor      foreground color
//      @P visible   bool
//      @P generated bool        generated by layout
//      @P selected  bool
//      @P placement Placement   ABOVE, BELOW
//      @P pos       QPointF     position relativ to parent
//      @P pagePos   QPointF     position in page coordinated, read only
//      @P userOff   QPointF     manual offset to layout position
//      @P bbox      QRectF      Bounding box relative to pos and userOff, read only
//-------------------------------------------------------------------

class Element : public QObject {
      Q_OBJECT
      Q_ENUMS(ElementType)
      Q_ENUMS(DynamicType)
      Q_ENUMS(Placement)

   public:
      //-------------------------------------------------------------------
      //    The value of this enum determines the "stacking order"
      //    of elements on the canvas.
      //   Note: keep in sync with array elementNames[] in element.cpp
      //-------------------------------------------------------------------

       enum ElementType {
            INVALID = 0,
            SYMBOL,
            TEXT,
            INSTRUMENT_NAME,
            SLUR_SEGMENT,
            STAFF_LINES,
            BAR_LINE,
            STEM_SLASH,
            LINE,
            BRACKET,

            ARPEGGIO,
            ACCIDENTAL,
            STEM,             // list STEM before NOTE: notes in TAB might 'break' stems
            NOTE,             // and this requires stems to be drawn before notes
            CLEF,
            KEYSIG,
            TIMESIG,
            REST,
            BREATH,
            GLISSANDO,

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
            RUBBERBAND,
            TAB_DURATION_SYMBOL,
            FSYMBOL,
            PAGE,
            HAIRPIN,
            OTTAVA,
            PEDAL,
            TRILL,
            TEXTLINE,
            NOTELINE,
            SEGMENT,
            SYSTEM,
            COMPOUND,
            CHORD,
            SLUR,
            ELEMENT,
            ELEMENT_LIST,
            STAFF_LIST,
            MEASURE_LIST,
            LAYOUT,
            HBOX,
            VBOX,
            TBOX,
            FBOX,
            ACCIDENTAL_BRACKET,
            ICON,
            OSSIA,
            BAGPIPE_EMBELLISHMENT,

            MAXTYPE
            };

      //---------------------------------------------------------
      //   DynamicRange
      //---------------------------------------------------------

      enum DynamicRange {
            DYNAMIC_STAFF, DYNAMIC_PART, DYNAMIC_SYSTEM
            };
      enum Placement  { ABOVE, BELOW };

   private:
      Q_PROPERTY(ElementType type    READ type)
      Q_PROPERTY(Element* parent     READ parent     WRITE setParent)
      Q_PROPERTY(int track           READ track      WRITE setTrack)
      Q_PROPERTY(QColor color        READ color      WRITE undoSetColor)
      Q_PROPERTY(bool selected       READ selected   WRITE setSelected)
      Q_PROPERTY(bool generated      READ generated  WRITE setGenerated)
      Q_PROPERTY(bool visible        READ visible    WRITE setVisible)
      Q_PROPERTY(Placement placement READ placement  WRITE undoSetPlacement)
      Q_PROPERTY(QPointF pos         READ scriptPos  WRITE scriptSetPos)
      Q_PROPERTY(QPointF pagePos     READ pagePos)
      Q_PROPERTY(QPointF userOff     READ scriptUserOff WRITE scriptSetUserOff)
      Q_PROPERTY(QRectF  bbox        READ bbox )

      LinkedElements* _links;
      Element* _parent;

      bool _selected;             ///< set if element is selected
      bool _generated;            ///< automatically generated Element
      bool _visible;              ///< visibility attribute
      Placement _placement;

      mutable ElementFlags _flags;

      int _track;                 ///< staffIdx * VOICES + voice
      QColor _color;
      qreal _mag;                 ///< standard magnification (derived value)

      QPointF _pos;               ///< Reference position, relative to _parent.
      QPointF _userOff;           ///< offset from normal layout position:
                                  ///< user dragged object this amount.
      QPointF _readPos;

      mutable QRectF _bbox;       ///< Bounding box relative to _pos + _userOff
                                  ///< valid after call to layout()
      uint _tag;                  ///< tag bitmask

   protected:

      Score* _score;

      QPointF _startDragPosition;   ///< used during drag

   public:
      Element(Score* s = 0);
      Element(const Element&);
      virtual ~Element();
      Element &operator=(const Element&);
      Q_INVOKABLE virtual Ms::Element* clone() const = 0;
      virtual Element* linkedClone();
      QList<Element*> linkList() const;

      void linkTo(Element*);
      int lid() const                         { return _links ? _links->lid() : 0; }
      LinkedElements* links() const           { return _links;      }
      void setLinks(LinkedElements* le)       { _links = le;        }

      Score* score() const                    { return _score;      }
      virtual void setScore(Score* s)         { _score = s;         }
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
      virtual void move(qreal xd, qreal yd)   { _pos += QPointF(xd, yd); }
      virtual void move(const QPointF& s)     { _pos += s;               }

      virtual QPointF pagePos() const;          ///< position in page coordinates
      virtual QPointF canvasPos() const;        ///< position in canvas coordinates
      qreal pageX() const;
      qreal canvasX() const;

      const QPointF& userOff() const          { return _userOff;  }
      void setUserOff(const QPointF& o)       { _userOff = o;     }
      void setUserXoffset(qreal v)            { _userOff.setX(v); }
      void setUserYoffset(qreal v)            { _userOff.setY(v); }

      qreal& rUserXoffset()                   { return _userOff.rx(); }
      qreal& rUserYoffset()                   { return _userOff.ry(); }

      // function versions for scripts: use coords in spatium units rather than raster
      // and route pos changes to userOff
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
      virtual QPainterPath shape() const;
      virtual qreal baseLine() const          { return -height();       }

      virtual ElementType type() const = 0;
      virtual int subType() const             { return type(); }  // for select gui
      bool isChordRest() const;
      bool isDurationElement() const;
      bool isSLine() const;

      virtual void draw(QPainter*) const {}

      void writeProperty(Xml& xml, P_ID id) const;
      virtual void writeProperties(Xml& xml) const;
      virtual bool readProperties(XmlReader&);

      virtual void write(Xml&) const;
      virtual void read(XmlReader&);

      virtual QRectF drag(EditData*);
      virtual void endDrag()                  {}
      virtual QLineF dragAnchor() const       { return QLineF(); }

      virtual bool isEditable() const         { return !_generated; }
      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void editDrag(const EditData&);
      virtual void endEditDrag()                               {}
      virtual void endEdit()                                   {}
      virtual void updateGrips(int* grips, QRectF*) const      { *grips = 0;       }
      virtual QPointF gripAnchor(int) const   { return QPointF(); }
      virtual void setGrip(int, const QPointF&);
      virtual QPointF getGrip(int) const;

      int track() const                       { return _track; }
      virtual void setTrack(int val)          { _track = val;  }

      virtual int z() const                   { return type() * 100; }  // stacking order

      int staffIdx() const                    { return _track >> 2;        }
      int voice() const                       { return _track & 3;         }
      void setVoice(int v)                    { _track = (_track / VOICES) + v; }
      Staff* staff() const;

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);

      virtual void layout() {}
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

      // debug functions
      virtual void dump() const;
      Q_INVOKABLE const char* name() const;
      virtual QString userName() const;
      void dumpQPointF(const char*) const;

      virtual Space space() const      { return Space(0.0, width()); }

      QColor color() const             { return _color; }
      QColor curColor() const;
      void setColor(const QColor& c)     { _color = c;    }
      void undoSetColor(const QColor& c);

      static ElementType readType(XmlReader& node, QPointF*, Fraction*);

      QByteArray mimeData(const QPointF&) const;
/**
 Return true if this element accepts a drop at canvas relative \a pos
 of given element \a type and \a subtype.

 Reimplemented by elements that accept drops. Used to change cursor shape while
 dragging to indicate drop targets.
*/
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const { return false; }

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

      mutable int itemDiscovered;     ///< helper flag for bsp

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      virtual void reset();

      virtual qreal mag() const                { return _mag;   }
      qreal magS() const;
      void setMag(qreal val)           { _mag = val;    }

      bool isText() const;

      qreal point(const Spatium sp) const { return sp.val() * spatium(); }

      //
      // check element for consistency; return false if element
      // is not valid
      //
      virtual bool check() const { return true; }

      QPointF startDragPosition() const           { return _startDragPosition; }
      void setStartDragPosition(const QPointF& v) { _startDragPosition = v; }

      static const char* name(ElementType type);
      Q_INVOKABLE static Ms::Element* create(Ms::Element::ElementType type, Score*);
      static ElementType name2type(const QStringRef&);
      static Element* name2Element(const QStringRef&, Score*);

      void setFlag(ElementFlag f, bool v)  {
            if (v)
                  _flags |= f;
            else
                  _flags &= ~f;
            }
      bool flag(ElementFlag f) const   { return _flags & f; }
      void setFlags(ElementFlags f)    { _flags = f;    }
      ElementFlags flags() const       { return _flags; }
      virtual bool systemFlag() const  { return false;  }
      bool selectable() const          { return flag(ELEMENT_SELECTABLE);  }
      void setSelectable(bool val)     { setFlag(ELEMENT_SELECTABLE, val); }
      bool dropTarget() const          { return flag(ELEMENT_DROP_TARGET); }
      void setDropTarget(bool v) const {
            if (v)
                  _flags |= ELEMENT_DROP_TARGET;
            else
                  _flags &= ~ELEMENT_DROP_TARGET;
            }
      virtual bool isMovable() const   { return flag(ELEMENT_MOVABLE);     }
      bool isSegment() const           { return flag(ELEMENT_SEGMENT);     }
      uint tag() const                 { return _tag;                      }
      void setTag(uint val)            { _tag = val;                       }

      virtual QVariant getProperty(P_ID) const;
      virtual bool setProperty(P_ID, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;
      virtual void resetProperty(P_ID id)          {
            QVariant v = propertyDefault(id);
            if (v.isValid())
                  setProperty(id, v);
            }
      virtual PropertyStyle propertyStyle(P_ID) const { return PropertyStyle::NOSTYLE; }

      void undoChangeProperty(P_ID, const QVariant&);
      void undoPushProperty(P_ID);

      virtual void styleChanged() {}
      };

//---------------------------------------------------------
//   ElementList
//---------------------------------------------------------

// class ElementList : public std::list<Element*> {
class ElementList : public QList<Element*> {
   public:
      ElementList() {}
      bool remove(Element*);
      void replace(Element* old, Element* n);
      void write(Xml&) const;
      void write(Xml&, const char* name) const;
      };

//-------------------------------------------------------------------
//   @@ StaffLines
///   The StaffLines class is the graphic representation of a staff,
///   it draws the horizontal staff lines.
//-------------------------------------------------------------------

class StaffLines : public Element {
      Q_OBJECT

      qreal dist;
      qreal lw;
      int lines;

   public:
      StaffLines(Score*);
      virtual StaffLines* clone() const    { return new StaffLines(*this); }
      virtual ElementType type() const     { return ElementType(STAFF_LINES); }
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

      virtual Line* clone() const { return new Line(*this); }
      virtual ElementType type() const { return ElementType(LINE); }
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

      QList<Element*> elemente;

   protected:
      const QList<Element*>& getElemente() const { return elemente; }

   public:
      Compound(Score*);
      Compound(const Compound&);
      virtual ElementType type() const = 0;

      virtual void draw(QPainter*) const;
      virtual void addElement(Element*, qreal x, qreal y);
      void clear();
      virtual void setSelected(bool f);
      virtual void setVisible(bool);
      virtual void layout();
      };

//---------------------------------------------------------
//   @@ RubberBand
//---------------------------------------------------------

class RubberBand : public Element {
      Q_OBJECT

      QPointF _p1, _p2;

   public:
      RubberBand(Score* s) : Element(s) {}
      virtual RubberBand* clone() const { return new RubberBand(*this); }
      virtual ElementType type() const { return ElementType(RUBBERBAND); }
      virtual void draw(QPainter*) const;

      void set(const QPointF& p1, const QPointF& p2) { _p1 = p1; _p2 = p2; }
      QPointF p1() const { return _p1; }
      QPointF p2() const { return _p2; }
      };

extern bool elementLessThan(const Element* const, const Element* const);
extern void collectElements(void* data, Element* e);


}     // namespace Ms

Q_DECLARE_METATYPE(Ms::Element::ElementType)
Q_DECLARE_METATYPE(Ms::Element::DynamicRange)
Q_DECLARE_METATYPE(Ms::Element::Placement)

#endif

