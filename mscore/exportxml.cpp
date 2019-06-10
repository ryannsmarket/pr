//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2013 Werner Schweer and others
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

/**
 MusicXML export.
 */

// TODO: trill lines need to be handled the same way as slurs
// in MuseScore they are measure level elements, while in MusicXML
// they are attached to notes (as ornaments)

//=========================================================
//  LVI FIXME
//
//  Evaluate paramenter handling between the various classes, could be simplified
//=========================================================

// TODO LVI 2011-10-30: determine how to report export errors.
// Currently all output (both debug and error reports) are done using qDebug.

#include <math.h>
#include "config.h"
//#include "musescore.h"
#include "file.h"
#include "libmscore/score.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/sig.h"
#include "libmscore/key.h"
#include "libmscore/clef.h"
#include "libmscore/note.h"
#include "libmscore/segment.h"
#include "libmscore/xml.h"
#include "libmscore/beam.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/style.h"
#include "musicxml.h"
#include "libmscore/slur.h"
#include "libmscore/hairpin.h"
#include "libmscore/dynamic.h"
#include "libmscore/barline.h"
#include "libmscore/timesig.h"
#include "libmscore/ottava.h"
#include "libmscore/pedal.h"
#include "libmscore/text.h"
#include "libmscore/tuplet.h"
#include "libmscore/lyrics.h"
#include "libmscore/volta.h"
#include "libmscore/keysig.h"
#include "libmscore/bracket.h"
#include "libmscore/arpeggio.h"
#include "libmscore/jump.h"
#include "libmscore/marker.h"
#include "libmscore/tremolo.h"
#include "libmscore/trill.h"
#include "libmscore/harmony.h"
#include "libmscore/tempotext.h"
#include "libmscore/sym.h"
#include "libmscore/pitchspelling.h"
#include "libmscore/utils.h"
#include "libmscore/articulation.h"
#include "libmscore/page.h"
#include "libmscore/system.h"
#include "libmscore/element.h"
#include "libmscore/glissando.h"
#include "libmscore/navigate.h"
#include "libmscore/spanner.h"
#include "libmscore/drumset.h"
#include "preferences.h"
#include "libmscore/mscore.h"
#include "libmscore/accidental.h"
#include "libmscore/breath.h"
#include "libmscore/chordline.h"
#include "libmscore/figuredbass.h"
#include "libmscore/stringdata.h"
#include "libmscore/rehearsalmark.h"
#include "thirdparty/qzip/qzipwriter_p.h"
#include "libmscore/fret.h"
#include "libmscore/tie.h"
#include "libmscore/undo.h"
#include "libmscore/textline.h"
#include "libmscore/fermata.h"
#include "musicxmlfonthandler.h"

namespace Ms {

//---------------------------------------------------------
//   local defines for debug output
//---------------------------------------------------------

// #define DEBUG_CLEF true
// #define DEBUG_REPEATS true
// #define DEBUG_TICK true

#ifdef DEBUG_CLEF
#define clefDebug(...) qDebug(__VA_ARGS__)
#else
#define clefDebug(...) {}
#endif

//---------------------------------------------------------
//   typedefs
//---------------------------------------------------------

typedef QMap<int, const FiguredBass*> FigBassMap;

//---------------------------------------------------------
//   attributes -- prints <attributes> tag when necessary
//---------------------------------------------------------

class Attributes {
      bool inAttributes;

public:
      Attributes() { start(); }
      void doAttr(XmlWriter& xml, bool attr);
      void start();
      void stop(XmlWriter& xml);
      };

//---------------------------------------------------------
//   doAttr - when necessary change state and print <attributes> tag
//---------------------------------------------------------

void Attributes::doAttr(XmlWriter& xml, bool attr)
      {
      if (!inAttributes && attr) {
            xml.stag("attributes");
            inAttributes = true;
            }
      else if (inAttributes && !attr) {
            xml.etag();
            inAttributes = false;
            }
      }

//---------------------------------------------------------
//   start -- initialize
//---------------------------------------------------------

void Attributes::start()
      {
      inAttributes = false;
      }

//---------------------------------------------------------
//   stop -- print </attributes> tag when necessary
//---------------------------------------------------------

void Attributes::stop(XmlWriter& xml)
      {
      if (inAttributes) {
            xml.etag();
            inAttributes = false;
            }
      }

//---------------------------------------------------------
//   notations -- prints <notations> tag when necessary
//---------------------------------------------------------

class Notations {
      bool notationsPrinted;

public:
      Notations() { notationsPrinted = false; }
      void tag(XmlWriter& xml);
      void etag(XmlWriter& xml);
      };

//---------------------------------------------------------
//   articulations -- prints <articulations> tag when necessary
//---------------------------------------------------------

class Articulations {
      bool articulationsPrinted;

public:
      Articulations() { articulationsPrinted = false; }
      void tag(XmlWriter& xml);
      void etag(XmlWriter& xml);
      };

//---------------------------------------------------------
//   ornaments -- prints <ornaments> tag when necessary
//---------------------------------------------------------

class Ornaments {
      bool ornamentsPrinted;

public:
      Ornaments() { ornamentsPrinted = false; }
      void tag(XmlWriter& xml);
      void etag(XmlWriter& xml);
      };

//---------------------------------------------------------
//   technical -- prints <technical> tag when necessary
//---------------------------------------------------------

class Technical {
      bool technicalPrinted;

public:
      Technical() { technicalPrinted = false; }
      void tag(XmlWriter& xml);
      void etag(XmlWriter& xml);
      };

//---------------------------------------------------------
//   slur handler -- prints <slur> tags
//---------------------------------------------------------

class SlurHandler {
      const Slur* slur[MAX_NUMBER_LEVEL];
      bool started[MAX_NUMBER_LEVEL];
      int findSlur(const Slur* s) const;

public:
      SlurHandler();
      void doSlurs(const ChordRest* chordRest, Notations& notations, XmlWriter& xml);

private:
      void doSlurStart(const Slur* s, Notations& notations, XmlWriter& xml);
      void doSlurStop(const Slur* s, Notations& notations, XmlWriter& xml);
      };

//---------------------------------------------------------
//   glissando handler -- prints <glissando> tags
//---------------------------------------------------------

class GlissandoHandler {
      const Note* glissNote[MAX_NUMBER_LEVEL];
      const Note* slideNote[MAX_NUMBER_LEVEL];
      int findNote(const Note* note, int type) const;

public:
      GlissandoHandler();
      void doGlissandoStart(Glissando* gliss, Notations& notations, XmlWriter& xml);
      void doGlissandoStop(Glissando* gliss, Notations& notations, XmlWriter& xml);
      };

//---------------------------------------------------------
//   ExportMusicXml
//---------------------------------------------------------

typedef QHash<const Chord*, const Trill*> TrillHash;
typedef QMap<const Instrument*, int> MxmlInstrumentMap;

class ExportMusicXml {
      Score* _score;
      XmlWriter _xml;
      SlurHandler sh;
      GlissandoHandler gh;
      Fraction _tick;
      Attributes _attr;
      TextLine const* brackets[MAX_NUMBER_LEVEL];
      TextLineBase const* dashes[MAX_NUMBER_LEVEL];
      Hairpin const* hairpins[MAX_NUMBER_LEVEL];
      Ottava const* ottavas[MAX_NUMBER_LEVEL];
      Trill const* trills[MAX_NUMBER_LEVEL];
      int div;
      double millimeters;
      int tenths;
      TrillHash _trillStart;
      TrillHash _trillStop;
      MxmlInstrumentMap instrMap;

      int findBracket(const TextLine* tl) const;
      int findDashes(const TextLineBase* tl) const;
      int findHairpin(const Hairpin* tl) const;
      int findOttava(const Ottava* tl) const;
      int findTrill(const Trill* tl) const;
      void chord(Chord* chord, int staff, const std::vector<Lyrics*>* ll, bool useDrumset);
      void rest(Rest* chord, int staff);
      void clef(int staff, const ClefType ct, const QString& extraAttributes = "");
      void timesig(TimeSig* tsig);
      void keysig(const KeySig* ks, ClefType ct, int staff = 0, bool visible = true);
      void barlineLeft(Measure* m);
      void barlineRight(Measure* m);
      void lyrics(const std::vector<Lyrics*>* ll, const int trk);
      void work(const MeasureBase* measure);
      void calcDivMoveToTick(const Fraction& t);
      void calcDivisions();
      void keysigTimesig(const Measure* m, const Part* p);
      void chordAttributes(Chord* chord, Notations& notations, Technical& technical,
                           TrillHash& trillStart, TrillHash& trillStop);
      void wavyLineStartStop(Chord* chord, Notations& notations, Ornaments& ornaments,
                             TrillHash& trillStart, TrillHash& trillStop);
      void print(const Measure* const m, const int partNr, const int firstStaffOfPart, const int nrStavesInPart);
      void findAndExportClef(Measure* m, const int staves, const int strack, const int etrack);
      void exportDefaultClef(const Part* const part, const Measure* const m);
      void writeElement(Element* el, const Measure* m, int sstaff, bool useDrumset);

public:
      ExportMusicXml(Score* s)
            : _xml(s)
            {
            _score = s; _tick = { 0,1 }; div = 1; tenths = 40;
            millimeters = _score->spatium() * tenths / (10 * DPMM);
            }
      void write(QIODevice* dev);
      void credits(XmlWriter& xml);
      void moveToTick(const Fraction& t);
      void words(Text const* const text, int staff);
      void rehearsal(RehearsalMark const* const rmk, int staff);
      void hairpin(Hairpin const* const hp, int staff, const Fraction& tick);
      void ottava(Ottava const* const ot, int staff, const Fraction& tick);
      void pedal(Pedal const* const pd, int staff, const Fraction& tick);
      void textLine(TextLine const* const tl, int staff, const Fraction& tick);
      void dynamic(Dynamic const* const dyn, int staff);
      void symbol(Symbol const* const sym, int staff);
      void tempoText(TempoText const* const text, int staff);
      void harmony(Harmony const* const, FretDiagram const* const fd, int offset = 0);
      Score* score() const { return _score; };
      double getTenthsFromInches(double) const;
      double getTenthsFromDots(double) const;
      };

//---------------------------------------------------------
//   addPositioningAttributes
//   According to the specs (common.dtd), all direction-type and note elements must be relative to the measure
//   while all other elements are relative to their position or the nearest note.
//---------------------------------------------------------

static QString addPositioningAttributes(Element const* const el, bool isSpanStart = true)
      {
      if (!preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT))
            return "";

      //qDebug("single el %p _pos x,y %f %f _userOff x,y %f %f spatium %f",
      //       el, el->ipos().x(), el->ipos().y(), el->offset().x(), el->offset().y(), el->spatium());

      const float positionElipson = 0.1f;
      float defaultX = 0;
      float defaultY = 0;
      float relativeX = 0;
      float relativeY = 0;
      float spatium = el->spatium();

      const SLine* span = nullptr;
      if (el->isSLine())
            span = static_cast<const SLine*>(el);

      if (span && !span->segmentsEmpty()) {
            if (isSpanStart) {
                  const auto seg = span->frontSegment();
                  const auto offset = seg->offset();
                  const auto p = seg->pos();
                  relativeX = offset.x();
                  defaultY = p.y();

                  //qDebug("sline start seg %p seg->pos x,y %f %f seg->userOff x,y %f %f spatium %f",
                  //       seg, p.x(), p.y(), seg->offset().x(), seg->offset().y(), seg->spatium());

                  }
            else {
                  const auto seg = span->backSegment();
                  const auto userOff = seg->offset(); // This is the offset accessible from the inspector
                  const auto userOff2 = seg->userOff2(); // Offset of the actual dragged anchor, which doesn't affect the inspector offset
                  //auto pos = seg->pos();
                  //auto pos2 = seg->pos2();

                  //qDebug("sline stop seg %p seg->pos2 x,y %f %f seg->userOff2 x,y %f %f spatium %f",
                  //       seg, pos2.x(), pos2.y(), seg->userOff2().x(), seg->userOff2().y(), seg->spatium());

                  // For an SLine, the actual offset equals the sum of userOff and userOff2,
                  // as userOff moves the SLine as a whole
                  relativeX = userOff.x() + userOff2.x();

                  // Following would probably required for non-horizontal SLines:
                  //defaultY = pos.y() + pos2.y();
                  }
            }
      else {
            defaultX = el->ipos().x();   // Note: for some elements, Finale Notepad seems to work slightly better w/o default-x
            defaultY = el->ipos().y();
            relativeX = el->offset().x();
            relativeY = el->offset().y();
            }

      // convert into spatium tenths for MusicXML
      defaultX *=  10 / spatium;
      defaultY *=  -10 / spatium;
      relativeX *=  10 / spatium;
      relativeY *=  -10 / spatium;

      QString res;
      if (fabsf(defaultX) > positionElipson)
            res += QString(" default-x=\"%1\"").arg(QString::number(defaultX, 'f', 2));
      if (fabsf(defaultY) > positionElipson)
            res += QString(" default-y=\"%1\"").arg(QString::number(defaultY, 'f', 2));
      if (fabsf(relativeX) > positionElipson)
            res += QString(" relative-x=\"%1\"").arg(QString::number(relativeX, 'f', 2));
      if (fabsf(relativeY) > positionElipson)
            res += QString(" relative-y=\"%1\"").arg(QString::number(relativeY, 'f', 2));

