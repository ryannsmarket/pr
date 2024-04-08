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

#include "dialogview.h"

#include <QStyle>
#include <QWindow>
#include <QScreen>
#include <QApplication>

#include "log.h"

using namespace muse::uicomponents;

static const int DIALOG_WINDOW_FRAME_HEIGHT(20);

DialogView::DialogView(QQuickItem* parent)
    : PopupView(parent)
{
    setObjectName("DialogView");
    setClosePolicies(NoAutoClose);
}

bool DialogView::isDialog() const
{
    return true;
}

void DialogView::onHidden()
{
    PopupView::onHidden();

    if (m_loop.isRunning()) {
        m_loop.exit();
    }
}

QScreen* DialogView::resolveScreen() const
{
    QWindow* qMainWindow = mainWindow()->qWindow();
    QScreen* mainWindowScreen = qMainWindow->screen();
    if (!mainWindowScreen) {
        mainWindowScreen = QGuiApplication::primaryScreen();
    }

    return mainWindowScreen;
}

void DialogView::updateGeometry()
{
    const QScreen* screen = resolveScreen();
    QRect anchorRect = screen->availableGeometry();

    const QWindow* qMainWindow = mainWindow()->qWindow();
    bool mainWindowVisible = qMainWindow->isVisible();
    QRect referenceRect = qMainWindow->geometry();
    int frameHeight = frameless() ? 0 : DIALOG_WINDOW_FRAME_HEIGHT;

    if (referenceRect.isEmpty() || !mainWindowVisible) {
        referenceRect = anchorRect;
    }

    QRect dlgRect = viewGeometry();
    qreal dlgActualWidth = contentItem()->width();
    qreal dlgActualHeight = contentItem()->height() + frameHeight;

    // position the dialog in the center of the main window
    dlgRect.moveLeft(referenceRect.x() + (referenceRect.width() - dlgActualWidth) / 2);
    dlgRect.moveTop(referenceRect.y() + (referenceRect.height() - dlgActualHeight) / 2);

    dlgRect.moveLeft(dlgRect.x() + m_localPos.x());
    dlgRect.moveTop(dlgRect.y() + m_localPos.y());

    // try to move the dialog if it doesn't fit on the screen

    int titleBarHeight = QApplication::style()->pixelMetric(QStyle::PM_TitleBarHeight);

    if (dlgRect.left() <= anchorRect.left()) {
        dlgRect.moveLeft(anchorRect.left() + frameHeight);
    }

    if (dlgRect.top() - titleBarHeight <= anchorRect.top()) {
        dlgRect.moveTop(anchorRect.top() + titleBarHeight + frameHeight);
    }

    if (dlgRect.right() >= anchorRect.right()) {
        dlgRect.moveRight(anchorRect.right() - frameHeight);
    }

    if (dlgRect.bottom() >= anchorRect.bottom()) {
        dlgRect.moveBottom(anchorRect.bottom() - frameHeight);
    }

    // if after moving the dialog does not fit on the screen, then adjust the size of the dialog
    if (!anchorRect.contains(dlgRect)) {
        anchorRect -= QMargins(frameHeight, frameHeight + titleBarHeight,
                               frameHeight, frameHeight);
        dlgRect = anchorRect.intersected(dlgRect);
    }

    m_globalPos = dlgRect.topLeft();

    setContentWidth(dlgRect.width());
    setContentHeight(dlgRect.height());

    if (m_window) {
        m_window->show(resolveScreen(), dlgRect, m_openPolicy != OpenPolicy::NoActivateFocus);
    }
}

QRect DialogView::viewGeometry() const
{
    return QRect(m_globalPos.toPoint(), QSize(contentWidth(), contentHeight()));
}

void DialogView::exec()
{
    open();
    m_loop.exec();

    activateNavigationParentControl();
}

void DialogView::show()
{
    open();
}

void DialogView::hide()
{
    close();
}

void DialogView::raise()
{
    if (isOpened()) {
        m_window->raise();
    }
}

void DialogView::accept()
{
    setErrCode(Ret::Code::Ok);
    close();
}

void DialogView::reject(int code)
{
    if (code > 0) {
        setErrCode(static_cast<Ret::Code>(code));
    } else {
        setErrCode(Ret::Code::Cancel);
    }

    close();
}
