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
#include "layout.h"

#include "libmscore/score.h"
#include "libmscore/masterscore.h"
#include "libmscore/measure.h"
#include "libmscore/scorefont.h"
#include "libmscore/bracket.h"
#include "libmscore/chordrest.h"
#include "libmscore/box.h"
#include "libmscore/marker.h"
#include "libmscore/barline.h"
#include "libmscore/undo.h"
#include "libmscore/part.h"
#include "libmscore/keysig.h"
#include "libmscore/chord.h"
#include "libmscore/stem.h"
#include "libmscore/lyrics.h"
#include "libmscore/measurenumber.h"
#include "libmscore/fingering.h"
#include "libmscore/mmrestrange.h"
#include "libmscore/stafflines.h"
#include "libmscore/tuplet.h"
#include "libmscore/tie.h"

#include "layoutcontext.h"
#include "layoutmeasure.h"
#include "layoutsystem.h"

using namespace mu::engraving;
using namespace Ms;
//---------------------------------------------------------
//   CmdStateLocker
//---------------------------------------------------------

class CmdStateLocker
{
    Score* m_score = nullptr;
public:
    CmdStateLocker(Score* s)
        : m_score(s) { m_score->cmdState().lock(); }
    ~CmdStateLocker() { m_score->cmdState().unlock(); }
};

//---------------------------------------------------------
//   almostZero
//---------------------------------------------------------

static bool inline almostZero(qreal value)
{
    // 1e-3 is close enough to zero to see it as zero.
    return value > -1e-3 && value < 1e-3;
}

Layout::Layout(Ms::Score* score)
    : m_score(score)
{
}