      return res;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Notations::tag(XmlWriter& xml)
      {
      if (!notationsPrinted)
            xml.stag("notations");
      notationsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Notations::etag(XmlWriter& xml)
      {
      if (notationsPrinted)
            xml.etag();
      notationsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Articulations::tag(XmlWriter& xml)
      {
      if (!articulationsPrinted)
            xml.stag("articulations");
      articulationsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Articulations::etag(XmlWriter& xml)
      {
      if (articulationsPrinted)
            xml.etag();
      articulationsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Ornaments::tag(XmlWriter& xml)
      {
      if (!ornamentsPrinted)
            xml.stag("ornaments");
      ornamentsPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Ornaments::etag(XmlWriter& xml)
      {
      if (ornamentsPrinted)
            xml.etag();
      ornamentsPrinted = false;
      }

//---------------------------------------------------------
//   tag
//---------------------------------------------------------

void Technical::tag(XmlWriter& xml)
      {
      if (!technicalPrinted)
            xml.stag("technical");
      technicalPrinted = true;
      }

//---------------------------------------------------------
//   etag
//---------------------------------------------------------

void Technical::etag(XmlWriter& xml)
      {
      if (technicalPrinted)
            xml.etag();
      technicalPrinted = false;
      }

//---------------------------------------------------------
//   color2xml
//---------------------------------------------------------

/**
 Return \a el color.
 */

static QString color2xml(const Element* el)
      {
      if (el->color() != MScore::defaultColor)
            return QString(" color=\"%1\"").arg(el->color().name().toUpper());
      else
            return "";
      }

//---------------------------------------------------------
//   slurHandler
//---------------------------------------------------------

SlurHandler::SlurHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            slur[i] = 0;
            started[i] = false;
            }
      }

static QString slurTieLineStyle(const SlurTie* s)
      {
      QString lineType;
      QString rest;
      switch (s->lineType()) {
            case 1:
                  lineType = "dotted";
                  break;
            case 2:
                  lineType = "dashed";
                  break;
            default:
                  lineType = "";
            }
      if (!lineType.isEmpty())
            rest = QString(" line-type=\"%1\"").arg(lineType);
      return rest;
      }

//---------------------------------------------------------
//   findSlur -- get index of slur in slur table
//   return -1 if not found
//---------------------------------------------------------

int SlurHandler::findSlur(const Slur* s) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (slur[i] == s) return i;
      return -1;
      }

//---------------------------------------------------------
//   findFirstChordRest -- find first chord or rest (in musical order) for slur s
//   note that this is not necessarily the same as s->startElement()
//---------------------------------------------------------

static const ChordRest* findFirstChordRest(const Slur* s)
      {
      const Element* e1 = s->startElement();
      if (!e1 || !(e1->isChordRest())) {
            qDebug("no valid start element for slur %p", s);
            return nullptr;
            }

      const Element* e2 = s->endElement();
      if (!e2 || !(e2->isChordRest())) {
            qDebug("no valid end element for slur %p", s);
            return nullptr;
            }

      if (e1->tick() < e2->tick())
            return static_cast<const ChordRest*>(e1);
      else if (e1->tick() > e2->tick())
            return static_cast<const ChordRest*>(e2);

      if (e1->isRest() || e2->isRest()) {
            return nullptr;
            }

      const auto c1 = static_cast<const Chord*>(e1);
      const auto c2 = static_cast<const Chord*>(e2);

      // c1->tick() == c2->tick()
      if (!c1->isGrace() && !c2->isGrace()) {
            // slur between two regular notes at the same tick
            // probably shouldn't happen but handle just in case
            qDebug("invalid slur between chords %p and %p at tick %d", c1, c2, c1->tick().ticks());
            return 0;
            }
      else if (c1->isGraceBefore() && !c2->isGraceBefore())
            return c1;        // easy case: c1 first
      else if (c1->isGraceAfter() && !c2->isGraceAfter())
            return c2;        // easy case: c2 first
      else if (c2->isGraceBefore() && !c1->isGraceBefore())
            return c2;        // easy case: c2 first
      else if (c2->isGraceAfter() && !c1->isGraceAfter())
            return c1;        // easy case: c1 first
      else {
            // both are grace before or both are grace after -> compare grace indexes
            // (note: higher means closer to the non-grace chord it is attached to)
            if ((c1->isGraceBefore() && c1->graceIndex() < c2->graceIndex())
                || (c1->isGraceAfter() && c1->graceIndex() > c2->graceIndex()))
                  return c1;
            else
                  return c2;
            }
      }

//---------------------------------------------------------
//   doSlurs
//---------------------------------------------------------

void SlurHandler::doSlurs(const ChordRest* chordRest, Notations& notations, XmlWriter& xml)
      {
      // loop over all slurs twice, first to handle the stops, then the starts
      for (int i = 0; i < 2; ++i) {
            // search for slur(s) starting or stopping at this chord
            for (const auto it : chordRest->score()->spanner()) {
                  auto sp = it.second;
                  if (sp->generated() || sp->type() != ElementType::SLUR)
                        continue;
                  if (chordRest == sp->startElement() || chordRest == sp->endElement()) {
                        const auto s = static_cast<const Slur*>(sp);
                        const auto firstChordRest = findFirstChordRest(s);
                        if (firstChordRest) {
                              if (i == 0) {
                                    // first time: do slur stops
                                    if (firstChordRest != chordRest)
                                          doSlurStop(s, notations, xml);
                                    }
                              else {
                                    // second time: do slur starts
                                    if (firstChordRest == chordRest)
                                          doSlurStart(s, notations, xml);
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   doSlurStart
//---------------------------------------------------------

void SlurHandler::doSlurStart(const Slur* s, Notations& notations, XmlWriter& xml)
      {
      // check if on slur list (i.e. stop already seen)
      int i = findSlur(s);
      // compose tag
      QString tagName = "slur";
      tagName += slurTieLineStyle(s); // define line type
      tagName += color2xml(s);
      tagName += QString(" type=\"start\" placement=\"%1\"")
            .arg(s->up() ? "above" : "below");
      tagName += addPositioningAttributes(s, true);

      if (i >= 0) {
            // remove from list and print start
            slur[i] = 0;
            started[i] = false;
            notations.tag(xml);
            tagName += QString(" number=\"%1\"").arg(i + 1);
            xml.tagE(tagName);
            }
      else {
            // find free slot to store it
            i = findSlur(0);
            if (i >= 0) {
                  slur[i] = s;
                  started[i] = true;
                  notations.tag(xml);
                  tagName += QString(" number=\"%1\"").arg(i + 1);
                  xml.tagE(tagName);
                  }
            else
                  qDebug("no free slur slot");
            }
      }

//---------------------------------------------------------
//   doSlurStop
//---------------------------------------------------------

// Note: a slur may start in a higher voice in the same measure.
// In that case it is not yet started (i.e. on the active slur list)
// when doSlurStop() is executed. Handle this slur as follows:
// - generate stop anyway and put it on the slur list
// - doSlurStart() starts slur but doesn't store it

void SlurHandler::doSlurStop(const Slur* s, Notations& notations, XmlWriter& xml)
      {
      // check if on slur list
      int i = findSlur(s);
      if (i < 0) {
            // if not, find free slot to store it
            i = findSlur(0);
            if (i >= 0) {
                  slur[i] = s;
                  started[i] = false;
                  notations.tag(xml);
                  QString tagName = QString("slur type=\"stop\" number=\"%1\"").arg(i + 1);
                  tagName += addPositioningAttributes(s, false);
                  xml.tagE(tagName);
                  }
            else
                  qDebug("no free slur slot");
            }
      else {
            // found (already started), stop it and remove from list
            slur[i] = 0;
            started[i] = false;
            notations.tag(xml);
            QString tagName = QString("slur type=\"stop\" number=\"%1\"").arg(i + 1);
            tagName += addPositioningAttributes(s, false);
            xml.tagE(tagName);
            }
      }

//---------------------------------------------------------
//   glissando
//---------------------------------------------------------

// <notations>
//   <slide line-type="solid" number="1" type="start"/>
//   </notations>

// <notations>
//   <glissando line-type="wavy" number="1" type="start"/>
//   </notations>

static void glissando(const Glissando* gli, int number, bool start, Notations& notations, XmlWriter& xml)
      {
      GlissandoType st = gli->glissandoType();
      QString tagName;
      switch (st) {
            case GlissandoType::STRAIGHT:
                  tagName = "slide line-type=\"solid\"";
                  break;
            case GlissandoType::WAVY:
                  tagName = "glissando line-type=\"wavy\"";
                  break;
            default:
                  qDebug("unknown glissando subtype %d", int(st));
                  return;
                  break;
            }
      tagName += QString(" number=\"%1\" type=\"%2\"").arg(number).arg(start ? "start" : "stop");
      tagName += color2xml(gli);
      tagName += addPositioningAttributes(gli, start);
      notations.tag(xml);
      if (start && gli->showText() && gli->text() != "")
            xml.tag(tagName, gli->text());
      else
            xml.tagE(tagName);
      }

//---------------------------------------------------------
//   GlissandoHandler
//---------------------------------------------------------

GlissandoHandler::GlissandoHandler()
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            glissNote[i] = 0;
            slideNote[i] = 0;
            }
      }

//---------------------------------------------------------
//   findNote -- get index of Note in note table for subtype type
//   return -1 if not found
//---------------------------------------------------------

int GlissandoHandler::findNote(const Note* note, int type) const
      {
      if (type != 0 && type != 1) {
            qDebug("GlissandoHandler::findNote: unknown glissando subtype %d", type);
            return -1;
            }
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (type == 0 && slideNote[i] == note) return i;
            if (type == 1 && glissNote[i] == note) return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   doGlissandoStart
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStart(Glissando* gliss, Notations& notations, XmlWriter& xml)
      {
      GlissandoType type = gliss->glissandoType();
      if (type != GlissandoType::STRAIGHT && type != GlissandoType::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %d", int(type));
            return;
            }
      Note* note = static_cast<Note*>(gliss->startElement());
      // check if on chord list
      int i = findNote(note, int(type));
      if (i >= 0) {
            // print error and remove from list
            qDebug("doGlissandoStart: note for glissando/slide %p already on list", gliss);
            if (type == GlissandoType::STRAIGHT) slideNote[i] = 0;
            if (type == GlissandoType::WAVY) glissNote[i] = 0;
            }
      // find free slot to store it
      i = findNote(0, int(type));
      if (i >= 0) {
            if (type == GlissandoType::STRAIGHT) slideNote[i] = note;
            if (type == GlissandoType::WAVY) glissNote[i] = note;
            glissando(gliss, i + 1, true, notations, xml);
            }
      else
            qDebug("doGlissandoStart: no free slot");
      }

//---------------------------------------------------------
//   doGlissandoStop
//---------------------------------------------------------

void GlissandoHandler::doGlissandoStop(Glissando* gliss, Notations& notations, XmlWriter& xml)
      {
      GlissandoType type = gliss->glissandoType();
      if (type != GlissandoType::STRAIGHT && type != GlissandoType::WAVY) {
            qDebug("doGlissandoStart: unknown glissando subtype %d", int(type));
            return;
            }
      Note* note = static_cast<Note*>(gliss->startElement());
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            if (type == GlissandoType::STRAIGHT && slideNote[i] == note) {
                  slideNote[i] = 0;
                  glissando(gliss, i + 1, false, notations, xml);
                  return;
                  }
            if (type == GlissandoType::WAVY && glissNote[i] == note) {
                  glissNote[i] = 0;
                  glissando(gliss, i + 1, false, notations, xml);
                  return;
                  }
            }
      qDebug("doGlissandoStop: glissando note %p not found", note);
      }

//---------------------------------------------------------
//   directions anchor -- anchor directions at another element or a specific tick
//---------------------------------------------------------

class DirectionsAnchor {
      Element* direct;        // the element containing the direction
      Element* anchor;        // the element it is attached to
      bool start;             // whether it is attached to start or end
      Fraction tick;          // the timestamp

public:
      DirectionsAnchor(Element* a, bool s, const Fraction& t) { direct = 0; anchor = a; start = s; tick = t; }
      DirectionsAnchor(const Fraction& t) { direct = 0; anchor = 0; start = true; tick = t; }
      Element* getDirect() { return direct; }
      Element* getAnchor() { return anchor; }
      bool getStart() { return start; }
      Fraction getTick() { return tick; }
      void setDirect(Element* d) { direct = d; }
      };

//---------------------------------------------------------
// trill handling
//---------------------------------------------------------

// Find chords to attach trills to. This is necessary because in MuseScore
// trills are spanners (thus attached to segments), while in MusicXML trills
// are attached to notes.
// TBD: must trill end be in the same staff as trill started ?
// if so, no need to pass in strack and etrack (trill has a track)

static void findTrillAnchors(const Trill* trill, Chord*& startChord, Chord*& stopChord)
      {
      const Segment* seg     = trill->startSegment();
      const Fraction endTick = trill->tick2();
      const int strack       = trill->track();
      // try to find chords in the same track:
      // find a track with suitable chords both for start and stop
      for (int i = 0; i < VOICES; ++i) {
            Element* el = seg->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != ElementType::CHORD)
                  continue;
            startChord = static_cast<Chord*>(el);
            Segment* s = trill->score()->tick2segmentEnd(strack + i, endTick);
            if (!s)
                  continue;
            el = s->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != ElementType::CHORD)
                  continue;
            stopChord = static_cast<Chord*>(el);
            return;
            }
      // try to find start/stop chords in different tracks
      for (int i = 0; i < VOICES; ++i) {
            Element* el = seg->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != ElementType::CHORD)
                  continue;
            startChord = static_cast<Chord*>(el);
            break;      // first chord found is OK
            }
      for (int i = 0; i < VOICES; ++i) {
            Segment* s = trill->score()->tick2segmentEnd(strack + i, endTick);
            if (!s)
                  continue;
            Element* el = s->element(strack + i);
            if (!el)
                  continue;
            if (el->type() != ElementType::CHORD)
                  continue;
            stopChord = static_cast<Chord*>(el);
            break;      // first chord found is OK
            }
      }

// find all trills in this measure and this part

static void findTrills(Measure* measure, int strack, int etrack, TrillHash& trillStart, TrillHash& trillStop)
      {
      // loop over all spanners in this measure
      Fraction stick = measure->tick();
      Fraction etick = measure->tick() + measure->ticks();
      for (auto it = measure->score()->spanner().lower_bound(stick.ticks()); it != measure->score()->spanner().upper_bound(etick.ticks()); ++it) {
            Spanner* e = it->second;
            //qDebug("findTrills 1 trill %p type %d track %d tick %d", e, e->type(), e->track(), e->tick());
            if (e->type() == ElementType::TRILL && strack <= e->track() && e->track() < etrack
                && e->tick() >= measure->tick() && e->tick() < (measure->tick() + measure->ticks()))
                  {
                  //qDebug("findTrills 2 trill %p", e);
                  // a trill is found starting in this segment, trill end time is known
                  // determine notes to write trill start and stop
                  const Trill* tr = static_cast<const Trill*>(e);
                  Chord* startChord = 0;  // chord where trill starts
                  Chord* stopChord = 0;   // chord where trill stops

                  findTrillAnchors(tr, startChord, stopChord);
                  //qDebug("findTrills 3 tr %p startChord %p stopChord %p", tr, startChord, stopChord);

                  if (startChord && stopChord) {
                        trillStart.insert(startChord, tr);
                        trillStop.insert(stopChord, tr);
                        }
                  }
            }
      }

//---------------------------------------------------------
// helpers for ::calcDivisions
//---------------------------------------------------------

typedef QList<int> IntVector;
static IntVector integers;
static IntVector primes;

// check if all integers can be divided by d

static bool canDivideBy(int d)
      {
      bool res = true;
      for (int i = 0; i < integers.count(); i++) {
            if ((integers[i] <= 1) || ((integers[i] % d) != 0)) {
                  res = false;
                  }
            }
      return res;
      }

// divide all integers by d

static void divideBy(int d)
      {
      for (int i = 0; i < integers.count(); i++) {
            integers[i] /= d;
            }
      }

static void addInteger(int len)
      {
      if (!integers.contains(len)) {
            integers.append(len);
            }
      }

//---------------------------------------------------------
//   calcDivMoveToTick
//---------------------------------------------------------

void ExportMusicXml::calcDivMoveToTick(const Fraction& t)
      {
      if (t < _tick) {
#ifdef DEBUG_TICK
            qDebug("backup %d", (tick - t).ticks());
#endif
            addInteger((_tick - t).ticks());
            }
      else if (t > _tick) {
#ifdef DEBUG_TICK
            qDebug("forward %d", (t - tick).ticks());;
#endif
            addInteger((t - _tick).ticks());
            }
      _tick = t;
      }

//---------------------------------------------------------
// isTwoNoteTremolo - determine is chord is part of two note tremolo
//---------------------------------------------------------

static bool isTwoNoteTremolo(Chord* chord)
      {
      return (chord->tremolo() && chord->tremolo()->twoNotes());
      }

//---------------------------------------------------------
//  calcDivisions
//---------------------------------------------------------

// Loop over all voices in all staffs and determine a suitable value for divisions.

// Length of time in MusicXML is expressed in "units", which should allow expressing all time values
// as an integral number of units. Divisions contains the number of units in a quarter note.
// MuseScore uses division (480) midi ticks to represent a quarter note, which expresses all note values
// plus triplets and quintuplets as integer values. Solution is to collect all time values required,
// and divide them by the highest common denominator, which is implemented as a series of
// divisions by prime factors. Initialize the list with division to make sure a quarter note can always
// be written as an integral number of units.

/**
 */

void ExportMusicXml::calcDivisions()
      {
      // init
      integers.clear();
      primes.clear();
      integers.append(MScore::division);
      primes.append(2);
      primes.append(3);
      primes.append(5);

      const QList<Part*>& il = _score->parts();

      for (int idx = 0; idx < il.size(); ++idx) {

            Part* part = il.at(idx);
            _tick = { 0,1 };

            int staves = part->nstaves();
            int strack = _score->staffIdx(part) * VOICES;
            int etrack = strack + staves * VOICES;

            for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {

                  if (mb->type() != ElementType::MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;

                  for (int st = strack; st < etrack; ++st) {
                        // sstaff - xml staff number, counting from 1 for this
                        // instrument
                        // special number 0 -> don’t show staff number in
                        // xml output (because there is only one staff)

                        int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                        sstaff /= VOICES;

                        for (Segment* seg = m->first(); seg; seg = seg->next()) {

                              Element* el = seg->element(st);
                              if (!el)
                                    continue;

                              // must ignore start repeat to prevent spurious backup/forward
                              if (el->type() == ElementType::BAR_LINE && static_cast<BarLine*>(el)->barLineType() == BarLineType::START_REPEAT)
                                    continue;

                              if (_tick != seg->tick())
                                    calcDivMoveToTick(seg->tick());

                              if (el->isChordRest()) {
                                    Fraction l = toChordRest(el)->actualTicks();
                                    if (el->isChord()) {
                                          if (isTwoNoteTremolo(toChord(el)))
                                                l = l * Fraction(1,2);
                                          }
#ifdef DEBUG_TICK
                                    qDebug("chordrest %d", l);
#endif
                                    addInteger(l.ticks());
                                    _tick += l;
                                    }
                              }
                        }
                  // move to end of measure (in case of incomplete last voice)
                  calcDivMoveToTick(m->endTick());
                  }
            }

      // do it: divide by all primes as often as possible
      for (int u = 0; u < primes.count(); u++)
            while (canDivideBy(primes[u]))
                  divideBy(primes[u]);

      div = MScore::division / integers[0];
#ifdef DEBUG_TICK
      qDebug("divisions=%d div=%d", integers[0], div);
#endif
      }

//---------------------------------------------------------
//   writePageFormat
//---------------------------------------------------------

static void writePageFormat(const Score* const s, XmlWriter& xml, double conversion)
      {
      xml.stag("page-layout");

      xml.tag("page-height", s->styleD(Sid::pageHeight) * conversion);
      xml.tag("page-width", s->styleD(Sid::pageWidth) * conversion);

      QString type("both");
      if (s->styleB(Sid::pageTwosided)) {
            type = "even";
            xml.stag(QString("page-margins type=\"%1\"").arg(type));
            xml.tag("left-margin",   s->styleD(Sid::pageEvenLeftMargin) * conversion);
            xml.tag("right-margin",  s->styleD(Sid::pageOddLeftMargin) * conversion);
            xml.tag("top-margin",    s->styleD(Sid::pageEvenTopMargin)  * conversion);
            xml.tag("bottom-margin", s->styleD(Sid::pageEvenBottomMargin) * conversion);
            xml.etag();
            type = "odd";
            }
      xml.stag(QString("page-margins type=\"%1\"").arg(type));
      xml.tag("left-margin",   s->styleD(Sid::pageOddLeftMargin) * conversion);
      xml.tag("right-margin",  s->styleD(Sid::pageEvenLeftMargin) * conversion);
      xml.tag("top-margin",    s->styleD(Sid::pageOddTopMargin) * conversion);
      xml.tag("bottom-margin", s->styleD(Sid::pageOddBottomMargin) * conversion);
      xml.etag();

      xml.etag();
      }

//---------------------------------------------------------
//   defaults
//---------------------------------------------------------

// _spatium = DPMM * (millimeter * 10.0 / tenths);

static void defaults(XmlWriter& xml, const Score* const s, double& millimeters, const int& tenths)
      {
      xml.stag("defaults");
      xml.stag("scaling");
      xml.tag("millimeters", millimeters);
      xml.tag("tenths", tenths);
      xml.etag();

      writePageFormat(s, xml, INCH / millimeters * tenths);

      // TODO: also write default system layout here
      // when exporting only manual or no breaks, system-distance is not written at all

      // font defaults
      // as MuseScore supports dozens of different styles, while MusicXML only has defaults
      // for music (TODO), words and lyrics, use Tid STAFF (typically used for words)
      // and LYRIC1 to get MusicXML defaults

      // TODO xml.tagE("music-font font-family=\"TBD\" font-size=\"TBD\"");
      xml.tagE(QString("word-font font-family=\"%1\" font-size=\"%2\"").arg(s->styleSt(Sid::staffTextFontFace)).arg(s->styleD(Sid::staffTextFontSize)));
      xml.tagE(QString("lyric-font font-family=\"%1\" font-size=\"%2\"").arg(s->styleSt(Sid::lyricsOddFontFace)).arg(s->styleD(Sid::lyricsOddFontSize)));
      xml.etag();
      }


//---------------------------------------------------------
//   creditWords
//---------------------------------------------------------

static void creditWords(XmlWriter& xml, Score* s, double x, double y, QString just, QString val, const QList<TextFragment>& words)
      {
      const QString mtf = s->styleSt(Sid::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(s->styleSt(Sid::staffTextFontFace));
      defFmt.setFontSize(s->styleD(Sid::staffTextFontSize));

      // export formatted
      xml.stag("credit page=\"1\"");
      QString attr = QString(" default-x=\"%1\"").arg(x);
      attr += QString(" default-y=\"%1\"").arg(y);
      attr += " justify=\"" + just + "\"";
      attr += " valign=\"" + val + "\"";
      MScoreTextToMXML mttm("credit-words", attr, defFmt, mtf);
      mttm.writeTextFragments(words, xml);
      xml.etag();
      }

//---------------------------------------------------------
//   parentHeight
//---------------------------------------------------------

static double parentHeight(const Element* element)
      {
      const Element* parent = element->parent();

      if (!parent)
            return 0;

      if (parent->type() == ElementType::VBOX) {
            return parent->height();
            }

      return 0;
      }

//---------------------------------------------------------
//   credits
//---------------------------------------------------------

void ExportMusicXml::credits(XmlWriter& xml)
      {
      const MeasureBase* measure = _score->measures()->first();
      QString rights = _score->metaTag("copyright");

      // determine page formatting
      const double h  = getTenthsFromInches(_score->styleD(Sid::pageHeight));
      const double w  = getTenthsFromInches(_score->styleD(Sid::pageWidth));
      const double lm = getTenthsFromInches(_score->styleD(Sid::pageOddLeftMargin));
      const double rm = getTenthsFromInches(_score->styleD(Sid::pageEvenLeftMargin));
      //const double tm = getTenthsFromInches(_score->styleD(Sid::pageOddTopMargin));
      const double bm = getTenthsFromInches(_score->styleD(Sid::pageOddBottomMargin));
      //qDebug("page h=%g w=%g lm=%g rm=%g tm=%g bm=%g", h, w, lm, rm, tm, bm);

      // write the credits
      if (measure) {
            for (const Element* element : measure->el()) {
                  if (element->type() == ElementType::TEXT) {
                        const Text* text = (const Text*)element;
                        const double ph = getTenthsFromDots(parentHeight(text));

                        double tx = w / 2;
                        double ty = h - getTenthsFromDots(text->pagePos().y());

                        Align al = text->align();
                        QString just;
                        QString val;

                        if (al & Align::RIGHT) {
                              just = "right";
                              tx   = w - rm;
                              }
                        else if (al & Align::HCENTER) {
                              just = "center";
                              // tx already set correctly
                              }
                        else {
                              just = "left";
                              tx   = lm;
                              }

                        if (al & Align::BOTTOM) {
                              val = "bottom";
                              ty -= ph;
                              }
                        else if (al & Align::VCENTER) {
                              val = "middle";
                              ty -= ph / 2;
                              }
                        else if (al & Align::BASELINE) {
                              val = "baseline";
                              ty -= ph / 2;
                              }
                        else {
                              val = "top";
                              // ty already set correctly
                              }

                        creditWords(xml, _score, tx, ty, just, val, text->fragmentList());
                        }
                  }
            }

      if (!rights.isEmpty()) {
            // put copyright at the bottom center of the page
            // note: as the copyright metatag contains plain text, special XML characters must be escaped
            TextFragment f(XmlWriter::xmlString(rights));
            f.changeFormat(FormatId::FontFamily, _score->styleSt(Sid::footerFontFace));
            f.changeFormat(FormatId::FontSize, _score->styleD(Sid::footerFontSize));
            QList<TextFragment> list;
            list.append(f);
            creditWords(xml, _score, w / 2, bm, "center", "bottom", list);
            }
      }

//---------------------------------------------------------
//   midipitch2xml
//---------------------------------------------------------

static int alterTab[12] = { 0,   1,   0,   1,   0,  0,   1,   0,   1,   0,   1,   0 };
static char noteTab[12] = { 'C', 'C', 'D', 'D', 'E', 'F', 'F', 'G', 'G', 'A', 'A', 'B' };

static void midipitch2xml(int pitch, char& c, int& alter, int& octave)
      {
      // 60 = C 4
      c      = noteTab[pitch % 12];
      alter  = alterTab[pitch % 12];
      octave = pitch / 12 - 1;
      //qDebug("midipitch2xml(pitch %d) step %c, alter %d, octave %d", pitch, c, alter, octave);
      }

//---------------------------------------------------------
//   tabpitch2xml
//---------------------------------------------------------

static void tabpitch2xml(const int pitch, const int tpc, QString& s, int& alter, int& octave)
      {
      s      = tpc2stepName(tpc);
      alter  = tpc2alterByKey(tpc, Key::C);
      octave = (pitch - alter) / 12 - 1;
      if (alter < -2 || 2 < alter)
            qDebug("tabpitch2xml(pitch %d, tpc %d) problem:  step %s, alter %d, octave %d",
                   pitch, tpc, qPrintable(s), alter, octave);
      /*
      else
            qDebug("tabpitch2xml(pitch %d, tpc %d) step %s, alter %d, octave %d",
                   pitch, tpc, qPrintable(s), alter, octave);
       */
      }

//---------------------------------------------------------
//   pitch2xml
//---------------------------------------------------------

// TODO validation

static void pitch2xml(const Note* note, QString& s, int& alter, int& octave)
      {

      const Staff* st = note->staff();
      const Instrument* instr = st->part()->instrument();   // TODO: tick
      const Interval intval = instr->transpose();

      s      = tpc2stepName(note->tpc());
      alter  = tpc2alterByKey(note->tpc(), Key::C);
      // note that pitch must be converted to concert pitch
      // in order to calculate the correct octave
      octave = (note->pitch() - intval.chromatic - alter) / 12 - 1;

      //
      // HACK:
      // On percussion clefs there is no relationship between
      // note->pitch() and note->line()
      // note->line() is determined by drumMap
      //
      Fraction tick        = note->chord()->tick();
      ClefType ct     = st->clef(tick);
      if (ct == ClefType::PERC || ct == ClefType::PERC2) {
            alter = 0;
            octave = line2pitch(note->line(), ct, Key::C) / 12 - 1;
            }

      // correct for ottava lines
      int ottava = 0;
      switch (note->ppitch() - note->pitch()) {
            case  24: ottava =  2; break;
            case  12: ottava =  1; break;
            case   0: ottava =  0; break;
            case -12: ottava = -1; break;
            case -24: ottava = -2; break;
            default:  qDebug("pitch2xml() tick=%d pitch()=%d ppitch()=%d",
                             tick.ticks(), note->pitch(), note->ppitch());
            }
      octave += ottava;

      //qDebug("pitch2xml(pitch %d, tpc %d, ottava %d clef %hhd) step    %s, alter    %d, octave    %d",
      //       note->pitch(), note->tpc(), ottava, clef, qPrintable(s), alter, octave);
      }

// unpitch2xml -- calculate display-step and display-octave for an unpitched note
// note:
// even though this produces the correct step/octave according to Recordare's tutorial
// Finale Notepad 2012 does not import a three line staff with percussion clef correctly
// Same goes for Sibelius 6 in case of three or five line staff with percussion clef

static void unpitch2xml(const Note* note, QString& s, int& octave)
      {
      static char table1[]  = "FEDCBAG";

      Fraction tick        = note->chord()->tick();
      Staff* st       = note->staff();
      ClefType ct     = st->clef(tick);
      // offset in lines between staff with current clef and with G clef
      int clefOffset  = ClefInfo::pitchOffset(ct) - ClefInfo::pitchOffset(ClefType::G);
      // line note would be on on a five line staff with G clef
      // note top line is line 0, bottom line is line 8
      int line5g      = note->line() - clefOffset;
      // in MusicXML with percussion clef, step and octave are determined as if G clef is used
      // when stafflines is not equal to five, in MusicXML the bottom line is still E4.
      // in MuseScore assumes line 0 is F5
      // MS line numbers (top to bottom) plus correction to get lowest line at E4 (line 8)
      // 1 line staff: 0             -> correction 8
      // 3 line staff: 2, 4, 6       -> correction 2
      // 5 line staff: 0, 2, 4, 6, 8 -> correction 0
      // TODO handle other # staff lines ?
      if (st->lines(Fraction(0,1)) == 1) line5g += 8;
      if (st->lines(Fraction(0,1)) == 3) line5g += 2;
      // index in table1 to get step
      int stepIdx     = (line5g + 700) % 7;
      // get step
      s               = table1[stepIdx];
      // calculate octave, offset "3" correcting for the fact that an octave starts
      // with C instead of F
      octave =(3 - line5g + 700) / 7 + 5 - 100;
      // qDebug("ExportMusicXml::unpitch2xml(%p) clef %d clef.po %d clefOffset %d staff.lines %d note.line %d line5g %d step %c oct %d",
      //        note, ct, clefTable[ct].pitchOffset, clefOffset, st->lines(), note->line(), line5g, step, octave);
      }

//---------------------------------------------------------
//   tick2xml
//    set type + dots depending on tick len
//---------------------------------------------------------

static QString tick2xml(const Fraction& ticks, int* dots)
      {
      TDuration t;
      t.setVal(ticks.ticks());
      *dots = t.dots();
      return t.name();
      }

//---------------------------------------------------------
//   findVolta -- find volta starting in measure m
//---------------------------------------------------------

static Volta* findVolta(Measure* m, bool left)
      {
      Fraction stick = m->tick();
      Fraction etick = m->tick() + m->ticks();
      auto spanners = m->score()->spannerMap().findOverlapping(stick.ticks(), etick.ticks());
      for (auto i : spanners) {
            Spanner* el = i.value;
            if (el->type() != ElementType::VOLTA)
                  continue;
            if (left && el->tick() == stick)
                  return (Volta*) el;
            if (!left && el->tick2() == etick)
                  return (Volta*) el;
            }
      return 0;
      }

//---------------------------------------------------------
//   ending
//---------------------------------------------------------

static void ending(XmlWriter& xml, Volta* v, bool left)
      {
      QString number = "";
      QString type = "";
      for (int i : v->endings()) {
            if (!number.isEmpty())
                  number += ", ";
            number += QString("%1").arg(i);
            }
      if (left) {
            type = "start";
            }
      else {
            Volta::Type st = v->voltaType();
            switch (st) {
                  case Volta::Type::OPEN:
                        type = "discontinue";
                        break;
                  case Volta::Type::CLOSED:
                        type = "stop";
                        break;
                  default:
                        qDebug("unknown volta subtype %d", int(st));
                        type = "unknown";
                        break;
                  }
            }
      QString voltaXml = QString("ending number=\"%1\" type=\"%2\"").arg(number).arg(type);
      voltaXml += addPositioningAttributes(v, left);
      xml.tagE(voltaXml);
      }

//---------------------------------------------------------
//   barlineLeft -- search for and handle barline left
//---------------------------------------------------------

void ExportMusicXml::barlineLeft(Measure* m)
      {
      bool rs = m->repeatStart();
      Volta* volta = findVolta(m, true);
      if (!rs && !volta) return;
      _attr.doAttr(_xml, false);
      _xml.stag(QString("barline location=\"left\""));
      if (rs)
            _xml.tag("bar-style", QString("heavy-light"));
      if (volta)
            ending(_xml, volta, true);
      if (rs)
            _xml.tagE("repeat direction=\"forward\"");
      _xml.etag();
      }

//---------------------------------------------------------
//   barlineRight -- search for and handle barline right
//---------------------------------------------------------

void ExportMusicXml::barlineRight(Measure* m)
      {
      const Measure* mmR1 = m->mmRest1(); // the multi measure rest this measure is covered by
      const Measure* mmRLst = mmR1->isMMRest() ? mmR1->mmRestLast() : 0; // last measure of replaced sequence of empty measures
      // note: use barlinetype as found in multi measure rest for last measure of replaced sequence
      BarLineType bst = m == mmRLst ? mmR1->endBarLineType() : m->endBarLineType();
      bool visible = m->endBarLineVisible();

      bool needBarStyle = (bst != BarLineType::NORMAL && bst != BarLineType::START_REPEAT) || !visible;
      Volta* volta = findVolta(m, false);
      // detect short and tick barlines
      QString special = "";
      if (bst == BarLineType::NORMAL) {
            const BarLine* bl = m->endBarLine();
            if (bl && !bl->spanStaff()) {
                  if (bl->spanFrom() == BARLINE_SPAN_TICK1_FROM && bl->spanTo() == BARLINE_SPAN_TICK1_TO)
                        special = "tick";
                  if (bl->spanFrom() == BARLINE_SPAN_TICK2_FROM && bl->spanTo() == BARLINE_SPAN_TICK2_TO)
                        special = "tick";
                  if (bl->spanFrom() == BARLINE_SPAN_SHORT1_FROM && bl->spanTo() == BARLINE_SPAN_SHORT1_TO)
                        special = "short";
                  if (bl->spanFrom() == BARLINE_SPAN_SHORT2_FROM && bl->spanTo() == BARLINE_SPAN_SHORT2_FROM)
                        special = "short";
                  }
            }
      if (!needBarStyle && !volta && special.isEmpty())
            return;

      _xml.stag(QString("barline location=\"right\""));
      if (needBarStyle) {
            if (!visible) {
                  _xml.tag("bar-style", QString("none"));
                  }
            else {
                  switch (bst) {
                        case BarLineType::DOUBLE:
                              _xml.tag("bar-style", QString("light-light"));
                              break;
                        case BarLineType::END_REPEAT:
                              _xml.tag("bar-style", QString("light-heavy"));
                              break;
                        case BarLineType::BROKEN:
                              _xml.tag("bar-style", QString("dashed"));
                              break;
                        case BarLineType::DOTTED:
                              _xml.tag("bar-style", QString("dotted"));
                              break;
                        case BarLineType::END:
                        case BarLineType::END_START_REPEAT:
                              _xml.tag("bar-style", QString("light-heavy"));
                              break;
                        default:
                              qDebug("ExportMusicXml::bar(): bar subtype %d not supported", int(bst));
                              break;
                        }
                  }
            }
      else if (!special.isEmpty()) {
            _xml.tag("bar-style", special);
            }

      if (volta) {
            ending(_xml, volta, false);
            }

      if (bst == BarLineType::END_REPEAT || bst == BarLineType::END_START_REPEAT) {
            if (m->repeatCount() > 2) {
                  _xml.tagE(QString("repeat direction=\"backward\" times=\"%1\"").arg(m->repeatCount()));
                  } else {
                  _xml.tagE("repeat direction=\"backward\"");
                  }
            }

      _xml.etag();
      }

//---------------------------------------------------------
//   moveToTick
//---------------------------------------------------------

void ExportMusicXml::moveToTick(const Fraction& t)
      {
      // qDebug("ExportMusicXml::moveToTick(t=%d) tick=%d", t, tick);
      if (t < _tick) {
#ifdef DEBUG_TICK
            qDebug(" -> backup");
#endif
            _attr.doAttr(_xml, false);
            _xml.stag("backup");
            _xml.tag("duration", (_tick - t).ticks() / div);
            _xml.etag();
            }
      else if (t > _tick) {
#ifdef DEBUG_TICK
            qDebug(" -> forward");
#endif
            _attr.doAttr(_xml, false);
            _xml.stag("forward");
            _xml.tag("duration", (t - _tick).ticks() / div);
            _xml.etag();
            }
      _tick = t;
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

void ExportMusicXml::timesig(TimeSig* tsig)
      {
      TimeSigType st = tsig->timeSigType();
      Fraction ts = tsig->sig();
      int z = ts.numerator();
      int n = ts.denominator();
      QString ns = tsig->numeratorString();

      _attr.doAttr(_xml, true);
      QString tagName = "time";
      if (st == TimeSigType::FOUR_FOUR)
            tagName += " symbol=\"common\"";
      else if (st == TimeSigType::ALLA_BREVE)
            tagName += " symbol=\"cut\"";
      if (!tsig->visible())
            tagName += " print-object=\"no\"";
      tagName += color2xml(tsig);
      _xml.stag(tagName);

      QRegExp rx("^\\d+(\\+\\d+)+$"); // matches a compound numerator
      if (rx.exactMatch(ns))
            // if compound numerator, exported as is
            _xml.tag("beats", ns);
      else
            // else fall back and use the numerator as integer
            _xml.tag("beats", z);
      _xml.tag("beat-type", n);
      _xml.etag();
      }

//---------------------------------------------------------
//   accSymId2alter
//---------------------------------------------------------

static double accSymId2alter(SymId id)
      {
      double res = 0;
      switch (id) {
            case SymId::accidentalDoubleFlat:                      res = -2;   break;
            case SymId::accidentalThreeQuarterTonesFlatZimmermann: res = -1.5; break;
            case SymId::accidentalFlat:                            res = -1;   break;
            case SymId::accidentalQuarterToneFlatStein:            res = -0.5; break;
            case SymId::accidentalNatural:                         res =  0;   break;
            case SymId::accidentalQuarterToneSharpStein:           res =  0.5; break;
            case SymId::accidentalSharp:                           res =  1;   break;
            case SymId::accidentalThreeQuarterTonesSharpStein:     res =  1.5; break;
            case SymId::accidentalDoubleSharp:                     res =  2;   break;
            default: qDebug("accSymId2alter: unsupported sym %s", Sym::id2name(id));
            }
      return res;
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

void ExportMusicXml::keysig(const KeySig* ks, ClefType ct, int staff, bool visible)
      {
      static char table2[]  = "CDEFGAB";
      int po = ClefInfo::pitchOffset(ct); // actually 7 * oct + step for topmost staff line
      //qDebug("keysig st %d key %d custom %d ct %hhd st %d", staff, kse.key(), kse.custom(), ct, staff);
      //qDebug(" pitch offset clef %d stp %d oct %d ", po, po % 7, po / 7);

      QString tagName = "key";
      if (staff)
            tagName += QString(" number=\"%1\"").arg(staff);
      if (!visible)
            tagName += " print-object=\"no\"";
      tagName += color2xml(ks);
      _attr.doAttr(_xml, true);
      _xml.stag(tagName);

      const KeySigEvent kse = ks->keySigEvent();
      const QList<KeySym> keysyms = kse.keySymbols();
      if (kse.custom() && !kse.isAtonal() && keysyms.size() > 0) {

            // non-traditional key signature
            // MusicXML order is left-to-right order, while KeySims in keySymbols()
            // are in insertion order -> sorting required

            // first put the KeySyms in a map
            QMap<qreal, KeySym> map;
            for (const KeySym& ksym : keysyms) {
                  map.insert(ksym.spos.x(), ksym);
                  }
            // then write them (automatically sorted on key)
            for (const KeySym& ksym : map) {
                  int line = static_cast<int>(round(2 * ksym.spos.y()));
                  int step = (po - line) % 7;
                  //qDebug(" keysym sym %d spos %g,%g pos %g,%g -> line %d step %d",
                  //       ksym.sym, ksym.spos.x(), ksym.spos.y(), ksym.pos.x(), ksym.pos.y(), line, step);
                  _xml.tag("key-step", QString(QChar(table2[step])));
                  _xml.tag("key-alter", accSymId2alter(ksym.sym));
                  _xml.tag("key-accidental", accSymId2MxmlString(ksym.sym));
                  }
            }
      else {
            // traditional key signature
            _xml.tag("fifths", static_cast<int>(kse.key()));
            switch (kse.mode()) {
                  case KeyMode::NONE:     _xml.tag("mode", "none"); break;
                  case KeyMode::MAJOR:    _xml.tag("mode", "major"); break;
                  case KeyMode::MINOR:    _xml.tag("mode", "minor"); break;
                  case KeyMode::UNKNOWN:
                  default:
                        if (kse.custom())
                              _xml.tag("mode", "none");
                  }
            }
      _xml.etag();
      }

//---------------------------------------------------------
//   clef
//---------------------------------------------------------

void ExportMusicXml::clef(int staff, const ClefType ct, const QString& extraAttributes)
      {
      clefDebug("ExportMusicXml::clef(staff %d, clef %hhd)", staff, ct);

      QString tagName = "clef";
      if (staff)
            tagName += QString(" number=\"%1\"").arg(staff);
      tagName += extraAttributes;
      _attr.doAttr(_xml, true);
      _xml.stag(tagName);

      QString sign = ClefInfo::sign(ct);
      int line   = ClefInfo::line(ct);
      _xml.tag("sign", sign);
      _xml.tag("line", line);
      if (ClefInfo::octChng(ct))
            _xml.tag("clef-octave-change", ClefInfo::octChng(ct));
      _xml.etag();
      }

//---------------------------------------------------------
//   tupletStartStop
//---------------------------------------------------------

// LVIFIX: add placement to tuplet support
// <notations>
//   <tuplet type="start" placement="above" bracket="no"/>
// </notations>

static void tupletStartStop(ChordRest* cr, Notations& notations, XmlWriter& xml)
      {
      Tuplet* t = cr->tuplet();
      if (!t) return;
      if (cr == t->elements().front()) {
            notations.tag(xml);
            QString tupletTag = "tuplet type=\"start\"";
            tupletTag += " bracket=";
            tupletTag += t->hasBracket() ? "\"yes\"" : "\"no\"";
            if (t->numberType() == TupletNumberType::SHOW_RELATION)
                  tupletTag += " show-number=\"both\"";
            if (t->numberType() == TupletNumberType::NO_TEXT)
                  tupletTag += " show-number=\"none\"";
            xml.tagE(tupletTag);
            }
      if (cr == t->elements().back()) {
            notations.tag(xml);
            xml.tagE("tuplet type=\"stop\"");
            }
      }

//---------------------------------------------------------
//   findTrill -- get index of trill in trill table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findTrill(const Trill* tr) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (trills[i] == tr) return i;
      return -1;
      }

//---------------------------------------------------------
//   wavyLineStartStop
//---------------------------------------------------------

void ExportMusicXml::wavyLineStartStop(Chord* chord, Notations& notations, Ornaments& ornaments,
                                       TrillHash& trillStart, TrillHash& trillStop)
      {
      if (trillStop.contains(chord)) {
            const Trill* tr = trillStop.value(chord);
            int n = findTrill(tr);
            if (n >= 0)
                  // trill stop after trill start
                  trills[n] = 0;
            else {
                  // trill stop before trill start
                  n = findTrill(0);
                  if (n >= 0)
                        trills[n] = tr;
                  else
                        qDebug("too many overlapping trills (chord %p staff %d tick %d)",
                               chord, chord->staffIdx(), chord->tick().ticks());
                  }
            if (n >= 0) {
                  notations.tag(_xml);
                  ornaments.tag(_xml);
                  QString trillXml = QString("wavy-line type=\"stop\" number=\"%1\"").arg(n + 1);
                  trillXml += addPositioningAttributes(tr, false);
                  _xml.tagE(trillXml);
                  }
            trillStop.remove(chord);
            }
      if (trillStart.contains(chord)) {
            const Trill* tr = trillStart.value(chord);
            int n = findTrill(tr);
            if (n >= 0)
                  qDebug("wavyLineStartStop error");
            else {
                  n = findTrill(0);
                  if (n >= 0) {
                        trills[n] = tr;
                        // mscore only supports wavy-line with trill-mark
                        notations.tag(_xml);
                        ornaments.tag(_xml);
                        _xml.tagE("trill-mark");
                        QString tagName = "wavy-line type=\"start\"";
                        tagName += QString(" number=\"%1\"").arg(n + 1);
                        tagName += color2xml(tr);
                        tagName += addPositioningAttributes(tr, true);
                        _xml.tagE(tagName);
                        }
                  else
                        qDebug("too many overlapping trills (chord %p staff %d tick %d)",
                               chord, chord->staffIdx(), chord->tick().ticks());
                  trillStart.remove(chord);
                  }
            }
      }

//---------------------------------------------------------
//   hasBreathMark - determine if chord has breath-mark
//---------------------------------------------------------

static Breath* hasBreathMark(Chord* ch)
      {
      Fraction tick = ch->tick() + ch->actualTicks();
      Segment* s = ch->measure()->findSegment(SegmentType::Breath, tick);
      return s ? toBreath(s->element(ch->track())) : 0;
      }

//---------------------------------------------------------
//   tremoloSingleStartStop
//---------------------------------------------------------

static void tremoloSingleStartStop(Chord* chord, Notations& notations, Ornaments& ornaments, XmlWriter& xml)
      {
      if (chord->tremolo()) {
            Tremolo* tr = chord->tremolo();
            int count = 0;
            TremoloType st = tr->tremoloType();
            QString type = "";

            if (chord->tremoloChordType() == TremoloChordType::TremoloSingle) {
                  type = "single";
                  switch (st) {
                        case TremoloType::R8:  count = 1; break;
                        case TremoloType::R16: count = 2; break;
                        case TremoloType::R32: count = 3; break;
                        case TremoloType::R64: count = 4; break;
                        default: qDebug("unknown tremolo single %d", int(st)); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloFirstNote) {
                  type = "start";
                  switch (st) {
                        case TremoloType::C8:  count = 1; break;
                        case TremoloType::C16: count = 2; break;
                        case TremoloType::C32: count = 3; break;
                        case TremoloType::C64: count = 4; break;
                        default: qDebug("unknown tremolo double %d", int(st)); break;
                        }
                  }
            else if (chord->tremoloChordType() == TremoloChordType::TremoloSecondNote) {
                  type = "stop";
                  switch (st) {
                        case TremoloType::C8:  count = 1; break;
                        case TremoloType::C16: count = 2; break;
                        case TremoloType::C32: count = 3; break;
                        case TremoloType::C64: count = 4; break;
                        default: qDebug("unknown tremolo double %d", int(st)); break;
                        }
                  }
            else qDebug("unknown tremolo subtype %d", int(st));


            if (type != "" && count > 0) {
                  notations.tag(xml);
                  ornaments.tag(xml);
                  QString tagName = "tremolo";
                  tagName += QString(" type=\"%1\"").arg(type);
                  if (type == "single" || type == "start")
                        tagName += color2xml(tr);
                  xml.tag(tagName, count);
                  }
            }
      }


//---------------------------------------------------------
//   fermatas
//---------------------------------------------------------

static void fermatas(const QVector<Element*>& cra, XmlWriter& xml, Notations& notations)
      {
      for (const Element* e : cra) {
            if (!e->isFermata())
                  continue;
            const Fermata* a = toFermata(e);
            notations.tag(xml);
            QString tagName = "fermata";
            tagName += QString(" type=\"%1\"").arg(a->placement() == Placement::ABOVE ? "upright" : "inverted");
            tagName += color2xml(a);
            SymId id = a->symId();
            if (id == SymId::fermataAbove || id == SymId::fermataBelow)
                  xml.tagE(tagName);
            // MusicXML does not support the very short fermata nor short fermata (Henze),
            // export as short fermata (better than not exporting at all)
            else if (id == SymId::fermataShortAbove || id == SymId::fermataShortBelow
                     || id == SymId::fermataShortHenzeAbove || id == SymId::fermataShortHenzeBelow
                     || id == SymId::fermataVeryShortAbove || id == SymId::fermataVeryShortBelow)
                  xml.tag(tagName, "angled");
            // MusicXML does not support the very long fermata  nor long fermata (Henze),
            // export as long fermata (better than not exporting at all)
            else if (id == SymId::fermataLongAbove || id == SymId::fermataLongBelow
                     || id == SymId::fermataLongHenzeAbove || id == SymId::fermataLongHenzeBelow
                     || id == SymId::fermataVeryLongAbove || id == SymId::fermataVeryLongBelow)
                  xml.tag(tagName, "square");
            }
      }

//---------------------------------------------------------
//   symIdToArtic
//---------------------------------------------------------

static QString symIdToArtic(const SymId sid)
      {
      switch (sid) {
            case SymId::articAccentAbove:
                  return "accent placement=\"above\"";
                  break;

            case SymId::articAccentBelow:
                  return "accent placement=\"below\"";
                  break;

            case SymId::articStaccatoAbove:
            case SymId::articAccentStaccatoAbove:
            case SymId::articMarcatoStaccatoAbove:
                  return "staccato placement=\"above\"";
                  break;

            case SymId::articStaccatoBelow:
            case SymId::articAccentStaccatoBelow:
            case SymId::articMarcatoStaccatoBelow:
                  return "staccato placement=\"below\"";
                  break;

            case SymId::articStaccatissimoAbove:
            case SymId::articStaccatissimoStrokeAbove:
            case SymId::articStaccatissimoWedgeAbove:
                  return "staccatissimo placement=\"above\"";
                  break;

            case SymId::articStaccatissimoBelow:
            case SymId::articStaccatissimoStrokeBelow:
            case SymId::articStaccatissimoWedgeBelow:
                  return "staccatissimo placement=\"below\"";
                  break;

            case SymId::articTenutoAbove:
            case SymId::articMarcatoTenutoAbove:
                  return "tenuto placement=\"above\"";
                  break;

            case SymId::articTenutoBelow:
            case SymId::articMarcatoTenutoBelow:
                  return "tenuto placement=\"below\"";
                  break;

            case SymId::articMarcatoAbove:
                  return "strong-accent placement=\"above\"";
                  break;

            case SymId::articMarcatoBelow:
                  return "strong-accent placement=\"below\"";
                  break;

            case SymId::articTenutoStaccatoAbove:
                  return "detached-legato placement=\"above\"";
                  break;

            case SymId::articTenutoStaccatoBelow:
                  return "detached-legato placement=\"below\"";
                  break;

            default:
                  ;       // nothing
                  break;
            }

      return "";
      }

//---------------------------------------------------------
//   symIdToOrnam
//---------------------------------------------------------

static QString symIdToOrnam(const SymId sid)
      {
      switch (sid) {
            case SymId::ornamentTurnInverted:
                  return "inverted-turn";
                  break;
            case SymId::ornamentTurn:
                  return "turn";
                  break;
            case SymId::ornamentTrill:
                  return "trill-mark";
                  break;
            case SymId::ornamentMordentInverted:
                  return "mordent";
                  // return "inverted-mordent";
                  break;
            case SymId::ornamentMordent:
                  // return "mordent";
                  return "inverted-mordent";
                  break;
            case SymId::ornamentTremblement:
                  return "inverted-mordent long=\"yes\"";
                  break;
            case SymId::ornamentPrallMordent:
                  return "mordent long=\"yes\"";
                  break;
            case SymId::ornamentUpPrall:
                  return "inverted-mordent long=\"yes\" approach=\"below\"";
                  break;
            case SymId::ornamentPrecompMordentUpperPrefix:
                  return "inverted-mordent long=\"yes\" approach=\"above\"";
                  break;
            case SymId::ornamentUpMordent:
                  return "mordent long=\"yes\" approach=\"below\"";
                  break;
            case SymId::ornamentDownMordent:
                  return "mordent long=\"yes\" approach=\"above\"";
                  break;
            case SymId::ornamentPrallDown:
                  return "inverted-mordent long=\"yes\" departure=\"below\"";
                  break;
            case SymId::ornamentPrallUp:
                  return "inverted-mordent long=\"yes\" departure=\"above\"";
                  break;
            case SymId::ornamentLinePrall:
                  // MusicXML 3.0 does not distinguish between downprall and lineprall
                  return "inverted-mordent long=\"yes\" approach=\"above\"";
                  break;
            case SymId::ornamentPrecompSlide:
                  return "schleifer";
                  break;

            default:
                  ; // nothing
                  break;
            }

      return "";
      }

//---------------------------------------------------------
//   symIdToTechn
//---------------------------------------------------------

static QString symIdToTechn(const SymId sid)
      {
      switch (sid) {
            case SymId::brassMuteClosed:
                  return "stopped";
                  break;
            case SymId::stringsHarmonic:
                  return "x"; // will be overruled but must be non-empty
                  break;
            case SymId::stringsUpBow:
                  return "up-bow";
                  break;
            case SymId::stringsDownBow:
                  return "down-bow";
                  break;
            case SymId::pluckedSnapPizzicatoAbove:
                  return "snap-pizzicato";
                  break;
            case SymId::brassMuteOpen:
                  return "open-string";
                  break;
            case SymId::stringsThumbPosition:
                  return "thumb-position";
                  break;
            default:
                  ; // nothing
                  break;
            }

      return "";
      }

//---------------------------------------------------------
//   writeChordLines
//---------------------------------------------------------

static void writeChordLines(const Chord* const chord, XmlWriter& xml, Notations& notations, Articulations& articulations)
      {
      for (Element* e : chord->el()) {
            qDebug("chordAttributes: el %p type %d (%s)", e, int(e->type()), e->name());
            if (e->type() == ElementType::CHORDLINE) {
                  ChordLine const* const cl = static_cast<ChordLine*>(e);
                  QString subtype;
                  switch (cl->chordLineType()) {
                        case ChordLineType::FALL:
                              subtype = "falloff";
                              break;
                        case ChordLineType::DOIT:
                              subtype = "doit";
                              break;
                        case ChordLineType::PLOP:
                              subtype = "plop";
                              break;
                        case ChordLineType::SCOOP:
                              subtype = "scoop";
                              break;
                        default:
                              qDebug("unknown ChordLine subtype %d", int(cl->chordLineType()));
                        }
                  if (subtype != "") {
                        notations.tag(xml);
                        articulations.tag(xml);
                        xml.tagE(subtype);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   chordAttributes
//---------------------------------------------------------

void ExportMusicXml::chordAttributes(Chord* chord, Notations& notations, Technical& technical,
                                     TrillHash& trillStart, TrillHash& trillStop)
      {
      QVector<Element*> fl;
      for (Element* e : chord->segment()->annotations()) {
            if (e->track() == chord->track() && e->isFermata())
                  fl.push_back(e);
            }
      fermatas(fl, _xml, notations);

      const QVector<Articulation*> na = chord->articulations();
      // first the attributes whose elements are children of <articulations>
      Articulations articulations;
      for (const Articulation* a : na) {
            auto sid = a->symId();
            auto mxmlArtic = symIdToArtic(sid);

            if (mxmlArtic != "") {
                  if (sid == SymId::articMarcatoAbove || sid == SymId::articMarcatoBelow) {
                        if (a->up())
                              mxmlArtic += " type=\"up\"";
                        else
                              mxmlArtic += " type=\"down\"";
                        }

                  notations.tag(_xml);
                  articulations.tag(_xml);
                  _xml.tagE(mxmlArtic);
                  }
            }

      if (Breath* b = hasBreathMark(chord)) {
            notations.tag(_xml);
            articulations.tag(_xml);
            _xml.tagE(b->isCaesura() ? "caesura" : "breath-mark");
            }

      writeChordLines(chord, _xml, notations, articulations);

      articulations.etag(_xml);

      // then the attributes whose elements are children of <ornaments>
      Ornaments ornaments;
      for (const Articulation* a : na) {
            auto sid = a->symId();
            auto mxmlOrnam = symIdToOrnam(sid);

            if (mxmlOrnam != "") {
                  notations.tag(_xml);
                  ornaments.tag(_xml);
                  _xml.tagE(mxmlOrnam);
                  }
            }

      tremoloSingleStartStop(chord, notations, ornaments, _xml);
      wavyLineStartStop(chord, notations, ornaments, trillStart, trillStop);
      ornaments.etag(_xml);

      // and finally the attributes whose elements are children of <technical>
      for (const Articulation* a : na) {
            auto sid = a->symId();

            if (sid == SymId::stringsHarmonic) {
                  notations.tag(_xml);
                  technical.tag(_xml);
                  _xml.stag("harmonic");
                  _xml.tagE("natural");
                  _xml.etag();
                  }
            else {
                  auto mxmlTechn = symIdToTechn(sid);
                  if (mxmlTechn != "") {
                        notations.tag(_xml);
                        technical.tag(_xml);
                        _xml.tagE(mxmlTechn);
                        }
                  }
            }

      // check if all articulations were handled
      for (const Articulation* a : na) {
            auto sid = a->symId();
            if (symIdToArtic(sid) == ""
                && symIdToOrnam(sid) == ""
                && symIdToTechn(sid) == "") {
                  qDebug("unknown chord attribute %s", qPrintable(a->userName()));
                  }
            }
      }

//---------------------------------------------------------
//   arpeggiate
//---------------------------------------------------------

// <notations>
//   <arpeggiate direction="up"/>
//   </notations>

static void arpeggiate(Arpeggio* arp, bool front, bool back, XmlWriter& xml, Notations& notations)
      {
      QString tagName = "";
      switch (arp->arpeggioType()) {
            case ArpeggioType::NORMAL:
                  notations.tag(xml);
                  tagName = "arpeggiate";
                  break;
            case ArpeggioType::UP:          // fall through
            case ArpeggioType::UP_STRAIGHT: // not supported by MusicXML, export as normal arpeggio
                  notations.tag(xml);
                  tagName = "arpeggiate direction=\"up\"";
                  break;
            case ArpeggioType::DOWN:          // fall through
            case ArpeggioType::DOWN_STRAIGHT: // not supported by MusicXML, export as normal arpeggio
                  notations.tag(xml);
                  tagName = "arpeggiate direction=\"down\"";
                  break;
            case ArpeggioType::BRACKET:
                  if (front) {
                        notations.tag(xml);
                        tagName = "non-arpeggiate type=\"bottom\"";
                        }
                  if (back) {
                        notations.tag(xml);
                        tagName = "non-arpeggiate type=\"top\"";
                        }
                  break;
            default:
                  qDebug("unknown arpeggio subtype %d", int(arp->arpeggioType()));
                  break;
            }

      tagName += addPositioningAttributes(arp);
      if (tagName != "")
            xml.tagE(tagName);
      }

//---------------------------------------------------------
//   determineTupletNormalTicks
//---------------------------------------------------------

/**
 Determine the ticks in the normal type for the tuplet \a chord.
 This is non-zero only if chord if part of a tuplet containing
 different length duration elements.
 TODO determine how to handle baselen with dots and verify correct behaviour.
 TODO verify if baseLen should always be correctly set
      (it seems after MusicXMLimport this is not the case)
 */

static int determineTupletNormalTicks(ChordRest const* const chord)
      {
      Tuplet const* const t = chord->tuplet();
      if (!t)
            return 0;
      /*
      qDebug("determineTupletNormalTicks t %p baselen %d", t, t->baseLen().ticks());
      for (int i = 0; i < t->elements().size(); ++i)
            qDebug("determineTupletNormalTicks t %p i %d ticks %d", t, i, t->elements().at(i)->duration().ticks());
            */
      for (unsigned int i = 1; i < t->elements().size(); ++i)
            if (t->elements().at(0)->ticks() != t->elements().at(i)->ticks())
                  return t->baseLen().ticks().ticks();
      if (t->elements().size() != (unsigned)(t->ratio().numerator()))
            return t->baseLen().ticks().ticks();
      return 0;
      }

//---------------------------------------------------------
//   writeBeam
//---------------------------------------------------------

//  beaming
//    <beam number="1">start</beam>
//    <beam number="1">end</beam>
//    <beam number="1">continue</beam>
//    <beam number="1">backward hook</beam>
//    <beam number="1">forward hook</beam>

static void writeBeam(XmlWriter& xml, ChordRest* cr, Beam* b)
      {
      const auto& elements = b->elements();
      int idx = elements.indexOf(cr);
      if (idx == -1) {
            qDebug("Beam::writeMusicXml(): cannot find ChordRest");
            return;
            }
      int blp = -1; // beam level previous chord
      int blc = -1; // beam level current chord
      int bln = -1; // beam level next chord
      // find beam level previous chord
      for (int i = idx - 1; blp == -1 && i >= 0; --i) {
            ChordRest* crst = elements[i];
            if (crst->type() == ElementType::CHORD)
                  blp = (static_cast<Chord*>(crst))->beams();
            }
      // find beam level current chord
      if (cr->type() == ElementType::CHORD)
            blc = (static_cast<Chord*>(cr))->beams();
      // find beam level next chord
      for (int i = idx + 1; bln == -1 && i < elements.size(); ++i) {
            ChordRest* crst = elements[i];
            if (crst->type() == ElementType::CHORD)
                  bln = (static_cast<Chord*>(crst))->beams();
            }
      for (int i = 1; i <= blc; ++i) {
            QString s;
            if (blp < i && bln >= i) s = "begin";
            else if (blp < i && bln < i) {
                  if (bln > 0) s = "forward hook";
                  else if (blp > 0) s = "backward hook";
                  }
            else if (blp >= i && bln < i)
                  s = "end";
            else if (blp >= i && bln >= i)
                  s = "continue";
            if (s != "")
                  xml.tag(QString("beam number=\"%1\"").arg(i), s);
            }
      }

//---------------------------------------------------------
//   instrId
//---------------------------------------------------------

static QString instrId(int partNr, int instrNr)
      {
      return QString("id=\"P%1-I%2\"").arg(partNr).arg(instrNr);
      }

//---------------------------------------------------------
//   writeNotehead
//---------------------------------------------------------

static void writeNotehead(XmlWriter& xml, const Note* const note)
      {
      QString noteheadTagname = QString("notehead");
      noteheadTagname += color2xml(note);
      bool leftParenthesis = false, rightParenthesis = false;
      for (Element* elem : note->el()) {
            if (elem->type() == ElementType::SYMBOL) {
                  Symbol* s = static_cast<Symbol*>(elem);
                  if (s->sym() == SymId::noteheadParenthesisLeft)
                        leftParenthesis = true;
                  else if (s->sym() == SymId::noteheadParenthesisRight)
                        rightParenthesis = true;
                  }
            }
      if (rightParenthesis && leftParenthesis)
            noteheadTagname += " parentheses=\"yes\"";
      if (note->headType() == NoteHead::Type::HEAD_QUARTER)
            noteheadTagname += " filled=\"yes\"";
      else if ((note->headType() == NoteHead::Type::HEAD_HALF) || (note->headType() == NoteHead::Type::HEAD_WHOLE))
            noteheadTagname += " filled=\"no\"";
      if (note->headGroup() == NoteHead::Group::HEAD_SLASH)
            xml.tag(noteheadTagname, "slash");
      else if (note->headGroup() == NoteHead::Group::HEAD_TRIANGLE_UP)
            xml.tag(noteheadTagname, "triangle");
      else if (note->headGroup() == NoteHead::Group::HEAD_DIAMOND)
            xml.tag(noteheadTagname, "diamond");
      else if (note->headGroup() == NoteHead::Group::HEAD_PLUS)
            xml.tag(noteheadTagname, "cross");
      else if (note->headGroup() == NoteHead::Group::HEAD_CROSS)
            xml.tag(noteheadTagname, "x");
      else if (note->headGroup() == NoteHead::Group::HEAD_XCIRCLE)
            xml.tag(noteheadTagname, "circle-x");
      else if (note->headGroup() == NoteHead::Group::HEAD_TRIANGLE_DOWN)
            xml.tag(noteheadTagname, "inverted triangle");
      else if (note->headGroup() == NoteHead::Group::HEAD_SLASHED1)
            xml.tag(noteheadTagname, "slashed");
      else if (note->headGroup() == NoteHead::Group::HEAD_SLASHED2)
            xml.tag(noteheadTagname, "back slashed");
      else if (note->headGroup() == NoteHead::Group::HEAD_DO)
            xml.tag(noteheadTagname, "do");
      else if (note->headGroup() == NoteHead::Group::HEAD_RE)
            xml.tag(noteheadTagname, "re");
      else if (note->headGroup() == NoteHead::Group::HEAD_MI)
            xml.tag(noteheadTagname, "mi");
      else if (note->headGroup() == NoteHead::Group::HEAD_FA && !note->chord()->up())
            xml.tag(noteheadTagname, "fa");
      else if (note->headGroup() == NoteHead::Group::HEAD_FA && note->chord()->up())
            xml.tag(noteheadTagname, "fa up");
      else if (note->headGroup() == NoteHead::Group::HEAD_LA)
            xml.tag(noteheadTagname, "la");
      else if (note->headGroup() == NoteHead::Group::HEAD_TI)
            xml.tag(noteheadTagname, "ti");
      else if (note->headGroup() == NoteHead::Group::HEAD_SOL)
            xml.tag(noteheadTagname, "so");
      else if (note->color() != MScore::defaultColor)
            xml.tag(noteheadTagname, "normal");
      else if (rightParenthesis && leftParenthesis)
            xml.tag(noteheadTagname, "normal");
      else if (note->headType() != NoteHead::Type::HEAD_AUTO)
            xml.tag(noteheadTagname, "normal");
      }

//---------------------------------------------------------
//   writeFingering
//---------------------------------------------------------

static void writeFingering(XmlWriter& xml, Notations& notations, Technical& technical, const Note* const note)
      {
      for (const Element* e : note->el()) {
            if (e->type() == ElementType::FINGERING) {
                  Text* f = (Text*)e;
                  notations.tag(xml);
                  technical.tag(xml);
                  QString t = MScoreTextToMXML::toPlainText(f->xmlText());
                  if (f->tid() == Tid::RH_GUITAR_FINGERING)
                        xml.tag("pluck", t);
                  else if (f->tid() == Tid::LH_GUITAR_FINGERING)
                        xml.tag("fingering", t);
                  else if (f->tid() == Tid::FINGERING) {
                        // for generic fingering, try to detect plucking
                        // (backwards compatibility with MuseScore 1.x)
                        // p, i, m, a, c represent the plucking finger
                        if (t == "p" || t == "i" || t == "m" || t == "a" || t == "c")
                              xml.tag("pluck", t);
                        else
                              xml.tag("fingering", t);
                        }
                  else if (f->tid() == Tid::STRING_NUMBER) {
                        bool ok;
                        int i = t.toInt(&ok);
                        if (ok) {
                              if (i == 0)
                                    xml.tagE("open-string");
                              else if (i > 0)
                                    xml.tag("string", t);
                              }
                        if (!ok || i < 0)
                              qDebug("invalid string number '%s'", qPrintable(t));
                        }
                  else
                        qDebug("unknown fingering style");
                  }
            else {
                  // TODO
                  }
            }
      }

//---------------------------------------------------------
//   stretchCorrActTicks
//---------------------------------------------------------

static int stretchCorrActTicks(const Note* const note)
      {
      // time signature stretch factor
      const Fraction str = note->chord()->staff()->timeStretch(note->chord()->tick());
      // chord's actual ticks corrected for stretch
      return (note->chord()->actualTicks() * str).ticks();
      }

//---------------------------------------------------------
//   tremoloCorrection
//---------------------------------------------------------

// duration correction for two note tremolo
static int tremoloCorrection(const Note* const note)
      {
      int tremCorr = 1;
      if (isTwoNoteTremolo(note->chord())) tremCorr = 2;
      return tremCorr;
      }

//---------------------------------------------------------
//   writeTypeAndDots
//---------------------------------------------------------

static void writeTypeAndDots(XmlWriter& xml, const Note* const note)
      {
      // type
      int dots = 0;
      Tuplet* t = note->chord()->tuplet();
      int actNotes = 1;
      int nrmNotes = 1;
      if (t) {
            actNotes = t->ratio().numerator();
            nrmNotes = t->ratio().denominator();
            }

      const auto strActTicks = stretchCorrActTicks(note);
      Fraction tt = Fraction::fromTicks(strActTicks * actNotes * tremoloCorrection(note) / nrmNotes);
      QString s = tick2xml(tt, &dots);
      if (s.isEmpty())
            qDebug("no note type found for ticks %d", strActTicks);

      if (note->small())
            xml.tag("type size=\"cue\"", s);
      else
            xml.tag("type", s);
      for (int ni = dots; ni > 0; ni--)
            xml.tagE("dot");
      }

//---------------------------------------------------------
//   writeTimeModification
//---------------------------------------------------------

static void writeTimeModification(XmlWriter& xml, const Note* const note)
      {
      // time modification for two note tremolo
      // TODO: support tremolo in tuplet ?
      if (tremoloCorrection(note) == 2) {
            xml.stag("time-modification");
            xml.tag("actual-notes", 2);
            xml.tag("normal-notes", 1);
            xml.etag();
            }

      // time modification for tuplet
      const auto t = note->chord()->tuplet();
      if (t) {
            auto actNotes = t->ratio().numerator();
            auto nrmNotes = t->ratio().denominator();
            auto nrmTicks = determineTupletNormalTicks(note->chord());
            // TODO: remove following duplicated code (present for both notes and rests)
            xml.stag("time-modification");
            xml.tag("actual-notes", actNotes);
            xml.tag("normal-notes", nrmNotes);
            //qDebug("nrmTicks %d", nrmTicks);
            if (nrmTicks > 0) {
                  int nrmDots = 0;
                  QString nrmType = tick2xml(Fraction::fromTicks(nrmTicks), &nrmDots);
                  if (nrmType.isEmpty())
                        qDebug("no note type found for ticks %d", nrmTicks);
                  else {
                        xml.tag("normal-type", nrmType);
                        for (int ni = nrmDots; ni > 0; ni--)
                              xml.tagE("normal-dot");
                        }
                  }
            xml.etag();
            }
      }

//---------------------------------------------------------
//   writePitch
//---------------------------------------------------------

static void writePitch(XmlWriter& xml, const Note* const note, const bool useDrumset)
      {
      // step / alter / octave
      QString step;
      int alter = 0;
      int octave = 0;
      const auto chord = note->chord();
      if (chord->staff() && chord->staff()->isTabStaff(Fraction(0,1))) {
            tabpitch2xml(note->pitch(), note->tpc(), step, alter, octave);
            }
      else {
            if (!useDrumset) {
                  pitch2xml(note, step, alter, octave);
                  }
            else {
                  unpitch2xml(note, step, octave);
                  }
            }
      xml.stag(useDrumset ? "unpitched" : "pitch");
      xml.tag(useDrumset  ? "display-step" : "step", step);
      // Check for microtonal accidentals and overwrite "alter" tag
      auto acc = note->accidental();
      double alter2 = 0.0;
      if (acc) {
            switch (acc->accidentalType()) {
                  case AccidentalType::MIRRORED_FLAT:  alter2 = -0.5; break;
                  case AccidentalType::SHARP_SLASH:    alter2 = 0.5;  break;
                  case AccidentalType::MIRRORED_FLAT2: alter2 = -1.5; break;
                  case AccidentalType::SHARP_SLASH4:   alter2 = 1.5;  break;
                  default:                                             break;
                  }
            }
      if (alter && !alter2)
            xml.tag("alter", alter);
      if (!alter && alter2)
            xml.tag("alter", alter2);
      // TODO what if both alter and alter2 are present? For Example: playing with transposing instruments
      xml.tag(useDrumset ? "display-octave" : "octave", octave);
      xml.etag();
      }

//---------------------------------------------------------
//   writeAccidental
//---------------------------------------------------------

static void writeAccidental(XmlWriter& xml, const Note* const note)
      {
      auto acc = note->accidental();
      if (acc) {
            QString s = accidentalType2MxmlString(acc->accidentalType());
            if (s != "") {
                  if (note->accidental()->bracket() != AccidentalBracket::NONE)
                        xml.tag("accidental parentheses=\"yes\"", s);
                  else
                        xml.tag("accidental", s);
                  }
            }
      }

//---------------------------------------------------------
//   notePosition
//---------------------------------------------------------

static QString notePosition(const ExportMusicXml* const expMxml, const Note* const note)
      {
      QString res;

      if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)) {
            const double pageHeight  = expMxml->getTenthsFromInches(expMxml->score()->styleD(Sid::pageHeight));

            const auto chord = note->chord();

            double measureX = expMxml->getTenthsFromDots(chord->measure()->pagePos().x());
            double measureY = pageHeight - expMxml->getTenthsFromDots(chord->measure()->pagePos().y());
            double noteX = expMxml->getTenthsFromDots(note->pagePos().x());
            double noteY = pageHeight - expMxml->getTenthsFromDots(note->pagePos().y());

            res += QString(" default-x=\"%1\"").arg(QString::number(noteX - measureX,'f',2));
            res += QString(" default-y=\"%1\"").arg(QString::number(noteY - measureY,'f',2));
            }

      return res;
      }

//---------------------------------------------------------
//   chord
//---------------------------------------------------------

/**
 Write \a chord on \a staff with lyriclist \a ll.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::chord(Chord* chord, int staff, const std::vector<Lyrics*>* ll, bool useDrumset)
      {
      Part* part = chord->score()->staff(chord->track() / VOICES)->part();
      int partNr = _score->parts().indexOf(part);
      int instNr = instrMap.value(part->instrument(_tick), -1);
      /*
      qDebug("chord() %p parent %p isgrace %d #gracenotes %d graceidx %d",
             chord, chord->parent(), chord->isGrace(), chord->graceNotes().size(), chord->graceIndex());
      qDebug("track %d tick %d part %p nr %d instr %p nr %d",
             chord->track(), chord->tick(), part, partNr, part->instrument(tick), instNr);
      for (Element* e : chord->el())
            qDebug("chord %p el %p", chord, e);
       */
      std::vector<Note*> nl = chord->notes();
      bool grace = chord->isGrace();
      if (!grace) _tick += chord->actualTicks();
#ifdef DEBUG_TICK
      qDebug("ExportMusicXml::chord() oldtick=%d", tick);
      qDebug("notetype=%d grace=%d", gracen, grace);
      qDebug(" newtick=%d", tick);
#endif

      for (Note* note : nl) {
            QString val;

            _attr.doAttr(_xml, false);
            QString noteTag = QString("note");

            noteTag += notePosition(this, note);

            if (!note->visible()) {
                  noteTag += QString(" print-object=\"no\"");
                  }
            //TODO support for OFFSET_VAL
            if (note->veloType() == Note::ValueType::USER_VAL) {
                  int velo = note->veloOffset();
                  noteTag += QString(" dynamics=\"%1\"").arg(QString::number(velo * 100.0 / 90.0,'f',2));
                  }
            _xml.stag(noteTag);

            if (grace) {
                  if (note->noteType() == NoteType::ACCIACCATURA)
                        _xml.tagE("grace slash=\"yes\"");
                  else
                        _xml.tagE("grace");
                  }
            if (note != nl.front())
                  _xml.tagE("chord");
            else if (note->chord()->small()) // need this only once per chord
                  _xml.tagE("cue");

            writePitch(_xml, note, useDrumset);

            // duration
            if (!grace)
                  _xml.tag("duration", stretchCorrActTicks(note) / div);

            if (note->tieBack())
                  _xml.tagE("tie type=\"stop\"");
            if (note->tieFor())
                  _xml.tagE("tie type=\"start\"");

            // instrument for multi-instrument or unpitched parts
            if (!useDrumset) {
                  if (instrMap.size() > 1 && instNr >= 0)
                        _xml.tagE(QString("instrument %1").arg(instrId(partNr + 1, instNr + 1)));
                  }
            else
                  _xml.tagE(QString("instrument %1").arg(instrId(partNr + 1, note->pitch() + 1)));

            // voice
            // for a single-staff part, staff is 0, which needs to be corrected
            // to calculate the correct voice number
            int voice = (staff-1) * VOICES + note->chord()->voice() + 1;
            if (staff == 0)
                  voice += VOICES;

            _xml.tag("voice", voice);

            writeTypeAndDots(_xml, note);
            writeAccidental(_xml, note);
            writeTimeModification(_xml, note);

            // no stem for whole notes and beyond
            if (chord->noStem() || chord->measure()->slashStyle(chord->staffIdx())) {
                  _xml.tag("stem", QString("none"));
                  }
            else if (note->chord()->stem()) {
                  _xml.tag("stem", QString(note->chord()->up() ? "up" : "down"));
                  }

            writeNotehead(_xml, note);

            // LVIFIX: check move() handling
            if (staff)
                  _xml.tag("staff", staff + note->chord()->staffMove());

            if (note == nl.front() && chord->beam())
                  writeBeam(_xml, chord, chord->beam());

            Notations notations;
            Technical technical;

            const Tie* tieBack = note->tieBack();
            if (tieBack) {
                  notations.tag(_xml);
                  _xml.tagE("tied type=\"stop\"");
                  }
            const Tie* tieFor = note->tieFor();
            if (tieFor) {
                  notations.tag(_xml);
                  QString rest = slurTieLineStyle(tieFor);
                  _xml.tagE(QString("tied type=\"start\"%1").arg(rest));
                  }

            if (note == nl.front()) {
                  if (!grace)
                        tupletStartStop(chord, notations, _xml);

                  sh.doSlurs(chord, notations, _xml);

                  chordAttributes(chord, notations, technical, _trillStart, _trillStop);
                  }

            writeFingering(_xml, notations, technical, note);

            // write tablature string / fret
            if (chord->staff() && chord->staff()->isTabStaff(Fraction(0,1)))
                  if (note->fret() >= 0 && note->string() >= 0) {
                        notations.tag(_xml);
                        technical.tag(_xml);
                        _xml.tag("string", note->string() + 1);
                        _xml.tag("fret", note->fret());
                        }

            technical.etag(_xml);
            if (chord->arpeggio()) {
                  arpeggiate(chord->arpeggio(), note == nl.front(), note == nl.back(), _xml, notations);
                  }
            for (Spanner* spanner : note->spannerFor())
                  if (spanner->type() == ElementType::GLISSANDO) {
                        gh.doGlissandoStart(static_cast<Glissando*>(spanner), notations, _xml);
                        }
            for (Spanner* spanner : note->spannerBack())
                  if (spanner->type() == ElementType::GLISSANDO) {
                        gh.doGlissandoStop(static_cast<Glissando*>(spanner), notations, _xml);
                        }
            // write glissando (only for last note)
            /*
            Chord* ch = nextChord(chord);
            if ((note == nl.back()) && ch && ch->glissando()) {
                  gh.doGlissandoStart(ch, notations, xml);
                  }
            if (chord->glissando()) {
                  gh.doGlissandoStop(chord, notations, xml);
                  }
            */
            notations.etag(_xml);
            // write lyrics (only for first note)
            if (!grace && (note == nl.front()) && ll)
                  lyrics(ll, chord->track());
            _xml.etag();
            }
      }

//---------------------------------------------------------
//   rest
//---------------------------------------------------------

/**
 Write \a rest on \a staff.

 For a single-staff part, \a staff equals zero, suppressing the <staff> element.
 */

void ExportMusicXml::rest(Rest* rest, int staff)
      {
      static char table2[]  = "CDEFGAB";
#ifdef DEBUG_TICK
      qDebug("ExportMusicXml::rest() oldtick=%d", tick);
#endif
      _attr.doAttr(_xml, false);

      QString noteTag = QString("note");
      noteTag += color2xml(rest);
      if (!rest->visible() ) {
            noteTag += QString(" print-object=\"no\"");
            }
      _xml.stag(noteTag);

      int yOffsSt   = 0;
      int oct       = 0;
      int stp       = 0;
      ClefType clef = rest->staff()->clef(rest->tick());
      int po        = ClefInfo::pitchOffset(clef);

      // Determine y position, but leave at zero in case of tablature staff
      // as no display-step or display-octave should be written for a tablature staff,

      if (clef != ClefType::TAB && clef != ClefType::TAB_SERIF && clef != ClefType::TAB4 && clef != ClefType::TAB4_SERIF) {
            double yOffsSp = rest->offset().y() / rest->spatium();              // y offset in spatium (negative = up)
            yOffsSt = -2 * int(yOffsSp > 0.0 ? yOffsSp + 0.5 : yOffsSp - 0.5); // same rounded to int (positive = up)

            po -= 4;    // pitch middle staff line (two lines times two steps lower than top line)
            po += yOffsSt; // rest "pitch"
            oct = po / 7; // octave
            stp = po % 7; // step
            }

      // Either <rest/>
      // or <rest><display-step>F</display-step><display-octave>5</display-octave></rest>
      if (yOffsSt == 0) {
            _xml.tagE("rest");
            }
      else {
            _xml.stag("rest");
            _xml.tag("display-step", QString(QChar(table2[stp])));
            _xml.tag("display-octave", oct - 1);
            _xml.etag();
            }

      TDuration d = rest->durationType();
      Fraction tickLen = rest->actualTicks();
      if (d.type() == TDuration::DurationType::V_MEASURE) {
            // to avoid forward since rest->ticklen=0 in this case.
            tickLen = rest->measure()->ticks();
            }
      _tick += tickLen;
#ifdef DEBUG_TICK
      qDebug(" tickLen=%d newtick=%d", tickLen, tick);
#endif

      _xml.tag("duration", tickLen.ticks() / div);

      // for a single-staff part, staff is 0, which needs to be corrected
      // to calculate the correct voice number
      int voice = (staff-1) * VOICES + rest->voice() + 1;
      if (staff == 0)
            voice += VOICES;
      _xml.tag("voice", voice);

      // do not output a "type" element for whole measure rest
      if (d.type() != TDuration::DurationType::V_MEASURE) {
            QString s = d.name();
            int dots  = rest->dots();
            if (rest->small())
                  _xml.tag("type size=\"cue\"", s);
            else
                  _xml.tag("type", s);
            for (int i = dots; i > 0; i--)
                  _xml.tagE("dot");
            }

      if (rest->tuplet()) {
            Tuplet* t = rest->tuplet();
            _xml.stag("time-modification");
            _xml.tag("actual-notes", t->ratio().numerator());
            _xml.tag("normal-notes", t->ratio().denominator());
            int nrmTicks = determineTupletNormalTicks(rest);
            if (nrmTicks > 0) {
                  int nrmDots = 0;
                  QString nrmType = tick2xml(Fraction::fromTicks(nrmTicks), &nrmDots);
                  if (nrmType.isEmpty())
                        qDebug("no note type found for ticks %d", nrmTicks);
                  else {
                        _xml.tag("normal-type", nrmType);
                        for (int ni = nrmDots; ni > 0; ni--)
                              _xml.tagE("normal-dot");
                        }
                  }
            _xml.etag();
            }

      if (staff)
            _xml.tag("staff", staff);

      Notations notations;
      QVector<Element*> fl;
      for (Element* e : rest->segment()->annotations()) {
            if (e->isFermata() && e->track() == rest->track())
                  fl.push_back(e);
            }
      fermatas(fl, _xml, notations);

      sh.doSlurs(rest, notations, _xml);

      tupletStartStop(rest, notations, _xml);
      notations.etag(_xml);

      _xml.etag();
      }

//---------------------------------------------------------
//   directionTag
//---------------------------------------------------------

static void directionTag(XmlWriter& xml, Attributes& attr, Element const* const el = 0)
      {
      attr.doAttr(xml, false);
      QString tagname = QString("direction");
      if (el) {
            /*
             qDebug("directionTag() spatium=%g elem=%p tp=%d (%s)\ndirectionTag()  x=%g y=%g xsp,ysp=%g,%g w=%g h=%g userOff.y=%g",
                    el->spatium(),
                    el,
                    el->type(),
                    el->name(),
                    el->x(), el->y(),
                    el->x()/el->spatium(), el->y()/el->spatium(),
                    el->width(), el->height(),
                    el->offset().y()
                   );
             */
            const Element* pel = 0;
            const LineSegment* seg = 0;
            if (el->type() == ElementType::HAIRPIN || el->type() == ElementType::OTTAVA
                || el->type() == ElementType::PEDAL || el->type() == ElementType::TEXTLINE) {
                  // handle elements derived from SLine
                  // find the system containing the first linesegment
                  const SLine* sl = static_cast<const SLine*>(el);
                  if (!sl->segmentsEmpty()) {
                        seg = toLineSegment(sl->frontSegment());
                        /*
                         qDebug("directionTag()  seg=%p x=%g y=%g w=%g h=%g cpx=%g cpy=%g userOff.y=%g",
                                seg, seg->x(), seg->y(),
                                seg->width(), seg->height(),
                                seg->pagePos().x(), seg->pagePos().y(),
                                seg->offset().y());
                         */
                        pel = seg->parent();
                        }
                  }
            else if (el->type() == ElementType::DYNAMIC
                     || el->type() == ElementType::INSTRUMENT_CHANGE
                     || el->type() == ElementType::REHEARSAL_MARK
                     || el->type() == ElementType::STAFF_TEXT
                     || el->type() == ElementType::SYMBOL
                     || el->type() == ElementType::TEXT) {
                  // handle other elements attached (e.g. via Segment / Measure) to a system
                  // find the system containing this element
                  for (const Element* e = el; e; e = e->parent()) {
                        if (e->type() == ElementType::SYSTEM) pel = e;
                        }
                  }
            else
                  qDebug("directionTag() element %p tp=%d (%s) not supported",
                         el, int(el->type()), el->name());

            /*
             if (pel) {
             qDebug("directionTag()  prnt tp=%d (%s) x=%g y=%g w=%g h=%g userOff.y=%g",
                    pel->type(),
                    pel->name(),
                    pel->x(), pel->y(),
                    pel->width(), pel->height(),
                    pel->offset().y());
                  }
             */

            if (pel && pel->type() == ElementType::SYSTEM) {
                  /*
                  const System* sys = static_cast<const System*>(pel);
                  QRectF bb = sys->staff(el->staffIdx())->bbox();
                  qDebug("directionTag()  syst=%p sys x=%g y=%g cpx=%g cpy=%g",
                         sys, sys->pos().x(),  sys->pos().y(),
                         sys->pagePos().x(),
                         sys->pagePos().y()
                        );
                  qDebug("directionTag()  staf x=%g y=%g w=%g h=%g",
                         bb.x(), bb.y(),
                         bb.width(), bb.height());
                  // element is above the staff if center of bbox is above center of staff
                  qDebug("directionTag()  center diff=%g", el->y() + el->height() / 2 - bb.y() - bb.height() / 2);
                   */

                  if (el->isHairpin() || el->isOttava() || el->isPedal() || el->isTextLine()) {
                        // for the line type elements the reference point is vertically centered
                        // actual position info is in the segments
                        // compare the segment's canvas ypos with the staff's center height
                        // if (seg->pagePos().y() < sys->pagePos().y() + bb.y() + bb.height() / 2)
                        if (el->placement() == Placement::ABOVE)
                              tagname += " placement=\"above\"";
                        else
                              tagname += " placement=\"below\"";
                        }
                  else if (el->isDynamic()) {
                        tagname += " placement=\"";
                        tagname += el->placement() == Placement::ABOVE ? "above" : "below";
                        tagname += "\"";
                        }
                  else {
                        /*
                        qDebug("directionTag()  staf ely=%g elh=%g bby=%g bbh=%g",
                               el->y(), el->height(),
                               bb.y(), bb.height());
                         */
                        // if (el->y() + el->height() / 2 < /*bb.y() +*/ bb.height() / 2)
                        if (el->placement() == Placement::ABOVE)
                              tagname += " placement=\"above\"";
                        else
                              tagname += " placement=\"below\"";
                        }
                  } // if (pel && ...
            }
      xml.stag(tagname);
      }

//---------------------------------------------------------
//   directionETag
//---------------------------------------------------------

static void directionETag(XmlWriter& xml, int staff, int offs = 0)
      {
      if (offs)
            xml.tag("offset", offs);
      if (staff)
            xml.tag("staff", staff);
      xml.etag();
      }

//---------------------------------------------------------
//   partGroupStart
//---------------------------------------------------------

static void partGroupStart(XmlWriter& xml, int number, BracketType bracket)
      {
      xml.stag(QString("part-group type=\"start\" number=\"%1\"").arg(number));
      QString br = "";
      switch (bracket) {
            case BracketType::NO_BRACKET:
                  br = "none";
                  break;
            case BracketType::NORMAL:
                  br = "bracket";
                  break;
            case BracketType::BRACE:
                  br = "brace";
                  break;
            case BracketType::LINE:
                  br = "line";
                  break;
            case BracketType::SQUARE:
                  br = "square";
                  break;
            default:
                  qDebug("bracket subtype %d not understood", int(bracket));
            }
      if (br != "")
            xml.tag("group-symbol", br);
      xml.etag();
      }

//---------------------------------------------------------
//   words
//---------------------------------------------------------

static bool findUnit(TDuration::DurationType val, QString& unit)
      {
      unit = "";
      switch (val) {
            case TDuration::DurationType::V_HALF: unit = "half"; break;
            case TDuration::DurationType::V_QUARTER: unit = "quarter"; break;
            case TDuration::DurationType::V_EIGHTH: unit = "eighth"; break;
            default: qDebug("findUnit: unknown DurationType %d", int(val));
            }
      return true;
      }

static bool findMetronome(const QList<TextFragment>& list,
                          QList<TextFragment>& wordsLeft,  // words left of metronome
                          bool& hasParen,      // parenthesis
                          QString& metroLeft,  // left part of metronome
                          QString& metroRight, // right part of metronome
                          QList<TextFragment>& wordsRight // words right of metronome
                          )
      {
      QString words = MScoreTextToMXML::toPlainTextPlusSymbols(list);
      //qDebug("findMetronome('%s')", qPrintable(words));
      hasParen   = false;
      metroLeft  = "";
      metroRight = "";
      int metroPos = -1;   // metronome start position
      int metroLen = 0;    // metronome length

      int indEq  = words.indexOf('=');
      if (indEq <= 0)
            return false;

      int len1 = 0;
      TDuration dur;

      // find first note, limiting search to the part left of the first '=',
      // to prevent matching the second note in a "note1 = note2" metronome
      int pos1 = TempoText::findTempoDuration(words.left(indEq), len1, dur);
      QRegExp eq("\\s*=\\s*");
      int pos2 = eq.indexIn(words, pos1 + len1);
      if (pos1 != -1 && pos2 == pos1 + len1) {
            int len2 = eq.matchedLength();
            if (words.length() > pos2 + len2) {
                  QString s1 = words.mid(0, pos1);     // string to the left of metronome
                  QString s2 = words.mid(pos1, len1);  // first note
                  QString s3 = words.mid(pos2, len2);  // equals sign
                  QString s4 = words.mid(pos2 + len2); // string to the right of equals sign
                  /*
                  qDebug("found note and equals: '%s'%s'%s'%s'",
                         qPrintable(s1),
                         qPrintable(s2),
                         qPrintable(s3),
                         qPrintable(s4)
                         );
                   */

                  // now determine what is to the right of the equals sign
                  // must have either a (dotted) note or a number at start of s4
                  int len3 = 0;
                  QRegExp nmb("\\d+");
                  int pos3 = TempoText::findTempoDuration(s4, len3, dur);
                  if (pos3 == -1) {
                        // did not find note, try to find a number
                        pos3 = nmb.indexIn(s4);
                        if (pos3 == 0)
                              len3 = nmb.matchedLength();
                        }
                  if (pos3 == -1)
                        // neither found
                        return false;

                  QString s5 = s4.mid(0, len3); // number or second note
                  QString s6 = s4.mid(len3);    // string to the right of metronome
                  /*
                  qDebug("found right part: '%s'%s'",
                         qPrintable(s5),
                         qPrintable(s6)
                         );
                   */

                  // determine if metronome has parentheses
                  // left part of string must end with parenthesis plus optional spaces
                  // right part of string must have parenthesis (but not in first pos)
                  int lparen = s1.indexOf("(");
                  int rparen = s6.indexOf(")");
                  hasParen = (lparen == s1.length() - 1 && rparen == 0);

                  metroLeft = s2;
                  metroRight = s5;

                  metroPos = pos1;               // metronome position
                  metroLen = len1 + len2 + len3; // metronome length
                  if (hasParen) {
                        metroPos -= 1;           // move left one position
                        metroLen += 2;           // add length of '(' and ')'
                        }

                  // calculate starting position corrected for surrogate pairs
                  // (which were ignored by toPlainTextPlusSymbols())
                  int corrPos = metroPos;
                  for (int i = 0; i < metroPos; ++i)
                        if (words.at(i).isHighSurrogate())
                              --corrPos;
                  metroPos = corrPos;

                  /*
                  qDebug("-> found '%s'%s' hasParen %d metro pos %d len %d",
                         qPrintable(metroLeft),
                         qPrintable(metroRight),
                         hasParen, metroPos, metroLen
                         );
                   */
                  QList<TextFragment> mid; // not used
                  MScoreTextToMXML::split(list, metroPos, metroLen, wordsLeft, mid, wordsRight);
                  return true;
                  }
            }
      return false;
      }

static void beatUnit(XmlWriter& xml, const TDuration dur)
      {
      int dots = dur.dots();
      QString unit;
      findUnit(dur.type(), unit);
      xml.tag("beat-unit", unit);
      while (dots > 0) {
            xml.tagE("beat-unit-dot");
            --dots;
            }
      }

static void wordsMetrome(XmlWriter& xml, Score* s, TextBase const* const text)
      {
      //qDebug("wordsMetrome('%s')", qPrintable(text->xmlText()));
      const QList<TextFragment> list = text->fragmentList();
      QList<TextFragment>       wordsLeft;  // words left of metronome
      bool hasParen;                        // parenthesis
      QString metroLeft;                    // left part of metronome
      QString metroRight;                   // right part of metronome
      QList<TextFragment>       wordsRight; // words right of metronome

      // set the default words format
      const QString mtf = s->styleSt(Sid::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(s->styleSt(Sid::staffTextFontFace));
      defFmt.setFontSize(s->styleD(Sid::staffTextFontSize));

      if (findMetronome(list, wordsLeft, hasParen, metroLeft, metroRight, wordsRight)) {
            if (wordsLeft.size() > 0) {
                  xml.stag("direction-type");
                  QString attr; // TODO TBD
                  attr += addPositioningAttributes(text);
                  MScoreTextToMXML mttm("words", attr, defFmt, mtf);
                  mttm.writeTextFragments(wordsLeft, xml);
                  xml.etag();
                  }

            xml.stag("direction-type");
            QString tagName = QString("metronome parentheses=\"%1\"").arg(hasParen ? "yes" : "no");
            tagName += addPositioningAttributes(text);
            xml.stag(tagName);
            int len1 = 0;
            TDuration dur;
            TempoText::findTempoDuration(metroLeft, len1, dur);
            beatUnit(xml, dur);

            if (TempoText::findTempoDuration(metroRight, len1, dur) != -1)
                  beatUnit(xml, dur);
            else
                  xml.tag("per-minute", metroRight);

            xml.etag();
            xml.etag();

            if (wordsRight.size() > 0) {
                  xml.stag("direction-type");
                  QString attr; // TODO TBD
                  attr += addPositioningAttributes(text);
                  MScoreTextToMXML mttm("words", attr, defFmt, mtf);
                  mttm.writeTextFragments(wordsRight, xml);
                  xml.etag();
                  }
            }

      else {
            xml.stag("direction-type");
            QString attr;
            if (text->hasFrame()) {
                  if (text->circle())
                        attr = " enclosure=\"circle\"";
                  else
                        attr = " enclosure=\"rectangle\"";
                  }
            attr += addPositioningAttributes(text);
            MScoreTextToMXML mttm("words", attr, defFmt, mtf);
            //qDebug("words('%s')", qPrintable(text->text()));
            mttm.writeTextFragments(text->fragmentList(), xml);
            xml.etag();
            }
      }

void ExportMusicXml::tempoText(TempoText const* const text, int staff)
      {
      /*
      qDebug("ExportMusicXml::tempoText(TempoText='%s')", qPrintable(text->xmlText()));
      */
      _attr.doAttr(_xml, false);
      _xml.stag(QString("direction placement=\"%1\"").arg((text->placement() ==Placement::BELOW ) ? "below" : "above"));
      wordsMetrome(_xml, _score, text);
      /*
      int offs = text->mxmlOff();
      if (offs)
            xml.tag("offset", offs);
      */
      if (staff)
            _xml.tag("staff", staff);
      _xml.tagE(QString("sound tempo=\"%1\"").arg(QString::number(text->tempo()*60.0)));
      _xml.etag();
      }

//---------------------------------------------------------
//   words
//---------------------------------------------------------

void ExportMusicXml::words(Text const* const text, int staff)
      {
      /*
      qDebug("ExportMusicXml::words userOff.x=%f userOff.y=%f xmlText='%s' plainText='%s'",
             text->offset().x(), text->offset().y(),
             qPrintable(text->xmlText()),
             qPrintable(text->plainText()));
      */

      if (text->plainText() == "") {
            // sometimes empty Texts are present, exporting would result
            // in invalid MusicXML (as an empty direction-type would be created)
            return;
            }

      directionTag(_xml, _attr, text);
      wordsMetrome(_xml, _score, text);
      directionETag(_xml, staff);
      }

//---------------------------------------------------------
//   rehearsal
//---------------------------------------------------------

void ExportMusicXml::rehearsal(RehearsalMark const* const rmk, int staff)
      {
      if (rmk->plainText() == "") {
            // sometimes empty Texts are present, exporting would result
            // in invalid MusicXML (as an empty direction-type would be created)
            return;
            }

      directionTag(_xml, _attr, rmk);
      _xml.stag("direction-type");
      QString attr;
      attr += addPositioningAttributes(rmk);
      if (!rmk->hasFrame()) attr = " enclosure=\"none\"";
      // set the default words format
      const QString mtf = _score->styleSt(Sid::MusicalTextFont);
      CharFormat defFmt;
      defFmt.setFontFamily(_score->styleSt(Sid::staffTextFontFace));
      defFmt.setFontSize(_score->styleD(Sid::staffTextFontSize));
      // write formatted
      MScoreTextToMXML mttm("rehearsal", attr, defFmt, mtf);
      mttm.writeTextFragments(rmk->fragmentList(), _xml);
      _xml.etag();
      directionETag(_xml, staff);
      }

//---------------------------------------------------------
//   findDashes -- get index of hairpin in dashes table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findDashes(const TextLineBase* hp) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (dashes[i] == hp) return i;
      return -1;
      }

//---------------------------------------------------------
//   findHairpin -- get index of hairpin in hairpin table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findHairpin(const Hairpin* hp) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (hairpins[i] == hp) return i;
      return -1;
      }

//---------------------------------------------------------
//   fontSyleToXML
//---------------------------------------------------------

static QString fontSyleToXML(const FontStyle style)
      {
      QString res;
      if (style & FontStyle::Bold)
            res += " font-weight=\"bold\"";
      else if (style & FontStyle::Italic)
            res += " font-style=\"italic\"";
      else if (style & FontStyle::Underline)
            res += " underline=\"1\"";
      return res;
      }

//---------------------------------------------------------
//   hairpin
//---------------------------------------------------------

void ExportMusicXml::hairpin(Hairpin const* const hp, int staff, const Fraction& tick)
      {
      const auto isLineType = hp->isLineType();
      int n;
      if (isLineType) {
            n = findDashes(hp);
            if (n >= 0)
                  dashes[n] = nullptr;
            else {
                  n = findDashes(nullptr);
                  if (n >= 0)
                        dashes[n] = hp;
                  else {
                        qDebug("too many overlapping dashes (hp %p staff %d tick %d)", hp, staff, tick.ticks());
                        return;
                        }
                  }
            }
      else {
            n = findHairpin(hp);
            if (n >= 0)
                  hairpins[n] = nullptr;
            else {
                  n = findHairpin(nullptr);
                  if (n >= 0)
                        hairpins[n] = hp;
                  else {
                        qDebug("too many overlapping hairpins (hp %p staff %d tick %d)", hp, staff, tick.ticks());
                        return;
                        }
                  }
            }

      directionTag(_xml, _attr, hp);
      if (isLineType) {
            if (hp->tick() == tick) {
                  _xml.stag("direction-type");
                  QString tag = "words";
                  tag += QString(" font-family=\"%1\"").arg(hp->getProperty(Pid::BEGIN_FONT_FACE).toString());
                  tag += QString(" font-size=\"%1\"").arg(hp->getProperty(Pid::BEGIN_FONT_SIZE).toReal());
                  tag += fontSyleToXML(static_cast<FontStyle>(hp->getProperty(Pid::BEGIN_FONT_STYLE).toInt()));
                  tag += addPositioningAttributes(hp, hp->tick() == tick);
                  _xml.tag(tag, hp->getProperty(Pid::BEGIN_TEXT));
                  _xml.etag();

                  _xml.stag("direction-type");
                  tag = "dashes type=\"start\"";
                  tag += QString(" number=\"%1\"").arg(n + 1);
                  tag += addPositioningAttributes(hp, hp->tick() == tick);
                  _xml.tagE(tag);
                  _xml.etag();
                  }
            else {
                  _xml.stag("direction-type");
                  _xml.tagE(QString("dashes type=\"stop\" number=\"%1\"").arg(n + 1));
                  _xml.etag();
                  }
            }
      else {
            _xml.stag("direction-type");
            QString tag = "wedge type=";
            if (hp->tick() == tick) {
                  if (hp->hairpinType() == HairpinType::CRESC_HAIRPIN) {
                        tag += "\"crescendo\"";
                        if (hp->hairpinCircledTip()) {
                              tag += " niente=\"yes\"";
                              }
                        }
                  else {
                        tag += "\"diminuendo\"";
                        }
                  }
            else {
                  tag += "\"stop\"";
                  if (hp->hairpinCircledTip() && hp->hairpinType() == HairpinType::DECRESC_HAIRPIN) {
                        tag += " niente=\"yes\"";
                        }
                  }
            tag += QString(" number=\"%1\"").arg(n + 1);
            tag += addPositioningAttributes(hp, hp->tick() == tick);
            _xml.tagE(tag);
            _xml.etag();
            }
      directionETag(_xml, staff);
      }

//---------------------------------------------------------
//   findOttava -- get index of ottava in ottava table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findOttava(const Ottava* ot) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (ottavas[i] == ot) return i;
      return -1;
      }

//---------------------------------------------------------
//   ottava
// <octave-shift type="down" size="8" relative-y="14"/>
// <octave-shift type="stop" size="8"/>
//---------------------------------------------------------

void ExportMusicXml::ottava(Ottava const* const ot, int staff, const Fraction& tick)
      {
      int n = findOttava(ot);
      if (n >= 0)
            ottavas[n] = 0;
      else {
            n = findOttava(0);
            if (n >= 0)
                  ottavas[n] = ot;
            else {
                  qDebug("too many overlapping ottavas (ot %p staff %d tick %d)", ot, staff, tick.ticks());
                  return;
                  }
            }

      directionTag(_xml, _attr, ot);
      _xml.stag("direction-type");

      QString octaveShiftXml;
      OttavaType st = ot->ottavaType();
      if (ot->tick() == tick) {
            const char* sz = 0;
            const char* tp = 0;
            switch (st) {
                  case OttavaType::OTTAVA_8VA:
                        sz = "8";
                        tp = "down";
                        break;
                  case OttavaType::OTTAVA_15MA:
                        sz = "15";
                        tp = "down";
                        break;
                  case OttavaType::OTTAVA_8VB:
                        sz = "8";
                        tp = "up";
                        break;
                  case OttavaType::OTTAVA_15MB:
                        sz = "15";
                        tp = "up";
                        break;
                  default:
                        qDebug("ottava subtype %d not understood", int(st));
                  }
            if (sz && tp)
                  octaveShiftXml = QString("octave-shift type=\"%1\" size=\"%2\" number=\"%3\"").arg(tp).arg(sz).arg(n + 1);
            }
      else {
            if (st == OttavaType::OTTAVA_8VA || st == OttavaType::OTTAVA_8VB)
                  octaveShiftXml = QString("octave-shift type=\"stop\" size=\"8\" number=\"%1\"").arg(n + 1);
            else if (st == OttavaType::OTTAVA_15MA || st == OttavaType::OTTAVA_15MB)
                  octaveShiftXml = QString("octave-shift type=\"stop\" size=\"15\" number=\"%1\"").arg(n + 1);
            else
                  qDebug("ottava subtype %d not understood", int(st));
            }
      octaveShiftXml += addPositioningAttributes(ot, ot->tick() == tick);
      _xml.tagE(octaveShiftXml);
      _xml.etag();
      directionETag(_xml, staff);
      }

//---------------------------------------------------------
//   pedal
//---------------------------------------------------------

void ExportMusicXml::pedal(Pedal const* const pd, int staff, const Fraction& tick)
      {
      directionTag(_xml, _attr, pd);
      _xml.stag("direction-type");
      QString pedalXml;
      if (pd->tick() == tick)
            pedalXml = "pedal type=\"start\" line=\"yes\"";
      else
            pedalXml = "pedal type=\"stop\" line=\"yes\"";
      pedalXml += addPositioningAttributes(pd, pd->tick() == tick);
      _xml.tagE(pedalXml);
      _xml.etag();
      directionETag(_xml, staff);
      }

//---------------------------------------------------------
//   findBracket -- get index of bracket in bracket table
//   return -1 if not found
//---------------------------------------------------------

int ExportMusicXml::findBracket(const TextLine* tl) const
      {
      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i)
            if (brackets[i] == tl) return i;
      return -1;
      }

//---------------------------------------------------------
//   textLine
//---------------------------------------------------------

void ExportMusicXml::textLine(TextLine const* const tl, int staff, const Fraction& tick)
      {
      int n;
      // special case: a dashed line w/o hooks is written as dashes
      const auto isDashes = tl->lineStyle() == Qt::DashLine && (tl->beginHookType() == HookType::NONE) && (tl->endHookType() == HookType::NONE);

      if (isDashes) {
            n = findDashes(tl);
            if (n >= 0)
                  dashes[n] = nullptr;
            else {
                  n = findBracket(nullptr);
                  if (n >= 0)
                        dashes[n] = tl;
                  else {
                        qDebug("too many overlapping dashes (tl %p staff %d tick %d)", tl, staff, tick.ticks());
                        return;
                        }
                  }
            }
      else {
            n = findBracket(tl);
            if (n >= 0)
                  brackets[n] = nullptr;
            else {
                  n = findBracket(nullptr);
                  if (n >= 0)
                        brackets[n] = tl;
                  else {
                        qDebug("too many overlapping textlines (tl %p staff %d tick %d)", tl, staff, tick.ticks());
                        return;
                        }
                  }
            }

      QString rest;
      QPointF p;

      QString lineEnd = "none";
      QString type;
      bool hook = false;
      double hookHeight = 0.0;
      if (tl->tick() == tick) {
            if (!isDashes) {
                  QString lineType;
                  switch (tl->lineStyle()) {
                        case Qt::SolidLine:
                              lineType = "solid";
                              break;
                        case Qt::DashLine:
                              lineType = "dashed";
                              break;
                        case Qt::DotLine:
                              lineType = "dotted";
                              break;
                        default:
                              lineType = "solid";
                        }
                  rest += QString(" line-type=\"%1\"").arg(lineType);
                  }
            hook       = tl->beginHookType() != HookType::NONE;
            hookHeight = tl->beginHookHeight().val();
            if (!tl->segmentsEmpty())
                  p = tl->frontSegment()->offset();
            // offs = tl->mxmlOff();
            type = "start";
            }
      else {
            hook = tl->endHookType() != HookType::NONE;
            hookHeight = tl->endHookHeight().val();
            if (!tl->segmentsEmpty())
                  p = (toLineSegment(tl->backSegment()))->userOff2();
            // offs = tl->mxmlOff2();
            type = "stop";
            }

      if (hook) {
            if (hookHeight < 0.0) {
                  lineEnd = "up";
                  hookHeight *= -1.0;
                  }
            else
                  lineEnd = "down";
            rest += QString(" end-length=\"%1\"").arg(hookHeight * 10);
            }

      rest += addPositioningAttributes(tl, tl->tick() == tick);

      directionTag(_xml, _attr, tl);
      if (!tl->beginText().isEmpty() && tl->tick() == tick) {
            _xml.stag("direction-type");
            _xml.tag("words", tl->beginText());
            _xml.etag();
            }
      _xml.stag("direction-type");
      if (isDashes)
            _xml.tagE(QString("dashes type=\"%1\" number=\"%2\"").arg(type, QString::number(n + 1)));
      else
            _xml.tagE(QString("bracket type=\"%1\" number=\"%2\" line-end=\"%3\"%4").arg(type, QString::number(n + 1), lineEnd, rest));
      _xml.etag();
      /*
      if (offs)
            xml.tag("offset", offs);
      */
      directionETag(_xml, staff);
      }

//---------------------------------------------------------
//   dynamic
//---------------------------------------------------------

// In MuseScore dynamics are essentially user-defined texts, therefore the ones
// supported by MusicXML need to be filtered out. Everything not recognized
// as MusicXML dynamics is written as words.

void ExportMusicXml::dynamic(Dynamic const* const dyn, int staff)
      {
      QSet<QString> set; // the valid MusicXML dynamics
      set << "f" << "ff" << "fff" << "ffff" << "fffff" << "ffffff"
          << "fp" << "fz"
          << "mf" << "mp"
          << "p" << "pp" << "ppp" << "pppp" << "ppppp" << "pppppp"
          << "rf" << "rfz"
          << "sf" << "sffz" << "sfp" << "sfpp" << "sfz";

      directionTag(_xml, _attr, dyn);

      _xml.stag("direction-type");

      QString tagName = "dynamics";
      tagName += addPositioningAttributes(dyn);
      _xml.stag(tagName);
      QString dynTypeName = dyn->dynamicTypeName();
      if (set.contains(dynTypeName)) {
            _xml.tagE(dynTypeName);
            }
      else if (dynTypeName != "") {
            QString dynText = dynTypeName;
            if (dyn->dynamicType() == Dynamic::Type::OTHER)
                  dynText = dyn->plainText();
            _xml.tag("other-dynamics", dynText);
            }
      _xml.etag();

      _xml.etag();

      /*
      int offs = dyn->mxmlOff();
      if (offs)
            xml.tag("offset", offs);
      */
      if (staff)
            _xml.tag("staff", staff);

      if (dyn->velocity() > 0)
            _xml.tagE(QString("sound dynamics=\"%1\"").arg(QString::number(dyn->velocity() * 100.0 / 90.0, 'f', 2)));

      _xml.etag();
      }

//---------------------------------------------------------
//   symbol
//---------------------------------------------------------

// TODO: remove dependency on symbol name and replace by a more stable interface
// changes in sym.cpp r2494 broke MusicXML export of pedals (again)

void ExportMusicXml::symbol(Symbol const* const sym, int staff)
      {
      QString name = Sym::id2name(sym->sym());
      QString mxmlName = "";
      if (name == "keyboardPedalPed")
            mxmlName = "pedal type=\"start\"";
      else if (name == "keyboardPedalUp")
            mxmlName = "pedal type=\"stop\"";
      else {
            qDebug("ExportMusicXml::symbol(): %s not supported", qPrintable(name));
            return;
            }
      directionTag(_xml, _attr, sym);
      mxmlName += addPositioningAttributes(sym);
      _xml.stag("direction-type");
      _xml.tagE(mxmlName);
      _xml.etag();
      directionETag(_xml, staff);
      }

//---------------------------------------------------------
//   lyrics
//---------------------------------------------------------

void ExportMusicXml::lyrics(const std::vector<Lyrics*>* ll, const int trk)
      {
      for (const Lyrics* l :* ll) {
            if (l && !l->xmlText().isEmpty()) {
                  if ((l)->track() == trk) {
                        QString lyricXml = QString("lyric number=\"%1\"").arg((l)->no() + 1);
                        lyricXml += color2xml(l);
                        lyricXml += addPositioningAttributes(l);
                        _xml.stag(lyricXml);
                        Lyrics::Syllabic syl = (l)->syllabic();
                        QString s = "";
                        switch (syl) {
                              case Lyrics::Syllabic::SINGLE: s = "single"; break;
                              case Lyrics::Syllabic::BEGIN:  s = "begin";  break;
                              case Lyrics::Syllabic::END:    s = "end";    break;
                              case Lyrics::Syllabic::MIDDLE: s = "middle"; break;
                              default:
                                    qDebug("unknown syllabic %d", int(syl));
                              }
                        _xml.tag("syllabic", s);
                        QString attr; // TODO TBD
                        // set the default words format
                        const QString mtf       = _score->styleSt(Sid::MusicalTextFont);
                        CharFormat defFmt;
                        defFmt.setFontFamily(_score->styleSt(Sid::lyricsEvenFontFace));
                        defFmt.setFontSize(_score->styleD(Sid::lyricsOddFontSize));
                        // write formatted
                        MScoreTextToMXML mttm("text", attr, defFmt, mtf);
                        mttm.writeTextFragments(l->fragmentList(), _xml);
#if 0
                        /*
                         Temporarily disabled because it doesn't work yet (and thus breaks the regression test).
                         See MusicXml::xmlLyric: "// TODO-WS      l->setTick(tick);"
                        if((l)->endTick() > 0)
                              xml.tagE("extend");
                        */
#else
                        if (l->ticks().isNotZero())
                              _xml.tagE("extend");
#endif
                        _xml.etag();
                        }
                  }
            }
      }

//---------------------------------------------------------
//   directionJump -- write jump
//---------------------------------------------------------

// LVIFIX: TODO coda and segno should be numbered uniquely

static void directionJump(XmlWriter& xml, const Jump* const jp)
      {
      Jump::Type jtp = jp->jumpType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (jtp == Jump::Type::DC) {
            if (jp->xmlText() == "")
                  words = "D.C.";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DC_AL_FINE) {
            if (jp->xmlText() == "")
                  words = "D.C. al Fine";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DC_AL_CODA) {
            if (jp->xmlText() == "")
                  words = "D.C. al Coda";
            else
                  words = jp->xmlText();
            sound = "dacapo=\"yes\"";
            }
      else if (jtp == Jump::Type::DS_AL_CODA) {
            if (jp->xmlText() == "")
                  words = "D.S. al Coda";
            else
                  words = jp->xmlText();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == Jump::Type::DS_AL_FINE) {
            if (jp->xmlText() == "")
                  words = "D.S. al Fine";
            else
                  words = jp->xmlText();
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else if (jtp == Jump::Type::DS) {
            words = "D.S.";
            if (jp->jumpTo() == "")
                  sound = "dalsegno=\"1\"";
            else
                  sound = "dalsegno=\"" + jp->jumpTo() + "\"";
            }
      else
            qDebug("jump type=%d not implemented", static_cast<int>(jtp));

      if (sound != "") {
            xml.stag(QString("direction placement=\"%1\"").arg((jp->placement() == Placement::BELOW ) ? "below" : "above"));
            xml.stag("direction-type");
            QString positioning = "";
            positioning += addPositioningAttributes(jp);
            if (type != "") xml.tagE(type + positioning);
            if (words != "") xml.tag("words" + positioning, words);
            xml.etag();
            if (sound != "") xml.tagE(QString("sound ") + sound);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   directionMarker -- write marker
//---------------------------------------------------------

static void directionMarker(XmlWriter& xml, const Marker* const m)
      {
      Marker::Type mtp = m->markerType();
      QString words = "";
      QString type  = "";
      QString sound = "";
      if (mtp == Marker::Type::CODA) {
            type = "coda";
            if (m->label() == "")
                  sound = "coda=\"1\"";
            else
                  // LVIFIX hack: force label to "coda" to match to coda label
                  // sound = "coda=\"" + m->label() + "\"";
                  sound = "coda=\"coda\"";
            }
      else if (mtp == Marker::Type::SEGNO) {
            type = "segno";
            if (m->label() == "")
                  sound = "segno=\"1\"";
            else
                  sound = "segno=\"" + m->label() + "\"";
            }
      else if (mtp == Marker::Type::FINE) {
            words = "Fine";
            sound = "fine=\"yes\"";
            }
      else if (mtp == Marker::Type::TOCODA) {
            if (m->xmlText() == "")
                  words = "To Coda";
            else
                  words = m->xmlText();
            if (m->label() == "")
                  sound = "tocoda=\"1\"";
            else
                  sound = "tocoda=\"" + m->label() + "\"";
            }
      else
            qDebug("marker type=%d not implemented", int(mtp));

      if (sound != "") {
            xml.stag(QString("direction placement=\"%1\"").arg((m->placement() == Placement::BELOW ) ? "below" : "above"));
            xml.stag("direction-type");
            QString positioning = "";
            positioning += addPositioningAttributes(m);
            if (type != "") xml.tagE(type + positioning);
            if (words != "") xml.tag("words" + positioning, words);
            xml.etag();
            if (sound != "") xml.tagE(QString("sound ") + sound);
            xml.etag();
            }
      }

//---------------------------------------------------------
//  findTrackForAnnotations
//---------------------------------------------------------

// An annotation is attached to the staff, with track set
// to the lowest track in the staff. Find a track for it
// (the lowest track in this staff that has a chord or rest)

static int findTrackForAnnotations(int track, Segment* seg)
      {
      if (seg->segmentType() != SegmentType::ChordRest)
            return -1;

      int staff = track / VOICES;
      int strack = staff * VOICES;      // start track of staff containing track
      int etrack = strack + VOICES;     // end track of staff containing track + 1

      for (int i = strack; i < etrack; i++)
            if (seg->element(i))
                  return i;

      return -1;
      }

//---------------------------------------------------------
//  repeatAtMeasureStart -- write repeats at begin of measure
//---------------------------------------------------------

static void repeatAtMeasureStart(XmlWriter& xml, Attributes& attr, Measure* m, int strack, int etrack, int track)
      {
      // loop over all segments
      for (Element* e : m->el()) {
            int wtrack = -1; // track to write jump
            if (strack <= e->track() && e->track() < etrack)
                  wtrack = findTrackForAnnotations(e->track(), m->first(SegmentType::ChordRest));
            if (track != wtrack)
                  continue;
            switch (e->type()) {
                  case ElementType::MARKER:
                        {
                        // filter out the markers at measure Start
                        const Marker* const mk = static_cast<const Marker* const>(e);
                        Marker::Type mtp = mk->markerType();
                        if (   mtp == Marker::Type::SEGNO
                               || mtp == Marker::Type::CODA
                               ) {
                              qDebug(" -> handled");
                              attr.doAttr(xml, false);
                              directionMarker(xml, mk);
                              }
                        else if (   mtp == Marker::Type::FINE
                                    || mtp == Marker::Type::TOCODA
                                    ) {
                              // ignore
                              }
                        else {
                              qDebug("repeatAtMeasureStart: marker %d not implemented", int(mtp));
                              }
                        }
                        break;
                  default:
                        qDebug("repeatAtMeasureStart: direction type %s at tick %d not implemented",
                               Element::name(e->type()), m->tick().ticks());
                        break;
                  }
            }
      }

//---------------------------------------------------------
//  repeatAtMeasureStop -- write repeats at end of measure
//---------------------------------------------------------

static void repeatAtMeasureStop(XmlWriter& xml, Measure* m, int strack, int etrack, int track)
      {
      for (Element* e : m->el()) {
            int wtrack = -1; // track to write jump
            if (strack <= e->track() && e->track() < etrack)
                  wtrack = findTrackForAnnotations(e->track(), m->first(SegmentType::ChordRest));
            if (track != wtrack)
                  continue;
            switch (e->type()) {
                  case ElementType::MARKER:
                        {
                        // filter out the markers at measure stop
                        const Marker* const mk = static_cast<const Marker* const>(e);
                        Marker::Type mtp = mk->markerType();
                        if (mtp == Marker::Type::FINE || mtp == Marker::Type::TOCODA) {
                              directionMarker(xml, mk);
                              }
                        else if (mtp == Marker::Type::SEGNO || mtp == Marker::Type::CODA) {
                              // ignore
                              }
                        else {
                              qDebug("repeatAtMeasureStop: marker %d not implemented", int(mtp));
                              }
                        }
                        break;
                  case ElementType::JUMP:
                        directionJump(xml, static_cast<const Jump* const>(e));
                        break;
                  default:
                        qDebug("repeatAtMeasureStop: direction type %s at tick %d not implemented",
                               Element::name(e->type()), m->tick().ticks());
                        break;
                  }
            }
      }

//---------------------------------------------------------
//  work -- write the <work> element
//  note that order must be work-number, work-title
//  also write <movement-number> and <movement-title>
//  data is taken from the score metadata instead of the Text elements
//---------------------------------------------------------

void ExportMusicXml::work(const MeasureBase* /*measure*/)
      {
      QString workTitle  = _score->metaTag("workTitle");
      QString workNumber = _score->metaTag("workNumber");
      if (!(workTitle.isEmpty() && workNumber.isEmpty())) {
            _xml.stag("work");
            if (!workNumber.isEmpty())
                  _xml.tag("work-number", workNumber);
            if (!workTitle.isEmpty())
                  _xml.tag("work-title", workTitle);
            _xml.etag();
            }
      if (!_score->metaTag("movementNumber").isEmpty())
            _xml.tag("movement-number", _score->metaTag("movementNumber"));
      if (!_score->metaTag("movementTitle").isEmpty())
            _xml.tag("movement-title", _score->metaTag("movementTitle"));
      }

#if 0
//---------------------------------------------------------
//   elementRighter // used for harmony order
//---------------------------------------------------------

static bool elementRighter(const Element* e1, const Element* e2)
      {
      return e1->x() < e2->x();
      }
#endif

//---------------------------------------------------------
//  measureStyle -- write measure-style
//---------------------------------------------------------

// this is done at the first measure of a multi-meaure rest
// note: for a normal measure, mmRest1 is the measure itself,
// for a multi-meaure rest, it is the replacing measure

static void measureStyle(XmlWriter& xml, Attributes& attr, Measure* m)
      {
      const Measure* mmR1 = m->mmRest1();
      if (m != mmR1 && m == mmR1->mmRestFirst()) {
            attr.doAttr(xml, true);
            xml.stag("measure-style");
            xml.tag("multiple-rest", mmR1->mmRestCount());
            xml.etag();
            }
      }

//---------------------------------------------------------
//  findFretDiagram
//---------------------------------------------------------

static const FretDiagram* findFretDiagram(int strack, int etrack, int track, Segment* seg)
      {
      if (seg->segmentType() == SegmentType::ChordRest) {
            for (const Element* e : seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack && e->type() == ElementType::FRET_DIAGRAM)
                        return static_cast<const FretDiagram*>(e);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//  annotations
//---------------------------------------------------------

// In MuseScore, Element::FRET_DIAGRAM and Element::HARMONY are separate annotations,
// in MusicXML they are combined in the harmony element. This means they have to be matched.
// TODO: replace/repair current algorithm (which can only handle one FRET_DIAGRAM and one HARMONY)

static void annotations(ExportMusicXml* exp, XmlWriter&, int strack, int etrack, int track, int sstaff, Segment* seg)
      {
      if (seg->segmentType() == SegmentType::ChordRest) {

            const FretDiagram* fd = findFretDiagram(strack, etrack, track, seg);
            // if (fd) qDebug("annotations seg %p found fretboard diagram %p", seg, fd);

            for (const Element* e : seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        switch (e->type()) {
                              case ElementType::SYMBOL:
                                    exp->symbol(static_cast<const Symbol*>(e), sstaff);
                                    break;
                              case ElementType::TEMPO_TEXT:
                                    exp->tempoText(static_cast<const TempoText*>(e), sstaff);
                                    break;
                              case ElementType::STAFF_TEXT:
                              case ElementType::TEXT:
                              case ElementType::INSTRUMENT_CHANGE:
                                    exp->words(static_cast<const Text*>(e), sstaff);
                                    break;
                              case ElementType::DYNAMIC:
                                    exp->dynamic(static_cast<const Dynamic*>(e), sstaff);
                                    break;
                              case ElementType::HARMONY:
                                    // qDebug("annotations seg %p found harmony %p", seg, e);
                                    exp->harmony(static_cast<const Harmony*>(e), fd /*, sstaff */);
                                    fd = 0; // make sure to write only once ...
                                    break;
                              case ElementType::REHEARSAL_MARK:
                                    exp->rehearsal(static_cast<const RehearsalMark*>(e), sstaff);
                                    break;
                              case ElementType::FIGURED_BASS: // handled separately by figuredBass()
                              case ElementType::FRET_DIAGRAM: // handled using findFretDiagram()
                              case ElementType::JUMP:         // ignore
                                    break;
                              default:
                                    qDebug("annotations: direction type %s at tick %d not implemented",
                                           Element::name(e->type()), seg->tick().ticks());
                                    break;
                              }
                        }
                  } // for
            if (fd)
                  // found fd but no harmony, cannot write (MusicXML would be invalid)
                  qDebug("annotations seg %p found fretboard diagram %p w/o harmony: cannot write",
                         seg, fd);
            }
      }

//---------------------------------------------------------
//  figuredBass
//---------------------------------------------------------

static void figuredBass(XmlWriter& xml, int strack, int etrack, int track, const ChordRest* cr, FigBassMap& fbMap, int divisions)
      {
      Segment* seg = cr->segment();
      if (seg->segmentType() == SegmentType::ChordRest) {
            for (const Element* e : seg->annotations()) {

                  int wtrack = -1; // track to write annotation

                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        if (e->type() == ElementType::FIGURED_BASS) {
                              const FiguredBass* fb = dynamic_cast<const FiguredBass*>(e);
                              //qDebug("figuredbass() track %d seg %p fb %p seg %p tick %d ticks %d cr %p tick %d ticks %d",
                              //       track, seg, fb, fb->segment(), fb->segment()->tick(), fb->ticks(), cr, cr->tick(), cr->actualTicks());
                              bool extend = fb->ticks() > cr->actualTicks();
                              if (extend) {
                                    //qDebug("figuredbass() extend to %d + %d = %d",
                                    //       cr->tick(), fb->ticks(), cr->tick() + fb->ticks());
                                    fbMap.insert(strack, fb);
                                    }
                              else
                                    fbMap.remove(strack);
                              const Fraction crEndTick = cr->tick() + cr->actualTicks();
                              const Fraction fbEndTick = fb->segment()->tick() + fb->ticks();
                              const bool writeDuration = fb->ticks() < cr->actualTicks();
                              fb->writeMusicXML(xml, true, crEndTick.ticks(), fbEndTick.ticks(),
                                                writeDuration, divisions);

                              // Check for changing figures under a single note (each figure stored in a separate segment)
                              for (Segment* segNext = seg->next(); segNext && segNext->element(track) == NULL; segNext = segNext->next()) {
                                    for (Element* annot : segNext->annotations()) {
                                          if (annot->type() == ElementType::FIGURED_BASS && annot->track() == track) {
                                                fb = dynamic_cast<const FiguredBass*>(annot);
                                                fb->writeMusicXML(xml, true, 0, 0, true, divisions);
                                                }
                                          }
                                    }
                              // no extend can be pending
                              return;
                              }
                        }
                  }
            // check for extend pending
            if (fbMap.contains(strack)) {
                  const FiguredBass* fb = fbMap.value(strack);
                  Fraction crEndTick = cr->tick() + cr->actualTicks();
                  Fraction fbEndTick = fb->segment()->tick() + fb->ticks();
                  bool writeDuration = fb->ticks() < cr->actualTicks();
                  if (cr->tick() < fbEndTick) {
                        //qDebug("figuredbass() at tick %d extend only", cr->tick());
                        fb->writeMusicXML(xml, false, crEndTick.ticks(), fbEndTick.ticks(), writeDuration, divisions);
                        }
                  if (fbEndTick <= crEndTick) {
                        //qDebug("figuredbass() at tick %d extend done", cr->tick() + cr->actualTicks());
                        fbMap.remove(strack);
                        }
                  }
            }
      }

//---------------------------------------------------------
//  spannerStart
//---------------------------------------------------------

// for each spanner start:
// find start track
// find stop track
// if stop track < start track
//   get data from list of already stopped spanners
// else
//   calculate data
// write start if in right track

static void spannerStart(ExportMusicXml* exp, int strack, int etrack, int track, int sstaff, Segment* seg)
      {
      if (seg->segmentType() == SegmentType::ChordRest) {
            Fraction stick = seg->tick();
            for (auto it = exp->score()->spanner().lower_bound(stick.ticks()); it != exp->score()->spanner().upper_bound(stick.ticks()); ++it) {
                  Spanner* e = it->second;

                  int wtrack = -1; // track to write spanner
                  if (strack <= e->track() && e->track() < etrack)
                        wtrack = findTrackForAnnotations(e->track(), seg);

                  if (track == wtrack) {
                        switch (e->type()) {
                              case ElementType::HAIRPIN:
                                    exp->hairpin(static_cast<const Hairpin*>(e), sstaff, seg->tick());
                                    break;
                              case ElementType::OTTAVA:
                                    exp->ottava(static_cast<const Ottava*>(e), sstaff, seg->tick());
                                    break;
                              case ElementType::PEDAL:
                                    exp->pedal(static_cast<const Pedal*>(e), sstaff, seg->tick());
                                    break;
                              case ElementType::TEXTLINE:
                                    exp->textLine(static_cast<const TextLine*>(e), sstaff, seg->tick());
                                    break;
                              case ElementType::TRILL:
                                    // ignore (written as <note><notations><ornaments><wavy-line>)
                                    break;
                              case ElementType::SLUR:
                                    // ignore (written as <note><notations><slur>)
                                    break;
                              default:
                                    qDebug("spannerStart: direction type %s at tick %d not implemented",
                                           Element::name(e->type()), seg->tick().ticks());
                                    break;
                              }
                        }
                  } // for
            }
      }

//---------------------------------------------------------
//  spannerStop
//---------------------------------------------------------

// called after writing each chord or rest to check if a spanner must be stopped
// loop over all spanners and find spanners in strack ending at tick2
// note that more than one voice may contains notes ending at tick2,
// remember which spanners have already been stopped (the "stopped" set)

static void spannerStop(ExportMusicXml* exp, int strack, const Fraction& tick2, int sstaff, QSet<const Spanner*>& stopped)
      {
      for (auto it : exp->score()->spanner()) {
            Spanner* e = it.second;

            if (e->tick2() != tick2 || e->track() != strack)
                  continue;

            if (!stopped.contains(e)) {
                  stopped.insert(e);
                  switch (e->type()) {
                        case ElementType::HAIRPIN:
                              exp->hairpin(static_cast<const Hairpin*>(e), sstaff, Fraction(-1,1));
                              break;
                        case ElementType::OTTAVA:
                              exp->ottava(static_cast<const Ottava*>(e), sstaff, Fraction(-1,1));
                              break;
                        case ElementType::PEDAL:
                              exp->pedal(static_cast<const Pedal*>(e), sstaff, Fraction(-1,1));
                              break;
                        case ElementType::TEXTLINE:
                              exp->textLine(static_cast<const TextLine*>(e), sstaff, Fraction(-1,1));
                              break;
                        case ElementType::TRILL:
                              // ignore (written as <note><notations><ornaments><wavy-line>
                              break;
                        case ElementType::SLUR:
                              // ignore (written as <note><notations><slur>)
                              break;
                        default:
                              qDebug("spannerStop: direction type %s at tick2 %d not implemented",
                                     Element::name(e->type()), tick2.ticks());
                              break;
                        }
                  }
            } // for
      }

//---------------------------------------------------------
//  keysigTimesig
//---------------------------------------------------------

/**
 Output attributes at start of measure: key, time
 */

void ExportMusicXml::keysigTimesig(const Measure* m, const Part* p)
      {
      int strack = p->startTrack();
      int etrack = p->endTrack();
      //qDebug("keysigTimesig m %p strack %d etrack %d", m, strack, etrack);

      // search all staves for non-generated key signatures
      QMap<int, KeySig*> keysigs; // map staff to key signature
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->tick() > m->tick())
                  break;
            for (int t = strack; t < etrack; t += VOICES) {
                  Element* el = seg->element(t);
                  if (!el)
                        continue;
                  if (el->type() == ElementType::KEYSIG) {
                        //qDebug(" found keysig %p track %d", el, el->track());
                        int st = (t - strack) / VOICES;
                        if (!el->generated())
                              keysigs[st] = static_cast<KeySig*>(el);
                        }
                  }
            }

      //ClefType ct = rest->staff()->clef(rest->tick());

      // write the key signatues
      if (!keysigs.isEmpty()) {
            // determine if all staves have a keysig and all keysigs are identical
            // in that case a single <key> is written, without number=... attribute
            int nstaves = p->nstaves();
            bool singleKey = true;
            // check if all staves have a keysig
            for (int i = 0; i < nstaves; i++)
                  if (!keysigs.contains(i))
                        singleKey = false;
            // check if all keysigs are identical
            if (singleKey)
                  for (int i = 1; i < nstaves; i++)
                        if (!(keysigs.value(i)->key() == keysigs.value(0)->key()))
                              singleKey = false;

            // write the keysigs
            //qDebug(" singleKey %d", singleKey);
            if (singleKey) {
                  // keysig applies to all staves
                  keysig(keysigs.value(0), p->staff(0)->clef(m->tick()), 0, keysigs.value(0)->visible());
                  }
            else {
                  // staff-specific keysigs
                  for (int st : keysigs.keys())
                        keysig(keysigs.value(st), p->staff(st)->clef(m->tick()), st + 1, keysigs.value(st)->visible());
                  }
            }
      else {
            // always write a keysig at tick = 0
            if (m->tick().isZero()) {
                  //KeySigEvent kse;
                  //kse.setKey(Key::C);
                  KeySig* ks = new KeySig(_score);
                  ks->setKey(Key::C);
                  keysig(ks, p->staff(0)->clef(m->tick()));
                  delete ks;
                  }
            }

      TimeSig* tsig = 0;
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            if (seg->tick() > m->tick())
                  break;
            Element* el = seg->element(strack);
            if (el && el->type() == ElementType::TIMESIG)
                  tsig = (TimeSig*) el;
            }
      if (tsig)
            timesig(tsig);
      }

//---------------------------------------------------------
//  identification -- write the identification
//---------------------------------------------------------

static void identification(XmlWriter& xml, Score const* const score)
      {
      xml.stag("identification");

      QStringList creators;
      // the creator types commonly found in MusicXML
      creators << "arranger" << "composer" << "lyricist" << "poet" << "translator";
      for (QString type : creators) {
            QString creator = score->metaTag(type);
            if (!creator.isEmpty())
                  xml.tag(QString("creator type=\"%1\"").arg(type), creator);
            }

      if (!score->metaTag("copyright").isEmpty())
            xml.tag("rights", score->metaTag("copyright"));

      xml.stag("encoding");

      if (MScore::debugMode) {
            xml.tag("software", QString("MuseScore 0.7.0"));
            xml.tag("encoding-date", QString("2007-09-10"));
            }
      else {
            xml.tag("software", QString("MuseScore ") + QString(VERSION));
            xml.tag("encoding-date", QDate::currentDate().toString(Qt::ISODate));
            }

      // specify supported elements
      xml.tagE("supports element=\"accidental\" type=\"yes\"");
      xml.tagE("supports element=\"beam\" type=\"yes\"");
      // set support for print new-page and new-system to match user preference
      // for MusicxmlExportBreaks::MANUAL support is "no" because "yes" breaks Finale NotePad import
      if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)
          && preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::ALL) {
            xml.tagE("supports element=\"print\" attribute=\"new-page\" type=\"yes\" value=\"yes\"");
            xml.tagE("supports element=\"print\" attribute=\"new-system\" type=\"yes\" value=\"yes\"");
            }
      else {
            xml.tagE("supports element=\"print\" attribute=\"new-page\" type=\"no\"");
            xml.tagE("supports element=\"print\" attribute=\"new-system\" type=\"no\"");
            }
      xml.tagE("supports element=\"stem\" type=\"yes\"");

      xml.etag();

      if (!score->metaTag("source").isEmpty())
            xml.tag("source", score->metaTag("source"));

      xml.etag();
      }

//---------------------------------------------------------
//  findPartGroupNumber
//---------------------------------------------------------

static int findPartGroupNumber(int* partGroupEnd)
      {
      // find part group number
      for (int number = 0; number < MAX_PART_GROUPS; ++number)
            if (partGroupEnd[number] == -1)
                  return number;
      qDebug("no free part group number");
      return MAX_PART_GROUPS;
      }

//---------------------------------------------------------
//  scoreInstrument
//---------------------------------------------------------

static void scoreInstrument(XmlWriter& xml, const int partNr, const int instrNr, const QString& instrName)
      {
      xml.stag(QString("score-instrument %1").arg(instrId(partNr, instrNr)));
      xml.tag("instrument-name", instrName);
      xml.etag();
      }

//---------------------------------------------------------
//  midiInstrument
//---------------------------------------------------------

static void midiInstrument(XmlWriter& xml, const int partNr, const int instrNr,
                           const Instrument* instr, const Score* score, const int unpitched = 0)
      {
      xml.stag(QString("midi-instrument %1").arg(instrId(partNr, instrNr)));
      int midiChannel = score->masterScore()->midiChannel(instr->channel(0)->channel());
      if (midiChannel >= 0 && midiChannel < 16)
            xml.tag("midi-channel", midiChannel + 1);
      int midiProgram = instr->channel(0)->program();
      if (midiProgram >= 0 && midiProgram < 128)
            xml.tag("midi-program", midiProgram + 1);
      if (unpitched > 0)
            xml.tag("midi-unpitched", unpitched);
      xml.tag("volume", (instr->channel(0)->volume() / 127.0) * 100);  //percent
      xml.tag("pan", int(((instr->channel(0)->pan() - 63.5) / 63.5) * 90)); //-90 hard left, +90 hard right      xml.etag();
      xml.etag();
      }

//---------------------------------------------------------
//  initInstrMap
//---------------------------------------------------------

/**
 Initialize the Instrument* to number map for a Part
 Used to generate instrument numbers for a multi-instrument part
 */

static void initInstrMap(MxmlInstrumentMap& im, const InstrumentList* il, const Score* /*score*/)
      {
      im.clear();
      for (auto i = il->begin(); i != il->end(); ++i) {
            const Instrument* pinstr = i->second;
            if (!im.contains(pinstr))
                  im.insert(pinstr, im.size());
            }
      }

//---------------------------------------------------------
//  initReverseInstrMap
//---------------------------------------------------------

typedef QMap<int, const Instrument*> MxmlReverseInstrumentMap;

/**
 Initialize the number t Instrument* map for a Part
 Used to iterate in sequence over instrument numbers for a multi-instrument part
 */

static void initReverseInstrMap(MxmlReverseInstrumentMap& rim, const MxmlInstrumentMap& im)
      {
      rim.clear();
      for (const Instrument* i : im.keys()) {
            int instNr = im.value(i);
            rim.insert(instNr, i);
            }
      }

//---------------------------------------------------------
//  print
//---------------------------------------------------------

/**
 Handle the <print> element.
 When exporting layout and all breaks, a <print> with layout information
 is generated for the measure types TopSystem, NewSystem and newPage.
 When exporting layout but only manual or no breaks, a <print> with
 layout information is generated only for the measure type TopSystem,
 as it is assumed the system layout is broken by the importing application
 anyway and is thus useless.
 */

void ExportMusicXml::print(const Measure* const m, const int partNr, const int firstStaffOfPart, const int nrStavesInPart)
      {
      int currentSystem = NoSystem;
      Measure* previousMeasure = 0;

      for (MeasureBase* currentMeasureB = m->prev(); currentMeasureB; currentMeasureB = currentMeasureB->prev()) {
            if (currentMeasureB->type() == ElementType::MEASURE) {
                  previousMeasure = (Measure*) currentMeasureB;
                  break;
                  }
            }

      if (!previousMeasure)
            currentSystem = TopSystem;
      else {
            const auto mSystem = m->mmRest1()->system();
            const auto previousMeasureSystem = previousMeasure->mmRest1()->system();

            if (mSystem && previousMeasureSystem) {
                  if (mSystem->page() != previousMeasureSystem->page())
                        currentSystem = NewPage;
                  else if (mSystem != previousMeasureSystem)
                        currentSystem = NewSystem;
                  }
            }

      bool prevMeasLineBreak = false;
      bool prevMeasPageBreak = false;
      if (previousMeasure) {
            prevMeasLineBreak = previousMeasure->lineBreak();
            prevMeasPageBreak = previousMeasure->pageBreak();
            }

      if (currentSystem != NoSystem) {

            // determine if a new-system or new-page is required
            QString newThing;       // new-[system|page]="yes" or empty
            if (preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::ALL) {
                  if (currentSystem == NewSystem)
                        newThing = " new-system=\"yes\"";
                  else if (currentSystem == NewPage)
                        newThing = " new-page=\"yes\"";
                  }
            else if (preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::MANUAL) {
                  if (currentSystem == NewSystem && prevMeasLineBreak)
                        newThing = " new-system=\"yes\"";
                  else if (currentSystem == NewPage && prevMeasPageBreak)
                        newThing = " new-page=\"yes\"";
                  }

            // determine if layout information is required
            bool doLayout = false;
            if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)) {
                  if (currentSystem == TopSystem
                      || (preferences.musicxmlExportBreaks() == MusicxmlExportBreaks::ALL && newThing != "")) {
                        doLayout = true;
                        }
                  }

            if (doLayout) {
                  _xml.stag(QString("print%1").arg(newThing));
                  const double pageWidth  = getTenthsFromInches(score()->styleD(Sid::pageWidth));
                  const double lm = getTenthsFromInches(score()->styleD(Sid::pageOddLeftMargin));
                  const double rm = getTenthsFromInches(score()->styleD(Sid::pageWidth)
                                                        - score()->styleD(Sid::pagePrintableWidth) - score()->styleD(Sid::pageOddLeftMargin));
                  const double tm = getTenthsFromInches(score()->styleD(Sid::pageOddTopMargin));

                  // System Layout

                  // For a multi-meaure rest positioning is valid only
                  // in the replacing measure
                  // note: for a normal measure, mmRest1 is the measure itself,
                  // for a multi-meaure rest, it is the replacing measure
                  const Measure* mmR1 = m->mmRest1();
                  const System* system = mmR1->system();

                  // Put the system print suggestions only for the first part in a score...
                  if (partNr == 0) {

                        // Find the right margin of the system.
                        double systemLM = getTenthsFromDots(mmR1->pagePos().x() - system->page()->pagePos().x()) - lm;
                        double systemRM = pageWidth - rm - (getTenthsFromDots(system->bbox().width()) + lm);

                        _xml.stag("system-layout");
                        _xml.stag("system-margins");
                        _xml.tag("left-margin", QString("%1").arg(QString::number(systemLM,'f',2)));
                        _xml.tag("right-margin", QString("%1").arg(QString::number(systemRM,'f',2)) );
                        _xml.etag();

                        if (currentSystem == NewPage || currentSystem == TopSystem) {
                              const double topSysDist = getTenthsFromDots(mmR1->pagePos().y()) - tm;
                              _xml.tag("top-system-distance", QString("%1").arg(QString::number(topSysDist,'f',2)) );
                              }
                        if (currentSystem == NewSystem) {
                              // see System::layout2() for the factor 2 * score()->spatium()
                              const double sysDist = getTenthsFromDots(mmR1->pagePos().y()
                                                                       - previousMeasure->pagePos().y()
                                                                       - previousMeasure->bbox().height()
                                                                       + 2 * score()->spatium()
                                                                       );
                              _xml.tag("system-distance",
                                       QString("%1").arg(QString::number(sysDist,'f',2)));
                              }

                        _xml.etag();
                        }

                  // Staff layout elements.
                  for (int staffIdx = (firstStaffOfPart == 0) ? 1 : 0; staffIdx < nrStavesInPart; staffIdx++) {

                        // calculate distance between this and previous staff using the bounding boxes
                        const auto staffNr = firstStaffOfPart + staffIdx;
                        const auto prevBbox = system->staff(staffNr - 1)->bbox();
                        const auto staffDist = system->staff(staffNr)->bbox().y() - prevBbox.y() - prevBbox.height();

                        _xml.stag(QString("staff-layout number=\"%1\"").arg(staffIdx + 1));
                        _xml.tag("staff-distance", QString("%1").arg(QString::number(getTenthsFromDots(staffDist),'f',2)));
                        _xml.etag();
                        }

                  _xml.etag();
                  }
            else {
                  // !doLayout
                  if (newThing != "")
                        _xml.tagE(QString("print%1").arg(newThing));
                  }

            } // if (currentSystem ...

      }

//---------------------------------------------------------
//  exportDefaultClef
//---------------------------------------------------------

/**
 In case no clef is found, export a default clef with type determined by staff type.
 Note that a multi-measure rest starting in the first measure should be handled correctly.
 */

void ExportMusicXml::exportDefaultClef(const Part* const part, const Measure* const m)
      {
      const auto staves = part->nstaves();

      if (m->tick() == Fraction(0,1)) {
            const auto clefSeg = m->findSegment(SegmentType::HeaderClef, Fraction(0,1));

            if (clefSeg) {
                  for (int i = 0; i < staves; ++i) {

                        // sstaff - xml staff number, counting from 1 for this
                        // instrument
                        // special number 0 -> don’t show staff number in
                        // xml output (because there is only one staff)

                        auto sstaff = (staves > 1) ? i + 1 : 0;
                        auto track = part->startTrack() + VOICES * i;

                        if (clefSeg->element(track) == nullptr) {
                              ClefType ct { ClefType::G };
                              QString stafftype;
                              switch (part->staff(i)->staffType(Fraction(0,1))->group()) {
                                    case StaffGroup::TAB:
                                          ct = ClefType::TAB;
                                          stafftype = "tab";
                                          break;
                                    case StaffGroup::STANDARD:
                                          ct = ClefType::G;
                                          stafftype = "std";
                                          break;
                                    case StaffGroup::PERCUSSION:
                                          ct = ClefType::PERC;
                                          stafftype = "perc";
                                          break;
                                    }
                              qDebug("no clef found in first measure track %d (stafftype %s)", track, qPrintable(stafftype));
                              clef(sstaff, ct, " print-object=\"no\"");
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//  findAndExportClef
//---------------------------------------------------------

/**
 Make sure clefs at end of measure get exported at start of next measure.
 */

void ExportMusicXml::findAndExportClef(Measure* m, const int staves, const int strack, const int etrack)
      {
      Measure* prevMeasure = m->prevMeasure();
      Measure* mmR         = m->mmRest();       // the replacing measure in a multi-measure rest
      Fraction tick        = m->tick();
      Segment* cs1;
      Segment* cs2         = m->findSegment(SegmentType::Clef, tick);
      Segment* cs3;
      Segment* seg         = 0;

      if (prevMeasure)
            cs1 = prevMeasure->findSegment(SegmentType::Clef, tick);
      else
            cs1 = m->findSegment(SegmentType::HeaderClef, tick);

      if (mmR) {
            cs3 = mmR->findSegment(SegmentType::HeaderClef, tick);
            if (!cs3)
                  cs3 = mmR->findSegment(SegmentType::Clef, tick);
            }
      else
            cs3 = 0;

      if (cs1 && cs2) {
            // should only happen at begin of new system
            // when previous system ends with a non-generated clef
            seg = cs1;
            }
      else if (cs1)
            seg = cs1;
      else if (cs3) {
            // happens when the first measure is a multi-measure rest
            // containing a generated clef
            seg = cs3;
            }
      else
            seg = cs2;
      clefDebug("exportxml: clef segments cs1=%p cs2=%p cs3=%p seg=%p", cs1, cs2, cs3, seg);

      // output attribute at start of measure: clef
      if (seg) {
            for (int st = strack; st < etrack; st += VOICES) {
                  // sstaff - xml staff number, counting from 1 for this
                  // instrument
                  // special number 0 -> don’t show staff number in
                  // xml output (because there is only one staff)

                  int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                  sstaff /= VOICES;

                  Clef* cle = static_cast<Clef*>(seg->element(st));
                  if (cle) {
                        clefDebug("exportxml: clef at start measure ti=%d ct=%d gen=%d", tick, int(cle->clefType()), cle->generated());
                        // output only clef changes, not generated clefs at line beginning
                        // exception: at tick=0, export clef anyway
                        if (tick.isZero() || !cle->generated()) {
                              clefDebug("exportxml: clef exported");
                              clef(sstaff, cle->clefType(), color2xml(cle));
                              }
                        else {
                              clefDebug("exportxml: clef not exported");
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//  findPitchesUsed
//---------------------------------------------------------

/**
 Find the set of pitches actually used in a part.
 */

typedef QSet<int> pitchSet;       // the set of pitches used

static void addChordPitchesToSet(const Chord* c, pitchSet& set)
      {
      for (const Note* note : c->notes()) {
            qDebug("chord %p note %p pitch %d", c, note, note->pitch() + 1);
            set.insert(note->pitch());
            }
      }

static void findPitchesUsed(const Part* part, pitchSet& set)
      {
      int strack = part->startTrack();
      int etrack = part->endTrack();

      // loop over all chords in the part
      for (const MeasureBase* mb = part->score()->measures()->first(); mb; mb = mb->next()) {
            if (mb->type() != ElementType::MEASURE)
                  continue;
            const Measure* m = static_cast<const Measure*>(mb);
            for (int st = strack; st < etrack; ++st) {
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        const Element* el = seg->element(st);
                        if (!el)
                              continue;
                        if (el->type() == ElementType::CHORD)
                              {
                              // add grace and non-grace note pitches to the result set
                              const Chord* c = static_cast<const Chord*>(el);
                              if (c) {
                                    for (const Chord* g : c->graceNotesBefore()) {
                                          addChordPitchesToSet(g, set);
                                          }
                                    addChordPitchesToSet(c, set);
                                    for (const Chord* g : c->graceNotesAfter()) {
                                          addChordPitchesToSet(g, set);
                                          }
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//  partList
//---------------------------------------------------------

/**
 Write the part list to \a xml.
 */

static void partList(XmlWriter& xml, Score* score, const QList<Part*>& il, MxmlInstrumentMap& instrMap)
      {
      xml.stag("part-list");
      int staffCount = 0;                             // count sum of # staves in parts
      int partGroupEnd[MAX_PART_GROUPS];              // staff where part group ends (bracketSpan is in staves, not parts)
      for (int i = 0; i < MAX_PART_GROUPS; i++)
            partGroupEnd[i] = -1;
      for (int idx = 0; idx < il.size(); ++idx) {
            Part* part = il.at(idx);
            bool bracketFound = false;
            // handle brackets
            for (int i = 0; i < part->nstaves(); i++) {
                  Staff* st = part->staff(i);
                  if (st) {
                        for (int j = 0; j < st->bracketLevels() + 1; j++) {
                              if (st->bracketType(j) != BracketType::NO_BRACKET) {
                                    bracketFound = true;
                                    if (i == 0) {
                                          // OK, found bracket in first staff of part
                                          // filter out implicit brackets
                                          if (!(st->bracketSpan(j) == part->nstaves()
                                                && st->bracketType(j) == BracketType::BRACE)) {
                                                // add others
                                                int number = findPartGroupNumber(partGroupEnd);
                                                if (number < MAX_PART_GROUPS) {
                                                      partGroupStart(xml, number + 1, st->bracketType(j));
                                                      partGroupEnd[number] = staffCount + st->bracketSpan(j);
                                                      }
                                                }
                                          }
                                    else {
                                          // bracket in other staff not supported in MusicXML
                                          qDebug("bracket starting in staff %d not supported", i + 1);
                                          }
                                    }
                              }
                        }
                  }
            // handle bracket none
            if (!bracketFound && part->nstaves() > 1) {
                  int number = findPartGroupNumber(partGroupEnd);
                  if (number < MAX_PART_GROUPS) {
                        partGroupStart(xml, number + 1, BracketType::NO_BRACKET);
                        partGroupEnd[number] = idx + part->nstaves();
                        }
                  }

            xml.stag(QString("score-part id=\"P%1\"").arg(idx+1));
            initInstrMap(instrMap, part->instruments(), score);
            // by default export the parts long name as part-name
            if (part->longName() != "")
                  xml.tag("part-name", MScoreTextToMXML::toPlainText(part->longName()));
            else {
                  if (part->partName() != "") {
                        // use the track name if no part long name
                        // to prevent an empty track name on import
                        xml.tag("part-name print-object=\"no\"", MScoreTextToMXML::toPlainText(part->partName()));
                        }
                  else
                        // part-name is required
                        xml.tag("part-name", "");
                  }
            if (!part->shortName().isEmpty())
                  xml.tag("part-abbreviation", MScoreTextToMXML::toPlainText(part->shortName()));

            if (part->instrument()->useDrumset()) {
                  const Drumset* drumset = part->instrument()->drumset();
                  pitchSet pitches;
                  findPitchesUsed(part, pitches);
                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead != NoteHead::Group::HEAD_INVALID)
                              scoreInstrument(xml, idx + 1, i + 1, di.name);
                        else if (pitches.contains(i))
                              scoreInstrument(xml, idx + 1, i + 1, QString("Instrument %1").arg(i + 1));
                        }
                  int midiPort = part->midiPort() + 1;
                  if (midiPort >= 1 && midiPort <= 16)
                        xml.tag(QString("midi-device port=\"%1\"").arg(midiPort), "");

                  for (int i = 0; i < 128; ++i) {
                        DrumInstrument di = drumset->drum(i);
                        if (di.notehead != NoteHead::Group::HEAD_INVALID || pitches.contains(i))
                              midiInstrument(xml, idx + 1, i + 1, part->instrument(), score, i + 1);
                        }
                  }
            else {
                  MxmlReverseInstrumentMap rim;
                  initReverseInstrMap(rim, instrMap);
                  for (int instNr : rim.keys()) {
                        scoreInstrument(xml, idx + 1, instNr + 1, MScoreTextToMXML::toPlainText(rim.value(instNr)->trackName()));
                        }
                  for (auto ii = rim.constBegin(); ii != rim.constEnd(); ii++) {
                        int instNr = ii.key();
                        int midiPort = part->midiPort() + 1;
                        if (ii.value()->channel().size() > 0)
                              midiPort = score->masterScore()->midiMapping(ii.value()->channel(0)->channel())->port() + 1;
                        if (midiPort >= 1 && midiPort <= 16)
                              xml.tag(QString("midi-device %1 port=\"%2\"").arg(instrId(idx+1, instNr + 1)).arg(midiPort), "");
                        else
                              xml.tag(QString("midi-device %1").arg(instrId(idx+1, instNr + 1)), "");
                        midiInstrument(xml, idx + 1, instNr + 1, rim.value(instNr), score);
                        }
                  }

            xml.etag();
            staffCount += part->nstaves();
            for (int i = MAX_PART_GROUPS - 1; i >= 0; i--) {
                  int end = partGroupEnd[i];
                  if (end >= 0) {
                        if (staffCount >= end) {
                              xml.tagE(QString("part-group type=\"stop\" number=\"%1\"").arg(i + 1));
                              partGroupEnd[i] = -1;
                              }
                        }
                  }
            }
      xml.etag();

      }

//---------------------------------------------------------
//  writeElement
//---------------------------------------------------------

/**
 Write \a el.
 */

void ExportMusicXml::writeElement(Element* el, const Measure* m, int sstaff, bool useDrumset)
      {
      switch (el->type()) {

            case ElementType::CLEF:
                  {
                  // output only clef changes, not generated clefs
                  // at line beginning
                  // also ignore clefs at the start of a measure,
                  // these have already been output
                  // also ignore clefs at the end of a measure
                  // these will be output at the start of the next measure
                  const Clef* cle = static_cast<const Clef*>(el);
                  Fraction ti = cle->segment()->tick();
                  clefDebug("exportxml: clef in measure ti=%d ct=%d gen=%d", ti, int(cle->clefType()), el->generated());
                  if (el->generated()) {
                        clefDebug("exportxml: generated clef not exported");
                        break;
                        }
                  if (!el->generated() && ti != m->tick() && ti != m->endTick())
                        clef(sstaff, cle->clefType(), color2xml(cle));
                  else {
                        clefDebug("exportxml: clef not exported");
                        }
                  }
                  break;

            case ElementType::KEYSIG:
                  // ignore
                  break;

            case ElementType::TIMESIG:
                  // ignore
                  break;

            case ElementType::CHORD:
                  {
                  Chord* c = static_cast<Chord*>(el);
                  const auto ll = &c->lyrics();
                  // ise grace after
                  if (c) {
                        for (Chord* g : c->graceNotesBefore()) {
                              chord(g, sstaff, ll, useDrumset);
                              }
                        chord(c, sstaff, ll, useDrumset);
                        for (Chord* g : c->graceNotesAfter()) {
                              chord(g, sstaff, ll, useDrumset);
                              }
                        }
                  break;
                  }
            case ElementType::REST:
                  rest((Rest*)el, sstaff);
                  break;

            case ElementType::BAR_LINE:
                  // Following must be enforced (ref MusicXML barline.dtd):
                  // If location is left, it should be the first element in the measure;
                  // if location is right, it should be the last element.
                  // implementation note: BarLineType::START_REPEAT already written by barlineLeft()
                  // any bars left should be "middle"
                  // TODO: print barline only if middle
                  // if (el->subtype() != BarLineType::START_REPEAT)
                  //       bar((BarLine*) el);
                  break;
            case ElementType::BREATH:
                  // ignore, already exported as note articulation
                  break;

            default:
                  qDebug("ExportMusicXml::write unknown segment type %s", el->name());
                  break;
            }

      }

//---------------------------------------------------------
//  writeStaffDetails
//---------------------------------------------------------

/**
 Write the staff details for \a part to \a xml.
 */

static void writeStaffDetails(XmlWriter& xml, const Part* part)
      {
      const Instrument* instrument = part->instrument();
      int staves = part->nstaves();

      // staff details
      // TODO: decide how to handle linked regular / TAB staff
      //       currently exported as a two staff part ...
      for (int i = 0; i < staves; i++) {
            Staff* st = part->staff(i);
            if (st->lines(Fraction(0,1)) != 5 || st->isTabStaff(Fraction(0,1))) {
                  if (staves > 1)
                        xml.stag(QString("staff-details number=\"%1\"").arg(i+1));
                  else
                        xml.stag("staff-details");
                  xml.tag("staff-lines", st->lines(Fraction(0,1)));
                  if (st->isTabStaff(Fraction(0,1)) && instrument->stringData()) {
                        QList<instrString> l = instrument->stringData()->stringList();
                        for (int ii = 0; ii < l.size(); ii++) {
                              char step  = ' ';
                              int alter  = 0;
                              int octave = 0;
                              midipitch2xml(l.at(ii).pitch, step, alter, octave);
                              xml.stag(QString("staff-tuning line=\"%1\"").arg(ii+1));
                              xml.tag("tuning-step", QString("%1").arg(step));
                              if (alter)
                                    xml.tag("tuning-alter", alter);
                              xml.tag("tuning-octave", octave);
                              xml.etag();
                              }
                        }
                  xml.etag();
                  }
            }
      }

//---------------------------------------------------------
//  writeInstrumentDetails
//---------------------------------------------------------

/**
 Write the instrument details for \a part to \a xml.
 */

static void writeInstrumentDetails(XmlWriter& xml, const Part* part)
      {
      const Instrument* instrument = part->instrument();

      // instrument details
      if (instrument->transpose().chromatic) {        // TODO: tick
            xml.stag("transpose");
            xml.tag("diatonic",  instrument->transpose().diatonic % 7);
            xml.tag("chromatic", instrument->transpose().chromatic % 12);
            int octaveChange = instrument->transpose().chromatic / 12;
            if (octaveChange != 0)
                  xml.tag("octave-change", octaveChange);
            xml.etag();
            }
      }

//---------------------------------------------------------
//  write
//---------------------------------------------------------

/**
 Write the score to \a dev in MusicXML format.
 */

void ExportMusicXml::write(QIODevice* dev)
      {
      // must export in transposed pitch to prevent
      // losing the transposition information
      // if necessary, switch concert pitch mode off
      // before export and restore it after export
      bool concertPitch = score()->styleB(Sid::concertPitch);
      if (concertPitch) {
            score()->startCmd();
            score()->undo(new ChangeStyleVal(score(), Sid::concertPitch, false));
            score()->doLayout();    // this is only allowed in a cmd context to not corrupt the undo/redo stack
            }

      calcDivisions();

      for (int i = 0; i < MAX_NUMBER_LEVEL; ++i) {
            brackets[i] = nullptr;
            dashes[i] = nullptr;
            hairpins[i] = nullptr;
            ottavas[i] = nullptr;
            trills[i] = nullptr;
            }

      _xml.setDevice(dev);
      _xml.setCodec("UTF-8");
      _xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      _xml << "<!DOCTYPE score-partwise PUBLIC \"-//Recordare//DTD MusicXML 3.1 Partwise//EN\" \"http://www.musicxml.org/dtds/partwise.dtd\">\n";
      _xml.stag("score-partwise version=\"3.1\"");

      const MeasureBase* measure = _score->measures()->first();
      work(measure);

      identification(_xml, _score);

      if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT)) {
            defaults(_xml, _score, millimeters, tenths);
            credits(_xml);
            }

      const QList<Part*>& il = _score->parts();
      partList(_xml, _score, il, instrMap);

      int staffCount = 0;

      for (int idx = 0; idx < il.size(); ++idx) {
            Part* part = il.at(idx);
            _tick = { 0,1 };
            _xml.stag(QString("part id=\"P%1\"").arg(idx+1));

            int staves = part->nstaves();
            int strack = part->startTrack();
            int etrack = part->endTrack();

            _trillStart.clear();
            _trillStop.clear();
            initInstrMap(instrMap, part->instruments(), _score);

            int measureNo = 1;          // number of next regular measure
            int irregularMeasureNo = 1; // number of next irregular measure
            int pickupMeasureNo = 1;    // number of next pickup measure

            FigBassMap fbMap;           // pending figured bass extends

            for (MeasureBase* mb = _score->measures()->first(); mb; mb = mb->next()) {
                  if (mb->type() != ElementType::MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);


                  // pickup and other irregular measures need special care
                  QString measureTag = "measure number=";
                  if ((irregularMeasureNo + measureNo) == 2 && m->irregular()) {
                        measureTag += "\"0\" implicit=\"yes\"";
                        pickupMeasureNo++;
                        }
                  else if (m->irregular())
                        measureTag += QString("\"X%1\" implicit=\"yes\"").arg(irregularMeasureNo++);
                  else
                        measureTag += QString("\"%1\"").arg(measureNo++);
                  const bool isFirstActualMeasure = (irregularMeasureNo + measureNo + pickupMeasureNo) == 4;

                  if (preferences.getBool(PREF_EXPORT_MUSICXML_EXPORTLAYOUT))
                        measureTag += QString(" width=\"%1\"").arg(QString::number(m->bbox().width() / DPMM / millimeters * tenths,'f',2));

                  _xml.stag(measureTag);

                  print(m, idx, staffCount, staves);

                  _attr.start();

                  findTrills(m, strack, etrack, _trillStart, _trillStop);

                  // barline left must be the first element in a measure
                  barlineLeft(m);

                  // output attributes with the first actual measure (pickup or regular)
                  if (isFirstActualMeasure) {
                        _attr.doAttr(_xml, true);
                        _xml.tag("divisions", MScore::division / div);
                        }

                  // output attributes at start of measure: key, time
                  keysigTimesig(m, part);

                  // output attributes with the first actual measure (pickup or regular) only
                  if (isFirstActualMeasure) {
                        if (staves > 1)
                              _xml.tag("staves", staves);
                        if (instrMap.size() > 1)
                              _xml.tag("instruments", instrMap.size());
                        }

                  // make sure clefs at end of measure get exported at start of next measure
                  findAndExportClef(m, staves, strack, etrack);

                  // make sure a clef gets exported if none is found
                  exportDefaultClef(part, m);

                  // output attributes with the first actual measure (pickup or regular) only
                  if (isFirstActualMeasure) {
                        writeStaffDetails(_xml, part);
                        writeInstrumentDetails(_xml, part);
                        }

                  // output attribute at start of measure: measure-style
                  measureStyle(_xml, _attr, m);

                  // set of spanners already stopped in this measure
                  // required to prevent multiple spanner stops for the same spanner
                  QSet<const Spanner*> spannersStopped;

                  // MuseScore limitation: repeats are always in the first part
                  // and are implicitly placed at either measure start or stop
                  if (idx == 0)
                        repeatAtMeasureStart(_xml, _attr, m, strack, etrack, strack);

                  for (int st = strack; st < etrack; ++st) {
                        // sstaff - xml staff number, counting from 1 for this
                        // instrument
                        // special number 0 -> don’t show staff number in
                        // xml output (because there is only one staff)

                        int sstaff = (staves > 1) ? st - strack + VOICES : 0;
                        sstaff /= VOICES;
                        for (Segment* seg = m->first(); seg; seg = seg->next()) {
                              Element* el = seg->element(st);
                              if (!el) {
                                    continue;
                                    }
                              // must ignore start repeat to prevent spurious backup/forward
                              if (el->type() == ElementType::BAR_LINE && static_cast<BarLine*>(el)->barLineType() == BarLineType::START_REPEAT)
                                    continue;

                              // generate backup or forward to the start time of the element
                              if (_tick != seg->tick()) {
                                    _attr.doAttr(_xml, false);
                                    moveToTick(seg->tick());
                                    }

                              // handle annotations and spanners (directions attached to this note or rest)
                              if (el->isChordRest()) {
                                    _attr.doAttr(_xml, false);
                                    annotations(this, _xml, strack, etrack, st, sstaff, seg);
                                    // look for more harmony
                                    for (Segment* seg1 = seg->next(); seg1; seg1 = seg1->next()) {
                                          if (seg1->isChordRestType()) {
                                                Element* el1 = seg1->element(st);
                                                if (el1) // found a ChordRest, next harmony will be attach to this one
                                                      break;
                                                for (Element* annot : seg1->annotations()) {
                                                      if (annot->type() == ElementType::HARMONY && annot->track() == st)
                                                            harmony(toHarmony(annot), 0, (seg1->tick() - seg->tick()).ticks() / div);
                                                      }
                                                }
                                          }
                                    figuredBass(_xml, strack, etrack, st, static_cast<const ChordRest*>(el), fbMap, div);
                                    spannerStart(this, strack, etrack, st, sstaff, seg);
                                    }

                              // write element el if necessary
                              writeElement(el, m, sstaff, part->instrument()->useDrumset());

                              // handle annotations and spanners (directions attached to this note or rest)
                              if (el->isChordRest()) {
                                    int spannerStaff = (st / VOICES) * VOICES;
                                    spannerStop(this, spannerStaff, _tick, sstaff, spannersStopped);
                                    }

                              } // for (Segment* seg = ...
                        _attr.stop(_xml);
                        } // for (int st = ...
                  // move to end of measure (in case of incomplete last voice)
#ifdef DEBUG_TICK
                  qDebug("end of measure");
#endif
                  moveToTick(m->endTick());
                  if (idx == 0)
                        repeatAtMeasureStop(_xml, m, strack, etrack, strack);
                  // note: don't use "m->repeatFlags() & Repeat::END" here, because more
                  // barline types need to be handled besides repeat end ("light-heavy")
                  barlineRight(m);
                  _xml.etag();
                  }
            staffCount += staves;
            _xml.etag();
            }

      _xml.etag();

      if (concertPitch) {
            // restore concert pitch
            score()->endCmd(true);        // rollback
            }
      }

//---------------------------------------------------------
//   saveXml
//    return false on error
//---------------------------------------------------------

/**
 Save Score as MusicXML file \a name.

 Return false on error.
 */

bool saveXml(Score* score, QIODevice* device)
      {
      ExportMusicXml em(score);
      em.write(device);
      return true;
      }

bool saveXml(Score* score, const QString& name)
      {
      QFile f(name);
      if (!f.open(QIODevice::WriteOnly))
            return false;

      bool res = saveXml(score, &f) && (f.error() == QFile::NoError);
      f.close();
      return res;
      }

//---------------------------------------------------------
//   saveMxl
//    return false on error
//---------------------------------------------------------

/**
 Save Score as compressed MusicXML file \a name.

 Return false on error.
 */

// META-INF/container.xml:
// <?xml version="1.0" encoding="UTF-8"?>
// <container>
//     <rootfiles>
//         <rootfile full-path="testHello.xml"/>
//     </rootfiles>
// </container>

static void writeMxlArchive(Score* score, MQZipWriter& zipwriter, const QString& filename)
      {
      QBuffer cbuf;
      cbuf.open(QIODevice::ReadWrite);

      XmlWriter xml(score);
      xml.setDevice(&cbuf);
      xml.setCodec("UTF-8");
      xml << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      xml.stag("container");
      xml.stag("rootfiles");
      xml.stag(QString("rootfile full-path=\"%1\"").arg(filename));
      xml.etag();
      xml.etag();
      xml.etag();
      cbuf.seek(0);

      //uz.addDirectory("META-INF");
      zipwriter.addFile("META-INF/container.xml", cbuf.data());

      QBuffer dbuf;
      dbuf.open(QIODevice::ReadWrite);
      ExportMusicXml em(score);
      em.write(&dbuf);
      dbuf.seek(0);
      zipwriter.addFile(filename, dbuf.data());
      }

bool saveMxl(Score* score, QIODevice* device)
      {
      MQZipWriter uz(device);

      //anonymized filename since we don't know the actual one here
      QString fn = "score.xml";
      writeMxlArchive(score, uz, fn);
      uz.close();

      return true;
      }

bool saveMxl(Score* score, const QString& name)
      {
      MQZipWriter uz(name);

      QFileInfo fi(name);
      QString fn = fi.completeBaseName() + ".xml";
      writeMxlArchive(score, uz, fn);

      return true;
      }

double ExportMusicXml::getTenthsFromInches(double inches) const
      {
      return inches * INCH / millimeters * tenths;
      }

double ExportMusicXml::getTenthsFromDots(double dots) const
      {
      return dots / DPMM / millimeters * tenths;
      }

//---------------------------------------------------------
//   harmony
//---------------------------------------------------------

void ExportMusicXml::harmony(Harmony const* const h, FretDiagram const* const fd, int offset)
      {
      // this code was probably in place to allow chord symbols shifted *right* to export with offset
      // since this was at once time the only way to get a chord to appear over beat 3 in an empty 4/4 measure
      // but the value was calculated incorrectly (should be divided by spatium) and would be better off using offset anyhow
      // since we now support placement of chord symbols over "empty" beats directly,
      // and wedon't generally export position info for other elements
      // it's just as well to not bother doing so here
      //double rx = h->offset().x()*10;
      //QString relative;
      //if (rx > 0) {
      //      relative = QString(" relative-x=\"%1\"").arg(QString::number(rx,'f',2));
      //      }
      int rootTpc = h->rootTpc();
      if (rootTpc != Tpc::TPC_INVALID) {
            QString tagName = "harmony";
            bool frame = h->hasFrame();
            tagName += QString(" print-frame=\"%1\"").arg(frame ? "yes" : "no"); // .append(relative));
            tagName += color2xml(h);
            _xml.stag(tagName);
            _xml.stag("root");
            _xml.tag("root-step", tpc2stepName(rootTpc));
            int alter = int(tpc2alter(rootTpc));
            if (alter)
                  _xml.tag("root-alter", alter);
            _xml.etag();

            if (!h->xmlKind().isEmpty()) {
                  QString s = "kind";
                  QString kindText = h->musicXmlText();
                  if (h->musicXmlText() != "")
                        s += " text=\"" + kindText + "\"";
                  if (h->xmlSymbols() == "yes")
                        s += " use-symbols=\"yes\"";
                  if (h->xmlParens() == "yes")
                        s += " parentheses-degrees=\"yes\"";
                  _xml.tag(s, h->xmlKind());
                  QStringList l = h->xmlDegrees();
                  if (!l.isEmpty()) {
                        for (QString tag : l) {
                              QString degreeText;
                              if (h->xmlKind().startsWith("suspended")
                                  && tag.startsWith("add") && tag[3].isDigit()
                                  && !kindText.isEmpty() && kindText[0].isDigit()) {
                                    // hack to correct text for suspended chords whose kind text has degree information baked in
                                    // (required by some other applications)
                                    int tagDegree = tag.mid(3).toInt();
                                    QString kindTextExtension;
                                    for (int i = 0; i < kindText.length() && kindText[i].isDigit(); ++i)
                                          kindTextExtension[i] = kindText[i];
                                    int kindExtension = kindTextExtension.toInt();
                                    if (tagDegree <= kindExtension && (tagDegree & 1) && (kindExtension & 1))
                                          degreeText = " text=\"\"";
                                    }
                              _xml.stag("degree");
                              alter = 0;
                              int idx = 3;
                              if (tag[idx] == '#') {
                                    alter = 1;
                                    ++idx;
                                    }
                              else if (tag[idx] == 'b') {
                                    alter = -1;
                                    ++idx;
                                    }
                              _xml.tag(QString("degree-value%1").arg(degreeText), tag.mid(idx));
                              _xml.tag("degree-alter", alter);     // finale insists on this even if 0
                              if (tag.startsWith("add"))
                                    _xml.tag(QString("degree-type%1").arg(degreeText), "add");
                              else if (tag.startsWith("sub"))
                                    _xml.tag("degree-type", "subtract");
                              else if (tag.startsWith("alt"))
                                    _xml.tag("degree-type", "alter");
                              _xml.etag();
                              }
                        }
                  }
            else {
                  if (h->extensionName() == 0)
                        _xml.tag("kind", "");
                  else
                        _xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), "");
                  }

            int baseTpc = h->baseTpc();
            if (baseTpc != Tpc::TPC_INVALID) {
                  _xml.stag("bass");
                  _xml.tag("bass-step", tpc2stepName(baseTpc));
                  alter = int(tpc2alter(baseTpc));
                  if (alter) {
                        _xml.tag("bass-alter", alter);
                        }
                  _xml.etag();
                  }
            if (offset > 0)
                  _xml.tag("offset", offset);
            if (fd)
                  fd->writeMusicXML(_xml);

            _xml.etag();
            }
      else {
            //
            // export an unrecognized Chord
            // which may contain arbitrary text
            //
            if (h->hasFrame())
                  _xml.stag(QString("harmony print-frame=\"yes\""));     // .append(relative));
            else
                  _xml.stag(QString("harmony print-frame=\"no\""));      // .append(relative));
            _xml.stag("root");
            _xml.tag("root-step text=\"\"", "C");
            _xml.etag();       // root
            QString k = "kind text=\"" + h->hTextName() + "\"";
            _xml.tag(k, "none");
            _xml.etag();       // harmony
#if 0
            // prior to 2.0, MuseScore exported unrecognized chords as plain text
            xml.stag("direction");
            xml.stag("direction-type");
            xml.tag("words", h->text());
            xml.etag();
            xml.etag();
#endif
            }
#if 0
      // this is very old code that may never have actually been used
      xml.tag(QString("kind text=\"%1\"").arg(h->extensionName()), extension);
      for (int i = 0; i < h->numberOfDegrees(); i++) {
            HDegree hd = h->degree(i);
            HDegreeType tp = hd.type();
            if (tp == HDegreeType::ADD || tp == HDegreeType::ALTER || tp == HDegreeType::SUBTRACT) {
                  xml.stag("degree");
                  xml.tag("degree-value", hd.value());
                  xml.tag("degree-alter", hd.alter());
                  switch (tp) {
                        case HDegreeType::ADD:
                              xml.tag("degree-type", "add");
                              break;
                        case HDegreeType::ALTER:
                              xml.tag("degree-type", "alter");
                              break;
                        case HDegreeType::SUBTRACT:
                              xml.tag("degree-type", "subtract");
                              break;
                        default:
                              break;
                        }
                  xml.etag();
                  }
            }
#endif
      }

}
