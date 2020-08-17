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
#ifndef MU_VST_IVSTINSTANCEREGISTER_H
#define MU_VST_IVSTINSTANCEREGISTER_H

#include "modularity/imoduleexport.h"

namespace mu {
namespace vst {
class PluginInstance;
class IVSTInstanceRegister : MODULE_EXPORT_INTERFACE
{
    INTERFACE_ID(IVSTInstanceRegister)

    using instance_id = int;
    using instance_ptr = std::shared_ptr<PluginInstance>;
public:
    virtual ~IVSTInstanceRegister() = default;

    virtual unsigned int count() = 0;
    virtual instance_id addInstance(instance_ptr instance) = 0;
    virtual instance_ptr instance(instance_id id) = 0;
};
} // namespace vst
} // namespace mu

#endif // MU_VST_IVSTINSTANCEREGISTER_H
