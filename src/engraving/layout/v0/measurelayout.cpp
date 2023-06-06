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
#include "measurelayout.h"

#include "libmscore/ambitus.h"
#include "libmscore/barline.h"
#include "libmscore/beam.h"
#include "libmscore/factory.h"
#include "libmscore/keysig.h"
#include "libmscore/layoutbreak.h"
#include "libmscore/lyrics.h"
#include "libmscore/marker.h"
#include "libmscore/measure.h"
#include "libmscore/measurenumber.h"
#include "libmscore/measurerepeat.h"
#include "libmscore/mmrest.h"
#include "libmscore/mmrestrange.h"
#include "libmscore/ornament.h"
#include "libmscore/part.h"
#include "libmscore/spacer.h"
#include "libmscore/score.h"
#include "libmscore/stafflines.h"
#include "libmscore/system.h"
#include "libmscore/timesig.h"
#include "libmscore/trill.h"
#include "libmscore/undo.h"
#include "libmscore/utils.h"

#include "tlayout.h"
#include "layoutcontext.h"
#include "beamlayout.h"
#include "chordlayout.h"
#include "tremololayout.h"

#include "log.h"

using namespace mu::engraving;
using namespace mu::engraving::layout::v0;

//---------------------------------------------------------
//   layout2
//    called after layout of page
//---------------------------------------------------------

void MeasureLayout::layout2(Measure* item, LayoutContext& ctx)
{
    assert(item->explicitParent());
    assert(item->score()->nstaves() == item->m_mstaves.size());

    double _spatium = item->spatium();

    for (size_t staffIdx = 0; staffIdx < item->score()->nstaves(); ++staffIdx) {
        MStaff* ms = item->m_mstaves[staffIdx];
        Spacer* sp = ms->vspacerDown();
        if (sp) {
            TLayout::layout(sp, ctx);
            Staff* staff = item->score()->staff(staffIdx);
            int n = staff->lines(item->tick()) - 1;
            double y = item->system()->staff(staffIdx)->y();
            sp->setPos(_spatium * .5, y + n * _spatium * staff->staffMag(item->tick()));
        }
        sp = ms->vspacerUp();
        if (sp) {
            TLayout::layout(sp, ctx);
            double y = item->system()->staff(staffIdx)->y();
            sp->setPos(_spatium * .5, y - sp->gap());
        }
    }

    // layout LAYOUT_BREAK elements
    TLayout::layoutMeasureBase(item, ctx);

    //---------------------------------------------------
    //    layout ties
    //---------------------------------------------------

    Fraction stick = item->system()->measures().front()->tick();
    size_t tracks = item->score()->ntracks();
    static const SegmentType st { SegmentType::ChordRest };
    for (track_idx_t track = 0; track < tracks; ++track) {
        if (!item->score()->staff(track / VOICES)->show()) {
            track += VOICES - 1;
            continue;
        }
        for (Segment* seg = item->first(st); seg; seg = seg->next(st)) {
            ChordRest* cr = seg->cr(track);
            if (!cr) {
                continue;
            }

            if (cr->isChord()) {
                Chord* c = toChord(cr);
                ChordLayout::layoutSpanners(c, item->system(), stick, ctx);
            }
        }
    }

    item->layoutCrossStaff();
}

//---------------------------------------------------------
//   createMMRest
//    create a multimeasure rest
//    from firstMeasure to lastMeasure (inclusive)
//---------------------------------------------------------

