//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011-2012 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "accidental.h"
#include "arpeggio.h"
#include "articulation.h"
#include "bend.h"
#include "config.h"
#include "fingering.h"
#include "glissando.h"
#include "hairpin.h"
#include "hook.h"
#include "letring.h"
#include "lyrics.h"
#include "palmmute.h"
#include "pedal.h"
#include "score.h"
#include "stem.h"
#include "stemslash.h"
#include "trill.h"
#include "vibrato.h"
#include "xml.h"
#include "element.h"
#include "measure.h"
#include "notedot.h"
#include "segment.h"
#include "slur.h"
#include "tie.h"
#include "chordrest.h"
#include "chord.h"
#include "tuplet.h"
#include "beam.h"
#include "revisions.h"
#include "page.h"
#include "part.h"
#include "staff.h"
#include "system.h"
#include "keysig.h"
#include "clef.h"
#include "text.h"
#include "ottava.h"
#include "volta.h"
#include "excerpt.h"
#include "mscore.h"
#include "stafftype.h"
#include "sym.h"

#ifdef OMR
#include "omr/omr.h"
#include "omr/omrpage.h"
#endif

#include "sig.h"
#include "undo.h"
#include "imageStore.h"
#include "audio.h"
#include "barline.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"
#ifdef Q_OS_WIN
#include <windows.h>
#include <stdio.h>
#endif

