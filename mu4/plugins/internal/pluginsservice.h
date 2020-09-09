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

#ifndef MU_PLUGINS_PLUGINSSERVICE_H
#define MU_PLUGINS_PLUGINSSERVICE_H

#include "ipluginsservice.h"
#include "ipluginsconfiguration.h"
#include "system/ifilesystem.h"
#include "network/inetworkmanagercreator.h"
#include "framework/ui/iuiengine.h"

#include "modularity/ioc.h"
#include "io/path.h"
#include "async/asyncable.h"

namespace mu::plugins {
class PluginsService : public IPluginsService, public async::Asyncable
{
    INJECT(plugins, IPluginsConfiguration, configuration)
    INJECT(plugins, framework::IFileSystem, fileSystem)
    INJECT(plugins, framework::INetworkManagerCreator, networkManagerCreator)
    INJECT(plugins, framework::IUiEngine, uiEngine)

public:
    void init();

    mu::RetVal<PluginList> plugins(PluginsStatus status = PluginsStatus::All) const override;

    RetValCh<framework::Progress> install(const CodeKey& codeKey) override;
    RetValCh<framework::Progress> update(const CodeKey& codeKey) override;
    Ret uninstall(const CodeKey& codeKey) override;

    Ret start(const CodeKey& codeKey) override;
    Ret stop(const CodeKey& codeKey) override;

    async::Channel<Plugin> pluginChanged() const override;

private:
    bool isAccepted(const CodeKey& codeKey, PluginsStatus status) const;

    CodeKeyList installedPlugins() const;
    void setInstalledPlugins(const CodeKeyList& codeKeyList);

    void startPlugins(const CodeKeyList& codeKeyList);
    bool isInstalled(const CodeKey &codeKey) const;

    PluginList readPlugins() const;
    Plugin readPlugin(const io::path& path) const;
    io::paths scanFileSystemForPlugins() const;

    void notifyAboutPluginChanged(const CodeKey& codeKey);
    Plugin plugin(const CodeKey& codeKey) const;

    async::Channel<Plugin> m_pluginChanged;
};
}

#endif // MU_PLUGINS_PLUGINSSERVICE_H
