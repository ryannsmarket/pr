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
#include "pluginsmodule.h"

#include <QQmlEngine>

#include "modularity/ioc.h"
#include "ui/iuiengine.h"

using namespace mu::plugins;

static void plugins_init_qrc()
{
    Q_INIT_RESOURCE(plugins);
}

std::string PluginsModule::moduleName() const
{
    return "plugins";
}

void PluginsModule::registerExports()
{
}

void PluginsModule::registerResources()
{
    plugins_init_qrc();
}

void PluginsModule::registerUiTypes()
{
    framework::ioc()->resolve<framework::IUiEngine>(moduleName())->addSourceImportPath(plugins_QML_IMPORT);
}

void PluginsModule::onInit()
{
}
