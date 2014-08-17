//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "navigate.h"
#include "element.h"
#include "clef.h"
#include "score.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "system.h"
#include "segment.h"
#include "lyrics.h"
#include "harmony.h"
#include "utils.h"
#include "input.h"
#include "measure.h"
#include "page.h"

namespace Ms {

//---------------------------------------------------------
//   nextChordRest
//    return next Chord or Rest
//---------------------------------------------------------

ChordRest* nextChordRest(ChordRest* cr)
      {
      if (!cr)
            return 0;

      if (cr->isGrace()) {
            //
            // cr is a grace note

            Chord* c  = static_cast<Chord*>(cr);
            Chord* pc = static_cast<Chord*>(cr->parent());

            if (cr->isGraceBefore()) {
                  QList<Chord*> graceNotesBefore;
                  pc->getGraceNotesBefore(&graceNotesBefore);
                  auto i = std::find(graceNotesBefore.begin(), graceNotesBefore.end(), c);
                  if (i == graceNotesBefore.end())
                        return 0;   // unable to find self?
                  ++i;
                  if (i != graceNotesBefore.end())
                        return *i;
                  // if this was last grace note before, return parent
                  return pc;
                  }
            else {
                  QList<Chord*> graceNotesAfter;
                  pc->getGraceNotesAfter(&graceNotesAfter);
                  auto i = std::find(graceNotesAfter.begin(), graceNotesAfter.end(), c);
                  if (i == graceNotesAfter.end())
                        return 0;   // unable to find self?
                  ++i;
                  if (i != graceNotesAfter.end())
                        return *i;
                  // if this was last grace note after, fall through to find next main note
                  cr = pc;
                  }
            }
      else {
            //
            // cr is not a grace note
            if (cr->type() == Element::Type::CHORD) {
                  Chord* c = static_cast<Chord*>(cr);
                  if (!c->graceNotes().empty()) {
                        QList<Chord*> graceNotesAfter;
                        c->getGraceNotesAfter(&graceNotesAfter);
                        if (!graceNotesAfter.isEmpty())
                              return graceNotesAfter.first();
                        }
                  }
            }

      int track = cr->track();
      Segment::Type st = Segment::Type::ChordRest;

      for (Segment* seg = cr->segment()->next1(st); seg; seg = seg->next1(st)) {
            ChordRest* e = static_cast<ChordRest*>(seg->element(track));
            if (e) {
                  if (e->type() == Element::Type::CHORD) {
                        Chord* c = static_cast<Chord*>(e);
                        if (!c->graceNotes().empty()) {
                              QList<Chord*> graceNotesBefore;
                              c->getGraceNotesBefore(&graceNotesBefore);
                              if (!graceNotesBefore.isEmpty())
                                    return graceNotesBefore.first();
                              }
                        }
                  return e;
                  }
            }

      return 0;
      }

//---------------------------------------------------------
//   prevChordRest
//    return previous Chord or Rest
//---------------------------------------------------------

ChordRest* prevChordRest(ChordRest* cr)
      {
      if (!cr)
            return 0;

      if (cr->isGrace()) {
            //
            // cr is a grace note

            Chord* c  = static_cast<Chord*>(cr);
            Chord* pc = static_cast<Chord*>(cr->parent());

            if (cr->isGraceBefore()) {
                  QList<Chord*> graceNotesBefore;
                  pc->getGraceNotesBefore(&graceNotesBefore);
                  auto i = std::find(graceNotesBefore.begin(), graceNotesBefore.end(), c);
                  if (i == graceNotesBefore.end())
                        return 0;   // unable to find self?
                  if (i != graceNotesBefore.begin())
                        return *--i;
                  // if this was first grace note before, fall through to find previous main note
                  cr = pc;
                  }
            else {
                  QList<Chord*> graceNotesAfter;
                  pc->getGraceNotesAfter(&graceNotesAfter);
                  auto i = std::find(graceNotesAfter.begin(), graceNotesAfter.end(), c);
                  if (i == graceNotesAfter.end())
                        return 0;   // unable to find self?
                  if (i != graceNotesAfter.begin())
                        return *--i;
                  // if this was first grace note after, return parent
                  return pc;
                  }
            }
      else {
            //
            // cr is not a grace note
            if (cr->type() == Element::Type::CHORD) {
                  Chord* c = static_cast<Chord*>(cr);
                  if (!c->graceNotes().empty()) {
                        QList<Chord*> graceNotesBefore;
                        c->getGraceNotesBefore(&graceNotesBefore);
                        if (!graceNotesBefore.isEmpty())
                              return graceNotesBefore.last();
                        }
                  }
            }

      int track = cr->track();
      Segment::Type st = Segment::Type::ChordRest;
      for (Segment* seg = cr->segment()->prev1(st); seg; seg = seg->prev1(st)) {
            ChordRest* e = static_cast<ChordRest*>(seg->element(track));
            if (e) {
                  if (e->type() == Element::Type::CHORD) {
                        Chord* c = static_cast<Chord*>(e);
                        if (!c->graceNotes().empty()) {
                              QList<Chord*> graceNotesAfter;
                              c->getGraceNotesAfter(&graceNotesAfter);
                              if (!graceNotesAfter.isEmpty())
                                    return graceNotesAfter.last();
                              }
                        }
                  return e;
                  }
            }

      return 0;
      }

//---------------------------------------------------------
//   upAlt
//    element: Note() or Rest()
//    return: Note() or Rest()
//
//    return next higher pitched note in chord
//    move to previous track if at top of chord
//---------------------------------------------------------

Element* Score::upAlt(Element* element)
      {
      Element* re = 0;
      if (element->type() == Element::Type::REST)
            re = prevTrack(static_cast<Rest*>(element));
      else if (element->type() == Element::Type::NOTE) {
            Chord* chord = static_cast<Note*>(element)->chord();
            const QList<Note*>& notes = chord->notes();
            int idx = notes.indexOf(static_cast<Note*>(element));
            if (idx < notes.size()-1) {
                  ++idx;
                  re = notes.value(idx);
                  }
            else {
                  re = prevTrack(chord);
                  if (re->track() == chord->track())
                        re = element;
                  }
            }
      if (re == 0)
            return 0;
      if (re->type() == Element::Type::CHORD)
            re = static_cast<Chord*>(re)->notes().front();
      return re;
      }

//---------------------------------------------------------
//   upAltCtrl
//    select top note in chord
//---------------------------------------------------------

Note* Score::upAltCtrl(Note* note) const
      {
      return note->chord()->upNote();
      }

//---------------------------------------------------------
//   downAlt
//    return next lower pitched note in chord
//    move to previous track if at bottom of chord
//---------------------------------------------------------

Element* Score::downAlt(Element* element)
      {
      Element* re = 0;
      if (element->type() == Element::Type::REST)
            re = nextTrack(static_cast<Rest*>(element));
      else if (element->type() == Element::Type::NOTE) {
            Chord* chord = static_cast<Note*>(element)->chord();
            const QList<Note*>& notes = chord->notes();
            int idx = notes.indexOf(static_cast<Note*>(element));
            if (idx > 0) {
                  --idx;
                  re = notes.value(idx);
                  }
            else {
                  re = nextTrack(chord);
                  if (re->track() == chord->track())
                        re = element;
                  }
            }
      if (re == 0)
            return 0;
      if (re->type() == Element::Type::CHORD)
            re = static_cast<Chord*>(re)->notes().back();
      return re;
      }

//---------------------------------------------------------
//   downAltCtrl
//    niedrigste Note in Chord selektieren
//---------------------------------------------------------

Note* Score::downAltCtrl(Note* note) const
      {
      return note->chord()->downNote();
      }

//---------------------------------------------------------
//   upStaff
//---------------------------------------------------------

ChordRest* Score::upStaff(ChordRest* cr)
      {
      Segment* segment = cr->segment();

      if (cr->staffIdx() == 0)
            return cr;

      for (int track = (cr->staffIdx() - 1) * VOICES; track >= 0; --track) {
            Element* el = segment->element(track);
            if (!el)
                  continue;
            if (el->type() == Element::Type::NOTE)
                  el = static_cast<Note*>(el)->chord();
            if (el->isChordRest())
                  return static_cast<ChordRest*>(el);
            }
      return 0;
      }

//---------------------------------------------------------
//   downStaff
//---------------------------------------------------------

ChordRest* Score::downStaff(ChordRest* cr)
      {
      Segment* segment = cr->segment();
      int tracks = nstaves() * VOICES;

      if (cr->staffIdx() == nstaves() - 1)
            return cr;

      for (int track = (cr->staffIdx() + 1) * VOICES; track < tracks; --track) {
            Element* el = segment->element(track);
            if (!el)
                  continue;
            if (el->type() == Element::Type::NOTE)
                  el = static_cast<Note*>(el)->chord();
            if (el->isChordRest())
                  return static_cast<ChordRest*>(el);
            }
      return 0;
      }

//---------------------------------------------------------
//   nextTrack
//    returns note at or just before current (cr) position
//    in next track for this measure
//    that contains such an element
//---------------------------------------------------------

ChordRest* Score::nextTrack(ChordRest* cr)
      {
      if (!cr)
            return 0;

      ChordRest* el = 0;
      Measure* measure = cr->measure();
      int track = cr->track();
      int tracks = nstaves() * VOICES;

      while (!el) {
            // find next non-empty track
            while (++track < tracks){
                  if (measure->hasVoice(track))
                        break;
                  }
            // no more tracks, return original element
            if (track == tracks)
                  return cr;
            // find element at same or previous segment within this track
            for (Segment* segment = cr->segment(); segment; segment = segment->prev(Segment::Type::ChordRest)) {
                  el = static_cast<ChordRest*>(segment->element(track));
                  if (el)
                        break;
                  }
            }
      return el;
      }

//---------------------------------------------------------
//   prevTrack
//    returns ChordRest at or just before current (cr) position
//    in previous track for this measure
//    that contains such an element
//---------------------------------------------------------

ChordRest* Score::prevTrack(ChordRest* cr)
      {
      if (!cr)
            return 0;

      ChordRest* el = 0;
      Measure* measure = cr->measure();
      int track = cr->track();

      while (!el) {
            // find next non-empty track
            while (--track >= 0){
                  if (measure->hasVoice(track))
                        break;
                  }
            // no more tracks, return original element
            if (track < 0)
                  return cr;
            // find element at same or previous segment within this track
            for (Segment* segment = cr->segment(); segment != 0; segment = segment->prev(Segment::Type::ChordRest)) {
                  el = static_cast<ChordRest*>(segment->element(track));
                  if (el)
                        break;
                  }
            }
      return el;
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

ChordRest* Score::nextMeasure(ChordRest* element, bool selectBehavior)
      {
      if (!element)
            return 0;

      Measure* measure = element->measure()->nextMeasure();
      if (measure == 0)
            return 0;

      int endTick = element->measure()->last()->nextChordRest(element->track(), true)->tick();
      bool last   = false;

      if (selection().isRange()) {
            if (element->tick() != endTick && selection().tickEnd() <= endTick) {
                  measure = element->measure();
                  last = true;
                  }
            else if (element->tick() == endTick && selection().isEndActive())
                  last = true;
            }
      else if (element->tick() != endTick && selectBehavior) {
            measure = element->measure();
            last = true;
            }
      if (!measure) {
            measure = element->measure();
            last = true;
            }
      int staff = element->staffIdx();

      Segment* startSeg = last ? measure->last() : measure->first();
      for (Segment* seg = startSeg; seg; seg = last ? seg->prev() : seg->next()) {
            int etrack = (staff+1) * VOICES;
            for (int track = staff * VOICES; track < etrack; ++track) {
                  Element* pel = seg->element(track);

                  if (pel && pel->isChordRest())
                        return static_cast<ChordRest*>(pel);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

ChordRest* Score::prevMeasure(ChordRest* element)
      {
      if (!element)
            return 0;

      MeasureBase* mb = element->measure()->prev();
      while (mb && mb->type() != Element::Type::MEASURE)
            mb = mb->prev();

      Measure* measure = static_cast<Measure*>(mb);

      int startTick = element->measure()->first()->nextChordRest(element->track())->tick();
      bool last = false;

      if ((selection().isRange())
         && selection().isEndActive() && selection().startSegment()->tick() <= startTick)
            last = true;
      else if (element->tick() != startTick) {
            measure = element->measure();
            }
      if (!measure) {
            measure = element->measure();
            last = false;
            }

      int staff = element->staffIdx();

      Segment* startSeg = last ? measure->last() : measure->first();
      for (Segment* seg = startSeg; seg; seg = last ? seg->prev() : seg->next()) {
            int etrack = (staff+1) * VOICES;
            for (int track = staff * VOICES; track < etrack; ++track) {
                  Element* pel = seg->element(track);

                  if (pel && pel->isChordRest())
                        return static_cast<ChordRest*>(pel);
                  }
            }
      return 0;
      }

}

