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

#include "vstfxeditorview.h"

#include <QWindow>
#include <QResizeEvent>

#include "async/async.h"

#include "vsttypes.h"
#include "internal/vstplugin.h"

using namespace mu::vst;
using namespace Steinberg;

IMPLEMENT_FUNKNOWN_METHODS(VstFxEditorView, IPlugFrame, IPlugFrame::iid)

VstFxEditorView::VstFxEditorView(QWidget* parent)
    : QDialog(parent)
{
    setAttribute(Qt::WA_NativeWindow, true);
}

VstFxEditorView::VstFxEditorView(const VstFxEditorView& copy)
    : VstFxEditorView(copy.parentWidget())
{
}

VstFxEditorView::~VstFxEditorView()
{
    if (m_view) {
        m_view->removed();
    }
}

tresult VstFxEditorView::resizeView(IPlugView* view, ViewRect* newSize)
{
    setGeometry(QRect(geometry().x(), geometry().y(), newSize->getWidth(), newSize->getHeight()));
    view->onSize(newSize);

    return kResultTrue;
}

void VstFxEditorView::wrapPluginView()
{
    VstPluginPtr plugin;

    if (m_trackId == -1) {
        plugin = pluginsRegister()->masterFxPlugin(m_resourceId.toStdString());
    } else {
        plugin = pluginsRegister()->fxPlugin(m_trackId, m_resourceId.toStdString());
    }

    if (!plugin || !plugin->view()) {
        return;
    }

    m_view = plugin->view();

    if (m_view->isPlatformTypeSupported(currentPlatformUiType()) != Steinberg::kResultTrue) {
        return;
    }

    m_view->setFrame(this);

    Steinberg::tresult attached;
    attached = m_view->attached(reinterpret_cast<void*>(windowHandle()->winId()), currentPlatformUiType());
    if (attached != kResultOk) {
        return;
    }
}

FIDString VstFxEditorView::currentPlatformUiType() const
{
#ifdef Q_OS_MAC
    return Steinberg::kPlatformTypeNSView;
#elif defined(Q_OS_IOS)
    return Steinberg::kPlatformTypeUIView;
#elif defined(Q_OS_WIN)
    return Steinberg::kPlatformTypeHWND;
#else
    return Steinberg::kPlatformTypeX11EmbedWindowID;
#endif
}

int VstFxEditorView::trackId() const
{
    return m_trackId;
}

void VstFxEditorView::setTrackId(int newTrackId)
{
    if (m_trackId == newTrackId) {
        return;
    }
    m_trackId = newTrackId;
    emit trackIdChanged();

    if (!m_resourceId.isEmpty()) {
        wrapPluginView();
    }
}

const QString& VstFxEditorView::resourceId() const
{
    return m_resourceId;
}

void VstFxEditorView::setResourceId(const QString& newResourceId)
{
    if (m_resourceId == newResourceId) {
        return;
    }
    m_resourceId = newResourceId;
    emit resourceIdChanged();

    if (!m_resourceId.isEmpty()) {
        wrapPluginView();
    }
}
