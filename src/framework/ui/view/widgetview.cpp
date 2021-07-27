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

#include "widgetview.h"

#include <QWidget>

using namespace mu::ui;

WidgetView::WidgetView(QQuickItem* parent)
    : QQuickPaintedItem(parent)
{
    setFlag(QQuickItem::ItemAcceptsDrops, true);
    setFlag(QQuickItem::ItemHasContents, true);

    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

void WidgetView::paint(QPainter* painter)
{
    if (qWidget()) {
        qWidget()->render(painter, QPoint(), QRegion(),
                          QWidget::DrawWindowBackground | QWidget::DrawChildren);
    }
}

bool WidgetView::event(QEvent* event)
{
    if (!m_widget) {
        return QQuickItem::event(event);
    }

    bool ok = true;

    switch (event->type()) {
    case QEvent::HoverEnter:
    case QEvent::HoverMove:
    case QEvent::HoverLeave:
        ok = handleHoverEvent(dynamic_cast<QHoverEvent*>(event));
        break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
        setFocus(true);
        [[fallthrough]];
    default:
        ok = m_widget->handleEvent(event);
        break;
    }

    if (ok) {
        update();
    }

    return ok;
}

bool WidgetView::handleHoverEvent(QHoverEvent* event)
{
    QMouseEvent mouseEvent(QEvent::MouseMove, event->posF(),
                           Qt::NoButton, Qt::NoButton, event->modifiers());
    mouseEvent.setAccepted(event->isAccepted());
    mouseEvent.setTimestamp(event->timestamp());
    bool ok = m_widget->handleEvent(&mouseEvent);
    setCursor(qWidget()->cursor());
    return ok;
}

void WidgetView::componentComplete()
{
    QQuickItem::componentComplete();

    connect(this, &QQuickItem::widthChanged, [this]() {
        updateSizeConstraints();
    });

    connect(this, &QQuickItem::heightChanged, [this]() {
        updateSizeConstraints();
    });
}

QWidget* WidgetView::qWidget() const
{
    return m_widget ? m_widget->qWidget() : nullptr;
}

void WidgetView::updateSizeConstraints()
{
    if (qWidget()) {
        qWidget()->setMinimumSize(width(), height());
    }
}

void WidgetView::setWidget(std::shared_ptr<IDisplayableWidget> widget)
{
    m_widget = widget;
}
