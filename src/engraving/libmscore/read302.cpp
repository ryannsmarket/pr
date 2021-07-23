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

#include "style/style.h"
#include "style/defaultstyle.h"

#include "compat/chordlist.h"
#include "compat/readscorehook.h"

#include "xml.h"
#include "score.h"
#include "staff.h"
#include "revisions.h"
#include "part.h"
#include "page.h"
#include "scorefont.h"
#include "audio.h"
#include "sig.h"
#include "barline.h"
#include "excerpt.h"
#include "spanner.h"
#include "scoreorder.h"
#include "measurebase.h"

using namespace mu;
using namespace mu::engraving;

namespace Ms {
//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool Score::read(XmlReader& e, compat::ReadScoreHook& hooks)
{
    // HACK
    // style setting compatibility settings for minor versions
    // this allows new style settings to be added
    // with different default values for older vs newer scores
    // note: older templates get the default values for older scores
    // these can be forced back in MuseScore::getNewFile() if necessary
    QString programVersion = masterScore()->mscoreVersion();
    bool disableHarmonyPlay = MScore::harmonyPlayDisableCompatibility && !MScore::testMode;
    if (!programVersion.isEmpty() && programVersion < "3.5" && disableHarmonyPlay) {
        style().set(Sid::harmonyPlay, false);
    }

    while (e.readNextStartElement()) {
        e.setTrack(-1);
        const QStringRef& tag(e.name());
        if (tag == "Staff") {
            readStaff(e);
        } else if (tag == "Omr") {
            e.skipCurrentElement();
        } else if (tag == "Audio") {
            _audio = new Audio;
            _audio->read(e);
        } else if (tag == "showOmr") {
            masterScore()->setShowOmr(e.readInt());
        } else if (tag == "playMode") {
            _playMode = PlayMode(e.readInt());
        } else if (tag == "LayerTag") {
            int id = e.intAttribute("id");
            const QString& t = e.attribute("tag");
            QString val(e.readElementText());
            if (id >= 0 && id < 32) {
                _layerTags[id] = t;
                _layerTagComments[id] = val;
            }
        } else if (tag == "Layer") {
            Layer layer;
            layer.name = e.attribute("name");
            layer.tags = e.attribute("mask").toUInt();
            _layer.append(layer);
            e.readNext();
        } else if (tag == "currentLayer") {
            _currentLayer = e.readInt();
        } else if (tag == "Synthesizer") {
            _synthesizerState.read(e);
        } else if (tag == "page-offset") {
            _pageNumberOffset = e.readInt();
        } else if (tag == "Division") {
            _fileDivision = e.readInt();
        } else if (tag == "showInvisible") {
            _showInvisible = e.readInt();
        } else if (tag == "showUnprintable") {
            _showUnprintable = e.readInt();
        } else if (tag == "showFrames") {
            _showFrames = e.readInt();
        } else if (tag == "showMargins") {
            _showPageborders = e.readInt();
        } else if (tag == "markIrregularMeasures") {
            _markIrregularMeasures = e.readInt();
        } else if (tag == "Style") {
            hooks.onReadStyleTag302(this, e);
        } else if (tag == "copyright" || tag == "rights") {
            Text* text = new Text(this);
            text->read(e);
            setMetaTag("copyright", text->xmlText());
            delete text;
        } else if (tag == "movement-number") {
            setMetaTag("movementNumber", e.readElementText());
        } else if (tag == "movement-title") {
            setMetaTag("movementTitle", e.readElementText());
        } else if (tag == "work-number") {
            setMetaTag("workNumber", e.readElementText());
        } else if (tag == "work-title") {
            setMetaTag("workTitle", e.readElementText());
        } else if (tag == "source") {
            setMetaTag("source", e.readElementText());
        } else if (tag == "metaTag") {
            QString name = e.attribute("name");
            setMetaTag(name, e.readElementText());
        } else if (tag == "Order") {
            ScoreOrder order;
            order.read(e);
            if (order.isValid()) {
                setScoreOrder(order);
            }
        } else if (tag == "Part") {
            Part* part = new Part(this);
            part->read(e);
            _parts.push_back(part);
        } else if ((tag == "HairPin")
                   || (tag == "Ottava")
                   || (tag == "TextLine")
                   || (tag == "Volta")
                   || (tag == "Trill")
                   || (tag == "Slur")
                   || (tag == "Pedal")) {
            Spanner* s = toSpanner(Element::name2Element(tag, this));
            s->read(e);
            addSpanner(s);
        } else if (tag == "Excerpt") {
            if (MScore::noExcerpts) {
                e.skipCurrentElement();
            } else {
                if (isMaster()) {
                    Excerpt* ex = new Excerpt(static_cast<MasterScore*>(this));
                    ex->read(e);
                    excerpts().append(ex);
                } else {
                    qDebug("Score::read(): part cannot have parts");
                    e.skipCurrentElement();
                }
            }
        } else if (e.name() == "Tracklist") {
            int strack = e.intAttribute("sTrack",   -1);
            int dtrack = e.intAttribute("dstTrack", -1);
            if (strack != -1 && dtrack != -1) {
                e.tracks().insert(strack, dtrack);
            }
            e.skipCurrentElement();
        } else if (tag == "Score") {            // recursion
            if (MScore::noExcerpts) {
                e.skipCurrentElement();
            } else {
                e.tracks().clear();             // ???
                MasterScore* m = masterScore();
                Score* s = m->createScore();
                compat::ReadScoreHook compatHooks;
                compatHooks.installReadStyleHook(s);
                compatHooks.setupDefaultStyle();

                Excerpt* ex = new Excerpt(m);
                ex->setPartScore(s);
                e.setLastMeasure(nullptr);

                s->read(e, hooks);

                s->linkMeasures(m);
                ex->setTracks(e.tracks());
                m->addExcerpt(ex);
            }
        } else if (tag == "name") {
            QString n = e.readElementText();
            if (!isMaster()) {     //ignore the name if it's not a child score
                excerpt()->setTitle(n);
            }
        } else if (tag == "layoutMode") {
            QString s = e.readElementText();
            if (s == "line") {
                _layoutMode = LayoutMode::LINE;
            } else if (s == "system") {
                _layoutMode = LayoutMode::SYSTEM;
            } else {
                qDebug("layoutMode: %s", qPrintable(s));
            }
        } else {
            e.unknown();
        }
    }
    e.reconnectBrokenConnectors();
    if (e.error() != QXmlStreamReader::NoError) {
        qDebug("%s: xml read error at line %lld col %lld: %s",
               qPrintable(e.getDocName()), e.lineNumber(), e.columnNumber(),
               e.name().toUtf8().data());
        if (e.error() == QXmlStreamReader::CustomError) {
            MScore::lastError = e.errorString();
        } else {
            MScore::lastError = QObject::tr("XML read error at line %1, column %2: %3").arg(e.lineNumber()).arg(e.columnNumber()).arg(
                e.name().toString());
        }
        return false;
    }

    connectTies();
    relayoutForStyles(); // force relayout if certain style settings are enabled

    _fileDivision = MScore::division;

#if 0 // TODO:barline
      //
      //    sanity check for barLineSpan
      //
    for (Staff* st : staves()) {
        int barLineSpan = st->barLineSpan();
        int idx = st->idx();
        int n   = nstaves();
        if (idx + barLineSpan > n) {
            qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
            // span until last staff
            barLineSpan = n - idx;
            st->setBarLineSpan(barLineSpan);
        } else if (idx == 0 && barLineSpan == 0) {
            qDebug("bad span: idx %d  span %d staves %d", idx, barLineSpan, n);
            // span from the first staff until the start of the next span
            barLineSpan = 1;
            for (int i = 1; i < n; ++i) {
                if (staff(i)->barLineSpan() == 0) {
                    ++barLineSpan;
                } else {
                    break;
                }
            }
            st->setBarLineSpan(barLineSpan);
        }
        // check spanFrom
        int minBarLineFrom = st->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_FROM : MIN_BARLINE_SPAN_FROMTO;
        if (st->barLineFrom() < minBarLineFrom) {
            st->setBarLineFrom(minBarLineFrom);
        }
        if (st->barLineFrom() > st->lines(0) * 2) {
            st->setBarLineFrom(st->lines(0) * 2);
        }
        // check spanTo
        Staff* stTo = st->barLineSpan() <= 1 ? st : staff(idx + st->barLineSpan() - 1);
        // 1-line staves have special bar line spans
        int maxBarLineTo        = stTo->lines(0) == 1 ? BARLINE_SPAN_1LINESTAFF_TO : stTo->lines(0) * 2;
        if (st->barLineTo() < MIN_BARLINE_SPAN_FROMTO) {
            st->setBarLineTo(MIN_BARLINE_SPAN_FROMTO);
        }
        if (st->barLineTo() > maxBarLineTo) {
            st->setBarLineTo(maxBarLineTo);
        }
        // on single staff span, check spanFrom and spanTo are distant enough
        if (st->barLineSpan() == 1) {
            if (st->barLineTo() - st->barLineFrom() < MIN_BARLINE_FROMTO_DIST) {
                st->setBarLineFrom(0);
                st->setBarLineTo(0);
            }
        }
    }
#endif
    // Make sure every instrument has an instrumentId set.
    for (Part* part : parts()) {
        const InstrumentList* il = part->instruments();
        for (auto it = il->begin(); it != il->end(); it++) {
            static_cast<Instrument*>(it->second)->updateInstrumentId();
        }
    }

    if (!masterScore()->omr()) {
        masterScore()->setShowOmr(false);
    }

    fixTicks();

    for (Part* p : qAsConst(_parts)) {
        p->updateHarmonyChannels(false);
    }

    masterScore()->rebuildMidiMapping();
    masterScore()->updateChannel();

//      createPlayEvents();
    return true;
}

//---------------------------------------------------------
// linkMeasures
//---------------------------------------------------------

void Score::linkMeasures(Score* score)
{
    MeasureBase* mbMaster = score->first();
    for (MeasureBase* mb = first(); mb; mb = mb->next()) {
        if (!mb->isMeasure()) {
            continue;
        }
        while (mbMaster && !mbMaster->isMeasure()) {
            mbMaster = mbMaster->next();
        }
        if (!mbMaster) {
            qDebug("Measures in MasterScore and Score are not in sync.");
            break;
        }
        mb->linkTo(mbMaster);
        mbMaster = mbMaster->next();
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool MasterScore::read(XmlReader& e, compat::ReadScoreHook& hooks)
{
    if (!Score::read(e, hooks)) {
        return false;
    }
    for (Staff* s : staves()) {
        s->updateOttava();
    }
    setCreated(false);
    return true;
}

//---------------------------------------------------------
//   read301
//---------------------------------------------------------

Score::FileError MasterScore::read302(XmlReader& e, mu::engraving::compat::ReadScoreHook& hooks)
{
    while (e.readNextStartElement()) {
        const QStringRef& tag(e.name());
        if (tag == "programVersion") {
            setMscoreVersion(e.readElementText());
            parseVersion(mscoreVersion());
        } else if (tag == "programRevision") {
            setMscoreRevision(e.readIntHex());
        } else if (tag == "Score") {
            if (!read(e, hooks)) {
                if (e.error() == QXmlStreamReader::CustomError) {
                    return FileError::FILE_CRITICALLY_CORRUPTED;
                }
                return FileError::FILE_BAD_FORMAT;
            }
        } else if (tag == "Revision") {
            Revision* revision = new Revision;
            revision->read(e);
            revisions()->add(revision);
        }
    }
    return FileError::FILE_NO_ERROR;
}
}
