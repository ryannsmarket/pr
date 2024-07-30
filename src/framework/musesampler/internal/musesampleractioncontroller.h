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
#ifndef MUSE_MUSESAMPLER_MUSESAMPLERACTIONCONTROLLER_H
#define MUSE_MUSESAMPLER_MUSESAMPLERACTIONCONTROLLER_H

#include "actions/actionable.h"
#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"
#include "iinteractive.h"
#include "imusesamplerinfo.h"

namespace muse::musesampler {
class MuseSamplerActionController : public Injectable, public actions::Actionable, public async::Asyncable
{
    Inject<actions::IActionsDispatcher> dispatcher = { this };
    Inject<IInteractive> interactive = { this };
    Inject<IMuseSamplerInfo> museSamplerInfo = { this };

public:
    MuseSamplerActionController(const modularity::ContextPtr& iocCtx)
        : Injectable(iocCtx) {}

    using ReloadMuseSamplerFunc = std::function<bool ()>;

    void init(const ReloadMuseSamplerFunc& reloadMuseSampler);

private:
    void checkLibraryIsDetected();
    void reloadMuseSampler();

    ReloadMuseSamplerFunc m_reloadMuseSampler;
};
}

#endif // MUSE_MUSESAMPLER_MUSESAMPLERACTIONCONTROLLER_H
