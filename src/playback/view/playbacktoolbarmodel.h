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
#ifndef MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H
#define MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H

#include <QAbstractListModel>

#include "modularity/ioc.h"
#include "iplaybackcontroller.h"
#include "actions/iactionsdispatcher.h"
#include "actions/iactionsregister.h"
#include "async/asyncable.h"
#include "actions/actiontypes.h"
#include "uicomponents/uicomponentstypes.h"
#include "workspace/iworkspacemanager.h"

namespace mu::playback {
class PlaybackToolBarModel : public QAbstractListModel, public async::Asyncable
{
    Q_OBJECT

    INJECT(playback, actions::IActionsDispatcher, dispatcher)
    INJECT(playback, actions::IActionsRegister, actionsRegister)
    INJECT(playback, IPlaybackController, playbackController)
    INJECT(playback, workspace::IWorkspaceManager, workspaceManager)

public:
    explicit PlaybackToolBarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void handleAction(const QString& action);

private:
    enum Roles {
        CodeRole = Qt::UserRole + 1,
        HintRole,
        IconRole,
        EnabledRole,
        CheckedRole
    };

    void updateState();

    actions::ActionList currentWorkspaceActions() const;

    uicomponents::MenuItem& item(const actions::ActionCode& actionCode);
    uicomponents::MenuItem buildSettingsItem() const;

    QList<uicomponents::MenuItem> m_items;
};
}

#endif // MU_PLAYBACK_PLAYBACKTOOLBARMODEL_H