void Layout::doLayoutRange(const Fraction& st, const Fraction& et)
{
    CmdStateLocker cmdStateLocker(m_score);
    LayoutContext lc(m_score);

    Fraction stick(st);
    Fraction etick(et);
    Q_ASSERT(!(stick == Fraction(-1, 1) && etick == Fraction(-1, 1)));

    if (!m_score->last() || (m_score->lineMode() && !m_score->firstMeasure())) {
        qDebug("empty score");
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();
        qDeleteAll(m_score->pages());
        m_score->pages().clear();
        lc.getNextPage();
        return;
    }
//      if (!_systems.isEmpty())
//            return;
    bool layoutAll = stick <= Fraction(0, 1) && (etick < Fraction(0, 1) || etick >= m_score->masterScore()->last()->endTick());
    if (stick < Fraction(0, 1)) {
        stick = Fraction(0, 1);
    }
    if (etick < Fraction(0, 1)) {
        etick = m_score->last()->endTick();
    }

    lc.endTick = etick;
    m_score->_scoreFont = ScoreFont::fontByName(m_score->style().value(Sid::MusicalSymbolFont).toString());
    m_score->_noteHeadWidth = m_score->_scoreFont->width(SymId::noteheadBlack, m_score->spatium() / SPATIUM20);

    if (m_score->cmdState().layoutFlags & LayoutFlag::REBUILD_MIDI_MAPPING) {
        if (m_score->isMaster()) {
            m_score->masterScore()->rebuildMidiMapping();
        }
    }
    if (m_score->cmdState().layoutFlags & LayoutFlag::FIX_PITCH_VELO) {
        m_score->updateVelo();
    }
#if 0 // TODO: needed? It was introduced in ab9774ec4098512068b8ef708167d9aa6e702c50
    if (cmdState().layoutFlags & LayoutFlag::PLAY_EVENTS) {
        createPlayEvents();
    }
#endif

    //---------------------------------------------------
    //    initialize layout context lc
    //---------------------------------------------------

    MeasureBase* m = m_score->tick2measure(stick);
    if (m == 0) {
        m = m_score->first();
    }
    // start layout one measure earlier to handle clefs and cautionary elements
    if (m->prevMeasureMM()) {
        m = m->prevMeasureMM();
    } else if (m->prev()) {
        m = m->prev();
    }
    while (!m->isMeasure() && m->prev()) {
        m = m->prev();
    }

    // if the first measure of the score is part of a multi measure rest
    // m->system() will return a nullptr. We need to find the multi measure
    // rest which replaces the measure range

    if (!m->system() && m->isMeasure() && toMeasure(m)->hasMMRest()) {
        qDebug("  don’t start with mmrest");
        m = toMeasure(m)->mmRest();
    }

//      qDebug("start <%s> tick %d, system %p", m->name(), m->tick(), m->system());

    if (m_score->lineMode()) {
        lc.prevMeasure = 0;
        lc.nextMeasure = m;         //_showVBox ? first() : firstMeasure();
        lc.startTick   = m->tick();
        layoutLinear(layoutAll, lc);
        return;
    }
    if (!layoutAll && m->system()) {
        System* system  = m->system();
        int systemIndex = m_score->_systems.indexOf(system);
        lc.page         = system->page();
        lc.curPage      = m_score->pageIdx(lc.page);
        if (lc.curPage == -1) {
            lc.curPage = 0;
        }
        lc.curSystem   = system;
        lc.systemList  = m_score->_systems.mid(systemIndex);

        if (systemIndex == 0) {
            lc.nextMeasure = m_score->_showVBox ? m_score->first() : m_score->firstMeasure();
        } else {
            System* prevSystem = m_score->_systems[systemIndex - 1];
            lc.nextMeasure = prevSystem->measures().back()->next();
        }

        m_score->_systems.erase(m_score->_systems.begin() + systemIndex, m_score->_systems.end());
        if (!lc.nextMeasure->prevMeasure()) {
            lc.measureNo = 0;
            lc.tick      = Fraction(0, 1);
        } else {
            const MeasureBase* mb = lc.nextMeasure->prev();
            if (mb) {
                mb = mb->findPotentialSectionBreak();
            }
            LayoutBreak* sectionBreak = mb->sectionBreakElement();
            // TODO: also use mb in else clause here?
            // probably not, only actual measures have meaningful numbers
            if (sectionBreak && sectionBreak->startWithMeasureOne()) {
                lc.measureNo = 0;
            } else {
                lc.measureNo = lc.nextMeasure->prevMeasure()->no()                             // will be adjusted later with respect
                               + (lc.nextMeasure->prevMeasure()->irregular() ? 0 : 1);         // to the user-defined offset.
            }
            lc.tick      = lc.nextMeasure->tick();
        }
    } else {
//  qDebug("layoutAll, systems %p %d", &_systems, int(_systems.size()));
        //lc.measureNo   = 0;
        //lc.tick        = 0;
        // qDeleteAll(_systems);
        // _systems.clear();
        // lc.systemList  = _systems;
        // _systems.clear();

        for (System* s : qAsConst(m_score->_systems)) {
            for (Bracket* b : s->brackets()) {
                if (b->selected()) {
                    m_score->_selection.remove(b);
                    m_score->setSelectionChanged(true);
                }
            }
//                  for (SpannerSegment* ss : s->spannerSegments())
//                        ss->setParent(0);
            s->setParent(nullptr);
        }
        for (MeasureBase* mb = m_score->first(); mb; mb = mb->next()) {
            mb->setSystem(0);
            if (mb->isMeasure() && toMeasure(mb)->mmRest()) {
                toMeasure(mb)->mmRest()->setSystem(0);
            }
        }
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();

        qDeleteAll(m_score->pages());
        m_score->pages().clear();

        lc.nextMeasure = m_score->_showVBox ? m_score->first() : m_score->firstMeasure();
    }

    lc.prevMeasure = 0;

    LayoutMeasure::getNextMeasure(m_score, lc);
    lc.curSystem = LayoutSystem::collectSystem(lc, m_score);

    lc.layout();
}

//---------------------------------------------------------
//   layoutLinear
//---------------------------------------------------------

