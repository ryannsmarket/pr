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
#ifndef MU_MI_MULTIINSTANCESPROVIDER_H
#define MU_MI_MULTIINSTANCESPROVIDER_H

#include "../imultiinstancesprovider.h"

#include <functional>
#include <QEventLoop>
#include <QTimer>

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "actions/actionable.h"
#include "iinteractive.h"
#include "async/asyncable.h"
#include "ipc/ipcchannel.h"

namespace mu::mi {
class MultiInstancesProvider : public IMultiInstancesProvider, public actions::Actionable, public async::Asyncable
{
    INJECT(mi, actions::IActionsDispatcher, dispatcher)
    INJECT(mi, framework::IInteractive, interactive)

public:
    MultiInstancesProvider() = default;
    ~MultiInstancesProvider();

    void init();

    bool isScoreAlreadyOpened(const io::path& scorePath) const override;
    void activateWindowForScore(const io::path& scorePath) override;

    const std::string& selfID() const override;
    std::vector<InstanceMeta> instances() const override;
    async::Notification instancesChanged() const override;

private:

    void onMsg(const ipc::Msg& msg);

    ipc::IpcChannel* m_ipcChannel = nullptr;
    std::string m_selfID;

    async::Notification m_instancesChanged;
};
}

#endif // MU_MI_MULTIINSTANCESPROVIDER_H
