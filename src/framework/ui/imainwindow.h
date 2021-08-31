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
#ifndef MU_UI_IMAINWINDOW_H
#define MU_UI_IMAINWINDOW_H

#include "modularity/imoduleexport.h"

class QWindow;
class QScreen;

namespace mu::ui {
class IMainWindow : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IMainWindow)

public:
    virtual ~IMainWindow() = default;

    virtual QWindow* qWindow() const = 0;

    virtual QWindow* topWindow() const = 0;
    virtual void pushWindow(QWindow* w) = 0;
    virtual void popWindow(QWindow* w) = 0;

    virtual void requestShowOnBack() = 0;
    virtual void requestShowOnFront() = 0;

    virtual bool isFullScreen() const = 0;
    virtual void toggleFullScreen() = 0;
    virtual const QScreen* screen() const = 0;
};
}

#endif // MU_UI_IMAINWINDOW_H
