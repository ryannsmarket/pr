//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2019 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CHANGEMAP_H__
#define __CHANGEMAP_H__

#include "fraction.h"

/**
 \file
 Definition of class ChangeMap.
*/

namespace Ms {

enum class ChangeMethod : signed char {
      NORMAL,
      EXPONENTIAL,
      EASE_IN,
      EASE_OUT,
      EASE_IN_OUT       // and shake it all about
      };

enum class ChangeDirection : signed char {
      INCREASING,
      DECREASING
      };

//---------------------------------------------------------
///   ChangeEvent
///   item in ChangeMap
//---------------------------------------------------------

enum class ChangeEventType : char { FIX, RAMP };

class ChangeEvent {
      // Despite storing the tick as the key of the ChangeEvent in the ChangeMap, we also store it here.
      // Since we're not going to be changing it (well, not much, anyway), this is good because it provides
      // multiple ways to access the tick. Keeping it in sync isn't much trouble.
      Fraction tick;
      int value;
      ChangeEventType type;
      Fraction etick;
      ChangeMethod method;
      ChangeDirection direction;
      int cachedStart   { -1 };
      int cachedEnd     { -1 };

   public:
      ChangeEvent(Fraction t, int vel) : tick(t), value(vel), type(ChangeEventType::FIX) {}
      ChangeEvent(Fraction s, Fraction e, int diff, ChangeMethod m, ChangeDirection d)
            : tick(s), value(diff), type(ChangeEventType::RAMP), etick(e), method(m), direction(d) {}

      bool operator==(const ChangeEvent& event) const;
      bool operator!=(const ChangeEvent& event) const;

      friend class ChangeMap;
      };

//---------------------------------------------------------
//  ChangeMap
///  List of changes in a value.
//---------------------------------------------------------

class ChangeMap : public QMultiMap<Fraction, ChangeEvent> {
      bool cleanedUp    { false };
      static const int DEFAULT_VALUE  { 80 };   // TODO

      struct ChangeMethodItem {
            ChangeMethod method;
            const char* name;
            };

   public:
      ChangeMap() {}
      int val(Fraction tick);
      std::vector<std::pair<Fraction, Fraction>> changesInRange(Fraction stick, Fraction etick);

      void addFixed(Fraction tick, int value);
      void addRamp(Fraction stick, Fraction etick, int change, ChangeMethod method, ChangeDirection direction);
      void cleanup();

      void dump();

      static int interpolate(ChangeEvent& event, Fraction& tick);
      static QString changeMethodToName(ChangeMethod method);
      static ChangeMethod nameToChangeMethod(QString name);

      static const std::vector<ChangeMethodItem> changeMethodTable;
      };

}     // namespace Ms
#endif