void MeasureLayout::createMMRest(const LayoutOptions& options, Score* score, Measure* firstMeasure, Measure* lastMeasure,
                                 const Fraction& len)
{
    LayoutContext ctx(score);

    int numMeasuresInMMRest = 1;
    if (firstMeasure != lastMeasure) {
        for (Measure* m = firstMeasure->nextMeasure(); m; m = m->nextMeasure()) {
            ++numMeasuresInMMRest;
            m->setMMRestCount(-1);
            if (m->mmRest()) {
                score->undo(new ChangeMMRest(m, 0));
            }
            if (m == lastMeasure) {
                break;
            }
        }
    }

    // mmrMeasure coexists with n undisplayed measures of rests
    Measure* mmrMeasure = firstMeasure->mmRest();
    if (mmrMeasure) {
        // reuse existing mmrest
        if (mmrMeasure->ticks() != len) {
            Segment* s = mmrMeasure->findSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
            // adjust length
            mmrMeasure->setTicks(len);
            // move existing end barline
            if (s) {
                s->setRtick(len);
            }
        }
        mmrMeasure->removeSystemTrailer();
    } else {
        mmrMeasure = Factory::createMeasure(score->dummy()->system());
        mmrMeasure->setTicks(len);
        mmrMeasure->setTick(firstMeasure->tick());
        score->undo(new ChangeMMRest(firstMeasure, mmrMeasure));
    }
    mmrMeasure->setTimesig(firstMeasure->timesig());
    mmrMeasure->setPageBreak(lastMeasure->pageBreak());
    mmrMeasure->setLineBreak(lastMeasure->lineBreak());
    mmrMeasure->setMMRestCount(numMeasuresInMMRest);
    mmrMeasure->setNo(firstMeasure->no());

    //
    // set mmrMeasure with same barline as last underlying measure
    //
    Segment* lastMeasureEndBarlineSeg = lastMeasure->findSegmentR(SegmentType::EndBarLine, lastMeasure->ticks());
    if (lastMeasureEndBarlineSeg) {
        Segment* mmrEndBarlineSeg = mmrMeasure->undoGetSegmentR(SegmentType::EndBarLine, mmrMeasure->ticks());
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            EngravingItem* e = lastMeasureEndBarlineSeg->element(staffIdx * VOICES);
            if (e) {
                bool generated = e->generated();
                if (!mmrEndBarlineSeg->element(staffIdx * VOICES)) {
                    EngravingItem* eClone = generated ? e->clone() : e->linkedClone();
                    eClone->setGenerated(generated);
                    eClone->setParent(mmrEndBarlineSeg);
                    score->undoAddElement(eClone);
                } else {
                    BarLine* mmrEndBarline = toBarLine(mmrEndBarlineSeg->element(staffIdx * VOICES));
                    BarLine* lastMeasureEndBarline = toBarLine(e);
                    if (!generated && !mmrEndBarline->links()) {
                        score->undo(new Link(mmrEndBarline, lastMeasureEndBarline));
                    }
                    if (mmrEndBarline->barLineType() != lastMeasureEndBarline->barLineType()) {
                        // change directly when generating mmrests, do not change underlying measures or follow links
                        score->undo(new ChangeProperty(mmrEndBarline, Pid::BARLINE_TYPE,
                                                       PropertyValue::fromValue(lastMeasureEndBarline->barLineType()),
                                                       PropertyFlags::NOSTYLE));
                        score->undo(new ChangeProperty(mmrEndBarline, Pid::GENERATED, generated, PropertyFlags::NOSTYLE));
                    }
                }
            }
        }
    }

    //
    // if last underlying measure ends with clef change, show same at end of mmrest
    //
    Segment* lastMeasureClefSeg = lastMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef,
                                                            lastMeasure->ticks());
    if (lastMeasureClefSeg) {
        Segment* mmrClefSeg = mmrMeasure->undoGetSegment(lastMeasureClefSeg->segmentType(), lastMeasure->endTick());
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            const track_idx_t track = staff2track(staffIdx);
            EngravingItem* e = lastMeasureClefSeg->element(track);
            if (e && e->isClef()) {
                Clef* lastMeasureClef = toClef(e);
                if (!mmrClefSeg->element(track)) {
                    Clef* mmrClef = lastMeasureClef->generated() ? lastMeasureClef->clone() : toClef(
                        lastMeasureClef->linkedClone());
                    mmrClef->setParent(mmrClefSeg);
                    score->undoAddElement(mmrClef);
                } else {
                    Clef* mmrClef = toClef(mmrClefSeg->element(track));
                    mmrClef->setClefType(lastMeasureClef->clefType());
                    mmrClef->setShowCourtesy(lastMeasureClef->showCourtesy());
                }
            }
        }
    }

    mmrMeasure->setRepeatStart(firstMeasure->repeatStart() || lastMeasure->repeatStart());
    mmrMeasure->setRepeatEnd(firstMeasure->repeatEnd() || lastMeasure->repeatEnd());
    mmrMeasure->setSectionBreak(lastMeasure->sectionBreak());

    //
    // copy markers to mmrMeasure
    //
    ElementList oldList = mmrMeasure->takeElements();
    ElementList newList = lastMeasure->el();
    for (EngravingItem* e : firstMeasure->el()) {
        if (e->isMarker()) {
            newList.push_back(e);
        }
    }
    for (EngravingItem* e : newList) {
        bool found = false;
        for (EngravingItem* ee : oldList) {
            if (ee->type() == e->type() && ee->subtype() == e->subtype()) {
                mmrMeasure->add(ee);
                auto i = std::find(oldList.begin(), oldList.end(), ee);
                if (i != oldList.end()) {
                    oldList.erase(i);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            mmrMeasure->add(e->clone());
        }
    }
    for (EngravingItem* e : oldList) {
        delete e;
    }
    Segment* s = mmrMeasure->undoGetSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        track_idx_t track = staffIdx * VOICES;
        if (s->element(track) == 0) {
            MMRest* mmr = Factory::createMMRest(s);
            mmr->setDurationType(DurationType::V_MEASURE);
            mmr->setTicks(mmrMeasure->ticks());
            mmr->setTrack(track);
            mmr->setParent(s);
            score->undo(new AddElement(mmr));
        }
    }

    //
    // further check for clefs
    //
    Segment* underlyingSeg = lastMeasure->findSegmentR(SegmentType::Clef, lastMeasure->ticks());
    Segment* mmrSeg = mmrMeasure->findSegment(SegmentType::Clef, lastMeasure->endTick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::Clef, lastMeasure->ticks());
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setTrailer(underlyingSeg->trailer());
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            Clef* clef = toClef(underlyingSeg->element(track));
            if (clef) {
                if (mmrSeg->element(track) == 0) {
                    mmrSeg->add(clef->clone());
                } else {
                    //TODO: check if same clef
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        score->undo(new RemoveElement(mmrSeg));
    }

    //
    // check for time signature
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::TimeSig, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegment(SegmentType::TimeSig, firstMeasure->tick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::TimeSig, Fraction(0, 1));
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setHeader(underlyingSeg->header());
        for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            TimeSig* underlyingTimeSig = toTimeSig(underlyingSeg->element(track));
            if (underlyingTimeSig) {
                TimeSig* mmrTimeSig = toTimeSig(mmrSeg->element(track));
                if (!mmrTimeSig) {
                    mmrTimeSig = underlyingTimeSig->generated() ? underlyingTimeSig->clone() : toTimeSig(
                        underlyingTimeSig->linkedClone());
                    mmrTimeSig->setParent(mmrSeg);
                    score->undo(new AddElement(mmrTimeSig));
                } else {
                    mmrTimeSig->setSig(underlyingTimeSig->sig(), underlyingTimeSig->timeSigType());
                    mmrTimeSig->setNumeratorString(underlyingTimeSig->numeratorString());
                    mmrTimeSig->setDenominatorString(underlyingTimeSig->denominatorString());
                    TLayout::layout(mmrTimeSig, ctx);
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        score->undo(new RemoveElement(mmrSeg));
    }

    //
    // check for ambitus
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::Ambitus, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegment(SegmentType::Ambitus, firstMeasure->tick());
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::Ambitus, Fraction(0, 1));
        }
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            Ambitus* underlyingAmbitus = toAmbitus(underlyingSeg->element(track));
            if (underlyingAmbitus) {
                Ambitus* mmrAmbitus = toAmbitus(mmrSeg->element(track));
                if (!mmrAmbitus) {
                    mmrAmbitus = underlyingAmbitus->clone();
                    mmrAmbitus->setParent(mmrSeg);
                    score->undo(new AddElement(mmrAmbitus));
                } else {
                    mmrAmbitus->initFrom(underlyingAmbitus);
                    TLayout::layout(mmrAmbitus, ctx);
                }
            }
        }
    } else if (mmrSeg) {
        // TODO: remove elements from mmrSeg?
        score->undo(new RemoveElement(mmrSeg));
    }

    //
    // check for key signature
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    mmrSeg = mmrMeasure->findSegmentR(SegmentType::KeySig, Fraction(0, 1));
    if (underlyingSeg) {
        if (mmrSeg == 0) {
            mmrSeg = mmrMeasure->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
        }
        mmrSeg->setEnabled(underlyingSeg->enabled());
        mmrSeg->setHeader(underlyingSeg->header());
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            KeySig* underlyingKeySig  = toKeySig(underlyingSeg->element(track));
            if (underlyingKeySig) {
                KeySig* mmrKeySig = toKeySig(mmrSeg->element(track));
                if (!mmrKeySig) {
                    mmrKeySig = underlyingKeySig->generated() ? underlyingKeySig->clone() : toKeySig(
                        underlyingKeySig->linkedClone());
                    mmrKeySig->setParent(mmrSeg);
                    mmrKeySig->setGenerated(true);
                    score->undo(new AddElement(mmrKeySig));
                } else {
                    if (!(mmrKeySig->keySigEvent() == underlyingKeySig->keySigEvent())) {
                        bool addKey = underlyingKeySig->isChange();
                        score->undo(new ChangeKeySig(mmrKeySig, underlyingKeySig->keySigEvent(), mmrKeySig->showCourtesy(),
                                                     addKey));
                    }
                }
            }
        }
    } else if (mmrSeg) {
        mmrSeg->setEnabled(false);
        // TODO: remove elements from mmrSeg, then delete mmrSeg
        // previously we removed the segment if not empty,
        // but this resulted in "stale" keysig in mmrest after removed from underlying measure
        //undo(new RemoveElement(mmrSeg));
    }

    mmrMeasure->checkHeader();
    mmrMeasure->checkTrailer();

    //
    // check for rehearsal mark etc.
    //
    underlyingSeg = firstMeasure->findSegmentR(SegmentType::ChordRest, Fraction(0, 1));
    if (underlyingSeg) {
        // clone elements from underlying measure to mmr
        for (EngravingItem* e : underlyingSeg->annotations()) {
            // look at elements in underlying measure
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isTripletFeel()
                  || e->isPlayTechAnnotation() || e->isInstrumentChange())) {
                continue;
            }
            // try to find a match in mmr
            bool found = false;
            for (EngravingItem* ee : s->annotations()) {
                if (mu::contains(e->linkList(), static_cast<EngravingObject*>(ee))) {
                    found = true;
                    break;
                }
            }
            // add to mmr if no match found
            if (!found) {
                EngravingItem* eClone = e->linkedClone();
                eClone->setParent(s);
                score->undo(new AddElement(eClone));
            }
        }

        // remove stray elements (possibly leftover from a previous layout of this mmr)
        // this should not happen since the elements are linked?
        const auto annotations = s->annotations(); // make a copy since we alter the list
        for (EngravingItem* e : annotations) { // look at elements in mmr
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isTripletFeel()
                  || e->isPlayTechAnnotation() || e->isInstrumentChange())) {
                continue;
            }
            // try to find a match in underlying measure
            bool found = false;
            for (EngravingItem* ee : underlyingSeg->annotations()) {
                if (mu::contains(e->linkList(), static_cast<EngravingObject*>(ee))) {
                    found = true;
                    break;
                }
            }
            // remove from mmr if no match found
            if (!found) {
                score->undo(new RemoveElement(e));
            }
        }
    }

    MeasureBase* nm = options.showVBox ? lastMeasure->next() : lastMeasure->nextMeasure();
    mmrMeasure->setNext(nm);
    mmrMeasure->setPrev(firstMeasure->prev());
}

