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

#include "excerpt.h"

#include <QRegularExpression>

#include "style/style.h"
#include "rw/xml.h"

#include "factory.h"
#include "barline.h"
#include "beam.h"
#include "box.h"
#include "bracketItem.h"
#include "chord.h"
#include "harmony.h"
#include "layoutbreak.h"
#include "lyrics.h"
#include "measure.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "slur.h"
#include "staff.h"
#include "stafftype.h"
#include "tempo.h"
#include "text.h"
#include "textframe.h"
#include "textline.h"
#include "tie.h"
#include "tiemap.h"
#include "tremolo.h"
#include "tuplet.h"
#include "tupletmap.h"
#include "undo.h"
#include "utils.h"
#include "masterscore.h"
#include "linkedobjects.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   Excerpt
//---------------------------------------------------------

Excerpt::Excerpt(const Excerpt& ex, bool copyPartScore)
    : QObject(), _oscore(ex._oscore), _title(ex._title), _parts(ex._parts), _tracks(ex._tracks)
{
    _partScore = (copyPartScore && ex._partScore) ? ex._partScore->clone() : nullptr;

    if (_partScore) {
        _partScore->setExcerpt(this);
    }
}

//---------------------------------------------------------
//   ~Excerpt
//---------------------------------------------------------

Excerpt::~Excerpt()
{
    delete _partScore;
}

