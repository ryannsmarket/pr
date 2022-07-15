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
#ifndef MU_NOTATION_INOTATIONVIEWSTATE_H
#define MU_NOTATION_INOTATIONVIEWSTATE_H

#include <memory>

#include "async/channel.h"
#include "retval.h"

#include "draw/types/transform.h"

namespace mu::notation {
class NotationPaintView;
class INotationViewState
{
public:
    virtual ~INotationViewState() = default;

    virtual bool isMatrixInited() const = 0;
    virtual void setMatrixInited(bool inited) = 0;

    virtual draw::Transform matrix() const = 0;
    virtual async::Channel<draw::Transform /*newMatrix*/, NotationPaintView* /*sender*/> matrixChanged() const = 0;
    virtual void setMatrix(const draw::Transform& matrix, NotationPaintView* sender) = 0;

    virtual ValCh<int> zoomPercentage() const = 0;
};

using INotationViewStatePtr = std::shared_ptr<INotationViewState>;
}

#endif // MU_NOTATION_INOTATIONVIEWSTATE_H
