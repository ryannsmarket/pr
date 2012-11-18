//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: staff.cpp 5333 2012-02-17 10:39:33Z miwarre $
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "mscore.h"
#include "staff.h"
#include "part.h"
#include "clef.h"
#include "xml.h"
#include "score.h"
#include "bracket.h"
#include "keysig.h"
#include "segment.h"
#include "style.h"
#include "measure.h"
#include "tablature.h"
#include "stafftype.h"
#include "undo.h"
#include "cleflist.h"
#include "timesig.h"
#include "instrtemplate.h"
#include "barline.h"

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Staff::idx() const
      {
      return _score->staffIdx(this);
      }

//---------------------------------------------------------
//   bracket
//---------------------------------------------------------

BracketType Staff::bracket(int idx) const
      {
      if (idx < _brackets.size())
            return _brackets[idx]._bracket;
      return NO_BRACKET;
      }

//---------------------------------------------------------
//   bracketSpan
//---------------------------------------------------------

int Staff::bracketSpan(int idx) const
      {
      if (idx < _brackets.size())
            return _brackets[idx]._bracketSpan;
      return 0;
      }

//---------------------------------------------------------
//   setBracket
//---------------------------------------------------------

void Staff::setBracket(int idx, BracketType val)
      {
      for (int i = _brackets.size(); i <= idx; ++i)
            _brackets.append(BracketItem());
      _brackets[idx]._bracket = val;
      while (!_brackets.isEmpty() && (_brackets.last()._bracket == NO_BRACKET))
            _brackets.removeLast();
      }

//---------------------------------------------------------
//   setBracketSpan
//---------------------------------------------------------

void Staff::setBracketSpan(int idx, int val)
      {
      Q_ASSERT(idx >= 0);
      Q_ASSERT(val >= 0);
      for (int i = _brackets.size(); i <= idx; ++i)
            _brackets.append(BracketItem());
      _brackets[idx]._bracketSpan = val;
      }

//---------------------------------------------------------
//   addBracket
//---------------------------------------------------------

void Staff::addBracket(BracketItem b)
      {
      if (!_brackets.isEmpty() && _brackets[0]._bracket == NO_BRACKET) {
            _brackets[0] = b;
            }
      else {
            //
            // create new bracket level
            //
            foreach(Staff* s, _score->staves()) {
                  if (s == this)
                        s->_brackets.append(b);
                  else
                        s->_brackets.append(BracketItem());
                  }
            }
      }

//---------------------------------------------------------
//   cleanupBrackets
//---------------------------------------------------------