namespace Ms {

//---------------------------------------------------------
//   writeMeasure
//---------------------------------------------------------

static void writeMeasure(XmlWriter& xml, MeasureBase* m, int staffIdx, bool writeSystemElements, bool forceTimeSig)
      {
      //
      // special case multi measure rest
      //
      if (m->isMeasure() || staffIdx == 0)
            m->write300old(xml, staffIdx, writeSystemElements, forceTimeSig);

      if (m->score()->styleB(Sid::createMultiMeasureRests) && m->isMeasure() && toMeasure(m)->mmRest())
            toMeasure(m)->mmRest()->write300old(xml, staffIdx, writeSystemElements, forceTimeSig);

      xml.setCurTick(m->endTick());
      }

//---------------------------------------------------------
//   ElementList::write300old
//---------------------------------------------------------

void ElementList::write300old(XmlWriter& xml) const
      {
      for (const Element* e : *this)
            e->write300old(xml);
      }

//---------------------------------------------------------
//   writeMovement300old
//---------------------------------------------------------

void Score::writeMovement300old(XmlWriter& xml, bool selectionOnly)
      {
      // if we have multi measure rests and some parts are hidden,
      // then some layout information is missing:
      // relayout with all parts set visible

      QList<Part*> hiddenParts;
      bool unhide = false;
      if (styleB(Sid::createMultiMeasureRests)) {
            for (Part* part : _parts) {
                  if (!part->show()) {
                        if (!unhide) {
                              startCmd();
                              unhide = true;
                              }
                        part->undoChangeProperty(Pid::VISIBLE, true);
                        hiddenParts.append(part);
                        }
                  }
            }
      if (unhide) {
            doLayout();
            for (Part* p : hiddenParts)
                  p->setShow(false);
            }

      xml.stag("Score");
      if (excerpt()) {
            Excerpt* e = excerpt();
            QMultiMap<int, int> trackList = e->tracks();
            QMapIterator<int, int> i(trackList);
            if (!(trackList.size() == e->parts().size() * VOICES) && !trackList.isEmpty()) {
                  while (i.hasNext()) {
                      i.next();
                      xml.tagE(QString("Tracklist sTrack=\"%1\" dstTrack=\"%2\"").arg(i.key()).arg(i.value()));
                      }
                  }
            }

      if (_layoutMode == LayoutMode::LINE)
            xml.tag("layoutMode", "line");

#ifdef OMR
      if (masterScore()->omr() && xml.writeOmr())
            masterScore()->omr()->write(xml);
#endif
      if (isMaster() && masterScore()->showOmr() && xml.writeOmr())
            xml.tag("showOmr", masterScore()->showOmr());
      if (_audio && xml.writeOmr()) {
            xml.tag("playMode", int(_playMode));
            _audio->write(xml);
            }

      for (int i = 0; i < 32; ++i) {
            if (!_layerTags[i].isEmpty()) {
                  xml.tag(QString("LayerTag id=\"%1\" tag=\"%2\"").arg(i).arg(_layerTags[i]),
                     _layerTagComments[i]);
                  }
            }
      int n = _layer.size();
      for (int i = 1; i < n; ++i) {       // dont save default variant
            const Layer& l = _layer[i];
            xml.tagE(QString("Layer name=\"%1\" mask=\"%2\"").arg(l.name).arg(l.tags));
            }
      xml.tag("currentLayer", _currentLayer);

      if (isTopScore() && !MScore::testMode)
            _synthesizerState.write(xml);

      if (pageNumberOffset())
            xml.tag("page-offset", pageNumberOffset());
      xml.tag("Division", MScore::division);
      xml.setCurTrack(-1);

      if (isTopScore())                   // only top score
            style().save(xml, true);       // save only differences to buildin style

      xml.tag("showInvisible",   _showInvisible);
      xml.tag("showUnprintable", _showUnprintable);
      xml.tag("showFrames",      _showFrames);
      xml.tag("showMargins",     _showPageborders);

      QMapIterator<QString, QString> i(_metaTags);
      while (i.hasNext()) {
            i.next();
            // do not output "platform" and "creationDate" in test mode
            if ((!MScore::testMode  && !MScore::saveTemplateMode) || (i.key() != "platform" && i.key() != "creationDate"))
                  xml.tag(QString("metaTag name=\"%1\"").arg(i.key().toHtmlEscaped()), i.value());
            }

      xml.setCurTrack(0);
      int staffStart;
      int staffEnd;
      MeasureBase* measureStart;
      MeasureBase* measureEnd;

      if (selectionOnly) {
            staffStart   = _selection.staffStart();
            staffEnd     = _selection.staffEnd();
            // make sure we select full parts
            Staff* sStaff = staff(staffStart);
            Part* sPart = sStaff->part();
            Staff* eStaff = staff(staffEnd - 1);
            Part* ePart = eStaff->part();
            staffStart = staffIdx(sPart);
            staffEnd = staffIdx(ePart) + ePart->nstaves();
            measureStart = _selection.startSegment()->measure();
            if (_selection.endSegment())
                  measureEnd   = _selection.endSegment()->measure()->next();
            else
                  measureEnd   = 0;
            }
      else {
            staffStart   = 0;
            staffEnd     = nstaves();
            measureStart = first();
            measureEnd   = 0;
            }

      // Let's decide: write midi mapping to a file or not
      masterScore()->checkMidiMapping();
      for (const Part* part : _parts) {
            if (!selectionOnly || ((staffIdx(part) >= staffStart) && (staffEnd >= staffIdx(part) + part->nstaves())))
                  part->write(xml);
            }

      xml.setCurTrack(0);
      xml.setTrackDiff(-staffStart * VOICES);
      if (measureStart) {
            for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
                  xml.stag(QString("Staff id=\"%1\"").arg(staffIdx + 1 - staffStart));
                  xml.setCurTick(measureStart->tick());
                  xml.setTickDiff(xml.curTick());
                  xml.setCurTrack(staffIdx * VOICES);
                  bool writeSystemElements = (staffIdx == staffStart);
                  bool firstMeasureWritten = false;
                  bool forceTimeSig = false;
                  for (MeasureBase* m = measureStart; m != measureEnd; m = m->next()) {
                        // force timesig if first measure and selectionOnly
                        if (selectionOnly && m->isMeasure()) {
                              if (!firstMeasureWritten) {
                                    forceTimeSig = true;
                                    firstMeasureWritten = true;
                                    }
                              else
                                    forceTimeSig = false;
                              }
                        writeMeasure(xml, m, staffIdx, writeSystemElements, forceTimeSig);
                        }
                  xml.etag();
                  }
            }
      xml.setCurTrack(-1);
      if (isMaster()) {
            if (!selectionOnly) {
                  for (const Excerpt* excerpt : excerpts()) {
                        if (excerpt->partScore() != this)
                              excerpt->partScore()->write300old(xml, false);       // recursion
                        }
                  }
            }
      else
            xml.tag("name", excerpt()->title());
      xml.etag();

