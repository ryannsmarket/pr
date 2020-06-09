//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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
#include "notationpaintview.h"

#include <QPainter>
#include <cmath>

#include "log.h"
#include "notationviewinputcontroller.h"
#include "actions/action.h"

using namespace mu::scene::notation;
using namespace mu::domain::notation;

static constexpr int PREF_UI_CANVAS_MISC_SELECTIONPROXIMITY = 6;

NotationPaintView::NotationPaintView()
    : QQuickPaintedItem()
{
    setFlag(ItemHasContents, true);
    setAcceptedMouseButtons(Qt::AllButtons);

    //! TODO
    double mag  = 0.267;//preferences.getDouble(PREF_SCORE_MAGNIFICATION) * (mscore->physicalDotsPerInch() / DPI);
    m_matrix = QTransform::fromScale(mag, mag);

    m_inputController = new NotationViewInputController(this);

    connect(this, &QQuickPaintedItem::widthChanged, this, &NotationPaintView::onViewSizeChanged);
    connect(this, &QQuickPaintedItem::heightChanged, this, &NotationPaintView::onViewSizeChanged);

    // actions
    dispatcher()->reg("domain/notation/file-open", [this](const actions::ActionName&) { open(); });
}

//! NOTE Temporary method for tests
void NotationPaintView::open()
{
    QString filePath = interactive()->selectOpeningFile("Score", "", "");
    if (filePath.isEmpty()) {
        return;
    }

    m_notation = notationCreator()->newNotation();
    IF_ASSERT_FAILED(m_notation) {
        return;
    }

    onViewSizeChanged();

    bool ok = m_notation->load(filePath.toStdString());
    if (!ok) {
        LOGE() << "failed load: " << filePath;
    }

    //! NOTE At the moment, only one notation, in the future it will change.
    globalContext()->setCurrentNotation(m_notation);

    m_notation->notationChanged().onNotify(this, [this]() {
        update();
    });

    onInputStateChanged();
    m_notation->inputStateChanged().onNotify(this, [this]() {
        onInputStateChanged();
    });

    m_notation->selectionChanged().onNotify(this, [this]() {
        onSelectionChanged();
    });

    update();
}

void NotationPaintView::onViewSizeChanged()
{
    if (!m_notation) {
        return;
    }

    QPoint p1 = toLogical(QPoint(0, 0));
    QPoint p2 = toLogical(QPoint(width(), height()));
    m_notation->setViewSize(QSizeF(p2.x() - p1.x(), p2.y() - p1.y()));
}

void NotationPaintView::onInputStateChanged()
{
    INotationInputState* is = m_notation->inputState();
    if (is->isNoteEnterMode()) {
        setAcceptHoverEvents(true);
    } else {
        setAcceptHoverEvents(false);
    }

    update();
}

void NotationPaintView::onSelectionChanged()
{
    QRectF selRect = m_notation->selection()->canvasBoundingRect();
    if (!selRect.isValid()) {
        return;
    }

    adjustCanvasPosition(selRect);
    update();
}

bool NotationPaintView::isNoteEnterMode() const
{
    if (!m_notation) {
        return false;
    }

    return m_notation->inputState()->isNoteEnterMode();
}

void NotationPaintView::showShadowNote(const QPointF& pos)
{
    m_notation->showShadowNote(pos);
    update();
}

void NotationPaintView::paint(QPainter* p)
{
    QRect rect(0, 0, width(), height());
    p->fillRect(rect, QColor("#D6E0E9"));

    p->setTransform(m_matrix);

    if (m_notation) {
        m_notation->paint(p, rect);
        m_notation->paintShadowNote(p);
    } else {
        p->drawText(10, 10, "no notation");
    }
}

qreal NotationPaintView::xoffset() const
{
    return m_matrix.dx();
}

qreal NotationPaintView::yoffset() const
{
    return m_matrix.dy();
}

QRect NotationPaintView::viewport() const
{
    return toLogical(QRect(0, 0, width(), height()));
}

void NotationPaintView::adjustCanvasPosition(const QRectF& logicRect)
{
    //! TODO This is very simple adjustment of position.
    //! Need to port the logic from ScoreView::adjustCanvasPosition
    QPoint posTL = logicRect.topLeft().toPoint();

    QRect viewRect = viewport();
    if (viewRect.contains(posTL)) {
        return;
    }

    posTL.setX(posTL.x() - 300);
    posTL.setY(posTL.y() - 300);
    moveCanvasToPosition(posTL);
}

void NotationPaintView::moveCanvasToPosition(const QPoint& logicPos)
{
    QPoint viewTL = toLogical(QPoint(0, 0));
    moveCanvas(viewTL.x() - logicPos.x(), viewTL.y() - logicPos.y());
}

void NotationPaintView::moveCanvas(int dx, int dy)
{
    m_matrix.translate(dx, dy);
    update();
}

void NotationPaintView::scrollVertical(int dy)
{
    m_matrix.translate(0, dy);
    update();
}

void NotationPaintView::scrollHorizontal(int dx)
{
    m_matrix.translate(dx, 0);
    update();
}

void NotationPaintView::zoomStep(qreal step, const QPoint& pos)
{
    qreal mag = m_matrix.m11();
    mag *= qPow(1.1, step);
    zoom(mag, pos);
}

void NotationPaintView::zoom(qreal mag, const QPoint& pos)
{
    //! TODO Zoom to point not completed
    mag = qBound(0.05, mag, 16.0);

    qreal cmag = m_matrix.m11();
    if (qFuzzyCompare(mag, cmag)) {
        return;
    }

    qreal deltamag = mag / mag;

    QPointF p1 = m_matrix.inverted().map(pos);

    m_matrix.setMatrix(mag, m_matrix.m12(), m_matrix.m13(), m_matrix.m21(),
                       mag, m_matrix.m23(), m_matrix.dx() * deltamag, m_matrix.dy() * deltamag, m_matrix.m33());

    QPointF p2 = m_matrix.inverted().map(pos);
    QPointF p3 = p2 - p1;
    int dx = std::lrint(p3.x() * cmag);
    int dy = std::lrint(p3.y() * cmag);

    m_matrix.translate(dx, dy);

    update();
}

void NotationPaintView::wheelEvent(QWheelEvent* ev)
{
    m_inputController->wheelEvent(ev);
}

void NotationPaintView::mousePressEvent(QMouseEvent* ev)
{
    m_inputController->mousePressEvent(ev);
}

void NotationPaintView::mouseMoveEvent(QMouseEvent* ev)
{
    m_inputController->mouseMoveEvent(ev);
}

void NotationPaintView::mouseReleaseEvent(QMouseEvent* ev)
{
    m_inputController->mouseReleaseEvent(ev);
}

void NotationPaintView::hoverMoveEvent(QHoverEvent* ev)
{
    m_inputController->hoverMoveEvent(ev);
}

QPoint NotationPaintView::toLogical(const QPoint& p) const
{
    return m_matrix.inverted().map(p);
}

QRect NotationPaintView::toLogical(const QRect& r) const
{
    return m_matrix.inverted().mapRect(r);
}

QPoint NotationPaintView::toPhysical(const QPoint& p) const
{
    return m_matrix.map(p);
}

std::shared_ptr<INotation> NotationPaintView::notation() const
{
    return m_notation;
}
