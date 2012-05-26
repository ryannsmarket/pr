//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "staffstate.h"
#include "score.h"
#include "instrtemplate.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "mscore.h"

//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

StaffState::StaffState(Score* score)
   : Element(score)
      {
      _subtype = STAFF_STATE_INSTRUMENT;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffState::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("subtype", _subtype);
      if (subtype() == STAFF_STATE_INSTRUMENT)
            _instrument.write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffState::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "subtype")
                  _subtype = StaffStateType(e.text().toInt());
            else if (e.tagName() == "Instrument")
                  _instrument.read(e);
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffState::draw(QPainter* painter) const
      {
      if (score()->printing())
            return;
      QPen pen(selected() ? MScore::selectColor[0] : MScore::layoutBreakColor,
         lw, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
      painter->setPen(pen);
      painter->setBrush(Qt::NoBrush);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffState::layout()
      {
      qreal _spatium = spatium();
      path      = QPainterPath();
      lw        = _spatium * 0.3;
      qreal h  = _spatium * 4;
      qreal w  = _spatium * 2.5;
//      qreal w1 = w * .6;

      switch(subtype()) {
            case STAFF_STATE_INSTRUMENT:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  path.moveTo(w * .5, h - _spatium * .5);
                  path.lineTo(w * .5, _spatium * 2);
                  path.moveTo(w * .5, _spatium * .8);
                  path.lineTo(w * .5, _spatium * 1.0);
                  break;

            case STAFF_STATE_TYPE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_VISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_INVISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            default:
                  qDebug("unknown layout break symbol\n");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      setPos(0.0, _spatium * -6.0);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void StaffState::setSubtype(const QString& s)
      {
      if (s == "instrument")
            setSubtype(STAFF_STATE_INSTRUMENT);
      else if (s == "type")
            setSubtype(STAFF_STATE_TYPE);
      else if (s == "visible")
            setSubtype(STAFF_STATE_VISIBLE);
      else if (s == "invisible")
            setSubtype(STAFF_STATE_INVISIBLE);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

QString StaffState::subtypeName() const
      {
      switch(subtype()) {
            case STAFF_STATE_INSTRUMENT:
                  return "instrument";
            case STAFF_STATE_TYPE:
                  return "type";
            case STAFF_STATE_VISIBLE:
                  return "visible";
            case STAFF_STATE_INVISIBLE:
                  return "invisible";
            default:
                  return "??";
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool StaffState::acceptDrop(MuseScoreView*, const QPointF&, Element*) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* StaffState::drop(const DropData& data)
      {
      Element* e = data.element;
      score()->undoChangeElement(this, e);
      return e;
      }

