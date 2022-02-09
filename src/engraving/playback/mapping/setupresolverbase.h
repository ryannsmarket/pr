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

#ifndef MU_ENGRAVING_SETUPRESOLVERBASE_H
#define MU_ENGRAVING_SETUPRESOLVERBASE_H

#include "mpe/events.h"

#include "libmscore/instrument.h"

namespace mu::engraving {
template<class T>
class SetupDataResolverBase
{
public:
    static bool isAbleToResolve(const Ms::Instrument* instrument)
    {
        IF_ASSERT_FAILED(instrument) {
            return false;
        }

        return T::supportsInstrument(instrument);
    }

    static void resolve(const Ms::Instrument* instrument, mpe::PlaybackSetupData& result)
    {
        IF_ASSERT_FAILED(instrument) {
            return;
        }

        result = T::doResolve(instrument);
    }
};
}

#endif // MU_ENGRAVING_SETUPRESOLVERBASE_H