bool Excerpt::containsPart(const Part* part) const
{
    for (Part* _part : _parts) {
        if (_part == part) {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------
//   nstaves
//---------------------------------------------------------

int Excerpt::nstaves() const
{
    int n { 0 };
    for (Part* p : _parts) {
        n += p->nstaves();
    }
    return n;
}

bool Excerpt::isEmpty() const
{
    return partScore() ? partScore()->parts().empty() : true;
}

void Excerpt::setTracks(const QMultiMap<int, int>& tracks)
{
    _tracks = tracks;

    for (Staff* staff : partScore()->staves()) {
        Staff* masterStaff = _oscore->staffById(staff->id());
        if (!masterStaff) {
            continue;
        }
        staff->updateVisibilityVoices(masterStaff);
    }
}

void Excerpt::removePart(const ID& id)
{
    int index = 0;
    for (const Part* part: parts()) {
        if (part->id() == id) {
            break;
        }
        ++index;
    }
    if (index >= _parts.size()) {
        return;
    }

    partScore()->undoRemovePart(partScore()->parts().at(index));
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Excerpt::read(XmlReader& e)
{
    const QList<Part*>& pl = _oscore->parts();
    QString name;
    while (e.readNextStartElement()) {
        const QStringRef& tag = e.name();
        if (tag == "name") {
            name = e.readElementText();
        } else if (tag == "title") {
            _title = e.readElementText().trimmed();
        } else if (tag == "part") {
            int partIdx = e.readInt();
            if (partIdx < 0 || partIdx >= pl.size()) {
                qDebug("Excerpt::read: bad part index");
            } else {
                _parts.append(pl.at(partIdx));
            }
        }
    }
    if (_title.isEmpty()) {
        _title = name.trimmed();
    }
}

//---------------------------------------------------------
//   operator!=
//---------------------------------------------------------

bool Excerpt::operator!=(const Excerpt& e) const
{
    if (e._oscore != _oscore) {
        return true;
    }
    if (e._title != _title) {
        return true;
    }
    if (e._parts != _parts) {
        return true;
    }
    if (e._tracks != _tracks) {
        return true;
    }
    return false;
}

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Excerpt::operator==(const Excerpt& e) const
{
    if (e._oscore != _oscore) {
        return false;
    }
    if (e._title != _title) {
        return false;
    }
    if (e._parts != _parts) {
        return false;
    }
    if (e._tracks != _tracks) {
        return false;
    }
    return true;
}

void Excerpt::updateTracks()
{
    QMultiMap<int, int> tracks;
    for (Staff* staff : partScore()->staves()) {
        Staff* masterStaff = oscore()->staffById(staff->id());
        if (!masterStaff) {
            continue;
        }

        int voice = 0;
        for (int i = 0; i < VOICES; i++) {
            if (!staff->isVoiceVisible(i)) {
                continue;
            }

            tracks.insert(masterStaff->idx() * VOICES + i % VOICES, staff->idx() * VOICES + voice % VOICES);
            voice++;
        }
    }

    setTracks(tracks);
}

void Excerpt::setVoiceVisible(Staff* staff, int voiceIndex, bool visible)
{
    TRACEFUNC;

    if (!staff) {
        return;
    }

    Staff* masterStaff = oscore()->staffById(staff->id());
    if (!masterStaff) {
        return;
    }

    int staffIndex = staff->idx();
    Ms::Fraction startTick = staff->score()->firstMeasure()->tick();
    Ms::Fraction endTick = staff->score()->lastMeasure()->tick();

    // update tracks
    staff->setVoiceVisible(voiceIndex, visible);
    updateTracks();

    // clone staff
    Staff* staffCopy = Factory::createStaff(staff->part());
    staffCopy->setId(staff->id());
    staffCopy->init(staff);

    // remove current staff, insert cloned
    partScore()->undoRemoveStaff(staff);
    int partStaffIndex = staffIndex - partScore()->staffIdx(staff->part());
    partScore()->undoInsertStaff(staffCopy, partStaffIndex);

    // clone master staff to current with mapped tracks
    cloneStaff2(masterStaff, staffCopy, startTick, endTick);

    // link master staff to cloned
    Staff* newStaff = partScore()->staffById(masterStaff->id());
    partScore()->undo(new Link(newStaff, masterStaff));
}

//---------------------------------------------------------
//   createExcerpt
//---------------------------------------------------------

void Excerpt::createExcerpt(Excerpt* excerpt)
{
    MasterScore* oscore = excerpt->oscore();
    Score* score        = excerpt->partScore();

    QList<Part*>& parts = excerpt->parts();
    QList<int> srcStaves;

    // clone layer:
    for (int i = 0; i < 32; ++i) {
        score->layerTags()[i] = oscore->layerTags()[i];
        score->layerTagComments()[i] = oscore->layerTagComments()[i];
    }
    score->setCurrentLayer(oscore->currentLayer());
    score->layer().clear();
    foreach (const Layer& l, oscore->layer()) {
        score->layer().append(l);
    }

    score->setPageNumberOffset(oscore->pageNumberOffset());

    // Set instruments and create linked staves
    for (const Part* part : parts) {
        Part* p = new Part(score);
        p->setId(part->id());
        p->setInstrument(*part->instrument());
        p->setPartName(part->partName());

        for (Staff* staff : *part->staves()) {
            Staff* s = Factory::createStaff(p);
            s->setId(staff->id());
            s->init(staff);
            s->setDefaultClefType(staff->defaultClefType());
            // the order of staff - s matters as staff should be the first entry in the
            // created link list to make primaryStaff() work
            // TODO: change implementation, maybe create an explicit "primary" flag
            score->undo(new Link(s, staff));
            score->appendStaff(s);
            srcStaves.append(staff->idx());
        }
        score->appendPart(p);
    }

    // Fill tracklist (map all tracks of a stave)
    if (excerpt->tracks().isEmpty()) {
        excerpt->updateTracks();
    }

    cloneStaves(oscore, score, srcStaves, excerpt->tracks());

    // create excerpt title and title frame for all scores if not already there
    MeasureBase* measure = oscore->first();

    if (!measure || !measure->isVBox()) {
        qDebug("original score has no header frame");
        oscore->insertMeasure(ElementType::VBOX, measure);
        measure = oscore->first();
    }
    VBox* titleFrameScore = toVBox(measure);

    measure = score->first();
    Q_ASSERT(measure->isVBox());

    VBox* titleFramePart = toVBox(measure);
    titleFramePart->copyValues(titleFrameScore);
    QString partLabel = excerpt->title();       // parts.front()->longName();
    if (!partLabel.isEmpty()) {
        Text* txt = Factory::createText(measure, TextStyleType::INSTRUMENT_EXCERPT);
        txt->setPlainText(partLabel);
        measure->add(txt);
        score->setMetaTag("partName", partLabel);
    }

    // initial layout of score
    score->addLayoutFlags(LayoutFlag::FIX_PITCH_VELO);
    score->doLayout();

    // handle transposing instruments
    if (oscore->styleB(Sid::concertPitch) != score->styleB(Sid::concertPitch)) {
        for (const Staff* staff : score->staves()) {
            if (staff->staffType(Fraction(0, 1))->group() == StaffGroup::PERCUSSION) {
                continue;
            }

            // if this staff has no transposition, and no instrument changes, we can skip it
            Interval interval = staff->part()->instrument()->transpose(); //tick?
            if (interval.isZero() && staff->part()->instruments()->size() == 1) {
                continue;
            }
            bool flip = false;
            if (oscore->styleB(Sid::concertPitch)) {
                interval.flip();          // flip the transposition for the original instrument
                flip = true;              // transposeKeys() will flip transposition for each instrument change
            }

            int staffIdx   = staff->idx();
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;

            Fraction endTick = Fraction(0, 1);
            if (score->lastSegment()) {
                endTick = score->lastSegment()->tick();
            }
            score->transposeKeys(staffIdx, staffIdx + 1, Fraction(0, 1), endTick, interval, true, flip);

            for (auto segment = score->firstSegmentMM(SegmentType::ChordRest); segment;
                 segment = segment->next1MM(SegmentType::ChordRest)) {
                interval = staff->part()->instrument(segment->tick())->transpose();
                if (interval.isZero()) {
                    continue;
                }
                if (oscore->styleB(Sid::concertPitch)) {
                    interval.flip();
                }

                for (auto e : segment->annotations()) {
                    if (!e->isHarmony() || (e->track() < startTrack) || (e->track() >= endTrack)) {
                        continue;
                    }
                    Harmony* h  = toHarmony(e);
                    int rootTpc = Ms::transposeTpc(h->rootTpc(), interval, true);
                    int baseTpc = Ms::transposeTpc(h->baseTpc(), interval, true);
                    // mmrests are on by default in part
                    // if this harmony is attached to an mmrest,
                    // be sure to transpose harmony in underlying measure as well
                    for (EngravingObject* se : h->linkList()) {
                        Harmony* hh = static_cast<Harmony*>(se);
                        // skip links to other staves (including in other scores)
                        if (hh->staff() != h->staff()) {
                            continue;
                        }
                        score->undoTransposeHarmony(hh, rootTpc, baseTpc);
                    }
                }
            }
        }
    }

    // update style values if spatium different for part
    if (oscore->spatium() != score->spatium()) {
        //score->spatiumChanged(oscore->spatium(), score->spatium());
        score->styleChanged();
    }

    // second layout of score
    score->setPlaylistDirty();
    oscore->rebuildMidiMapping();
    oscore->updateChannel();

    score->setLayoutAll();
    score->doLayout();
}

//---------------------------------------------------------
//   deleteExcerpt
//---------------------------------------------------------

void MasterScore::deleteExcerpt(Excerpt* excerpt)
{
    Q_ASSERT(excerpt->oscore() == this);
    Score* partScore = excerpt->partScore();

    if (!partScore) {
        qDebug("deleteExcerpt: no partScore");
        return;
    }

    // unlink the staves in the excerpt
    for (Staff* st : partScore->staves()) {
        bool hasLinksInMaster = false;
        if (st->links()) {
            for (auto le : *st->links()) {
                if (le->score() == this) {
                    hasLinksInMaster = true;
                    break;
                }
            }
        }
        if (hasLinksInMaster) {
            int staffIdx = st->idx();
            // unlink the spanners
            for (auto i = partScore->spanner().begin(); i != partScore->spanner().cend(); ++i) {
                Spanner* sp = i->second;
                if (sp->staffIdx() == staffIdx) {
                    sp->undoUnlink();
                }
            }
            int sTrack = staffIdx * VOICES;
            int eTrack = sTrack + VOICES;
            // unlink elements and annotation
            for (Segment* s = partScore->firstSegmentMM(SegmentType::All); s; s = s->next1MM()) {
                for (int track = eTrack - 1; track >= sTrack; --track) {
                    EngravingItem* el = s->element(track);
                    if (el) {
                        el->undoUnlink();
                    }
                }
                for (EngravingItem* e : s->annotations()) {
                    if (e->staffIdx() == staffIdx) {
                        e->undoUnlink();
                    }
                }
            }
            // unlink the staff
            undo(new Unlink(st));
        }
    }
    undo(new RemoveExcerpt(excerpt));
}

void MasterScore::initAndAddExcerpt(Excerpt* excerpt, bool fakeUndo)
{
    Score* score = new Score(masterScore());
    excerpt->setPartScore(score);
    score->style().set(Sid::createMultiMeasureRests, true);
    auto excerptCmd = new AddExcerpt(excerpt);
    if (fakeUndo) {
        excerptCmd->redo(nullptr);
    } else {
        score->undo(excerptCmd);
    }
    Excerpt::createExcerpt(excerpt);
}

//---------------------------------------------------------
//   cloneSpanner
//---------------------------------------------------------

static void cloneSpanner(Spanner* s, Score* score, int dstTrack, int dstTrack2)
{
    // don’t clone voltas for track != 0
    if ((s->isVolta() || (s->isTextLine() && toTextLine(s)->systemFlag())) && s->track() != 0) {
        return;
    }

    Spanner* ns = toSpanner(s->linkedClone());
    ns->setScore(score);
    ns->resetExplicitParent();
    ns->setTrack(dstTrack);
    ns->setTrack2(dstTrack2);

    if (ns->isSlur()) {
        // set start/end element for slur
        ChordRest* cr1 = s->startCR();
        ChordRest* cr2 = s->endCR();

        ns->setStartElement(0);
        ns->setEndElement(0);
        if (cr1 && cr1->links()) {
            for (EngravingObject* e : *cr1->links()) {
                ChordRest* cr = toChordRest(e);
                if (cr == cr1) {
                    continue;
                }
                if ((cr->score() == score) && (cr->tick() == ns->tick()) && cr->track() == dstTrack) {
                    ns->setStartElement(cr);
                    break;
                }
            }
        }
        if (cr2 && cr2->links()) {
            for (EngravingObject* e : *cr2->links()) {
                ChordRest* cr = toChordRest(e);
                if (cr == cr2) {
                    continue;
                }
                if ((cr->score() == score) && (cr->tick() == ns->tick2()) && cr->track() == dstTrack2) {
                    ns->setEndElement(cr);
                    break;
                }
            }
        }
        if (!ns->startElement()) {
            qDebug("clone Slur: no start element");
        }
        if (!ns->endElement()) {
            qDebug("clone Slur: no end element");
        }
    }
    score->undo(new AddElement(ns));
}

//---------------------------------------------------------
//   cloneTuplets
//---------------------------------------------------------

static void cloneTuplets(ChordRest* ocr, ChordRest* ncr, Tuplet* ot, TupletMap& tupletMap, Measure* m, int track)
{
    ot->setTrack(ocr->track());
    Tuplet* nt = tupletMap.findNew(ot);
    if (nt == 0) {
        nt = toTuplet(ot->linkedClone());
        nt->setTrack(track);
        nt->setParent(m);
        nt->setScore(ncr->score());
        tupletMap.add(ot, nt);

        Tuplet* nt1 = nt;
        while (ot->tuplet()) {
            Tuplet* nt2 = tupletMap.findNew(ot->tuplet());
            if (nt2 == 0) {
                nt2 = toTuplet(ot->tuplet()->linkedClone());
                nt2->setTrack(track);
                nt2->setParent(m);
                nt2->setScore(ncr->score());
                tupletMap.add(ot->tuplet(), nt2);
            }
            nt2->add(nt1);
            nt1->setTuplet(nt2);
            ot = ot->tuplet();
            nt1 = nt2;
        }
    }
    nt->add(ncr);
    ncr->setTuplet(nt);
}

//---------------------------------------------------------
//   processLinkedClone
//---------------------------------------------------------

void Excerpt::processLinkedClone(EngravingItem* ne, Score* score, int strack)
{
    // reset offset as most likely it will not fit
    PropertyFlags f = ne->propertyFlags(Pid::OFFSET);
    if (f == PropertyFlags::UNSTYLED) {
        ne->setPropertyFlags(Pid::OFFSET, PropertyFlags::STYLED);
        ne->resetProperty(Pid::OFFSET);
    }
    ne->setTrack(strack == -1 ? 0 : strack);
    ne->setScore(score);
}

//---------------------------------------------------------
//   cloneStaves
//---------------------------------------------------------

void Excerpt::cloneStaves(Score* oscore, Score* score, const QList<int>& sourceStavesIndexes, QMultiMap<int, int>& trackList)
{
    TieMap tieMap;

    MeasureBaseList* nmbl = score->measures();
    for (MeasureBase* mb = oscore->measures()->first(); mb; mb = mb->next()) {
        MeasureBase* nmb = 0;
        if (mb->isHBox()) {
            nmb = new HBox(score->dummy()->system());
        } else if (mb->isVBox()) {
            nmb = new VBox(score->dummy()->system());
        } else if (mb->isTBox()) {
            nmb = new TBox(score->dummy()->system());
            Text* text = toTBox(mb)->text();
            EngravingItem* ne = text->linkedClone();
            ne->setScore(score);
            nmb->add(ne);
        } else if (mb->isMeasure()) {
            Measure* m  = toMeasure(mb);
            Measure* nm = Factory::createMeasure(score->dummy()->system());
            nmb = nm;
            nm->setTick(m->tick());
            nm->setTicks(m->ticks());
            nm->setTimesig(m->timesig());

            nm->setRepeatCount(m->repeatCount());
            nm->setRepeatStart(m->repeatStart());
            nm->setRepeatEnd(m->repeatEnd());
            nm->setRepeatJump(m->repeatJump());

            nm->setIrregular(m->irregular());
            nm->setNo(m->no());
            nm->setNoOffset(m->noOffset());
            nm->setBreakMultiMeasureRest(m->breakMultiMeasureRest());

            for (int dstStaffIdx = 0; dstStaffIdx < sourceStavesIndexes.size(); ++dstStaffIdx) {
                nm->setStaffStemless(dstStaffIdx, m->stemless(sourceStavesIndexes[dstStaffIdx]));
            }
            int tracks = oscore->nstaves() * VOICES;

            if (sourceStavesIndexes.isEmpty() && trackList.isEmpty()) {
                tracks = 0;
            }

            for (int srcTrack = 0; srcTrack < tracks; ++srcTrack) {
                TupletMap tupletMap;            // tuplets cannot cross measure boundaries

                int strack = trackList.value(srcTrack, -1);

                Tremolo* tremolo = 0;
                for (Segment* oseg = m->first(); oseg; oseg = oseg->next()) {
                    Segment* ns = nullptr;           //create segment later, on demand
                    for (EngravingItem* e : oseg->annotations()) {
                        if (e->generated()) {
                            continue;
                        }
                        if ((e->track() == srcTrack && strack != -1) || (e->systemFlag() && srcTrack == 0)) {
                            EngravingItem* ne = e->linkedClone();
                            processLinkedClone(ne, score, strack);
                            if (!ns) {
                                ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                            }
                            ns->add(ne);
                            // for chord symbols,
                            // re-render with new style settings
                            if (ne->isHarmony()) {
                                Harmony* h = toHarmony(ne);
                                h->render();
                            } else if (ne->isFretDiagram()) {
                                Harmony* h = toHarmony(toFretDiagram(ne)->harmony());
                                if (h) {
                                    processLinkedClone(h, score, strack);
                                    h->render();
                                }
                            }
                        }
                    }

                    //If track is not mapped skip the following
                    if (trackList.value(srcTrack, -1) == -1) {
                        continue;
                    }

                    //There are probably more destination tracks for the same source
                    QList<int> t = trackList.values(srcTrack);

                    for (int track : qAsConst(t)) {
                        //Clone KeySig TimeSig and Clefs if voice 1 of source staff is not mapped to a track
                        EngravingItem* oef = oseg->element(trackZeroVoice(srcTrack));
                        if (oef && !oef->generated() && (oef->isTimeSig() || oef->isKeySig())
                            && !(trackList.size() == (score->excerpt()->nstaves() * VOICES))) {
                            EngravingItem* ne = oef->linkedClone();
                            ne->setTrack(trackZeroVoice(track));
                            ne->setScore(score);
                            ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                            ns->add(ne);
                        }

                        EngravingItem* oe = oseg->element(srcTrack);
                        int adjustedBarlineSpan = 0;
                        if (srcTrack % VOICES == 0 && oseg->segmentType() == SegmentType::BarLine) {
                            // mid-measure barline segment
                            // may need to clone barline from a previous staff and/or adjust span
                            int oIdx = srcTrack / VOICES;
                            if (!oe) {
                                // no barline on this staff in original score,
                                // but check previous staves
                                for (int i = oIdx - 1; i >= 0; --i) {
                                    oe = oseg->element(i * VOICES);
                                    if (oe) {
                                        break;
                                    }
                                }
                            }
                            if (oe) {
                                // barline found, now check span
                                BarLine* bl = toBarLine(oe);
                                int oSpan1 = bl->staff()->idx();
                                int oSpan2 = oSpan1 + bl->spanStaff();
                                if (oSpan1 <= oIdx && oIdx < oSpan2) {
                                    // this staff is within span
                                    // calculate adjusted span for excerpt
                                    int oSpan = oSpan2 - oIdx;
                                    adjustedBarlineSpan = qMin(oSpan, score->nstaves());
                                } else {
                                    // this staff is not within span
                                    oe = nullptr;
                                }
                            }
                        }

                        if (oe && !oe->generated()) {
                            EngravingItem* ne;
                            ne = oe->linkedClone();
                            ne->setTrack(track);

                            if (!(ne->track() % VOICES) && ne->isRest()) {
                                toRest(ne)->setGap(false);
                            }

                            ne->setScore(score);
                            if (oe->isBarLine() && adjustedBarlineSpan) {
                                BarLine* nbl = toBarLine(ne);
                                nbl->setSpanStaff(adjustedBarlineSpan);
                            } else if (oe->isChordRest()) {
                                ChordRest* ocr = toChordRest(oe);
                                ChordRest* ncr = toChordRest(ne);

                                if (ocr->beam() && !ocr->beam()->empty() && ocr->beam()->elements().front() == ocr) {
                                    Beam* nb = ocr->beam()->clone();
                                    nb->clear();
                                    nb->setTrack(track);
                                    nb->setScore(score);
                                    nb->add(ncr);
                                    ncr->setBeam(nb);
                                }

                                Tuplet* ot = ocr->tuplet();

                                if (ot) {
                                    cloneTuplets(ocr, ncr, ot, tupletMap, nm, track);
                                }

                                if (oe->isChord()) {
                                    Chord* och = toChord(ocr);
                                    Chord* nch = toChord(ncr);

                                    size_t n = och->notes().size();
                                    for (size_t i = 0; i < n; ++i) {
                                        Note* on = och->notes().at(i);
                                        Note* nn = nch->notes().at(i);
                                        if (on->tieFor()) {
                                            Tie* tie = toTie(on->tieFor()->linkedClone());
                                            tie->setScore(score);
                                            nn->setTieFor(tie);
                                            tie->setStartNote(nn);
                                            tie->setTrack(nn->track());
                                            tieMap.add(on->tieFor(), tie);
                                        }
                                        if (on->tieBack()) {
                                            Tie* tie = tieMap.findNew(on->tieBack());
                                            if (tie) {
                                                nn->setTieBack(tie);
                                                tie->setEndNote(nn);
                                            } else {
                                                qDebug("cloneStaves: cannot find tie");
                                            }
                                        }
                                        // add back spanners (going back from end to start spanner element
                                        // makes sure the 'other' spanner anchor element is already set up)
                                        // 'on' is the old spanner end note and 'nn' is the new spanner end note
                                        for (Spanner* oldSp : on->spannerBack()) {
                                            if (oldSp->startElement() && oldSp->endElement()
                                                && oldSp->startElement()->track() > oldSp->endElement()->track()) {
                                                continue;
                                            }
                                            Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                                            if (newStart != nullptr) {
                                                Spanner* newSp = toSpanner(oldSp->linkedClone());
                                                newSp->setNoteSpan(newStart, nn);
                                                score->addElement(newSp);
                                            } else {
                                                qDebug("cloneStaves: cannot find spanner start note");
                                            }
                                        }
                                        for (Spanner* oldSp : on->spannerFor()) {
                                            if (oldSp->startElement() && oldSp->endElement()
                                                && oldSp->startElement()->track() <= oldSp->endElement()->track()) {
                                                continue;
                                            }
                                            Note* newEnd = Spanner::endElementFromSpanner(oldSp, nn);
                                            if (newEnd != nullptr) {
                                                Spanner* newSp = toSpanner(oldSp->linkedClone());
                                                newSp->setNoteSpan(nn, newEnd);
                                                score->addElement(newSp);
                                            } else {
                                                qDebug("cloneStaves: cannot find spanner end note");
                                            }
                                        }
                                    }
                                    // two note tremolo
                                    if (och->tremolo() && och->tremolo()->twoNotes()) {
                                        if (och == och->tremolo()->chord1()) {
                                            if (tremolo) {
                                                qDebug("unconnected two note tremolo");
                                            }
                                            tremolo = toTremolo(och->tremolo()->linkedClone());
                                            tremolo->setScore(nch->score());
                                            tremolo->setParent(nch);
                                            tremolo->setTrack(nch->track());
                                            tremolo->setChords(nch, 0);
                                            nch->setTremolo(tremolo);
                                        } else if (och == och->tremolo()->chord2()) {
                                            if (!tremolo) {
                                                qDebug("first note for two note tremolo missing");
                                            } else {
                                                tremolo->setChords(tremolo->chord1(), nch);
                                                nch->setTremolo(tremolo);
                                            }
                                        } else {
                                            qDebug("inconsistent two note tremolo");
                                        }
                                    }
                                }
                            }
                            if (!ns) {
                                ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                            }
                            ns->add(ne);
                        }

                        Segment* tst = nm->segments().firstCRSegment();
                        if (srcTrack % VOICES && !(track % VOICES) && (!tst || (!tst->element(track)))) {
                            Segment* segment = nm->getSegment(SegmentType::ChordRest, nm->tick());
                            Rest* rest = Factory::createRest(segment);
                            rest->setTicks(nm->ticks());
                            rest->setDurationType(nm->ticks());
                            rest->setTrack(track);
                            segment->add(rest);
                        }
                    }
                }
            }
        }

        nmb->linkTo(mb);
        for (EngravingItem* e : mb->el()) {
            if (e->isLayoutBreak()) {
                LayoutBreakType st = toLayoutBreak(e)->layoutBreakType();
                if (st == LayoutBreakType::PAGE || st == LayoutBreakType::LINE) {
                    continue;
                }
            }
            int track = -1;
            if (e->track() != -1) {
                // try to map track
                track = trackList.value(e->track(), -1);
                if (track == -1) {
                    // even if track not in excerpt, we need to clone system elements
                    if (e->systemFlag()) {
                        track = 0;
                    } else {
                        continue;
                    }
                }
            }

            EngravingItem* ne;
            // link text - title, subtitle, also repeats (eg, coda/segno)
            // measure numbers are not stored in this list, but they should not be cloned anyhow
            // layout breaks other than section were skipped above,
            // but section breaks do need to be cloned & linked
            // other measure-attached elements (?) are cloned but not linked
            if (e->isText() && toText(e)->textStyleType() == TextStyleType::INSTRUMENT_EXCERPT) {
                // skip part name in score
                continue;
            } else if (e->isTextBase() || e->isLayoutBreak()) {
                ne = e->clone();
                ne->setAutoplace(true);
                ne->linkTo(e);
            } else {
                ne = e->clone();
            }
            ne->setScore(score);
            ne->setTrack(track);
            nmb->add(ne);
        }
        nmbl->add(nmb);
    }

    int n = sourceStavesIndexes.size();
    for (int dstStaffIdx = 0; dstStaffIdx < n; ++dstStaffIdx) {
        Staff* srcStaff = oscore->staff(sourceStavesIndexes[dstStaffIdx]);
        Staff* dstStaff = score->staff(dstStaffIdx);

        Measure* m = oscore->firstMeasure();
        Measure* nm = score->firstMeasure();

        while (m && nm) {
            nm->setMeasureRepeatCount(m->measureRepeatCount(srcStaff->idx()), dstStaffIdx);
            m = m->nextMeasure();
            nm = nm->nextMeasure();
        }

        if (srcStaff->isPrimaryStaff()) {
            int span = srcStaff->barLineSpan();
            int sIdx = srcStaff->idx();
            if (dstStaffIdx == 0 && span == 0) {
                // this is first staff of new score,
                // but it was somewhere within a barline span in the old score
                // so, find beginning of span
                for (int i = 0; i <= sIdx; ++i) {
                    span = oscore->staff(i)->barLineSpan();
                    if (i + span > sIdx) {
                        sIdx = i;
                        break;
                    }
                }
            }
            int eIdx = sIdx + span;
            for (int staffIdx = sIdx; staffIdx < eIdx; ++staffIdx) {
                if (!sourceStavesIndexes.contains(staffIdx)) {
                    --span;
                }
            }
            if (dstStaffIdx + span > n) {
                span = n - dstStaffIdx - 1;
            }
            dstStaff->setBarLineSpan(span);
            int idx = 0;
            for (BracketItem* bi : srcStaff->brackets()) {
                dstStaff->setBracketType(idx, bi->bracketType());
                dstStaff->setBracketSpan(idx, bi->bracketSpan());
                ++idx;
            }
        }
    }

    for (auto i : oscore->spanner()) {
        Spanner* s    = i.second;
        int dstTrack  = -1;
        int dstTrack2 = -1;

        if (s->isVolta() || (s->isTextLine() && toTextLine(s)->systemFlag())) {
            //always export voltas to first staff in part
            dstTrack  = 0;
            dstTrack2 = 0;
            cloneSpanner(s, score, dstTrack, dstTrack2);
        } else if (s->isHairpin()) {
            //always export these spanners to first voice of the destination staff

            QList<int> track1;
            for (int ii = s->track(); ii < s->track() + VOICES; ii++) {
                track1 += trackList.values(ii);
            }

            for (int track : qAsConst(track1)) {
                if (!(track % VOICES)) {
                    cloneSpanner(s, score, track, track);
                }
            }
        } else {
            if (trackList.value(s->track(), -1) == -1 || trackList.value(s->track2(), -1) == -1) {
                continue;
            }
            QList<int> track1 = trackList.values(s->track());
            QList<int> track2 = trackList.values(s->track2());

            if (track1.length() != track2.length()) {
                continue;
            }

            //export other spanner if staffidx matches
            for (int ii = 0; ii < track1.length(); ii++) {
                dstTrack = track1.at(ii);
                dstTrack2 = track2.at(ii);
                cloneSpanner(s, score, dstTrack, dstTrack2);
            }
        }
    }
}

//---------------------------------------------------------
//   cloneStaff
//    staves are in same score
//---------------------------------------------------------

void Excerpt::cloneStaff(Staff* srcStaff, Staff* dstStaff)
{
    Score* score = srcStaff->score();
    TieMap tieMap;

    score->undo(new Link(dstStaff, srcStaff));

    int srcStaffIdx = srcStaff->idx();
    int dstStaffIdx = dstStaff->idx();

    for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
        m->setMeasureRepeatCount(m->measureRepeatCount(srcStaffIdx), dstStaffIdx);

        int sTrack = srcStaffIdx * VOICES;
        int eTrack = sTrack + VOICES;
        for (int srcTrack = sTrack; srcTrack < eTrack; ++srcTrack) {
            TupletMap tupletMap;          // tuplets cannot cross measure boundaries
            int dstTrack = dstStaffIdx * VOICES + (srcTrack - sTrack);
            Tremolo* tremolo = 0;
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                EngravingItem* oe = seg->element(srcTrack);
                if (oe == 0 || oe->generated()) {
                    continue;
                }
                if (oe->isTimeSig()) {
                    continue;
                }
                EngravingItem* ne = 0;
                if (oe->isClef()) {
                    // only clone clef if it matches staff group and does not exists yet
                    Clef* clef = toClef(oe);
                    Fraction tick = seg->tick();
                    if (ClefInfo::staffGroup(clef->concertClef()) == dstStaff->constStaffType(Fraction(0, 1))->group()
                        && dstStaff->clefType(tick) != clef->clefTypeList()) {
                        ne = oe->clone();
                    }
                } else {
                    ne = oe->linkedClone();
                }
                if (ne) {
                    ne->setTrack(dstTrack);
                    ne->setParent(seg);
                    ne->setScore(score);
                    if (ne->isChordRest()) {
                        ChordRest* ncr = toChordRest(ne);
                        if (ncr->tuplet()) {
                            ncr->setTuplet(0);               //TODO nested tuplets
                        }
                    }
                    score->undoAddElement(ne);
                }
                if (oe->isChordRest()) {
                    ChordRest* ocr = toChordRest(oe);
                    ChordRest* ncr = toChordRest(ne);
                    Tuplet* ot     = ocr->tuplet();
                    if (ot) {
                        cloneTuplets(ocr, ncr, ot, tupletMap, m, dstTrack);
                    }

                    // remove lyrics from chord
                    // since only one set of lyrics is used with linked staves
                    foreach (Lyrics* l, ncr->lyrics()) {
                        if (l) {
                            l->unlink();
                        }
                    }
                    qDeleteAll(ncr->lyrics());
                    ncr->lyrics().clear();

                    for (EngravingItem* e : seg->annotations()) {
                        if (!e) {
                            qDebug("cloneStaff: corrupted annotation found.");
                            continue;
                        }
                        if (e->generated() || e->systemFlag()) {
                            continue;
                        }
                        if (e->track() != srcTrack) {
                            continue;
                        }
                        switch (e->type()) {
                        // exclude certain element types
                        // this should be same list excluded in Score::undoAddElement()
                        case ElementType::STAFF_TEXT:
                        case ElementType::SYSTEM_TEXT:
                        case ElementType::FRET_DIAGRAM:
                        case ElementType::HARMONY:
                        case ElementType::FIGURED_BASS:
                        case ElementType::DYNAMIC:
                        case ElementType::LYRICS:                     // not normally segment-attached
                            continue;
                        case ElementType::FERMATA:
                        {
                            // Fermatas are special since the belong to a segment but should
                            // be created and linked on each staff.
                            EngravingItem* ne1 = e->linkedClone();
                            ne1->setTrack(dstTrack);
                            ne1->setParent(seg);
                            ne1->setScore(score);
                            score->undo(new AddElement(ne1));
                            continue;
                        }
                        default:
                            if (toTextLine(e)->systemFlag()) {
                                continue;
                            }
                            EngravingItem* ne1 = e->clone();
                            ne1->setTrack(dstTrack);
                            ne1->setParent(seg);
                            ne1->setScore(score);
                            score->undoAddElement(ne1);
                        }
                    }
                    if (oe->isChord()) {
                        Chord* och = toChord(ocr);
                        Chord* nch = toChord(ncr);
                        size_t n = och->notes().size();
                        for (size_t i = 0; i < n; ++i) {
                            Note* on = och->notes().at(i);
                            Note* nn = nch->notes().at(i);
                            if (on->tieFor()) {
                                Tie* tie = toTie(on->tieFor()->linkedClone());
                                tie->setScore(score);
                                nn->setTieFor(tie);
                                tie->setStartNote(nn);
                                tie->setTrack(nn->track());
                                tieMap.add(on->tieFor(), tie);
                            }
                            if (on->tieBack()) {
                                Tie* tie = tieMap.findNew(on->tieBack());
                                if (tie) {
                                    nn->setTieBack(tie);
                                    tie->setEndNote(nn);
                                } else {
                                    qDebug("cloneStaff: cannot find tie");
                                }
                            }
                            // add back spanners (going back from end to start spanner element
                            // makes sure the 'other' spanner anchor element is already set up)
                            // 'on' is the old spanner end note and 'nn' is the new spanner end note
                            for (Spanner* oldSp : on->spannerBack()) {
                                Note* newStart = Spanner::startElementFromSpanner(oldSp, nn);
                                if (newStart != nullptr) {
                                    Spanner* newSp = toSpanner(oldSp->linkedClone());
                                    newSp->setNoteSpan(newStart, nn);
                                    score->addElement(newSp);
                                } else {
                                    qDebug("cloneStaff: cannot find spanner start note");
                                }
                            }
                        }
                        // two note tremolo
                        if (och->tremolo() && och->tremolo()->twoNotes()) {
                            if (och == och->tremolo()->chord1()) {
                                if (tremolo) {
                                    qDebug("unconnected two note tremolo");
                                }
                                tremolo = toTremolo(och->tremolo()->linkedClone());
                                tremolo->setScore(nch->score());
                                tremolo->setParent(nch);
                                tremolo->setTrack(nch->track());
                                tremolo->setChords(nch, 0);
                                nch->setTremolo(tremolo);
                            } else if (och == och->tremolo()->chord2()) {
                                if (!tremolo) {
                                    qDebug("first note for two note tremolo missing");
                                } else {
                                    tremolo->setChords(tremolo->chord1(), nch);
                                    nch->setTremolo(tremolo);
                                }
                            } else {
                                qDebug("inconsistent two note tremolo");
                            }
                        }
                    }
                }
            }
        }
    }

    for (auto i : score->spanner()) {
        Spanner* s = i.second;
        int staffIdx = s->staffIdx();
        int dstTrack = -1;
        int dstTrack2 = -1;
        if (!(s->isVolta() || (s->isTextLine() && toTextLine(s)->systemFlag()))) {
            //export other spanner if staffidx matches
            if (srcStaffIdx == staffIdx) {
                dstTrack = dstStaffIdx * VOICES + s->voice();
                dstTrack2 = dstStaffIdx * VOICES + (s->track2() % VOICES);
            }
        }
        if (dstTrack == -1) {
            continue;
        }
        cloneSpanner(s, score, dstTrack, dstTrack2);
    }
}

//---------------------------------------------------------
//   cloneStaff2
//    staves are potentially in different scores
//---------------------------------------------------------

void Excerpt::cloneStaff2(Staff* srcStaff, Staff* dstStaff, const Fraction& startTick, const Fraction& endTick)
{
    Score* oscore = srcStaff->score();
    Score* score  = dstStaff->score();

    Excerpt* oex = oscore->excerpt();
    Excerpt* ex  = score->excerpt();
    QMultiMap<int, int> otracks, tracks;
    if (oex) {
        otracks = oex->tracks();
    }
    if (ex) {
        tracks  = ex->tracks();
    }

    Measure* m1   = oscore->tick2measure(startTick);
    Measure* m2   = oscore->tick2measure(endTick);

    if (m2->tick() < endTick) { // end of score
        m2 = 0;
    }

    TieMap tieMap;

    int srcStaffIdx = srcStaff->idx();
    int dstStaffIdx = dstStaff->idx();

    int sTrack = srcStaffIdx * VOICES;
    int eTrack = sTrack + VOICES;

    QMap<int, int> map;
    for (int i = sTrack; i < eTrack; i++) {
        if (!oex && !ex) {
            map.insert(i, dstStaffIdx * VOICES + i % VOICES);
        } else if (oex && !ex) {
            if (otracks.key(i, -1) != -1) {
                map.insert(i, otracks.key(i));
            }
        } else if (!oex && ex) {
            for (int j : tracks.values(i)) {
                if (dstStaffIdx * VOICES <= j && j < (dstStaffIdx + 1) * VOICES) {
                    map.insert(i, j);
                    break;
                }
            }
        } else if (oex && ex) {
            if (otracks.key(i, -1) != -1) {
                for (int j : tracks.values(otracks.key(i))) {
                    if (dstStaffIdx * VOICES <= j && j < (dstStaffIdx + 1) * VOICES) {
                        map.insert(i, j);
                        break;
                    }
                }
            }
        }
    }

    if (map.isEmpty()) {
        for (int i = sTrack; i < eTrack; i++) {
            map.insert(i, dstStaffIdx * VOICES + i % VOICES);
        }
    }

    bool firstVoiceVisible = dstStaff->isVoiceVisible(0);

    for (Measure* m = m1; m && (m != m2); m = m->nextMeasure()) {
        Measure* nm = score->tick2measure(m->tick());
        nm->setMeasureRepeatCount(m->measureRepeatCount(srcStaffIdx), dstStaffIdx);
        for (int srcTrack : map.keys()) {
            TupletMap tupletMap;          // tuplets cannot cross measure boundaries
            int dstTrack = map.value(srcTrack);
            for (Segment* oseg = m->first(); oseg; oseg = oseg->next()) {
                Segment* ns = nm->getSegment(oseg->segmentType(), oseg->tick());
                EngravingItem* oef = oseg->element(trackZeroVoice(srcTrack));
                if (oef && !oef->generated() && (oef->isTimeSig() || oef->isKeySig())) {
                    if (!firstVoiceVisible) {
                        EngravingItem* ne = oef->linkedClone();
                        ne->setTrack(trackZeroVoice(dstTrack));
                        ne->setParent(ns);
                        ne->setScore(score);
                        score->undoAddElement(ne);
                    }
                }

                EngravingItem* oe = oseg->element(srcTrack);
                if (oe == 0 || oe->generated()) {
                    continue;
                }

                EngravingItem* ne = oe->linkedClone();
                ne->setTrack(dstTrack);
                ne->setParent(ns);
                ne->setScore(score);
                score->undoAddElement(ne);
                if (oe->isChordRest()) {
                    ChordRest* ocr = toChordRest(oe);
                    ChordRest* ncr = toChordRest(ne);
                    Tuplet* ot     = ocr->tuplet();
                    if (ot) {
                        Tuplet* nt = tupletMap.findNew(ot);
                        if (nt == 0) {
                            // nt = new Tuplet(*ot);
                            nt = toTuplet(ot->linkedClone());
                            nt->clear();
                            nt->setTrack(dstTrack);
                            nt->setParent(m);
                            tupletMap.add(ot, nt);
                        }
                        ncr->setTuplet(nt);
                        nt->add(ncr);
                    }

                    for (EngravingItem* e : oseg->annotations()) {
                        if (e->generated() || e->systemFlag()) {
                            continue;
                        }
                        if (e->track() != srcTrack) {
                            continue;
                        }
                        switch (e->type()) {
                        // exclude certain element types
                        // this should be same list excluded in Score::undoAddElement()
                        case ElementType::STAFF_TEXT:
                        case ElementType::SYSTEM_TEXT:
                        case ElementType::FRET_DIAGRAM:
                        case ElementType::HARMONY:
                        case ElementType::FIGURED_BASS:
                        case ElementType::DYNAMIC:
                        case ElementType::LYRICS:                     // not normally segment-attached
                            continue;
                        default:
                            if (e->isTextLine() && toTextLine(e)->systemFlag()) {
                                continue;
                            }
                            EngravingItem* ne1 = e->clone();
                            ne1->setTrack(dstTrack);
                            ne1->setParent(ns);
                            ne1->setScore(score);
                            score->undoAddElement(ne1);
                        }
                    }
                    if (oe->isChord()) {
                        Chord* och = toChord(ocr);
                        Chord* nch = toChord(ncr);
                        size_t n = och->notes().size();
                        for (size_t i = 0; i < n; ++i) {
                            Note* on = och->notes().at(i);
                            Note* nn = nch->notes().at(i);
                            if (on->tieFor()) {
                                Tie* tie = toTie(on->tieFor()->linkedClone());
                                tie->setScore(score);
                                nn->setTieFor(tie);
                                tie->setStartNote(nn);
                                tie->setTrack(nn->track());
                                tieMap.add(on->tieFor(), tie);
                            }
                            if (on->tieBack()) {
                                Tie* tie = tieMap.findNew(on->tieBack());
                                if (tie) {
                                    nn->setTieBack(tie);
                                    tie->setEndNote(nn);
                                } else {
                                    qDebug("cloneStaff2: cannot find tie");
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    for (auto i : oscore->spanner()) {
        Spanner* s = i.second;
        if (!(s->tick() >= startTick && s->tick2() < endTick)) {
            continue;
        }

        int staffIdx = s->staffIdx();
        int dstTrack = -1;
        int dstTrack2 = -1;
        if (!(s->isVolta() || (s->isTextLine() && s->systemFlag()))) {
            //export other spanner if staffidx matches
            if (srcStaffIdx == staffIdx) {
                dstTrack  = dstStaffIdx * VOICES + s->voice();
                dstTrack2 = dstStaffIdx * VOICES + (s->track2() % VOICES);
            }
        }
        if (dstTrack == -1) {
            continue;
        }
        cloneSpanner(s, score, dstTrack, dstTrack2);
    }
}

QList<Excerpt*> Excerpt::createExcerptsFromParts(const QList<Part*>& parts)
{
    QList<Excerpt*> result;

    for (Part* part : parts) {
        Excerpt* excerpt = new Excerpt(part->masterScore());
        excerpt->parts().append(part);

        for (int i = part->startTrack(), j = 0; i < part->endTrack(); ++i, ++j) {
            excerpt->tracks().insert(i, j);
        }

        QString title = formatTitle(part->partName(), result);
        excerpt->setTitle(title);
        result.append(excerpt);
    }

    return result;
}

Excerpt* Excerpt::createExcerptFromPart(Part* part)
{
    Excerpt* excerpt = createExcerptsFromParts({ part }).first();
    excerpt->setTitle(part->partName());

    return excerpt;
}

QString Excerpt::formatTitle(const QString& partName, const QList<Excerpt*>& excerptList)
{
    QString name = partName.simplified();
    int count = 0;      // no of occurrences of partName

    for (Excerpt* e : excerptList) {
        // if <partName> already exists, change <partName> to <partName 1>
        if (e->title().compare(name) == 0) {
            e->setTitle(e->title() + " 1");
        }

        QRegularExpression regex("^(.+)\\s\\d+$");
        QRegularExpressionMatch match = regex.match(e->title());
        if (match.hasMatch() && match.capturedTexts()[1] == name) {
            count++;
        }
    }

    if (count > 0) {
        name += QString(" %1").arg(count + 1);
    }

    return name;
}

//---------------------------------------------------------
//   setPartScore
//---------------------------------------------------------

void Excerpt::setPartScore(Score* s)
{
    _partScore = s;

    if (s) {
        s->setExcerpt(this);
    }
}
}