void Layout::layoutLinear(bool layoutAll, LayoutContext& lc)
{
    lc.score = m_score;
    resetSystems(layoutAll, lc);

    collectLinearSystem(lc);
//      hideEmptyStaves(systems().front(), true);     this does not make sense

    lc.layoutLinear();
}

//---------------------------------------------------------
//   resetSystems
//    in linear mode there is only one page
//    which contains one system
//---------------------------------------------------------

void Layout::resetSystems(bool layoutAll, LayoutContext& lc)
{
    Page* page = 0;
    if (layoutAll) {
        for (System* s : qAsConst(m_score->_systems)) {
            for (SpannerSegment* ss : s->spannerSegments()) {
                ss->setParent(0);
            }
        }
        qDeleteAll(m_score->_systems);
        m_score->_systems.clear();
        qDeleteAll(m_score->pages());
        m_score->pages().clear();
        if (!m_score->firstMeasure()) {
            qDebug("no measures");
            return;
        }

        for (MeasureBase* mb = m_score->first(); mb; mb = mb->next()) {
            mb->setSystem(0);
        }

        page = new Page(m_score);
        m_score->pages().push_back(page);
        page->bbox().setRect(0.0, 0.0, m_score->loWidth(), m_score->loHeight());
        page->setNo(0);

        System* system = new System(m_score);
        m_score->_systems.push_back(system);
        page->appendSystem(system);
        system->adjustStavesNumber(m_score->nstaves());
    } else {
        if (m_score->pages().isEmpty()) {
            return;
        }
        page = m_score->pages().front();
        System* system = m_score->systems().front();
        system->clear();
        system->adjustStavesNumber(m_score->nstaves());
    }
    lc.page = page;
}

//---------------------------------------------------------
//   collectLinearSystem
//   Append all measures to System. VBox is not included to System
//---------------------------------------------------------

void Layout::collectLinearSystem(LayoutContext& lc)
{
    System* system = m_score->systems().front();
    system->setInstrumentNames(/* longNames */ true);

    PointF pos;
    bool firstMeasure = true;       //lc.startTick.isZero();

    //set first measure to lc.nextMeasures for following
    //utilizing in getNextMeasure()
    lc.nextMeasure = m_score->_measures.first();
    lc.tick = Fraction(0, 1);
    LayoutMeasure::getNextMeasure(m_score, lc);

    while (lc.curMeasure) {
        qreal ww = 0.0;
        if (lc.curMeasure->isVBox() || lc.curMeasure->isTBox()) {
            lc.curMeasure->setParent(nullptr);
            LayoutMeasure::getNextMeasure(m_score, lc);
            continue;
        }
        system->appendMeasure(lc.curMeasure);
        if (lc.curMeasure->isMeasure()) {
            Measure* m = toMeasure(lc.curMeasure);
            if (m->mmRest()) {
                m->mmRest()->setSystem(nullptr);
            }
            if (firstMeasure) {
                system->layoutSystem(pos.rx());
                if (m->repeatStart()) {
                    Segment* s = m->findSegmentR(SegmentType::StartRepeatBarLine, Fraction(0, 1));
                    if (!s->enabled()) {
                        s->setEnabled(true);
                    }
                }
                m->addSystemHeader(true);
                pos.rx() += system->leftMargin();
                firstMeasure = false;
            } else if (m->header()) {
                m->removeSystemHeader();
            }
            if (m->trailer()) {
                m->removeSystemTrailer();
            }
            if (m->tick() >= lc.startTick && m->tick() <= lc.endTick) {
                // for measures in range, do full layout
                m->createEndBarLines(false);
                m->computeMinWidth();
                ww = m->width();
                m->stretchMeasure(ww);
            } else {
                // for measures not in range, use existing layout
                ww = m->width();
                if (m->pos() != pos) {
                    // fix beam positions
                    // other elements with system as parent are processed in layoutSystemElements()
                    // but full beam processing is expensive and not needed if we adjust position here
                    PointF p = pos - m->pos();
                    for (const Segment& s : m->segments()) {
                        if (!s.isChordRestType()) {
                            continue;
                        }
                        for (int track = 0; track < m_score->ntracks(); ++track) {
                            Element* e = s.element(track);
                            if (e) {
                                ChordRest* cr = toChordRest(e);
                                if (cr->beam() && cr->beam()->elements().front() == cr) {
                                    cr->beam()->rpos() += p;
                                }
                            }
                        }
                    }
                }
            }
            m->setPos(pos);
            m->layoutStaffLines();
        } else if (lc.curMeasure->isHBox()) {
            lc.curMeasure->setPos(pos + PointF(toHBox(lc.curMeasure)->topGap(), 0.0));
            lc.curMeasure->layout();
            ww = lc.curMeasure->width();
        }
        pos.rx() += ww;

        LayoutMeasure::getNextMeasure(m_score, lc);
    }

    system->setWidth(pos.x());
}