//---------------------------------------------------------
// validMMRestMeasure
//    return true if this might be a measure in a
//    multi measure rest
//---------------------------------------------------------

static bool validMMRestMeasure(const LayoutContext& ctx, Measure* m)
{
    if (m->irregular()) {
        return false;
    }

    int n = 0;
    for (Segment* s = m->first(); s; s = s->next()) {
        for (EngravingItem* e : s->annotations()) {
            if (!e->staff()->show()) {
                continue;
            }
            if (!(e->isRehearsalMark() || e->isTempoText() || e->isHarmony() || e->isStaffText() || e->isSystemText() || e->isTripletFeel()
                  || e->isPlayTechAnnotation() || e->isInstrumentChange())) {
                return false;
            }
        }
        if (s->isChordRestType()) {
            bool restFound = false;
            size_t tracks = ctx.score()->ntracks();
            for (track_idx_t track = 0; track < tracks; ++track) {
                if ((track % VOICES) == 0 && !ctx.score()->staff(track / VOICES)->show()) {
                    track += VOICES - 1;
                    continue;
                }
                if (s->element(track)) {
                    if (!s->element(track)->isRest()) {
                        return false;
                    }
                    restFound = true;
                }
            }
            for (EngravingItem* e : s->annotations()) {
                if (e->isFermata()) {
                    return false;
                }
            }
            if (restFound) {
                ++n;
            }
            // measure is not empty if there is more than one rest
            if (n > 1) {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------
//  breakMultiMeasureRest
//    return true if this measure should start a new
//    multi measure rest
//---------------------------------------------------------

static bool breakMultiMeasureRest(const LayoutContext& ctx, Measure* m)
{
    if (m->breakMultiMeasureRest()) {
        return true;
    }

    if (m->repeatStart()
        || (m->prevMeasure() && m->prevMeasure()->repeatEnd())
        || (m->isIrregular())
        || (m->prevMeasure() && m->prevMeasure()->isIrregular())
        || (m->prevMeasure() && (m->prevMeasure()->sectionBreak()))) {
        return true;
    }

    static std::set<ElementType> breakSpannerTypes {
        ElementType::VOLTA,
        ElementType::GRADUAL_TEMPO_CHANGE,
        ElementType::TEXTLINE,
    };
    // Break for spanners/textLines in this measure
    auto sl = ctx.score()->spannerMap().findOverlapping(m->tick().ticks(), m->endTick().ticks());
    for (auto i : sl) {
        Spanner* s = i.value;
        Fraction spannerStart = s->tick();
        Fraction spannerEnd = s->tick2();
        Fraction measureStart = m->tick();
        Fraction measureEnd = m->endTick();
        bool spannerStartsInside = spannerStart >= measureStart && spannerStart < measureEnd;
        bool spannerEndsInside = spannerEnd >= measureStart && spannerEnd < measureEnd;
        if (mu::contains(breakSpannerTypes, s->type()) && (spannerStartsInside || spannerEndsInside)) {
            return true;
        }
    }
    // Break for spanners/textLines starting or ending mid-way inside the *previous* measure
    Measure* prevMeas = m->prevMeasure();
    if (prevMeas) {
        auto prevMeasSpanners = ctx.score()->spannerMap().findOverlapping(prevMeas->tick().ticks(), prevMeas->endTick().ticks());
        for (auto i : prevMeasSpanners) {
            Spanner* s = i.value;
            Fraction spannerStart = s->tick();
            Fraction spannerEnd = s->tick2();
            Fraction measureStart = prevMeas->tick();
            Fraction measureEnd = prevMeas->endTick();
            bool spannerStartsInside = spannerStart > measureStart && spannerStart < measureEnd;
            bool spannerEndsInside = spannerEnd > measureStart && spannerEnd < measureEnd;
            if (mu::contains(breakSpannerTypes, s->type()) && (spannerStartsInside || spannerEndsInside)) {
                return true;
            }
        }
    }

    // break for marker in this measure
    for (EngravingItem* e : m->el()) {
        if (e->isMarker()) {
            Marker* mark = toMarker(e);
            if (!(mark->align() == AlignH::RIGHT)) {
                return true;
            }
        }
    }

    // break for marker & jump in previous measure
    Measure* pm = m->prevMeasure();
    if (pm) {
        for (EngravingItem* e : pm->el()) {
            if (e->isJump()) {
                return true;
            } else if (e->isMarker()) {
                Marker* mark = toMarker(e);
                if (mark->align() == AlignH::RIGHT) {
                    return true;
                }
            }
        }
    }

    // break for MeasureRepeat group
    for (size_t staffIdx = 0; staffIdx < ctx.score()->nstaves(); ++staffIdx) {
        if (m->isMeasureRepeatGroup(staffIdx)
            || (m->prevMeasure() && m->prevMeasure()->isMeasureRepeatGroup(staffIdx))) {
            return true;
        }
    }

    static std::set<ElementType> alwaysBreakTypes {
        ElementType::TEMPO_TEXT,
        ElementType::REHEARSAL_MARK
    };
    static std::set<ElementType> conditionalBreakTypes {
        ElementType::HARMONY,
        ElementType::STAFF_TEXT,
        ElementType::SYSTEM_TEXT,
        ElementType::TRIPLET_FEEL,
        ElementType::PLAYTECH_ANNOTATION,
        ElementType::INSTRUMENT_CHANGE
    };

    auto breakForAnnotation = [&](EngravingItem* e) {
        if (mu::contains(alwaysBreakTypes, e->type())) {
            return true;
        }
        bool breakForElement = e->systemFlag() || e->staff()->show();
        if (mu::contains(conditionalBreakTypes, e->type()) && breakForElement) {
            return true;
        }
        return false;
    };

    // Break for annotations found mid-way into the previous measure
    if (prevMeas) {
        for (Segment* s = prevMeas->first(); s; s = s->next()) {
            for (EngravingItem* e : s->annotations()) {
                if (!e->visible()) {
                    continue;
                }
                bool isInMidMeasure = e->rtick() > Fraction(0, 1);
                if (!isInMidMeasure) {
                    continue;
                }
                if (breakForAnnotation(e)) {
                    return true;
                }
            }
        }
    }

    for (Segment* s = m->first(); s; s = s->next()) {
        // Break for annotations in this measure
        for (EngravingItem* e : s->annotations()) {
            if (!e->visible()) {
                continue;
            }
            if (breakForAnnotation(e)) {
                return true;
            }
        }
        for (size_t staffIdx = 0; staffIdx < ctx.score()->nstaves(); ++staffIdx) {
            if (!ctx.score()->staff(staffIdx)->show()) {
                continue;
            }
            EngravingItem* e = s->element(staffIdx * VOICES);
            if (!e || e->generated()) {
                continue;
            }
            if (s->isStartRepeatBarLineType()) {
                return true;
            }
            if (s->isType(SegmentType::KeySig | SegmentType::TimeSig) && m->tick().isNotZero()) {
                return true;
            }
            if (s->isClefType()) {
                if (s->tick() != m->endTick() && m->tick().isNotZero()) {
                    return true;
                }
            }
        }
    }
    if (pm) {
        Segment* s = pm->findSegmentR(SegmentType::EndBarLine, pm->ticks());
        if (s) {
            for (size_t staffIdx = 0; staffIdx < ctx.score()->nstaves(); ++staffIdx) {
                BarLine* bl = toBarLine(s->element(staffIdx * VOICES));
                if (bl) {
                    BarLineType t = bl->barLineType();
                    if (t != BarLineType::NORMAL && t != BarLineType::BROKEN && t != BarLineType::DOTTED && !bl->generated()) {
                        return true;
                    } else {
                        break;
                    }
                }
            }
        }
        if (pm->findSegment(SegmentType::Clef, m->tick())) {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------
//   layoutDrumsetChord
//---------------------------------------------------------

static void layoutDrumsetChord(Chord* c, const Drumset* drumset, const StaffType* st, double spatium)
{
    for (Note* note : c->notes()) {
        int pitch = note->pitch();
        if (!drumset->isValid(pitch)) {
            // LOGD("unmapped drum note %d", pitch);
        } else if (!note->fixed()) {
            note->undoChangeProperty(Pid::HEAD_GROUP, int(drumset->noteHead(pitch)));
            int line = drumset->line(pitch);
            note->setLine(line);

            int off  = st->stepOffset();
            double ld = st->lineDistance().val();
            note->setPosY((line + off * 2.0) * spatium * .5 * ld);
        }
    }
}

void MeasureLayout::getNextMeasure(const LayoutOptions& options, LayoutContext& ctx)
{
    Score* score = ctx.score();
    ctx.prevMeasure = ctx.curMeasure;
    ctx.curMeasure  = ctx.nextMeasure;
    if (!ctx.curMeasure) {
        ctx.nextMeasure = options.showVBox ? score->first() : score->firstMeasure();
    } else {
        ctx.nextMeasure = options.showVBox ? ctx.curMeasure->next() : ctx.curMeasure->nextMeasure();
    }
    if (!ctx.curMeasure) {
        return;
    }

    int mno = adjustMeasureNo(ctx, ctx.curMeasure);

    if (ctx.curMeasure->isMeasure()) {
        if (ctx.score()->styleB(Sid::createMultiMeasureRests)) {
            Measure* m = toMeasure(ctx.curMeasure);
            Measure* nm = m;
            Measure* lm = nm;
            int n       = 0;
            Fraction len;

            while (validMMRestMeasure(ctx, nm)) {
                MeasureBase* mb = options.showVBox ? nm->next() : nm->nextMeasure();
                if (breakMultiMeasureRest(ctx, nm) && n) {
                    break;
                }
                if (nm != m) {
                    adjustMeasureNo(ctx, nm);
                }
                ++n;
                len += nm->ticks();
                lm = nm;
                if (!(mb && mb->isMeasure())) {
                    break;
                }
                nm = toMeasure(mb);
            }
            if (n >= score->styleI(Sid::minEmptyMeasures)) {
                createMMRest(options, score, m, lm, len);
                ctx.curMeasure  = m->mmRest();
                ctx.nextMeasure = options.showVBox ? lm->next() : lm->nextMeasure();
            } else {
                if (m->mmRest()) {
                    score->undo(new ChangeMMRest(m, 0));
                }
                m->setMMRestCount(0);
                ctx.measureNo = mno;
            }
        } else if (toMeasure(ctx.curMeasure)->isMMRest()) {
            LOGD("mmrest: no %d += %d", ctx.measureNo, toMeasure(ctx.curMeasure)->mmRestCount());
            ctx.measureNo += toMeasure(ctx.curMeasure)->mmRestCount() - 1;
        }
    }
    if (!ctx.curMeasure->isMeasure()) {
        ctx.curMeasure->setTick(ctx.tick);
        return;
    }

    //-----------------------------------------
    //    process one measure
    //-----------------------------------------

    Measure* measure = toMeasure(ctx.curMeasure);
    measure->moveTicks(ctx.tick - measure->tick());

    if (score->linearMode() && (measure->tick() < ctx.startTick || measure->tick() > ctx.endTick)) {
        // needed to reset segment widths if they can change after measure width is computed
        //for (Segment& s : measure->segments())
        //      s.createShapes();
        ctx.tick += measure->ticks();
        return;
    }

    measure->connectTremolo();

    //
    // calculate accidentals and note lines,
    // create stem and set stem direction
    //
    for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        const Staff* staff     = score->Score::staff(staffIdx);
        const Drumset* drumset
            = staff->part()->instrument(measure->tick())->useDrumset() ? staff->part()->instrument(measure->tick())->drumset() : 0;
        AccidentalState as;          // list of already set accidentals for this measure
        as.init(staff->keySigEvent(measure->tick()));

        // Trills may carry an accidental into this measure that requires a force-restate
        int ticks = measure->tick().ticks();
        auto spanners = score->spannerMap().findOverlapping(ticks, ticks, true);
        for (auto iter : spanners) {
            Spanner* spanner = iter.value;
            if (spanner->staffIdx() != staffIdx || !spanner->isTrill() || spanner->tick2() == measure->tick()) {
                continue;
            }
            Ornament* ornament = toTrill(spanner)->ornament();
            Note* trillNote = ornament ? ornament->noteAbove() : nullptr;
            if (trillNote && trillNote->accidental() && ornament->showAccidental() == OrnamentShowAccidental::DEFAULT) {
                int line = absStep(trillNote->tpc(), trillNote->epitch());
                as.setForceRestateAccidental(line, true);
            }
        }

        for (Segment& segment : measure->segments()) {
            // TODO? maybe we do need to process it here to make it possible to enable later
            //if (!segment.enabled())
            //      continue;
            if (segment.isKeySigType()) {
                KeySig* ks = toKeySig(segment.element(staffIdx * VOICES));
                if (!ks) {
                    continue;
                }
                Fraction tick = segment.tick();
                as.init(staff->keySigEvent(tick));
                TLayout::layout(ks, ctx);
            } else if (segment.isChordRestType()) {
                const StaffType* st = staff->staffTypeForElement(&segment);
                track_idx_t track     = staffIdx * VOICES;
                track_idx_t endTrack  = track + VOICES;

                for (track_idx_t t = track; t < endTrack; ++t) {
                    ChordRest* cr = segment.cr(t);
                    if (!cr) {
                        continue;
                    }
                    // Check if requested cross-staff is possible
                    if (cr->staffMove() || cr->storedStaffMove()) {
                        cr->checkStaffMoveValidity();
                    }

                    double m = staff->staffMag(&segment);
                    if (cr->isSmall()) {
                        m *= score->styleD(Sid::smallNoteMag);
                    }

                    if (cr->isChord()) {
                        Chord* chord = toChord(cr);
                        chord->cmdUpdateNotes(&as);
                        for (Chord* c : chord->graceNotes()) {
                            c->setMag(m * score->styleD(Sid::graceNoteMag));
                            c->setTrack(t);
                            ChordLayout::computeUp(c, ctx);
                            if (drumset) {
                                layoutDrumsetChord(c, drumset, st, score->spatium());
                            }
                            ChordLayout::layoutStem(c, ctx);
                            c->setBeamlet(nullptr); // Will be defined during beam layout
                        }
                        if (drumset) {
                            layoutDrumsetChord(chord, drumset, st, score->spatium());
                        }

                        ChordLayout::computeUp(chord, ctx);
                        ChordLayout::layoutStem(chord, ctx); // create stems needed to calculate spacing
                                                             // stem direction can change later during beam processing
                    }
                    cr->setMag(m);
                    cr->setBeamlet(nullptr); // Will be defined during beam layout
                }
            } else if (segment.isClefType()) {
                EngravingItem* e = segment.element(staffIdx * VOICES);
                if (e) {
                    toClef(e)->setSmall(true);
                    TLayout::layoutItem(e, ctx);
                }
            } else if (segment.isType(SegmentType::TimeSig | SegmentType::Ambitus | SegmentType::HeaderClef)) {
                EngravingItem* e = segment.element(staffIdx * VOICES);
                if (e) {
                    TLayout::layoutItem(e, ctx);
                }
            }
        }
    }

    BeamLayout::createBeams(score, ctx, measure);
    /* HACK: The real beam layout is computed at much later stage (you can't do the beams until you know
     * horizontal spacing). However, horizontal spacing needs to know stems extensions to avoid collision
     * with stems, and stems extensions depend on beams. Solution: we compute dummy beams here, *before*
     * horizontal spacing. It is pointless for the beams themselves, but it *does* correctly extend the
     * stems, thus allowing to compute horizontal spacing correctly. (M.S.) */
    for (Segment& s : measure->segments()) {
        if (!s.isChordRestType()) {
            continue;
        }
        BeamLayout::layoutNonCrossBeams(&s, ctx);
    }

    for (staff_idx_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
        for (Segment& segment : measure->segments()) {
            if (segment.isChordRestType()) {
                ChordLayout::layoutChords1(score, &segment, staffIdx, ctx);
                ChordLayout::resolveVerticalRestConflicts(score, &segment, staffIdx, ctx);
                for (voice_idx_t voice = 0; voice < VOICES; ++voice) {
                    ChordRest* cr = segment.cr(staffIdx * VOICES + voice);
                    if (cr) {
                        for (Lyrics* l : cr->lyrics()) {
                            if (l) {
                                TLayout::layout(l, ctx);
                            }
                        }
                    }
                }
            }
        }
    }

    for (Segment& segment : measure->segments()) {
        if (segment.isBreathType()) {
            for (EngravingItem* e : segment.elist()) {
                if (e && e->isBreath()) {
                    TLayout::layoutItem(e, ctx);
                }
            }
        } else if (segment.isChordRestType()) {
            for (EngravingItem* e : segment.annotations()) {
                if (e->isSymbol()) {
                    TLayout::layoutItem(e, ctx);
                }
            }
        }
    }

    Segment* seg = measure->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
    if (measure->repeatStart()) {
        if (!seg) {
            seg = measure->getSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
        }
        barLinesSetSpan(measure, seg, ctx);          // this also creates necessary barlines
        for (size_t staffIdx = 0; staffIdx < score->nstaves(); ++staffIdx) {
            BarLine* b = toBarLine(seg->element(staffIdx * VOICES));
            if (b) {
                b->setBarLineType(BarLineType::START_REPEAT);
                TLayout::layout(b, ctx);
            }
        }
    } else if (seg) {
        score->undoRemoveElement(seg);
    }

    for (Segment& s : measure->segments()) {
        if (s.isEndBarLineType()) {
            continue;
        }
        s.createShapes();
    }

    ChordLayout::updateGraceNotes(measure, ctx);

    measure->computeTicks(); // Must be called *after* Segment::createShapes() because it relies on the
    // Segment::visible() property, which is determined by Segment::createShapes().

    ctx.tick += measure->ticks();
}

//---------------------------------------------------------
//   adjustMeasureNo
//---------------------------------------------------------

int MeasureLayout::adjustMeasureNo(LayoutContext& lc, MeasureBase* m)
{
    lc.measureNo += m->noOffset();
    m->setNo(lc.measureNo);
    if (!m->irregular()) {          // don’t count measure
        ++lc.measureNo;
    }

    const LayoutBreak* layoutBreak = m->sectionBreakElement();
    if (layoutBreak && layoutBreak->startWithMeasureOne()) {
        lc.measureNo = 0;
    }

    return lc.measureNo;
}

/****************************************************************
 * computePreSpacingItems
 * Computes information that is needed before horizontal spacing.
 * Caution: assumes that the system is known! (which is why we
 * cannot compute this stuff in LayoutMeasure::getNextMeasure().)
 * **************************************************************/
void MeasureLayout::computePreSpacingItems(Measure* m, LayoutContext& ctx)
{
    // Compute chord properties
    bool isFirstChordInMeasure = true;
    ChordLayout::clearLineAttachPoints(m);
    for (Segment& seg : m->segments()) {
        if (!seg.isChordRestType()) {
            continue;
        }
        for (EngravingItem* e : seg.elist()) {
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* chord = toChord(e);

            ChordLayout::updateLineAttachPoints(chord, isFirstChordInMeasure, ctx);
            for (Chord* gn : chord->graceNotes()) {
                ChordLayout::updateLineAttachPoints(gn, false, ctx);
            }

            ChordLayout::layoutArticulations(chord, ctx);
            chord->checkStartEndSlurs();
            chord->computeKerningExceptions();
        }
        seg.createShapes();
        isFirstChordInMeasure = false;
    }
}

void MeasureLayout::layoutStaffLines(Measure* m, LayoutContext& ctx)
{
    int staffIdx = 0;
    for (MStaff* ms : m->m_mstaves) {
        if (m->isCutawayClef(staffIdx) && (m->score()->staff(staffIdx)->cutaway() || !m->visible(staffIdx))) {
            // draw short staff lines for a courtesy clef on a hidden measure
            Segment* clefSeg = m->findSegmentR(SegmentType::Clef, m->ticks());
            double staffMag = m->score()->staff(staffIdx)->staffMag(m->tick());
            double partialWidth = clefSeg
                                  ? m->width() - clefSeg->x() + clefSeg->minLeft() + m->score()->styleMM(Sid::clefLeftMargin) * staffMag
                                  : 0.0;
            ms->lines()->layoutPartialWidth(m->width(), partialWidth / (m->spatium() * staffMag), true);
        } else {
            // normal staff lines
            TLayout::layout(ms->lines(), ctx);
        }
        staffIdx += 1;
    }
}

void MeasureLayout::layoutMeasureNumber(Measure* m, LayoutContext& ctx)
{
    bool smn = m->showsMeasureNumber();

    String s;
    if (smn) {
        s = String::number(m->no() + 1);
    }

    unsigned nn = 1;
    bool nas = m->score()->styleB(Sid::measureNumberAllStaves);

    if (!nas) {
        //find first non invisible staff
        for (unsigned staffIdx = 0; staffIdx < m->m_mstaves.size(); ++staffIdx) {
            if (m->visible(staffIdx)) {
                nn = staffIdx;
                break;
            }
        }
    }
    for (unsigned staffIdx = 0; staffIdx < m->m_mstaves.size(); ++staffIdx) {
        MStaff* ms       = m->m_mstaves[staffIdx];
        MeasureNumber* t = ms->noText();
        if (t) {
            t->setTrack(staffIdx * VOICES);
        }
        if (smn && ((staffIdx == nn) || nas)) {
            if (t == 0) {
                t = new MeasureNumber(m);
                t->setTrack(staffIdx * VOICES);
                t->setGenerated(true);
                t->setParent(m);
                m->add(t);
            }
            t->setXmlText(s);
            TLayout::layout(t, ctx);
        } else {
            if (t) {
                if (t->generated()) {
                    m->score()->removeElement(t);
                } else {
                    m->score()->undo(new RemoveElement(t));
                }
            }
        }
    }
}

void MeasureLayout::layoutMMRestRange(Measure* m, LayoutContext& ctx)
{
    if (!m->isMMRest() || !m->score()->styleB(Sid::mmRestShowMeasureNumberRange)) {
        // Remove existing
        for (unsigned staffIdx = 0; staffIdx < m->m_mstaves.size(); ++staffIdx) {
            MStaff* ms = m->m_mstaves[staffIdx];
            MMRestRange* rr = ms->mmRangeText();
            if (rr) {
                if (rr->generated()) {
                    m->score()->removeElement(rr);
                } else {
                    m->score()->undo(new RemoveElement(rr));
                }
            }
        }

        return;
    }

    String s;
    if (m->mmRestCount() > 1) {
        // middle char is an en dash (not em)
        s = String(u"%1–%2").arg(m->no() + 1).arg(m->no() + m->mmRestCount());
    } else {
        // If the minimum range to create a mmrest is set to 1,
        // then simply show the measure number as there is no range
        s = String::number(m->no() + 1);
    }

    for (unsigned staffIdx = 0; staffIdx < m->m_mstaves.size(); ++staffIdx) {
        MStaff* ms = m->m_mstaves[staffIdx];
        MMRestRange* rr = ms->mmRangeText();
        if (!rr) {
            rr = new MMRestRange(m);
            rr->setTrack(staffIdx * VOICES);
            rr->setGenerated(true);
            rr->setParent(m);
            m->add(rr);
        }
        // setXmlText is reimplemented to take care of brackets
        rr->setXmlText(s);
        TLayout::layout(rr, ctx);
    }
}

//-----------------------------------------------------------------------------
//  layoutMeasureElements()
//  lays out all the element of a measure
//  LEGACY: this method used to be called stretchMeasure() and was used to
//  distribute the remaining space at the end of a system. That task is now
//  performed elsewhere and only the layout tasks are kept.
//-----------------------------------------------------------------------------

void MeasureLayout::layoutMeasureElements(Measure* m, LayoutContext& ctx)
{
    //---------------------------------------------------
    //    layout individual elements
    //---------------------------------------------------

    for (Segment& s : m->m_segments) {
        if (!s.enabled()) {
            continue;
        }

        // After the rest of the spacing is calculated we position grace-notes-after.
        ChordLayout::repositionGraceNotesAfter(&s);

        for (EngravingItem* e : s.elist()) {
            if (!e) {
                continue;
            }
            staff_idx_t staffIdx = e->staffIdx();
            bool modernMMRest = e->isMMRest() && !m->score()->styleB(Sid::oldStyleMultiMeasureRests);
            if ((e->isRest() && toRest(e)->isFullMeasureRest()) || e->isMMRest() || e->isMeasureRepeat()) {
                //
                // element has to be centered in free space
                //    x1 - left measure position of free space
                //    x2 - right measure position of free space

                Segment* s1;
                for (s1 = s.prevActive(); s1 && s1->allElementsInvisible(); s1 = s1->prevActive()) {
                }
                Segment* s2;
                if (modernMMRest) {
                    for (s2 = s.nextActive(); s2; s2 = s2->nextActive()) {
                        if (!s2->isChordRestType() && s2->element(staffIdx * VOICES)) {
                            break;
                        }
                    }
                } else {
                    // Ignore any stuff before the end bar line (such as courtesy clefs)
                    s2 = m->findSegment(SegmentType::EndBarLine, m->endTick());
                }

                double x1 = s1 ? s1->x() + s1->minRight() : 0;
                double x2 = s2 ? s2->x() - s2->minLeft() : m->width();

                if (e->isMMRest()) {
                    MMRest* mmrest = toMMRest(e);
                    // center multimeasure rest
                    double d = m->score()->styleMM(Sid::multiMeasureRestMargin);
                    double w = x2 - x1 - 2 * d;
                    bool headerException = m->header() && s.prev() && !s.prev()->isStartRepeatBarLineType();
                    if (headerException) { //needs this exception on header bar
                        x1 = s1 ? s1->x() + s1->width() : 0;
                        w = x2 - x1 - d;
                    }
                    mmrest->setWidth(w);
                    TLayout::layout(mmrest, ctx);
                    mmrest->setPosX(headerException ? (x1 - s.x()) : (x1 - s.x() + d));
                } else if (e->isMeasureRepeat() && !(toMeasureRepeat(e)->numMeasures() % 2)) {
                    // two- or four-measure repeat, center on following barline
                    double measureWidth = x2 - s.x() + .5 * (m->styleP(Sid::barWidth));
                    e->setPosX(measureWidth - .5 * e->width());
                } else {
                    // full measure rest or one-measure repeat, center within this measure
                    TLayout::layoutItem(e, ctx);
                    e->setPosX((x2 - x1 - e->width()) * .5 + x1 - s.x() - e->bbox().x());
                }
                s.createShape(staffIdx);            // DEBUG
            } else if (e->isRest()) {
                e->setPosX(0);
            } else if (e->isChord()) {
                Chord* c = toChord(e);
                if (c->tremolo()) {
                    Tremolo* tr = c->tremolo();
                    Chord* c1 = tr->chord1();
                    Chord* c2 = tr->chord2();
                    if (!tr->twoNotes() || (c1 && !c1->staffMove() && c2 && !c2->staffMove())) {
                        TLayout::layout(tr, ctx);
                    }
                }
                for (Chord* g : c->graceNotes()) {
                    if (g->tremolo()) {
                        Tremolo* tr = g->tremolo();
                        Chord* c1 = tr->chord1();
                        Chord* c2 = tr->chord2();
                        if (!tr->twoNotes() || (c1 && !c1->staffMove() && c2 && !c2->staffMove())) {
                            TLayout::layout(tr, ctx);
                        }
                    }
                }
            } else if (e->isBarLine()) {
                e->setPosY(0.0);
                // for end barlines, x position was set in createEndBarLines
                if (s.segmentType() != SegmentType::EndBarLine) {
                    e->setPosX(0.0);
                }
            }
        }
    }
}

void MeasureLayout::barLinesSetSpan(Measure* m, Segment* seg, LayoutContext& ctx)
{
    int track = 0;
    for (Staff* staff : m->score()->staves()) {
        BarLine* bl = toBarLine(seg->element(track));      // get existing bar line for this staff, if any
        if (bl) {
            if (bl->generated()) {
                bl->setSpanStaff(staff->barLineSpan());
                bl->setSpanFrom(staff->barLineFrom());
                bl->setSpanTo(staff->barLineTo());
            }
        } else {
            bl = Factory::createBarLine(seg);
            bl->setParent(seg);
            bl->setTrack(track);
            bl->setGenerated(true);
            bl->setSpanStaff(staff->barLineSpan());
            bl->setSpanFrom(staff->barLineFrom());
            bl->setSpanTo(staff->barLineTo());
            TLayout::layout(bl, ctx);
            m->score()->addElement(bl);
        }
        track += VOICES;
    }
}

//---------------------------------------------------------
//   createEndBarLines
//    actually creates or modifies barlines
//    return the width change for measure
//---------------------------------------------------------

double MeasureLayout::createEndBarLines(Measure* m, bool isLastMeasureInSystem, LayoutContext& ctx)
{
    size_t nstaves  = m->score()->nstaves();
    Segment* seg = m->findSegmentR(SegmentType::EndBarLine, m->ticks());
    Measure* nm  = m->nextMeasure();
    double blw    = 0.0;

    double oldWidth = m->width();

    if (nm && nm->repeatStart() && !m->repeatEnd() && !isLastMeasureInSystem && m->next() == nm) {
        // we may skip barline at end of a measure immediately before a start repeat:
        // next measure is repeat start, this measure is not a repeat end,
        // this is not last measure of system, no intervening frame
        if (!seg) {
            return 0.0;
        }
        seg->setEnabled(false);
    } else {
        BarLineType t = nm ? BarLineType::NORMAL : BarLineType::END;
        if (!seg) {
            seg = m->getSegmentR(SegmentType::EndBarLine, m->ticks());
        }
        seg->setEnabled(true);
        //
        //  Set flag "hasCourtesyKeySig" if this measure needs a courtesy key sig.
        //  This flag is later used to set a double end bar line and to actually
        //  create the courtesy key sig.
        //

        bool show = m->score()->styleB(Sid::genCourtesyKeysig) && !m->sectionBreak() && nm;

        m->setHasCourtesyKeySig(false);

        if (isLastMeasureInSystem && show) {
            Fraction tick = m->endTick();
            for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                Staff* staff     = m->score()->staff(staffIdx);
                KeySigEvent key1 = staff->keySigEvent(tick - Fraction::fromTicks(1));
                KeySigEvent key2 = staff->keySigEvent(tick);
                if (!(key1 == key2)) {
                    // locate a key sig. in next measure and, if found,
                    // check if it has court. sig turned off
                    Segment* s = nm->findSegment(SegmentType::KeySig, tick);
                    if (s) {
                        KeySig* ks = toKeySig(s->element(staffIdx * VOICES));
                        if (ks && !ks->showCourtesy()) {
                            continue;
                        }
                    }
                    m->setHasCourtesyKeySig(true);
                    t = BarLineType::DOUBLE;
                    break;
                }
            }
        }

        bool force = false;
        if (m->repeatEnd()) {
            t = BarLineType::END_REPEAT;
            force = true;
        } else if (isLastMeasureInSystem && m->nextMeasure() && m->nextMeasure()->repeatStart()) {
            t = BarLineType::NORMAL;
//                  force = true;
        }

        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            BarLine* bl  = toBarLine(seg->element(track));
            Staff* staff = m->score()->staff(staffIdx);
            if (!bl) {
                bl = Factory::createBarLine(seg);
                bl->setParent(seg);
                bl->setTrack(track);
                bl->setGenerated(true);
                bl->setSpanStaff(staff->barLineSpan());
                bl->setSpanFrom(staff->barLineFrom());
                bl->setSpanTo(staff->barLineTo());
                bl->setBarLineType(t);
                m->score()->addElement(bl);
            } else {
                // do not change bar line type if bar line is user modified
                // and its not a repeat start/end barline (forced)

                if (bl->generated()) {
                    bl->setSpanStaff(staff->barLineSpan());
                    bl->setSpanFrom(staff->barLineFrom());
                    bl->setSpanTo(staff->barLineTo());
                    bl->setBarLineType(t);
                } else {
                    if (bl->barLineType() != t) {
                        if (force) {
                            bl->undoChangeProperty(Pid::BARLINE_TYPE, PropertyValue::fromValue(t));
                            bl->setGenerated(true);
                        }
                    }
                }
            }

            TLayout::layout(bl, ctx);
            blw = std::max(blw, bl->width());
        }
        // right align within segment
        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            track_idx_t track = staffIdx * VOICES;
            BarLine* bl = toBarLine(seg->element(track));
            if (bl) {
                bl->movePosX(blw - bl->width());
            }
        }
        seg->createShapes();
    }

    // set relative position of end barline and clef
    // if end repeat, clef goes after, otherwise clef goes before
    Segment* clefSeg = m->findSegmentR(SegmentType::Clef, m->ticks());
    ClefToBarlinePosition clefToBarlinePosition = ClefToBarlinePosition::AUTO;
    if (clefSeg) {
        bool wasVisible = clefSeg->visible();
        int visibleInt = 0;
        for (staff_idx_t staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!m->score()->staff(staffIdx)->show()) {
                continue;
            }
            track_idx_t track = staffIdx * VOICES;
            Clef* clef = toClef(clefSeg->element(track));
            if (clef) {
                clefToBarlinePosition = clef->clefToBarlinePosition();
                bool showCourtesy = m->score()->genCourtesyClef() && clef->showCourtesy();         // normally show a courtesy clef
                // check if the measure is the last measure of the system or the last measure before a frame
                bool lastMeasure = isLastMeasureInSystem || (nm ? !(m->next() == nm) : true);
                if (!nm || m->isFinalMeasureOfSection() || (lastMeasure && !showCourtesy)) {
                    // hide the courtesy clef in the final measure of a section, or if the measure is the final measure of a system
                    // and the score style or the clef style is set to "not show courtesy clef",
                    // or if the clef is at the end of the very last measure of the score
                    clef->clear();
                    clefSeg->createShape(staffIdx);
                    if (visibleInt == 0) {
                        visibleInt = 1;
                    }
                } else {
                    TLayout::layout(clef, ctx);
                    clefSeg->createShape(staffIdx);
                    visibleInt = 2;
                }
            }
        }
        if (visibleInt == 2) {         // there is at least one visible clef in the clef segment
            clefSeg->setVisible(true);
        } else if (visibleInt == 1) {  // all (courtesy) clefs in the clef segment are not visible
            clefSeg->setVisible(false);
        } else { // should never happen
            LOGD("Clef Segment without Clef elements at tick %d/%d", clefSeg->tick().numerator(), clefSeg->tick().denominator());
        }
        if ((wasVisible != clefSeg->visible()) && m->system()) {   // recompute the width only if necessary
            m->computeWidth(m->system()->minSysTicks(), m->system()->maxSysTicks(), m->layoutStretch());
        }
        if (seg) {
            Segment* s1;
            Segment* s2;
            if (m->repeatEnd() && clefToBarlinePosition != ClefToBarlinePosition::BEFORE) {
                s1 = seg;
                s2 = clefSeg;
            } else {
                s1 = clefSeg;
                s2 = seg;
            }
            if (s1->next() != s2) {
                m->m_segments.remove(s1);
                m->m_segments.insert(s1, s2);
            }
        }
    }

    // fix segment layout
    Segment* s = seg->prevActive();
    if (s) {
        double x = s->xpos();
        m->computeWidth(s, x, false, m->system()->minSysTicks(), m->system()->maxSysTicks(), m->layoutStretch());
    }

    return m->width() - oldWidth;
}
