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
#include "vstmodule.h"

#include <QQmlEngine>

#include "ui/iinteractiveuriregister.h"
#include "ui/iuiengine.h"
#include "log.h"
#include "settings.h"
#include "modularity/ioc.h"

#include "internal/vstconfiguration.h"
#include "internal/vstpluginrepository.h"
#include "internal/synth/vstsynthesiser.h"

#include "devtools/vstpluginlistmodelexample.h"
#include "view/vstplugineditorview.h"

using namespace mu::vst;
using namespace mu::modularity;
using namespace mu::ui;

static std::shared_ptr<IVstConfiguration> s_configuration = std::make_shared<VstConfiguration>();
static std::shared_ptr<IVstPluginRepository> s_pluginRepo = std::make_shared<VstPluginRepository>();

static void vst_init_qrc()
{
    Q_INIT_RESOURCE(vst);
}

std::string VSTModule::moduleName() const
{
    return "vst";
}

void VSTModule::registerExports()
{
    ioc()->registerExport<IVstConfiguration>(moduleName(), s_configuration);
    ioc()->registerExport<IVstModulesRepository>(moduleName(), s_pluginModulesRepo);
    ioc()->registerExport<IVstPluginsRegister>(moduleName(), s_pluginsRegister);
}

void VSTModule::resolveImports()
{
    auto ir = ioc()->resolve<IInteractiveUriRegister>(moduleName());
    if (ir) {
        ir->registerUri(Uri("musescore://vsti/editor"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<VstiEditorView>("VstiEditorView")));

        ir->registerUri(Uri("musescore://vstfx/editor"),
                        ContainerMeta(ContainerType::QWidgetDialog, qRegisterMetaType<VstFxEditorView>("VstFxEditorView")));
    }

    auto synthResolver = ioc()->resolve<ISynthResolver>(moduleName());
    if (synthResolver) {
        synthResolver->registerResolver(AudioSourceType::Vsti, std::make_shared<VstiResolver>());
    }

    auto fxResolver = ioc()->resolve<IFxResolver>(moduleName());
    if (fxResolver) {
        fxResolver->registerResolver(AudioFxType::Vst, std::make_shared<VstFxResolver>());
    }
}

void VSTModule::registerResources()
{
    vst_init_qrc();
}

void VSTModule::registerUiTypes()
{
    qmlRegisterType<VstPluginListModelExample>("MuseScore.Vst", 1, 0, "VstPluginListModelExample");

    ioc()->resolve<ui::IUiEngine>(moduleName())->addSourceImportPath(vst_QML_IMPORT);
}

void VSTModule::onInit(const framework::IApplication::RunMode& mode)
{
    if (framework::IApplication::RunMode::Editor != mode) {
        return;
    }

    s_pluginRepo->loadAvailablePlugins();

    //!Note Please, don't remove this code, needed for tests of VST implementation
    /*auto sreg = ioc()->resolve<audio::synth::ISynthesizersRegister>(moduleName());

    if (sreg) {
        sreg->registerSynthesizer("Vst", std::make_shared<VstSynthesiser>(s_pluginRepo->pluginsMetaList().val.at(0)));
    }*/
}
