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
#include "rwregister.h"

#include "114/read114.h"
#include "206/read206.h"
#include "302/read302.h"
#include "400/read400.h"

#include "writer/writer.h"

using namespace mu::engraving;
using namespace mu::engraving::rw;

static const int LATEST_VERSION(400);

IReaderPtr RWRegister::reader(int version)
{
    if (version <= 114) {
        return std::make_shared<compat::Read114>();
    } else if (version <= 207) {
        return std::make_shared<compat::Read206>();
    } else if (version < 400 || MScore::testMode) {
        return std::make_shared<compat::Read302>();
    }

    return std::make_shared<rw400::Read400>();
}

IReaderPtr RWRegister::latestReader()
{
    return reader(LATEST_VERSION);
}

IWriterPtr RWRegister::latestWriter()
{
    return std::make_shared<rw400::Write400>();
}
