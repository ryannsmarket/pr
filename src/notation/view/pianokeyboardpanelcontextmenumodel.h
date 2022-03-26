/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
#ifndef MU_NOTATION_PIANOKEYBOARDPANELCONTEXTMENUMODEL_H
#define MU_NOTATION_PIANOKEYBOARDPANELCONTEXTMENUMODEL_H

#include "uicomponents/view/abstractmenumodel.h"
#include "actions/actionable.h"

#include "modularity/ioc.h"
#include "inotationconfiguration.h"
#include "actions/iactionsdispatcher.h"

namespace mu::notation {
class PianoKeyboardPanelContextMenuModel : public uicomponents::AbstractMenuModel, public actions::Actionable
{
    Q_OBJECT

    INJECT(notation, INotationConfiguration, configuration)
    INJECT(notation, actions::IActionsDispatcher, dispatcher)

    Q_PROPERTY(int numberOfKeys READ numberOfKeys NOTIFY numberOfKeysChanged)

public:
    explicit PianoKeyboardPanelContextMenuModel(QObject* parent = nullptr);

    Q_INVOKABLE void load() override;

    int numberOfKeys() const;

signals:
    void numberOfKeysChanged();

private:
    uicomponents::MenuItem* makeViewMenu();

    uicomponents::MenuItem* makeNumberOfKeysItem(const QString& title, int numberOfKeys);

    uicomponents::MenuItemList m_numberOfKeysItems;
};
}

#endif // MU_NOTATION_PIANOKEYBOARDPANELCONTEXTMENUMODEL_H
