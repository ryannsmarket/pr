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

#include "box.h"

#include <cmath>

#include "io/xml.h"

#include "textframe.h"
#include "text.h"
#include "score.h"
#include "barline.h"
#include "measurerepeat.h"
#include "symbol.h"
#include "system.h"
#include "image.h"
#include "layoutbreak.h"
#include "fret.h"
#include "mscore.h"
#include "stafftext.h"
#include "actionicon.h"
#include "measure.h"
#include "undo.h"

using namespace mu;
using namespace mu::draw;

namespace Ms {
static const ElementStyle boxStyle {
    { Sid::systemFrameDistance,                Pid::TOP_GAP },
    { Sid::frameSystemDistance,                Pid::BOTTOM_GAP },
};

static const ElementStyle hBoxStyle {
};

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

Box::Box(Score* score)
    : MeasureBase(score)
{
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Box::layout()
{
    MeasureBase::layout();
    for (Element* e : el()) {
        if (!e->isLayoutBreak()) {
            e->layout();
        }
    }
}

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Box::scanElements(void* data, void (* func)(void*, Element*), bool all)
{
    ScoreElement::scanElements(data, func, all);
    if (all || visible() || score()->showInvisible()) {
        func(data, this);
    }
}

//---------------------------------------------------------
//   computeMinWidth
//---------------------------------------------------------

void HBox::computeMinWidth()
{
    setWidth(point(boxWidth()) + topGap() + bottomGap());    // top/bottom is really left/right
}

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Box::draw(mu::draw::Painter* painter) const
{
    TRACE_OBJ_DRAW;
    if (score() && score()->printing()) {
        return;
    }

    const bool showBlueFrame = selected() || dropTarget();
    const bool showFrame = showBlueFrame || (score() ? score()->showFrames() : false);

    if (showFrame) {
        qreal lineWidth = spatium() * .15;
        Pen pen;
        pen.setWidthF(lineWidth);
        pen.setJoinStyle(PenJoinStyle::MiterJoin);
        pen.setCapStyle(PenCapStyle::SquareCap);
        pen.setColor(showBlueFrame ? MScore::selectColor[0] : MScore::frameMarginColor);
        pen.setDashPattern({ 1, 3 });

        painter->setBrush(BrushStyle::NoBrush);
        painter->setPen(pen);
        lineWidth *= 0.5;
        painter->drawRect(bbox().adjusted(lineWidth, lineWidth, -lineWidth, -lineWidth));
    }
}

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Box::edit(EditData&)
{
    return false;
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void Box::startEditDrag(EditData& ed)
{
    ElementEditData* eed = ed.getData(this);
    if (isHBox()) {
        eed->pushProperty(Pid::BOX_WIDTH);
    } else {
        eed->pushProperty(Pid::BOX_HEIGHT);
    }
}

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Box::editDrag(EditData& ed)
{
    if (isVBox()) {
        _boxHeight += Spatium(ed.delta.y() / spatium());
        if (ed.vRaster) {
            qreal vRaster = 1.0 / MScore::vRaster();
            int n = lrint(_boxHeight.val() / vRaster);
            _boxHeight = Spatium(vRaster * n);
        }
        bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
        system()->setHeight(height());
        triggerLayout();
    } else {
        _boxWidth += Spatium(ed.delta.x() / spatium());
        if (ed.hRaster) {
            qreal hRaster = 1.0 / MScore::hRaster();
            int n = lrint(_boxWidth.val() / hRaster);
            _boxWidth = Spatium(hRaster * n);
        }
        triggerLayout();
    }
    layout();
}

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Box::endEdit(EditData&)
{
    layout();
}

//---------------------------------------------------------
//   gripsPositions
//---------------------------------------------------------

std::vector<mu::PointF> HBox::gripsPositions(const EditData&) const
{
    RectF r(abbox());
    return { PointF(r.right(), r.top() + r.height() * .5) };
}

std::vector<PointF> VBox::gripsPositions(const EditData&) const
{
    RectF r(abbox());
    return { PointF(r.x() + r.width() * .5, r.bottom()) };
}

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Box::write(XmlWriter& xml) const
{
    xml.stag(this);
    writeProperties(xml);
    xml.etag();
}

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Box::writeProperties(XmlWriter& xml) const
{
    for (Pid id : {
            Pid::BOX_HEIGHT, Pid::BOX_WIDTH, Pid::TOP_GAP, Pid::BOTTOM_GAP,
            Pid::LEFT_MARGIN, Pid::RIGHT_MARGIN, Pid::TOP_MARGIN, Pid::BOTTOM_MARGIN, Pid::BOX_AUTOSIZE }) {
        writeProperty(xml, id);
    }
    Element::writeProperties(xml);
    for (const Element* e : el()) {
        e->write(xml);
    }
}

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Box::read(XmlReader& e)
{
    _leftMargin      = 0.0;
    _rightMargin     = 0.0;
    _topMargin       = 0.0;
    _bottomMargin    = 0.0;
    _boxHeight       = Spatium(0);       // override default set in constructor
    _boxWidth        = Spatium(0);
    MeasureBase::read(e);
    if (score()->mscVersion() < 302) {
        _isAutoSizeEnabled = false;     // disable auto-size for older scores by default.
    }
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Box::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());
    if (tag == "height") {
        _boxHeight = Spatium(e.readDouble());
    } else if (tag == "width") {
        _boxWidth = Spatium(e.readDouble());
    } else if (tag == "topGap") {
        _topGap = e.readDouble();
        if (score()->mscVersion() >= 206) {
            _topGap *= score()->spatium();
        }
        setPropertyFlags(Pid::TOP_GAP, PropertyFlags::UNSTYLED);
    } else if (tag == "bottomGap") {
        _bottomGap = e.readDouble();
        if (score()->mscVersion() >= 206) {
            _bottomGap *= score()->spatium();
        }
        setPropertyFlags(Pid::BOTTOM_GAP, PropertyFlags::UNSTYLED);
    } else if (tag == "leftMargin") {
        _leftMargin = e.readDouble();
    } else if (tag == "rightMargin") {
        _rightMargin = e.readDouble();
    } else if (tag == "topMargin") {
        _topMargin = e.readDouble();
    } else if (tag == "bottomMargin") {
        _bottomMargin = e.readDouble();
    } else if (tag == "boxAutoSize") {
        _isAutoSizeEnabled = e.readBool();
    } else if (tag == "Text") {
        Text* t;
        if (isTBox()) {
            t = toTBox(this)->text();
            t->read(e);
        } else {
            t = new Text(score());
            t->read(e);
            if (t->empty()) {
                qDebug("read empty text");
            } else {
                add(t);
            }
        }
    } else if (tag == "Symbol") {
        Symbol* s = new Symbol(score());
        s->read(e);
        add(s);
    } else if (tag == "Image") {
        if (MScore::noImages) {
            e.skipCurrentElement();
        } else {
            Image* image = new Image(score());
            image->setTrack(e.track());
            image->read(e);
            add(image);
        }
    } else if (tag == "FretDiagram") {
        FretDiagram* f = new FretDiagram(score());
        f->read(e);
        add(f);
    } else if (tag == "HBox") {
        HBox* hb = new HBox(score());
        hb->read(e);
        add(hb);
    } else if (tag == "VBox") {
        VBox* vb = new VBox(score());
        vb->read(e);
        add(vb);
    } else if (MeasureBase::readProperties(e)) {
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   add
///   Add new Element \a el to Box
//---------------------------------------------------------

void Box::add(Element* e)
{
    if (e->isText()) {
        toText(e)->setLayoutToParentWidth(true);
    }
    MeasureBase::add(e);
}

RectF Box::contentRect() const
{
    RectF result;

    for (const Element* element : el()) {
        result = result.united(element->bbox());
    }

    return result;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Box::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOX_HEIGHT:
        return _boxHeight;
    case Pid::BOX_WIDTH:
        return _boxWidth;
    case Pid::TOP_GAP:
        return _topGap;
    case Pid::BOTTOM_GAP:
        return _bottomGap;
    case Pid::LEFT_MARGIN:
        return _leftMargin;
    case Pid::RIGHT_MARGIN:
        return _rightMargin;
    case Pid::TOP_MARGIN:
        return _topMargin;
    case Pid::BOTTOM_MARGIN:
        return _bottomMargin;
    case Pid::BOX_AUTOSIZE:
        return (score()->mscVersion() >= 302) ? _isAutoSizeEnabled : false;
    default:
        return MeasureBase::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Box::setProperty(Pid propertyId, const QVariant& v)
{
    score()->addRefresh(canvasBoundingRect());
    switch (propertyId) {
    case Pid::BOX_HEIGHT:
        _boxHeight = v.value<Spatium>();
        break;
    case Pid::BOX_WIDTH:
        _boxWidth = v.value<Spatium>();
        break;
    case Pid::TOP_GAP:
        _topGap = v.toDouble();
        break;
    case Pid::BOTTOM_GAP:
        _bottomGap = v.toDouble();
        break;
    case Pid::LEFT_MARGIN:
        _leftMargin = v.toDouble();
        break;
    case Pid::RIGHT_MARGIN:
        _rightMargin = v.toDouble();
        break;
    case Pid::TOP_MARGIN:
        _topMargin = v.toDouble();
        break;
    case Pid::BOTTOM_MARGIN:
        _bottomMargin = v.toDouble();
        break;
    case Pid::BOX_AUTOSIZE:
        _isAutoSizeEnabled = v.toBool();
        break;
    default:
        return MeasureBase::setProperty(propertyId, v);
    }
    triggerLayout();
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Box::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::BOX_HEIGHT:
    case Pid::BOX_WIDTH:
        return Spatium(0.0);

    case Pid::TOP_GAP:
        return isHBox() ? 0.0 : score()->styleP(Sid::systemFrameDistance);
    case Pid::BOTTOM_GAP:
        return isHBox() ? 0.0 : score()->styleP(Sid::frameSystemDistance);

    case Pid::LEFT_MARGIN:
    case Pid::RIGHT_MARGIN:
    case Pid::TOP_MARGIN:
    case Pid::BOTTOM_MARGIN:
        return 0.0;
    case Pid::BOX_AUTOSIZE:
        return true;
    default:
        return MeasureBase::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   copyValues
//---------------------------------------------------------

void Box::copyValues(Box* origin)
{
    _boxHeight    = origin->boxHeight();
    _boxWidth     = origin->boxWidth();

    qreal factor  = magS() / origin->magS();
    _bottomGap    = origin->bottomGap() * factor;
    _topGap       = origin->topGap() * factor;
    _bottomMargin = origin->bottomMargin() * factor;
    _topMargin    = origin->topMargin() * factor;
    _leftMargin   = origin->leftMargin() * factor;
    _rightMargin  = origin->rightMargin() * factor;
}

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

HBox::HBox(Score* score)
    : Box(score)
{
    initElementStyle(&hBoxStyle);
    setBoxWidth(Spatium(5.0));
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HBox::layout()
{
    if (parent() && parent()->isVBox()) {
        VBox* vb = toVBox(parent());
        qreal x = vb->leftMargin() * DPMM;
        qreal y = vb->topMargin() * DPMM;
        qreal w = point(boxWidth());
        qreal h = vb->height() - (vb->topMargin() + vb->bottomMargin()) * DPMM;
        setPos(x, y);
        bbox().setRect(0.0, 0.0, w, h);
    } else if (system()) {
        bbox().setRect(0.0, 0.0, point(boxWidth()), system()->height());
    } else {
        bbox().setRect(0.0, 0.0, 50, 50);
    }
    Box::layout();
}

//---------------------------------------------------------
//   layout2
//    height (bbox) is defined now
//---------------------------------------------------------

void HBox::layout2()
{
    Box::layout();
}

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Box::acceptDrop(EditData& data) const
{
    if (data.dropElement->flag(ElementFlag::ON_STAFF)) {
        return false;
    }
    if (MScore::debugMode) {
        qDebug("<%s>", data.dropElement->name());
    }
    ElementType t = data.dropElement->type();
    switch (t) {
    case ElementType::LAYOUT_BREAK:
    case ElementType::TEXT:
    case ElementType::STAFF_TEXT:
    case ElementType::IMAGE:
    case ElementType::SYMBOL:
        return true;
    case ElementType::ACTION_ICON:
        switch (toActionIcon(data.dropElement)->actionType()) {
        case ActionIconType::VFRAME:
        case ActionIconType::TFRAME:
        case ActionIconType::FFRAME:
        case ActionIconType::HFRAME:
        case ActionIconType::MEASURE:
            return true;
        default:
            break;
        }
        break;
    case ElementType::BAR_LINE:
        return isHBox();
    default:
        break;
    }
    return false;
}

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Box::drop(EditData& data)
{
    Element* e = data.dropElement;
    if (e->flag(ElementFlag::ON_STAFF)) {
        return 0;
    }
    if (MScore::debugMode) {
        qDebug("<%s>", e->name());
    }
    switch (e->type()) {
    case ElementType::LAYOUT_BREAK:
    {
        LayoutBreak* lb = toLayoutBreak(e);
        if (pageBreak() || lineBreak()) {
            if (
                (lb->isPageBreak() && pageBreak())
                || (lb->isLineBreak() && lineBreak())
                || (lb->isSectionBreak() && sectionBreak())
                ) {
                //
                // if break already set
                //
                delete lb;
                break;
            }
            for (Element* elem : el()) {
                if (elem->type() == ElementType::LAYOUT_BREAK) {
                    score()->undoChangeElement(elem, e);
                    break;
                }
            }
            break;
        }
        lb->setTrack(-1);                 // these are system elements
        lb->setParent(this);
        score()->undoAddElement(lb);
        return lb;
    }

    case ElementType::STAFF_TEXT:
    {
        Text* text = new Text(score(), Tid::FRAME);
        text->setParent(this);
        text->setXmlText(toStaffText(e)->xmlText());
        score()->undoAddElement(text);
        delete e;
        return text;
    }

    case ElementType::ACTION_ICON:
        switch (toActionIcon(e)->actionType()) {
        case ActionIconType::VFRAME:
            score()->insertMeasure(ElementType::VBOX, this);
            break;
        case ActionIconType::TFRAME:
            score()->insertMeasure(ElementType::TBOX, this);
            break;
        case ActionIconType::FFRAME:
            score()->insertMeasure(ElementType::FBOX, this);
            break;
        case ActionIconType::HFRAME:
            score()->insertMeasure(ElementType::HBOX, this);
            break;
        case ActionIconType::MEASURE:
            score()->insertMeasure(ElementType::MEASURE, this);
            break;
        default:
            break;
        }
        break;

    case ElementType::TEXT:
    case ElementType::IMAGE:
    case ElementType::SYMBOL:
        e->setParent(this);
        score()->undoAddElement(e);
        return e;
    default:
        return 0;
    }
    return 0;
}

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

RectF HBox::drag(EditData& data)
{
    RectF r(canvasBoundingRect());
    qreal diff = data.evtDelta.x();
    qreal x1   = offset().x() + diff;
    if (parent()->type() == ElementType::VBOX) {
        VBox* vb = toVBox(parent());
        qreal x2 = parent()->width() - width() - (vb->leftMargin() + vb->rightMargin()) * DPMM;
        if (x1 < 0.0) {
            x1 = 0.0;
        } else if (x1 > x2) {
            x1 = x2;
        }
    }
    setOffset(PointF(x1, 0.0));
//      setStartDragPosition(data.delta);
    return canvasBoundingRect().united(r);
}

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void HBox::endEditDrag(EditData&)
{
    triggerLayout();
    score()->update();
}

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool HBox::isMovable() const
{
    return parent() && (parent()->isHBox() || parent()->isVBox());
}

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void HBox::writeProperties(XmlWriter& xml) const
{
    writeProperty(xml, Pid::CREATE_SYSTEM_HEADER);
    Box::writeProperties(xml);
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool HBox::readProperties(XmlReader& e)
{
    const QStringRef& tag(e.name());
    if (readProperty(tag, e, Pid::CREATE_SYSTEM_HEADER)) {
    } else if (Box::readProperties(e)) {
    } else {
        return false;
    }
    return true;
}

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant HBox::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::CREATE_SYSTEM_HEADER:
        return createSystemHeader();
    default:
        return Box::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool HBox::setProperty(Pid propertyId, const QVariant& v)
{
    switch (propertyId) {
    case Pid::CREATE_SYSTEM_HEADER:
        setCreateSystemHeader(v.toBool());
        triggerLayout();
        break;
    default:
        return Box::setProperty(propertyId, v);
    }
    return true;
}

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant HBox::propertyDefault(Pid id) const
{
    switch (id) {
    case Pid::CREATE_SYSTEM_HEADER:
        return true;
    default:
        return Box::propertyDefault(id);
    }
}

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

VBox::VBox(Score* score)
    : Box(score)
{
    initElementStyle(&boxStyle);
    setBoxHeight(Spatium(10.0));
    setLineBreak(true);
}

qreal VBox::minHeight() const
{
    return point(Spatium(10));
}

qreal VBox::maxHeight() const
{
    return point(Spatium(30));
}

QVariant VBox::getProperty(Pid propertyId) const
{
    switch (propertyId) {
    case Pid::BOX_AUTOSIZE:
        return isAutoSizeEnabled();
    default:
        return Box::getProperty(propertyId);
    }
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VBox::layout()
{
    setPos(PointF());

    if (system()) {
        bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
    } else {
        bbox().setRect(0.0, 0.0, 50, 50);
    }

    for (Element* e : el()) {
        if (!e->isLayoutBreak()) {
            e->layout();
        }
    }

    if (getProperty(Pid::BOX_AUTOSIZE).toBool()) {
        qreal contentHeight = contentRect().height();

        if (contentHeight < minHeight()) {
            contentHeight = minHeight();
        }

        setHeight(contentHeight);
    }

    MeasureBase::layout();
}

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

void VBox::startEditDrag(EditData& ed)
{
    if (isAutoSizeEnabled()) {
        setAutoSizeEnabled(false);
        setBoxHeight(Spatium(height() / spatium()));
    }
    Box::startEditDrag(ed);
}

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FBox::layout()
{
//      setPos(PointF());      // !?
    bbox().setRect(0.0, 0.0, system()->width(), point(boxHeight()));
    Box::layout();
}

//---------------------------------------------------------
//   add
///   Add new Element \a e to fret diagram box
//---------------------------------------------------------

void FBox::add(Element* e)
{
    e->setParent(this);
    if (e->isFretDiagram()) {
//            FretDiagram* fd = toFretDiagram(e);
//            fd->setFlag(ElementFlag::MOVABLE, false);
    } else {
        qDebug("FBox::add: element not allowed");
        return;
    }
    el().push_back(e);
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

QString Box::accessibleExtraInfo() const
{
    QString rez = "";
    for (Element* e : el()) {
        rez += " " + e->screenReaderInfo();
    }
    return rez;
}

//---------------------------------------------------------
//   accessibleExtraInfo
//---------------------------------------------------------

QString TBox::accessibleExtraInfo() const
{
    QString rez = _text->screenReaderInfo();
    return rez;
}
}
