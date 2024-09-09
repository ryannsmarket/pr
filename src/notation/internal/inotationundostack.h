/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

#ifndef MU_SCENE_NOTATION_INOTATIONUNDOSTACK_H
#define MU_SCENE_NOTATION_INOTATIONUNDOSTACK_H

#include "async/notification.h"
#include "async/channel.h"

#include "notation/notationtypes.h"

namespace mu::engraving {
class EditData;
enum class ElementType;
}

namespace mu::notation {
class INotationUndoStack
{
public:
    virtual ~INotationUndoStack() = default;

    virtual bool canUndo() const = 0;
    virtual void undo(mu::engraving::EditData*) = 0;
    virtual muse::async::Notification undoNotification() const = 0;

    virtual bool canRedo() const = 0;
    virtual void redo(mu::engraving::EditData*) = 0;
    virtual muse::async::Notification redoNotification() const = 0;

    virtual void prepareChanges(const muse::TranslatableString& actionName) = 0;
    virtual void rollbackChanges() = 0;
    virtual void commitChanges() = 0;

    virtual bool isStackClean() const = 0;

    virtual void lock() = 0;
    virtual void unlock() = 0;
    virtual bool isLocked() const = 0;

    virtual muse::TranslatableString topMostUndoActionName() const = 0;
    virtual muse::TranslatableString topMostRedoActionName() const = 0;

    virtual muse::async::Notification stackChanged() const = 0;
    virtual muse::async::Channel<ChangesRange> changesChannel() const = 0;
};

using INotationUndoStackPtr = std::shared_ptr<INotationUndoStack>;
}

#endif // MU_SCENE_NOTATION_INOTATIONUNDOSTACK_H
