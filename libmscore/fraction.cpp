//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "fraction.h"
#include "mscore.h"

//---------------------------------------------------------
//   gcd
//    greatest common divisor
//---------------------------------------------------------

static int gcd(int a, int b)
      {
      if (b == 0)
            return a < 0 ? -a : a;
      return gcd(b, a % b);
      }

//---------------------------------------------------------
//   lcm
//    least common multiple
//---------------------------------------------------------

static unsigned lcm(int a, int b)
      {
      return a * b / gcd(a, b);
      }

//---------------------------------------------------------
//   Fraction
//---------------------------------------------------------

Fraction::Fraction(int z, int n)
   : _numerator(z), _denominator(n)
      {
      }

//---------------------------------------------------------
//   reduce
//---------------------------------------------------------

void Fraction::reduce()
      {
      int tmp = gcd(_numerator, _denominator);
      _numerator /= tmp;
      _denominator  /= tmp;
      }

//---------------------------------------------------------
//   reduced
//---------------------------------------------------------

Fraction Fraction::reduced() const
      {
      int tmp = gcd(_numerator, _denominator);
      return Fraction(_numerator / tmp, _denominator / tmp);
      }

//---------------------------------------------------------
//   operator+=
//---------------------------------------------------------

Fraction& Fraction::operator+=(const Fraction& val)
      {
      const int tmp = lcm(_denominator, val._denominator);
      _numerator = _numerator * (tmp / _denominator) + val._numerator * (tmp / val._denominator);
      _denominator = tmp;
      return *this;
      }

bool Fraction::operator<(const Fraction& val) const
      {
      const int v = lcm(_denominator, val._denominator);
      return _numerator * (v / _denominator) < val._numerator * (v / val._denominator);
      }

bool Fraction::operator<=(const Fraction& val) const
      {
      const int v = lcm(_denominator, val._denominator);
      return _numerator * (v / _denominator) <= val._numerator * (v / val._denominator);
      }

bool Fraction::operator>=(const Fraction& val) const
      {
      const int v = lcm(_denominator, val._denominator);
      return _numerator * (v / _denominator) >= val._numerator * (v / val._denominator);
      }

bool Fraction::operator>(const Fraction& val) const
      {
      const int v = lcm(_denominator, val._denominator);
      return (_numerator * (v / _denominator)) > (val._numerator * (v / val._denominator));
      }

bool Fraction::operator==(const Fraction& val) const
      {
      const int v = lcm(_denominator, val._denominator);
      return (_numerator * (v / _denominator)) == (val._numerator * (v / val._denominator));
//      return (_numerator == val._numerator) && (_denominator == val._denominator);
      }

bool Fraction::operator!=(const Fraction& val) const
      {
      const int v = lcm(_denominator, val._denominator);
      return (_numerator * (v / _denominator)) != (val._numerator * (v / val._denominator));
//      return (_numerator != val._numerator) || (_denominator != val._denominator);
      }

//---------------------------------------------------------
//   operator-=
//---------------------------------------------------------

Fraction& Fraction::operator-=(const Fraction& val)
      {
      const unsigned tmp = lcm(_denominator, val._denominator);
      _numerator = _numerator * (tmp / _denominator) - val._numerator * (tmp / val._denominator);
      _denominator  = tmp;
      return *this;
      }

//---------------------------------------------------------
//   operator*=
//---------------------------------------------------------

Fraction& Fraction::operator*=(const Fraction& val)
      {
      _numerator *= val._numerator;
      _denominator  *= val._denominator;
      return *this;
      }

Fraction& Fraction::operator*=(int val)
      {
      _numerator *= val;
      return *this;
      }

//---------------------------------------------------------
//   operator/=
//---------------------------------------------------------

Fraction& Fraction::operator/=(const Fraction& val)
      {
      _numerator *= val._denominator;
      _denominator  *= val._numerator;
      return *this;
      }

Fraction& Fraction::operator/=(int val)
      {
      _denominator  *= val;
      return *this;
      }

//---------------------------------------------------------
//   fromTicks
//---------------------------------------------------------

Fraction Fraction::fromTicks(int ticks)
      {
      return Fraction(ticks, MScore::division * 4).reduced();
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int Fraction::ticks() const
      {
      return (_numerator * MScore::division * 4 + (_denominator/2)) / _denominator;
      }