void Staff::cleanupBrackets()
      {
      int index = idx();
      int n = _score->nstaves();
      for (int i = 0; i < _brackets.size(); ++i) {
            if (_brackets[i]._bracket != NO_BRACKET) {
                  int span = _brackets[i]._bracketSpan;
                  if (span > (n - index)) {
                        span = n - index;
                        _brackets[i]._bracketSpan = span;
                        }
                  }
            }
      for (int i = 0; i < _brackets.size(); ++i) {
            if (_brackets[i]._bracket != NO_BRACKET) {
                  int span = _brackets[i]._bracketSpan;
                  if (span <= 1)
                        _brackets[i] = BracketItem();
                  else {
                        // delete all other brackets with same span
                        for (int k = i + 1; k < _brackets.size(); ++k) {
                              if (span == _brackets[k]._bracketSpan)
                                    _brackets[k] = BracketItem();
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   partName
//---------------------------------------------------------

QString Staff::partName() const
      {
      return _part->partName();
      }

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff(Score* s)
      {
      _score          = s;
      _rstaff         = 0;
      _part           = 0;
      _keymap         = new KeyList;
      (*_keymap)[0]   = KeySigEvent(0);                  // default to C major
      _staffType      = _score->staffType(PITCHED_STAFF_TYPE);
      _small          = false;
      _invisible      = false;
      _userDist       = .0;
      _barLineSpan    = 1;
      _barLineFrom    = 0;
      _barLineTo      = (lines()-1)*2;
      _updateKeymap   = true;
      _linkedStaves   = 0;
      _initialClef    = ClefTypeList(CLEF_G, CLEF_G);
      }

Staff::Staff(Score* s, Part* p, int rs)
      {
      _score          = s;
      _rstaff         = rs;
      _part           = p;
      _keymap         = new KeyList;
      (*_keymap)[0]   = KeySigEvent(0);                  // default to C major
      _staffType      = _score->staffType(PITCHED_STAFF_TYPE);
      _small          = false;
      _invisible      = false;
      _userDist       = .0;
      _barLineSpan    = 1;
      _barLineFrom    = 0;
      _barLineTo      = (lines()-1)*2;
      _updateKeymap   = true;
      _linkedStaves   = 0;
      _initialClef    = ClefTypeList(CLEF_G, CLEF_G);
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
      if (_linkedStaves) {
            _linkedStaves->remove(this);
            if (_linkedStaves->isEmpty())
                  delete _linkedStaves;
            }
      delete _keymap;
      }

//---------------------------------------------------------
//   Staff::clefTypeList
//---------------------------------------------------------

ClefTypeList Staff::clefTypeList(int tick) const
      {
      ClefTypeList ctl = _initialClef;
      int track  = idx() * VOICES;
      for (Segment* s = score()->firstSegment(); s; s = s->next1()) {
            if (s->tick() > tick)
                  break;
            if (s->subtype() != Segment::SegClef)
                  continue;
            if (s->element(track) && !s->element(track)->generated())
                  ctl = static_cast<Clef*>(s->element(track))->clefTypeList();
            }
      return ctl;
      }

//---------------------------------------------------------
//   Staff::clef
//---------------------------------------------------------

ClefType Staff::clef(int tick) const
      {
      Clef* clef = 0;
      foreach(Clef* c, clefs) {
            if (c->segment()->tick() > tick)
                  break;
            clef = c;
            }
      return clef == 0 ? _initialClef._concertClef : clef->clefType();
      }

ClefType Staff::clef(Segment* segment) const
      {
      ClefType ct = _initialClef._concertClef;
      int track = idx() * VOICES;
      for (;;) {
            segment = segment->prev1(Segment::SegClef);
            if (segment == 0)
                  break;
            if (segment->element(track)) {
                  ct = static_cast<Clef*>(segment->element(track))->clefType();
                  break;
                  }
            }
      return ct;
      }

//---------------------------------------------------------
//   timeStretch
//---------------------------------------------------------

Fraction Staff::timeStretch(int tick) const
      {
      TimeSig* timesig = timeSig(tick);
      return timesig == 0 ? Fraction(1,1) : timesig->stretch();
      }

//---------------------------------------------------------
//   timeSig
//    lookup time signature before or at tick
//---------------------------------------------------------

TimeSig* Staff::timeSig(int tick) const
      {
      TimeSig* timesig = 0;
      foreach(TimeSig* ts, timesigs) {
            if (ts->segment()->tick() > tick)
                  break;
            timesig = ts;
            }
      return timesig;
      }

//---------------------------------------------------------
//   clefsGreater
//---------------------------------------------------------

static bool clefsGreater(const Clef* a, const Clef* b)
      {
      return a->segment()->tick() < b->segment()->tick();
      }

//---------------------------------------------------------
//   addClef
//---------------------------------------------------------

void Staff::addClef(Clef* clef)
      {
      if (clef->generated()) {
            if (clef->segment()->tick() == 0)
                  _initialClef._concertClef = clef->clefType();
            return;
            }
      if (clef->segment()->measure() == 0)
            abort();
      int tick = 0;
      if (!clefs.isEmpty())
            tick = clefs.back()->segment()->tick();
      clefs.append(clef);
      if (clef->segment()->tick() < tick)
            qSort(clefs.begin(), clefs.end(), clefsGreater);
      }

//---------------------------------------------------------
//   timesigsGreater
//---------------------------------------------------------

static bool timesigsGreater(const TimeSig* a, const TimeSig* b)
      {
      return a->segment()->tick() < b->segment()->tick();
      }

//---------------------------------------------------------
//   addTimeSig
//---------------------------------------------------------

void Staff::addTimeSig(TimeSig* timesig)
      {
      int tick = 0;
      if (!timesigs.isEmpty())
            tick = timesigs.back()->segment()->tick();
      timesigs.append(timesig);
      if (timesig->segment()->tick() < tick)
            qSort(timesigs.begin(), timesigs.end(), timesigsGreater);
      }

//---------------------------------------------------------
//   removeClef
//---------------------------------------------------------

void Staff::removeClef(Clef* clef)
      {
      clefs.removeOne(clef);
      }

//---------------------------------------------------------
//   removeTimeSig
//---------------------------------------------------------

void Staff::removeTimeSig(TimeSig* timesig)
      {
      timesigs.removeOne(timesig);
      }

//---------------------------------------------------------
//   Staff::key
//---------------------------------------------------------

KeySigEvent Staff::key(int tick) const
      {
      return _keymap->key(tick);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Staff::write(Xml& xml) const
      {
      int idx = score()->staffIdx(this);
      xml.stag(QString("Staff id=\"%1\"").arg(idx+1));
      if (linkedStaves()) {
            Score* s = score();
            if (s->parentScore())
                  s = s->parentScore();
            foreach(Staff* staff, linkedStaves()->staves()) {
                  if ((staff->score() == s) && (staff != this))
                        xml.tag("linkedTo", s->staffIdx(staff) + 1);
                  }
            }
      xml.tag("type", score()->staffTypeIdx(_staffType));
      if (small() && !xml.excerptmode)    // switch small staves to normal ones when extracting part
            xml.tag("small", small());
      if (invisible())
            xml.tag("invisible", invisible());
      foreach(const BracketItem& i, _brackets)
            xml.tagE("bracket type=\"%d\" span=\"%d\"", i._bracket, i._bracketSpan);
      // for economy and consistency, only output "from" and "to" attributes if different from default
      int defaultLineTo = _barLineSpan == 0 ? _barLineTo : (score()->staff(idx+_barLineSpan-1)->lines() - 1) * 2;
      if (_barLineSpan != 1 || _barLineFrom != 0 || _barLineTo != defaultLineTo) {
            if(_barLineFrom != 0 || _barLineTo != defaultLineTo)
                  xml.tag(QString("barLineSpan from=\"%1\" to=\"%2\"").arg(_barLineFrom).arg(_barLineTo), _barLineSpan);
            else
                  xml.tag("barLineSpan", _barLineSpan);
            }
      if (_userDist != 0.0)
            xml.tag("distOffset", _userDist / spatium());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            const QString& val(e.text());
            if (tag == "type") {
                  StaffType* st = score()->staffType(val.toInt());
                  if (st) {
                        _staffType = st;
                        _barLineTo = (lines()-1) * 2; // set default barLineTo according to staff type
                        }
                  }
            else if (tag == "small")
                  setSmall(val.toInt());
            else if (tag == "invisible")
                  setInvisible(val.toInt());
            else if (tag == "keylist")
                  _keymap->read(e, _score);
            else if (tag == "bracket") {
                  BracketItem b;
                  b._bracket = BracketType(e.attribute("type", "-1").toInt());
                  b._bracketSpan = e.attribute("span", "0").toInt();
                  _brackets.append(b);
                  }
            else if (tag == "barLineSpan") {
                  _barLineSpan = val.toInt();
                  _barLineFrom = e.attribute("from", "0").toInt();
// WARNING: following statement assume staff type is correctly set
                  // if no bar line or single staff span, set _barLineTo to this staff height
                  // if span to another staff (yet to be read), set to unknown
                  // (Score::read() will retrieve the correct height of the target staff)
                  int defaultLineTo = _barLineSpan <= 1 ? (lines() - 1) * 2 : UNKNOWN_BARLINE_TO;
                  _barLineTo   = e.attribute("to", QString::number(defaultLineTo)).toInt();
                  }
            else if (tag == "distOffset")
                  _userDist = e.text().toDouble() * spatium();
            else if (tag == "linkedTo") {
                  int v = val.toInt() - 1;
                  //
                  // if this is an excerpt, link staff to parentScore()
                  //
                  if (score()->parentScore()) {
                        Staff* st = score()->parentScore()->staff(v);
                        if (st)
                              linkTo(st);
                        else {
                              qDebug("staff %d not found in parent", v);
                              }
                        }
                  else {
                        int idx = score()->staffIdx(this);
                        if (v < idx)
                              linkTo(score()->staff(v));
                        }
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   height
//---------------------------------------------------------

qreal Staff::height() const
      {
      return (lines()-1) * spatium() * _staffType->lineDistance().val();
      }

//---------------------------------------------------------
//   spatium
//---------------------------------------------------------

qreal Staff::spatium() const
      {
      return _score->spatium() * mag();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Staff::mag() const
      {
      return _small ? score()->styleD(ST_smallStaffMag) : 1.0;
      }

//---------------------------------------------------------
//   setKey
//---------------------------------------------------------

void Staff::setKey(int tick, int st)
      {
      KeySigEvent ke;
      ke.setAccidentalType(st);

      setKey(tick, ke);
      }

void Staff::setKey(int tick, const KeySigEvent& st)
      {
      (*_keymap)[tick] = st;
      }

//---------------------------------------------------------
//   removeKey
//---------------------------------------------------------

void Staff::removeKey(int tick)
      {
      _keymap->erase(tick);
      }

//---------------------------------------------------------
//   channel
//---------------------------------------------------------

int Staff::channel(int tick,  int voice) const
      {
      if (_channelList[voice].isEmpty())
            return 0;
      QMap<int, int>::const_iterator i = _channelList[voice].lowerBound(tick);
      if (i == _channelList[voice].begin())
            return _channelList[voice].begin().value();
      --i;
      return i.value();
      }

//---------------------------------------------------------
//   lines
//---------------------------------------------------------

int Staff::lines() const
      {
      return _staffType->lines();
      }

//---------------------------------------------------------
//   setLines
//---------------------------------------------------------

void Staff::setLines(int val)
      {
      if (val == lines())
            return;
      //
      // create new staff type
      //
      StaffType* st = _staffType->clone();
      if(part())
            st->setName(partName());
      st->setLines(val);
      setStaffType(st);
      score()->addStaffType(st);
      }

//---------------------------------------------------------
//   line distance
//---------------------------------------------------------

qreal Staff::lineDistance() const
      {
      return _staffType->lineDistance().val();
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Staff::slashStyle() const
      {
      return _staffType->slashStyle();
      }

//---------------------------------------------------------
//   setSlashStyle
//---------------------------------------------------------

void Staff::setSlashStyle(bool val)
      {
      _staffType->setSlashStyle(val);
      }

//---------------------------------------------------------
//   linkTo
//---------------------------------------------------------

void Staff::linkTo(Staff* staff)
      {
      if (!_linkedStaves) {
            if (staff->linkedStaves()) {
                  _linkedStaves = staff->linkedStaves();
                  }
            else {
                  _linkedStaves = new LinkedStaves;
                  _linkedStaves->add(staff);
                  staff->setLinkedStaves(_linkedStaves);
                  }
            _linkedStaves->add(this);
            }
      else {
            qDebug("Staff::linkTo: staff already linked\n");
            abort();
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void LinkedStaves::add(Staff* staff)
      {
      _staves.append(staff);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void LinkedStaves::remove(Staff* staff)
      {
      _staves.removeOne(staff);
      }

//---------------------------------------------------------
//   primaryStaff
///   if there are linked staves, the primary staff is
///   the one who is played back
//---------------------------------------------------------

bool Staff::primaryStaff() const
      {
      QList<Staff*> s;
      if (!_linkedStaves)
            return true;
      foreach(Staff* staff, _linkedStaves->staves()) {
            if (staff->score() == score())
                  s.append(staff);
            }
      return s.front() == this;
      }

//---------------------------------------------------------
//   setStaffType
//---------------------------------------------------------

void Staff::setStaffType(StaffType* st)
      {
      if (_staffType == st)
            return;
      int linesOld = lines();
      int linesNew = st->lines();
      _staffType = st;
      if(linesNew != linesOld) {
            int sIdx = score()->staffIdx(this);
            if(sIdx < 0) {                      // staff does not belong to score (yet?)
                  _barLineFrom = 0;             // set default barLineFrom/To
                  _barLineTo = (linesNew-1)*2;  // set default barLineTo
                  }
            else                                // update barLineFrom/To in whole score context
                  score()->updateBarLineSpans(sIdx, linesOld, linesNew /*, true*/);
            }

      //
      //    check for right clef-type and fix
      //    if necessary
      //
      ClefType ct = clef(0);
      StaffGroup csg = clefTable[ct].staffGroup;

      if (_staffType->group() != csg) {
            switch(_staffType->group()) {
                  case TAB_STAFF:        ct = ClefType(score()->styleI(ST_tabClef)); break;
                  case PITCHED_STAFF:    ct = CLEF_G; break;      // TODO: use preferred clef for instrument
                  case PERCUSSION_STAFF: ct = CLEF_PERC; break;
                  }
            setInitialClef(ct);
            }
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Staff::init(const InstrumentTemplate* t, int cidx)
      {
      // set staff-type-independent parameters
      if (cidx > MAX_STAVES) {
            setSmall(false);
            setInitialClef(t->clefIdx[0]);
            }
      else {
            setSmall(t->smallStaff[cidx]);
            setInitialClef(t->clefIdx[cidx]);         // initial clef will be fixed to staff-type clef by setStaffType()
            setBracket(0, t->bracket[cidx]);
            setBracketSpan(0, t->bracketSpan[cidx]);
            setBarLineSpan(t->barlineSpan[cidx]);
            }

      // determine staff type and set number of lines accordingly
      // set lines AFTER setting the staff type, so if lines are different, the right staff type is cloned
      StaffType* st;
      if (t->useTablature && t->tablature) {
            setStaffType(score()->staffType(TAB_STAFF_TYPE));
            setLines(t->tablature->strings());        // use number of lines from tablature definition:
            }
      else {
            if (t->useDrumset)
                  st = score()->staffType(PERCUSSION_STAFF_TYPE);
            else
                  st = score()->staffType(PITCHED_STAFF_TYPE);
            setStaffType(st);
            setLines(t->staffLines[cidx]);            // use number of lines from instr. template
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Staff::spatiumChanged(qreal oldValue, qreal newValue)
      {
      _userDist = (_userDist / oldValue) * newValue;
      }

//---------------------------------------------------------
//   show
//---------------------------------------------------------

bool Staff::show() const
      {
      return _part->show();
      }