      if (unhide) {
            endCmd();
            undoRedo(true, 0);   // undo
            }
      }

//---------------------------------------------------------
//   write300old
//---------------------------------------------------------

void Score::write300old(XmlWriter& xml, bool selectionOnly)
      {
      if (isMaster()) {
            MasterScore* score = static_cast<MasterScore*>(this);
            while (score->prev())
                  score = score->prev();
            while (score) {
                  score->writeMovement300old(xml, selectionOnly);
                  score = score->next();
                  }
            }
      else
            writeMovement300old(xml, selectionOnly);
      }

//---------------------------------------------------------
//   writeSegments300old
//    ls  - write upto this segment (excluding)
//          can be zero
//---------------------------------------------------------

void Score::writeSegments300old(XmlWriter& xml, int strack, int etrack,
   Segment* fs, Segment* ls, bool writeSystemElements, bool clip, bool needFirstTick, bool forceTimeSig)
      {
      int endTick = ls ? ls->tick() : lastMeasure()->endTick();
      // in clipboard mode, ls might be in an mmrest
      // since we are traversing regular measures,
      // force them out of mmRest
      if (clip) {
            Measure* lm = ls ? ls->measure() : 0;
            Measure* fm = fs ? fs->measure() : 0;
            if (lm && lm->isMMRest()) {
                  lm = lm->mmRestLast();
                  if (lm)
                        ls = lm->nextMeasure() ? lm->nextMeasure()->first() : lastSegment();
                  else
                        qDebug("writeSegments: no measure for end segment in mmrest");
                  }
            if (fm && fm->isMMRest()) {
                  fm = fm->mmRestFirst();
                  if (fm)
                        fs = fm->first();
                  }
            }

      QList<Spanner*> spanners;
#if 0
      auto endIt   = spanner().upper_bound(endTick);
      for (auto i = spanner().begin(); i != endIt; ++i) {
            Spanner* s = i->second;
#else
      auto sl = spannerMap().findOverlapping(fs->tick(), endTick);
      for (auto i : sl) {
            Spanner* s = i.value;
#endif
            if (s->generated() || !xml.canWrite(s))
                  continue;
            // don't write voltas to clipboard
            if (clip && s->isVolta())
                  continue;
            spanners.push_back(s);
            }

      for (int track = strack; track < etrack; ++track) {
            if (!xml.canWriteVoice(track))
                  continue;

            bool timeSigWritten = false; // for forceTimeSig
            bool crWritten = false;      // for forceTimeSig
            bool keySigWritten = false;  // for forceTimeSig

            for (Segment* segment = fs; segment && segment != ls; segment = segment->next1()) {
                  if (!segment->enabled())
                        continue;
                  if (track == 0)
                        segment->setWritten(false);
                  Element* e = segment->element(track);
                  //
                  // special case: - barline span > 1
                  //               - part (excerpt) staff starts after
                  //                 barline element
                  bool needTick = (needFirstTick && segment == fs) || (segment->tick() != xml.curTick());
                  if ((segment->isEndBarLineType()) && !e && writeSystemElements && ((track % VOICES) == 0)) {
                        // search barline:
                        for (int idx = track - VOICES; idx >= 0; idx -= VOICES) {
                              if (segment->element(idx)) {
                                    int oDiff = xml.trackDiff();
                                    xml.setTrackDiff(idx);          // staffIdx should be zero
                                    segment->element(idx)->write300old(xml);
                                    xml.setTrackDiff(oDiff);
                                    break;
                                    }
                              }
                        }
                  for (Element* e : segment->annotations()) {
                        if (e->track() != track || e->generated() || (e->systemFlag() && !writeSystemElements))
                              continue;
                        if (needTick) {
                              // xml.tag("tick", segment->tick() - xml.tickDiff);
                              int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                              xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                              xml.setCurTick(segment->tick());
                              needTick = false;
                              }
                        e->write300old(xml);
                        }
                  Measure* m = segment->measure();
                  // don't write spanners for multi measure rests

                  if ((!(m && m->isMMRest())) && segment->isChordRestType()) {
                        for (Spanner* s : spanners) {
                              if (s->track() == track) {
                                    bool end = false;
                                    if (s->anchor() == Spanner::Anchor::CHORD || s->anchor() == Spanner::Anchor::NOTE)
                                          end = s->tick2() < endTick;
                                    else
                                          end = s->tick2() <= endTick;
                                    if (s->tick() == segment->tick() && (!clip || end)) {
                                          if (needTick) {
                                                // xml.tag("tick", segment->tick() - xml.tickDiff);
                                                int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                                                xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                                                xml.setCurTick(segment->tick());
                                                needTick = false;
                                                }
                                          s->write300old(xml);
                                          }
                                    }
                              if ((s->tick2() == segment->tick())
                                 && !s->isSlur()
                                 && (s->track2() == track || (s->track2() == -1 && s->track() == track))
                                 && (!clip || s->tick() >= fs->tick())
                                 ) {
                                    if (needTick) {
                                          // xml.tag("tick", segment->tick() - xml.tickDiff);
                                          int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                                          xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                                          xml.setCurTick(segment->tick());
                                          needTick = false;
                                          }
                                    xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(s)));
                                    }
                              }
                        }

                  if (!e || !xml.canWrite(e))
                        continue;
                  if (e->generated())
                        continue;
                  if (forceTimeSig && track2voice(track) == 0 && segment->segmentType() == SegmentType::ChordRest && !timeSigWritten && !crWritten) {
                        // we will miss a key sig!
                        if (!keySigWritten) {
                              Key k = score()->staff(track2staff(track))->key(segment->tick());
                              KeySig* ks = new KeySig(this);
                              ks->setKey(k);
                              ks->write300old(xml);
                              delete ks;
                              keySigWritten = true;
                              }
                        // we will miss a time sig!
                        Fraction tsf = sigmap()->timesig(segment->tick()).timesig();
                        TimeSig* ts = new TimeSig(this);
                        ts->setSig(tsf);
                        ts->write300old(xml);
                        delete ts;
                        timeSigWritten = true;
                        }
                  if (needTick) {
                        // xml.tag("tick", segment->tick() - xml.tickDiff);
                        int tick = xml.clipboardmode() ? segment->tick() : segment->rtick();
                        xml.tag("move", Fraction::fromTicks(tick + xml.tickDiff()));
                        xml.setCurTick(segment->tick());
                        needTick = false;
                        }
                  if (e->isChordRest()) {
                        ChordRest* cr = toChordRest(e);
                        cr->writeBeam(xml);
                        cr->writeTuplet(xml);
                        }
//                  if (segment->isEndBarLine() && (m->mmRestCount() < 0 || m->mmRest())) {
//                        BarLine* bl = toBarLine(e);
//TODO                        bl->setBarLineType(m->endBarLineType());
//                        bl->setVisible(m->endBarLineVisible());
//                        }
                  e->write300old(xml);
                  segment->write300old(xml);    // write only once
                  if (forceTimeSig) {
                        if (segment->segmentType() == SegmentType::KeySig)
                              keySigWritten = true;
                        if (segment->segmentType() == SegmentType::TimeSig)
                              timeSigWritten = true;
                        if (segment->segmentType() == SegmentType::ChordRest)
                              crWritten = true;
                        }
                  }

            //write spanner ending after the last segment, on the last tick
            if (clip || ls == 0) {
                  for (Spanner* s : spanners) {
                        if ((s->tick2() == endTick)
                          && !s->isSlur()
                          && (s->track2() == track || (s->track2() == -1 && s->track() == track))
                          && (!clip || s->tick() >= fs->tick())
                          ) {
                              xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(s)));
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   ChordRest::writeProperties300old
//---------------------------------------------------------

void ChordRest::writeProperties300old(XmlWriter& xml) const
      {
      DurationElement::writeProperties300old(xml);

      //
      // Beam::Mode default:
      //    REST  - Beam::Mode::NONE
      //    CHORD - Beam::Mode::AUTO
      //
      if ((isRest() && _beamMode != Beam::Mode::NONE) || (isChord() && _beamMode != Beam::Mode::AUTO)) {
            QString s;
            switch(_beamMode) {
                  case Beam::Mode::AUTO:    s = "auto"; break;
                  case Beam::Mode::BEGIN:   s = "begin"; break;
                  case Beam::Mode::MID:     s = "mid"; break;
                  case Beam::Mode::END:     s = "end"; break;
                  case Beam::Mode::NONE:    s = "no"; break;
                  case Beam::Mode::BEGIN32: s = "begin32"; break;
                  case Beam::Mode::BEGIN64: s = "begin64"; break;
                  case Beam::Mode::INVALID: s = "?"; break;
                  }
            xml.tag("BeamMode", s);
            }
      writeProperty(xml, Pid::SMALL);
      if (actualDurationType().dots())
            xml.tag("dots", actualDurationType().dots());
      writeProperty(xml, Pid::STAFF_MOVE);

      if (actualDurationType().isValid())
            xml.tag("durationType", actualDurationType().name());

      if (!duration().isZero() && (!actualDurationType().fraction().isValid()
         || (actualDurationType().fraction() != duration()))) {
            xml.tag("duration", duration());
            //xml.tagE("duration z=\"%d\" n=\"%d\"", duration().numerator(), duration().denominator());
            }

#ifndef NDEBUG
      if (_beam && (MScore::testMode || !_beam->generated()))
            xml.tag("Beam", _beam->id());
#else
      if (_beam && !_beam->generated())
            xml.tag("Beam", _beam->id());
#endif
      for (Lyrics* lyrics : _lyrics)
            lyrics->write300old(xml);
      if (!isGrace()) {
            Fraction t(globalDuration());
            if (staff())
                  t /= staff()->timeStretch(xml.curTick());
            xml.incCurTick(t.ticks());
            }
      for (auto i : score()->spanner()) {     // TODO: dont search whole list
            Spanner* s = i.second;
            if (s->generated() || !s->isSlur() || toSlur(s)->broken() || !xml.canWrite(s))
                  continue;

            if (s->startElement() == this) {
                  int id = xml.spannerId(s);
                  xml.tagE(QString("Slur type=\"start\" id=\"%1\"").arg(id));
                  }
            else if (s->endElement() == this) {
                  int id = xml.spannerId(s);
                  xml.tagE(QString("Slur type=\"stop\" id=\"%1\"").arg(id));
                  }
            }
      }

//---------------------------------------------------------
//   Chord::write300old
//---------------------------------------------------------

void Chord::write300old(XmlWriter& xml) const
      {
      for (Chord* c : _graceNotes) {
            c->writeBeam(xml);
            c->write300old(xml);
            }
      xml.stag("Chord");
      ChordRest::writeProperties300old(xml);
      for (const Articulation* a : _articulations)
            a->write300old(xml);
      switch (_noteType) {
            case NoteType::NORMAL:
                  break;
            case NoteType::ACCIACCATURA:
                  xml.tagE("acciaccatura");
                  break;
            case NoteType::APPOGGIATURA:
                  xml.tagE("appoggiatura");
                  break;
            case NoteType::GRACE4:
                  xml.tagE("grace4");
                  break;
            case NoteType::GRACE16:
                  xml.tagE("grace16");
                  break;
            case NoteType::GRACE32:
                  xml.tagE("grace32");
                  break;
            case NoteType::GRACE8_AFTER:
                  xml.tagE("grace8after");
                  break;
            case NoteType::GRACE16_AFTER:
                  xml.tagE("grace16after");
                  break;
            case NoteType::GRACE32_AFTER:
                  xml.tagE("grace32after");
                  break;
            default:
                  break;
            }

      if (_noStem)
            xml.tag("noStem", _noStem);
      else if (_stem && (_stem->isUserModified() || (_stem->userLen() != 0.0)))
            _stem->write300old(xml);
      if (_hook && _hook->isUserModified())
            _hook->write300old(xml);
      if (_stemSlash && _stemSlash->isUserModified())
            _stemSlash->write300old(xml);
      writeProperty(xml, Pid::STEM_DIRECTION);
      for (Note* n : _notes)
            n->write300old(xml);
      if (_arpeggio)
            _arpeggio->write300old(xml);
      if (_tremolo && tremoloChordType() != TremoloChordType::TremoloSecondNote)
            _tremolo->write300old(xml);
      for (Element* e : el())
            e->write300old(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Rest::write300old
//---------------------------------------------------------

void Rest::write300old(XmlWriter& xml) const
      {
      if (_gap)
            return;
      xml.stag(name());
      ChordRest::writeProperties300old(xml);
      el().write300old(xml);
      xml.etag();
      }

//--------------------------------------------------
//   Note::write300old
//---------------------------------------------------------

void Note::write300old(XmlWriter& xml) const
      {
      xml.stag("Note");
      Element::writeProperties(xml);

      if (_accidental)
            _accidental->write300old(xml);
      _el.write300old(xml);
      for (NoteDot* dot : _dots) {
            if (!dot->userOff().isNull() || !dot->visible() || dot->color() != Qt::black || dot->visible() != visible()) {
                  dot->write300old(xml);
                  break;
                  }
            }
      if (_tieFor)
            _tieFor->write300old(xml);
      if (_tieBack) {
            int id = xml.spannerId(_tieBack);
            xml.tagE(QString("endSpanner id=\"%1\"").arg(id));
            }
      if ((chord() == 0 || chord()->playEventType() != PlayEventType::Auto) && !_playEvents.empty()) {
            xml.stag("Events");
            for (const NoteEvent& e : _playEvents)
                  e.write(xml);
            xml.etag();
            }
      for (Pid id : { Pid::PITCH, Pid::TPC1, Pid::TPC2, Pid::SMALL, Pid::MIRROR_HEAD, Pid::DOT_POSITION,
         Pid::HEAD_GROUP, Pid::VELO_OFFSET, Pid::PLAY, Pid::TUNING, Pid::FRET, Pid::STRING,
         Pid::GHOST, Pid::HEAD_TYPE, Pid::VELO_TYPE, Pid::FIXED, Pid::FIXED_LINE
            }) {
            writeProperty(xml, id);
            }

      for (Spanner* e : _spannerFor)
            e->write300old(xml);
      for (Spanner* e : _spannerBack)
            xml.tagE(QString("endSpanner id=\"%1\"").arg(xml.spannerId(e)));

      xml.etag();
      }

//---------------------------------------------------------
//   Slur::write300old
//---------------------------------------------------------

void Slur::write300old(XmlWriter& xml) const
      {
      if (broken()) {
            qDebug("broken slur not written");
            return;
            }
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("Slur id=\"%1\"").arg(xml.spannerId(this)));
      SlurTie::writeProperties300old(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   DurationElement::writeProperties300old
//---------------------------------------------------------

void DurationElement::writeProperties300old(XmlWriter& xml) const
      {
      Element::writeProperties(xml);
      if (tuplet())
            xml.tag("Tuplet", tuplet()->id());
      }

//---------------------------------------------------------
//   Glissando::write300old
//---------------------------------------------------------

void Glissando::write300old(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      if (_showText && !_text.isEmpty())
            xml.tag("text", _text);

      for (auto id : { Pid::GLISS_TYPE, Pid::PLAY, Pid::GLISSANDO_STYLE } )
            writeProperty(xml, id);

      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Hairpin::write300old
//---------------------------------------------------------

void Hairpin::write300old(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      int id = xml.spannerId(this);
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id));
      xml.tag("subtype", int(_hairpinType));
      writeProperty(xml, Pid::VELO_CHANGE);
      writeProperty(xml, Pid::HAIRPIN_CIRCLEDTIP);
      writeProperty(xml, Pid::DYNAMIC_RANGE);
      writeProperty(xml, Pid::BEGIN_TEXT);
      writeProperty(xml, Pid::CONTINUE_TEXT);

      for (const StyledProperty* spp = styledProperties(); spp->sid != Sid::NOSTYLE; ++spp)
            writeProperty(xml, spp->pid);

      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   LetRing::write300old
//---------------------------------------------------------

void LetRing::write300old(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));

      for (const StyledProperty* spp = styledProperties(); spp->sid != Sid::NOSTYLE; ++spp)
            writeProperty(xml, spp->pid);

      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   SLine::write300old
//---------------------------------------------------------

void SLine::write300old(XmlWriter& xml) const
      {
      int id = xml.spannerId(this);
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id));
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   PalmMute::write300old
//---------------------------------------------------------

void PalmMute::write300old(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));

      for (const StyledProperty* spp = styledProperties(); spp->sid != Sid::NOSTYLE; ++spp)
            writeProperty(xml, spp->pid);

      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Pedal::write300old
//---------------------------------------------------------

void Pedal::write300old(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      int id = xml.spannerId(this);
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id));

      for (auto i : {
         Pid::END_HOOK_TYPE,
         Pid::BEGIN_TEXT,
         Pid::END_TEXT,
         Pid::LINE_WIDTH,
         Pid::LINE_STYLE,
         Pid::BEGIN_HOOK_TYPE
         }) {
            writeProperty(xml, i);
            }
      for (const StyledProperty* spp = styledProperties(); spp->sid != Sid::NOSTYLE; ++spp)
            writeProperty(xml, spp->pid);

      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   TextLineBase::write300old
//---------------------------------------------------------

void TextLineBase::write300old(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      writeProperties300old(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Tie::write300old
//---------------------------------------------------------

void Tie::write300old(XmlWriter& xml) const
      {
      xml.stag(QString("Tie id=\"%1\"").arg(xml.spannerId(this)));
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Trill::write300old
//---------------------------------------------------------

void Trill::write300old(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      xml.tag("subtype", trillTypeName());
      writeProperty(xml, Pid::PLAY);
      writeProperty(xml, Pid::ORNAMENT_STYLE);
      SLine::writeProperties(xml);
      if (_accidental)
            _accidental->write300old(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Vibrato::write
//---------------------------------------------------------

void Vibrato::write300old(XmlWriter& xml) const
      {
      if (!xml.canWrite(this))
            return;
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      xml.tag("subtype", vibratoTypeName());
      writeProperty(xml, Pid::PLAY);
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Volta::write300old
//---------------------------------------------------------

void Volta::write300old(XmlWriter& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(xml.spannerId(this)));
      TextLineBase::writeProperties(xml);
      QString s;
      for (int i : _endings) {
            if (!s.isEmpty())
                  s += ", ";
            s += QString("%1").arg(i);
            }
      xml.tag("endings", s);
      xml.etag();
      }

}