//---------------------------------------------------------
//   extendedStemLenWithTwoNotesTremolo
//    Goal: To extend stem of one of the chords to make the tremolo less steep
//    Returns a modified pair of stem lengths of two chords
//---------------------------------------------------------

std::pair<qreal, qreal> Layout::extendedStemLenWithTwoNoteTremolo(Tremolo* tremolo, qreal stemLen1, qreal stemLen2)
{
    const qreal spatium = tremolo->spatium();
    Chord* c1 = tremolo->chord1();
    Chord* c2 = tremolo->chord2();
    Stem* s1 = c1->stem();
    Stem* s2 = c2->stem();
    const qreal sgn1 = c1->up() ? -1.0 : 1.0;
    const qreal sgn2 = c2->up() ? -1.0 : 1.0;
    const qreal stemTipDistance = (s1 && s2) ? (s2->pagePos().y() + stemLen2) - (s1->pagePos().y() + stemLen1)
                                  : (c2->stemPos().y() + stemLen2) - (c1->stemPos().y() + stemLen1);

    // same staff & same direction: extend one of the stems
    if (c1->staffMove() == c2->staffMove() && c1->up() == c2->up()) {
        const bool stem1Higher = stemTipDistance > 0.0;
        if (std::abs(stemTipDistance) > 1.0 * spatium) {
            if ((c1->up() && !stem1Higher) || (!c1->up() && stem1Higher)) {
                return { stemLen1 + sgn1 * (std::abs(stemTipDistance) - 1.0 * spatium), stemLen2 };
            } else {   /* if ((c1->up() && stem1Higher) || (!c1->up() && !stem1Higher)) */
                return { stemLen1, stemLen2 + sgn2 * (std::abs(stemTipDistance) - 1.0 * spatium) };
            }
        }
    }

// TODO: cross-staff two-note tremolo. Currently doesn't generate the right result in some cases.
#if 0
    // cross-staff & beam between staves: extend both stems by the same length
    else if (tremolo->crossStaffBeamBetween()) {
        const qreal sw = tremolo->score()->styleS(Sid::tremoloStrokeWidth).val();
        const qreal td = tremolo->score()->styleS(Sid::tremoloDistance).val();
        const qreal tremoloMinHeight = ((tremolo->lines() - 1) * td + sw) * spatium;
        const qreal dy = c1->up() ? tremoloMinHeight - stemTipDistance : tremoloMinHeight + stemTipDistance;
        const bool tooShort = dy > 1.0 * spatium;
        const bool tooLong = dy < -1.0 * spatium;
        const qreal idealDistance = 1.0 * spatium - tremoloMinHeight;

        if (tooShort) {
            return { stemLen1 + sgn1 * (std::abs(stemTipDistance) - idealDistance) / 2.0,
                     stemLen2 + sgn2 * (std::abs(stemTipDistance) - idealDistance) / 2.0 };
        } else if (tooLong) {
            return { stemLen1 - sgn1 * (std::abs(stemTipDistance) + idealDistance) / 2.0,
                     stemLen2 - sgn2 * (std::abs(stemTipDistance) + idealDistance) / 2.0 };
        }
    }
#endif

    return { stemLen1, stemLen2 };
}
