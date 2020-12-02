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

#include "docktoolbar.h"

#include <QToolBar>

#include "eventswatcher.h"

using namespace mu::dock;

static const QString qss = QString("QToolBar { background: %1; border: 0; padding: 0; }");

DockToolBar::DockToolBar(QQuickItem* parent)
    : DockView(parent)
{
    m_tool.bar = new QToolBar();
    m_tool.bar->setAllowedAreas(Qt::AllToolBarAreas);
    m_tool.bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(this, &QQuickItem::visibleChanged, this, [this]() {
        m_tool.bar->setVisible(isVisible());
    });

    connect(m_tool.bar, &QToolBar::orientationChanged, [this](int orientation) {
        emit orientationChanged(orientation);
    });

    m_eventsWatcher = new EventsWatcher(this);
    m_tool.bar->installEventFilter(m_eventsWatcher);
    connect(m_eventsWatcher, &EventsWatcher::eventReceived, this, &DockToolBar::onToolbarEvent);
}

DockToolBar::~DockToolBar()
{
    delete m_tool.bar;
}

void DockToolBar::onComponentCompleted()
{
    m_tool.bar->setObjectName("w_" + objectName());
    m_tool.bar->setStyleSheet(qss.arg(color().name()));

    QWidget* w = view();
    w->setMinimumWidth(minimumWidth());
    w->setMinimumHeight(minimumHeight());
    m_tool.bar->addWidget(w);
}

void DockToolBar::updateStyle()
{
    m_tool.bar->setStyleSheet(qss.arg(color().name()));
}

void DockToolBar::onToolbarEvent(QEvent* e)
{
    if (QEvent::Resize == e->type()) {
        QResizeEvent* re = static_cast<QResizeEvent*>(e);
        view()->resize(re->size());
    } else if (QEvent::ShowToParent == e->type()) {
        view()->resize(m_tool.bar->size());
    }
}

DockToolBar::Widget DockToolBar::widget() const
{
    return m_tool;
}

int DockToolBar::orientation() const
{
    return m_tool.bar->orientation();
}

int DockToolBar::minimumHeight() const
{
    return m_minimumHeight;
}

int DockToolBar::minimumWidth() const
{
    return m_minimumWidth;
}

void DockToolBar::setMinimumHeight(int minimumHeight)
{
    if (m_minimumHeight == minimumHeight) {
        return;
    }

    m_minimumHeight = minimumHeight;
    if (view()) {
        view()->setMinimumHeight(minimumHeight);
    }

    emit minimumHeightChanged(m_minimumHeight);
}

void DockToolBar::setMinimumWidth(int minimumWidth)
{
    if (m_minimumWidth == minimumWidth) {
        return;
    }

    m_minimumWidth = minimumWidth;
    if (view()) {
        view()->setMinimumWidth(minimumWidth);
    }
    emit minimumWidthChanged(m_minimumWidth);
}
