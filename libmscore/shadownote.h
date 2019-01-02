//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SHADOWNOTE_H__
#define __SHADOWNOTE_H__

#include "element.h"
#include "durationtype.h"

namespace Ms {

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

/**
 Graphic representation of a shadow note,
 which shows the note insert position in note entry mode.
*/

class ShadowNote final : public Element {
      int _line;
      SymId _notehead;
      TDuration _duration;
      int _voice;
      bool _rest;
      bool _isGrace;
   public:
      ShadowNote(Score*);
      virtual ShadowNote* clone() const  { return new ShadowNote(*this); }
      virtual ElementType type() const   { return ElementType::SHADOW_NOTE; }
      virtual void layout();
      int line() const                   { return _line;   }
      void setLine(int n)                { _line = n;      }
      virtual void draw(QPainter*) const;

      void setState(SymId noteSymbol, int voice, TDuration duration, bool rest = false);
      void setGrace(bool grace);

      SymId getNoteFlag() const;
      bool computeUp() const;

      SymId notehead() const { return _notehead; }
      bool isValid() const;
      };


}     // namespace Ms
#endif

