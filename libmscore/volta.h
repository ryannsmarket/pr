//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: volta.h 5500 2012-03-28 16:28:26Z wschweer $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __VOLTA_H__
#define __VOLTA_H__

#include "textline.h"

class Score;
class Xml;
class Volta;
class Measure;

//---------------------------------------------------------
//   VoltaType
//---------------------------------------------------------

enum class VoltaType {
      OPEN, CLOSED
      };

extern void vdebug(int n);
extern LineSegment* voltaDebug;

//---------------------------------------------------------
//   @@ VoltaSegment
//---------------------------------------------------------

class VoltaSegment : public TextLineSegment {
      Q_OBJECT

   public:
      VoltaSegment(Score* s) : TextLineSegment(s) {}
      virtual ElementType type() const     { return VOLTA_SEGMENT; }
      virtual VoltaSegment* clone() const  { return new VoltaSegment(*this); }
      Volta* volta() const                 { return (Volta*)spanner(); }
      virtual void layout();
      };

//---------------------------------------------------------
//   @@ Volta
//   @P voltaType enum VoltaType OPEN, CLOSED
//---------------------------------------------------------

class Volta : public TextLine {
      Q_OBJECT
      Q_ENUMS(VoltaType)

   private:
      Q_PROPERTY(VoltaType voltaType READ voltaType WRITE undoSetVoltaType)

      VoltaType _voltaType;
      QList<int> _endings;

   public:
      Volta(Score* s);
      virtual Volta* clone()     const { return new Volta(*this); }
      virtual ElementType type() const { return VOLTA; }
      virtual LineSegment* createLineSegment();
      virtual void layout();

      virtual void write(Xml&) const;
      virtual void read(XmlReader& e);

      QList<int> endings() const           { return _endings; }
      QList<int>& endings()                { return _endings; }
      void setEndings(const QList<int>& l) { _endings = l;    }
      void setText(const QString& s);
      QString text() const;

      void setVoltaType(VoltaType val);
      void undoSetVoltaType(VoltaType val);
      VoltaType voltaType() const            { return _voltaType; }

      bool hasEnding(int repeat) const;
      Measure* startMeasure() const    { return (Measure*)startElement(); }
      Measure* endMeasure() const      { return (Measure*)endElement(); }
      void setStartMeasure(Measure* m) { setStartElement((Element*)m); }
      void setEndMeasure(Measure* m)   { setEndElement((Element*)m);   }

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual QVariant propertyDefault(P_ID) const;
      virtual void setYoff(qreal);
      };

Q_DECLARE_METATYPE(VoltaType)

#endif

