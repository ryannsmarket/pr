//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2016 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BARLINE_H__
#define __BARLINE_H__

#include "element.h"
#include "mscore.h"

namespace Ms {

class MuseScoreView;
class Segment;

static const int MIN_BARLINE_FROMTO_DIST        = 2;
static const int MIN_BARLINE_SPAN_FROMTO        = -2;

// bar line span for 1-line staves is special: goes from 2sp above the line to 2sp below the line;
static const int BARLINE_SPAN_1LINESTAFF_FROM   = -4;
static const int BARLINE_SPAN_1LINESTAFF_TO     = 4;

// data for some preset bar line span types
static const int BARLINE_SPAN_TICK1_FROM        = -1;
static const int BARLINE_SPAN_TICK1_TO          = -7;
static const int BARLINE_SPAN_TICK2_FROM        = -2;
static const int BARLINE_SPAN_TICK2_TO          = -6;
static const int BARLINE_SPAN_SHORT1_FROM       = 2;
static const int BARLINE_SPAN_SHORT1_TO         = -2;
static const int BARLINE_SPAN_SHORT2_FROM       = 1;
static const int BARLINE_SPAN_SHORT2_TO         = -1;

//---------------------------------------------------------
//   BarLineTableItem
//---------------------------------------------------------

struct BarLineTableItem {
      BarLineType type;
      const char* userName;       // user name, translatable
      const char* name;
      };

//---------------------------------------------------------
//   @@ BarLine
//
//   @P barLineType  enum  (BarLineType.NORMAL, .DOUBLE, .START_REPEAT, .END_REPEAT, .BROKEN, .END, .DOTTED)
//---------------------------------------------------------

class BarLine : public Element {
      Q_GADGET

      char _spanStaff         { false };       // span barline to next staff if true
      char _spanFrom          { 0 };           // line number on start and end staves
      char _spanTo            { 0 };
      BarLineType _barLineType { BarLineType::NORMAL };
      mutable qreal y1;
      mutable qreal y2;
      ElementList _el;        ///< fermata or other articulations

      // static variables used while dragging
      static bool _origSpanStaff;         // original span value before editing
      static int _origSpanFrom;
      static int _origSpanTo;
      static qreal yoff1;                 // used during drag edit to extend y1 and y2
      static qreal yoff2;

      void getY() const;
      void drawDots(QPainter* painter, qreal x) const;
      void drawTips(QPainter* painter, bool reversed, qreal x) const;
      bool isTop() const;
      bool isBottom() const;

   public:
      BarLine(Score* s = 0);
      virtual ~BarLine();
      BarLine(const BarLine&);
      BarLine &operator=(const BarLine&) = delete;

      virtual BarLine* clone() const override     { return new BarLine(*this); }
      virtual ElementType type() const override { return ElementType::BAR_LINE; }
      virtual void write(XmlWriter& xml) const override;
      virtual void read(XmlReader&) override;
      virtual void draw(QPainter*) const override;
      virtual QPointF pagePos() const override;      ///< position in canvas coordinates
      virtual void layout() override;
      void layout2();
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true) override;
      virtual void add(Element*) override;
      virtual void remove(Element*) override;
      virtual bool acceptDrop(const DropData&) const override;
      virtual Element* drop(const DropData&) override;
      virtual bool isEditable() const override    { return true; }

      Segment* segment() const        { return toSegment(parent()); }
      Measure* measure() const        { return toMeasure(parent()->parent()); }

      void setSpanStaff(bool val)     { _spanStaff = val;     }
      void setSpanFrom(int val)       { _spanFrom = val;      }
      void setSpanTo(int val)         { _spanTo = val;        }
      bool spanStaff() const          { return _spanStaff;    }
      int spanFrom() const            { return _spanFrom;     }
      int spanTo() const              { return _spanTo;       }

      virtual void startEdit(MuseScoreView*, const QPointF&) override;
      virtual void endEdit() override;
      virtual void editDrag(const EditData&) override;
      virtual void endEditDrag(const EditData&) override;
      virtual void updateGrips(Grip*, QVector<QRectF>&) const override;
      virtual int grips() const override { return 2; }
      virtual Shape shape() const override;

      ElementList* el()                  { return &_el; }
      const ElementList* el() const      { return &_el; }

      static QString userTypeName(BarLineType);
      static const BarLineTableItem* barLineTableItem(unsigned);

      QString barLineTypeName() const;
      static QString barLineTypeName(BarLineType t);
      void setBarLineType(const QString& s);
      void setBarLineType(BarLineType i) { _barLineType = i;     }
      BarLineType barLineType() const    { return _barLineType;  }
      static BarLineType barLineType(const QString&);

      virtual int subtype() const override         { return int(_barLineType); }
      virtual QString subtypeName() const override { return qApp->translate("barline", barLineTypeName().toUtf8()); }

      virtual QVariant getProperty(P_ID propertyId) const override;
      virtual bool setProperty(P_ID propertyId, const QVariant&) override;
      virtual QVariant propertyDefault(P_ID propertyId) const override;

      static void layoutWidth(Score*, BarLineType, qreal mag, qreal* lx, qreal* rx);

      virtual Element* nextElement() override;
      virtual Element* prevElement() override;

      virtual QString accessibleInfo() const override;
      virtual QString accessibleExtraInfo() const override;

      static const std::vector<BarLineTableItem> barLineTable;
      };
}     // namespace Ms

// Q_DECLARE_METATYPE(Ms::MSQE_BarLineType::E);

#endif

