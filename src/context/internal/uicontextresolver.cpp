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
#include "uicontextresolver.h"

#include "diagnostics/diagnosticutils.h"
#include "log.h"
#include "config.h"

using namespace mu::context;
using namespace mu::ui;

static const mu::Uri HOME_PAGE_URI("musescore://home");
static const mu::Uri NOTATION_PAGE_URI("musescore://notation");

static const QString NOTATION_NAVIGATION_PANEL("ScoreView");

void UiContextResolver::init()
{
    interactive()->currentUri().ch.onReceive(this, [this](const Uri&) {
        notifyAboutContextChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        notifyAboutContextChanged();
    });

    globalContext()->currentNotationChanged().onNotify(this, [this]() {
        auto notation = globalContext()->currentNotation();
        if (notation) {
            notation->interaction()->selectionChanged().onNotify(this, [this]() {
                notifyAboutContextChanged();
            });

            notation->interaction()->textEditingStarted().onNotify(this, [this]() {
                notifyAboutContextChanged();
            });

            notation->undoStack()->stackChanged().onNotify(this, [this]() {
                notifyAboutContextChanged();
            });
        }
        notifyAboutContextChanged();
    });

    navigationController()->navigationChanged().onNotify(this, [this]() {
        notifyAboutContextChanged();
    });
}

void UiContextResolver::notifyAboutContextChanged()
{
    m_currentUiContextChanged.notify();
}

UiContext UiContextResolver::currentUiContext() const
{
    TRACEFUNC;
    Uri currentUri = interactive()->currentUri().val;

#ifdef BUILD_DIAGNOSTICS
    currentUri = diagnostics::diagnosticCurrentUri(interactive()->stack());
#endif

    if (currentUri == HOME_PAGE_URI) {
        return context::UiCtxHomeOpened;
    }

    if (currentUri == NOTATION_PAGE_URI) {
        auto notation = globalContext()->currentNotation();
        if (!notation) {
            //! NOTE The notation page is open, but the notation itself is not loaded - we consider that the notation is not open.
            //! We need to think, maybe we need a separate value for this case.
            return context::UiCtxUnknown;
        }

        INavigationPanel* activePanel = navigationController()->activePanel();
        if (activePanel) {
            if (activePanel->name() == NOTATION_NAVIGATION_PANEL) {
                return context::UiCtxNotationFocused;
            }
        } else {
            if (m_notationViewFocusedCounter > 0) {
                return context::UiCtxNotationFocused;
            }
        }

        return context::UiCtxNotationOpened;
    }

    return context::UiCtxUnknown;
}

bool UiContextResolver::match(const ui::UiContext& currentCtx, const ui::UiContext& actCtx) const
{
    if (actCtx == context::UiCtxAny) {
        return true;
    }

    //! NOTE If the current context is `UiCtxNotationFocused`, then we allow `UiCtxNotationOpened` too
    if (currentCtx == context::UiCtxNotationFocused && actCtx == context::UiCtxNotationOpened) {
        return true;
    }

    return currentCtx == actCtx;
}

bool UiContextResolver::matchWithCurrent(const UiContext& ctx) const
{
    if (ctx == ui::UiCtxAny) {
        return true;
    }

    UiContext currentCtx = currentUiContext();
    return match(currentCtx, ctx);
}

mu::async::Notification UiContextResolver::currentUiContextChanged() const
{
    return m_currentUiContextChanged;
}

void UiContextResolver::onNotationViewFocuseChanged(bool focused)
{
    if (focused) {
        m_notationViewFocusedCounter++;
    } else {
        m_notationViewFocusedCounter--;
    }

    notifyAboutContextChanged();
}

bool UiContextResolver::isShortcutContextAllowed(const std::string& scContext) const
{
    //! NOTE If (when) there are many different contexts here,
    //! then the implementation of this method will need to be changed
    //! so that it does not become spaghetti-code.
    //! It would be nice if this context as part of the UI context,
    //! for this we should complicate the implementation of the UI context,
    //! probably make a tree, for example:
    //! NotationOpened
    //!     NotationFocused
    //!         NotationStaffTab

    static const std::string CTX_ANY("any");
    static const std::string CTX_NOTATION_OPENED("notation-opened");
    static const std::string CTX_NOTATION_STAFF_STANDARD("notation-staff-standard");
    static const std::string CTX_NOTATION_STAFF_TAB("notation-staff-tab");

    if (CTX_NOTATION_OPENED == scContext) {
        return matchWithCurrent(context::UiCtxNotationOpened);
    } else if (CTX_NOTATION_STAFF_STANDARD == scContext) {
        auto notation = globalContext()->currentNotation();
        if (!notation) {
            return false;
        }

        return notation->interaction()->noteInput()->state().staffGroup == Ms::StaffGroup::STANDARD;
    } else if (CTX_NOTATION_STAFF_TAB == scContext) {
        auto notation = globalContext()->currentNotation();
        if (!notation) {
            return false;
        }
        return notation->interaction()->noteInput()->state().staffGroup == Ms::StaffGroup::TAB;
    }

    IF_ASSERT_FAILED(CTX_ANY == scContext) {
        return true;
    }
    return true;
}
