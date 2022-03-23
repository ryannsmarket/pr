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

#ifndef __SLURTIE_H__
#define __SLURTIE_H__

#include "spanner.h"
#include "mscore.h"
#include "infrastructure/draw/painterpath.h"

namespace Ms {
//---------------------------------------------------------
//   SlurPos
//---------------------------------------------------------

struct SlurPos {
    mu::PointF p1;               // start point of slur
    System* system1;          // start system of slur
    mu::PointF p2;               // end point of slur
    System* system2;          // end system of slur
};

struct SlurOffsets {
    mu::PointF o[4];
};

//---------------------------------------------------------
//   UP
//---------------------------------------------------------

struct UP {
    mu::PointF p;              // layout position relative to pos()
    mu::PointF off;            // user offset in point units

    mu::PointF pos() const { return p + off; }
    bool operator!=(const UP& up) const { return p != up.p || off != up.off; }
};

//---------------------------------------------------------
//   CubicBezier
//    Helper class to optimize cubic Bezier curve points
//    calculation.
//---------------------------------------------------------

class CubicBezier
{
    mu::PointF p1;
    mu::PointF p2;
    mu::PointF p3;
    mu::PointF p4;

public:
    CubicBezier(mu::PointF _p1, mu::PointF _p2, mu::PointF _p3, mu::PointF _p4)
        : p1(_p1), p2(_p2), p3(_p3), p4(_p4) {}

    mu::PointF pointAtPercent(qreal t) const
    {
        Q_ASSERT(t >= 0.0 && t <= 1.0);
        const qreal r = 1.0 - t;
        const mu::PointF B123 = r * (r * p1 + t * p2) + t * (r * p2 + t * p3);
        const mu::PointF B234 = r * (r * p2 + t * p3) + t * (r * p3 + t * p4);
        return r * B123 + t * B234;
    }
};

class SlurTie;

//---------------------------------------------------------
//   SlurTieSegment
//---------------------------------------------------------

class SlurTieSegment : public SpannerSegment
{
protected:
    struct UP _ups[int(Grip::GRIPS)];

    mu::PainterPath path;
    mu::PainterPath shapePath;
    Shape _shape;

    SlurTieSegment(const ElementType& type, System*);
    SlurTieSegment(const SlurTieSegment&);

    virtual void changeAnchor(EditData&, EngravingItem*) = 0;
    std::vector<mu::LineF> gripAnchorLines(Grip grip) const override;

public:

    virtual void spatiumChanged(qreal, qreal) override;
    SlurTie* slurTie() const { return (SlurTie*)spanner(); }

    void startEditDrag(EditData& ed) override;
    void endEditDrag(EditData& ed) override;
    void editDrag(EditData&) override;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;
    void reset() override;
    void undoChangeProperty(Pid id, const mu::engraving::PropertyValue&, PropertyFlags ps) override;
    void move(const mu::PointF& s) override;
    bool isEditable() const override { return true; }

    void setSlurOffset(Grip i, const mu::PointF& val) { _ups[int(i)].off = val; }
    const UP& ups(Grip i) const { return _ups[int(i)]; }
    UP& ups(Grip i) { return _ups[int(i)]; }
    Shape shape() const override { return _shape; }

    bool needStartEditingAfterSelecting() const override { return true; }
    int gripsCount() const override { return int(Grip::GRIPS); }
    Grip initialEditModeGrip() const override { return Grip::END; }
    Grip defaultGrip() const override { return Grip::DRAG; }
    std::vector<mu::PointF> gripsPositions(const EditData& = EditData()) const override;

    void writeSlur(XmlWriter& xml, int no) const;
    void read(XmlReader&) override;
    virtual void drawEditMode(mu::draw::Painter* painter, EditData& editData, qreal currentViewScaling) override;
    virtual void computeBezier(mu::PointF so = mu::PointF()) = 0;
};

//-------------------------------------------------------------------
//   @@ SlurTie
//   @P lineType       int  (0 - solid, 1 - dotted, 2 - dashed, 3 - wide dashed)
//   @P slurDirection  enum (Direction.AUTO, Direction.DOWN, Direction.UP)
//-------------------------------------------------------------------

class SlurTie : public Spanner
{
    SlurStyleType _styleType = SlurStyleType::Undefined;

protected:
    bool _up;                 // actual direction

    DirectionV _slurDirection;
    void fixupSegments(unsigned nsegs);

public:
    SlurTie(const ElementType& type, EngravingItem* parent);
    SlurTie(const SlurTie&);
    ~SlurTie();

    bool up() const { return _up; }

    virtual void reset() override;

    DirectionV slurDirection() const { return _slurDirection; }
    void setSlurDirection(DirectionV d) { _slurDirection = d; }
    void undoSetSlurDirection(DirectionV d);

    virtual void layout2(const mu::PointF, int, struct UP&) {}
    virtual bool contains(const mu::PointF&) const { return false; }    // not selectable

    void read(XmlReader&) override;

    void writeProperties(XmlWriter& xml) const override;
    bool readProperties(XmlReader&) override;

    SlurStyleType styleType() const { return _styleType; }
    void setStyleType(SlurStyleType type) { _styleType = type; }

    virtual void slurPos(SlurPos*) = 0;
    virtual SlurTieSegment* newSlurTieSegment(System* parent) = 0;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid id) const override;
};
}

#endif
