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
#include "importgtp.h"

#include "serialization/zipreader.h"

#include "gtp/gp7dombuilder.h"
#include "libmscore/factory.h"
#include "libmscore/bracketItem.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/masterscore.h"
#include "libmscore/part.h"
#include "libmscore/staff.h"

using namespace mu::io;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   read
//---------------------------------------------------------

bool GuitarPro7::read(IODevice* io, bool createLinkedTabForce)
{
    f = io;
    previousTempo = -1;

    mu::ZipReader zip(io);
    mu::ByteArray fileData = zip.fileData("Content/score.gpif");
    mu::ByteArray partsData = zip.fileData("Content/PartConfiguration");
    zip.close();

    QByteArray partsArray = partsData.toQByteArrayNoCopy();
    IGPDomBuilder::GPProperties properties = readProperties(&partsArray);
    properties.createLinkedTabForce = createLinkedTabForce;

    QByteArray ba = fileData.toQByteArrayNoCopy();
    readGpif(&ba, properties);
    return true;
}

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

IGPDomBuilder::GPProperties GuitarPro7::readProperties(QByteArray* data)
{
    IGPDomBuilder::GPProperties properties;
    size_t partsInfoSize = data->size();
    const int numInstrOffset = 8;
    if (partsInfoSize <= numInstrOffset) {
        LOGE() << "failed to read gp properties";
        return properties;
    }

    int numberOfInstruments = static_cast<int>(data->at(numInstrOffset));
    if (partsInfoSize <= numInstrOffset + numberOfInstruments) {
        LOGE() << "failed to read gp properties";
        return properties;
    }

    using import_option_t = IGPDomBuilder::TabImportOption;
    std::vector<import_option_t>& partsImportOpts = properties.partsImportOptions;

    for (int i = numInstrOffset + 1; i <= numInstrOffset + numberOfInstruments; i++) {
        partsImportOpts.push_back(static_cast<import_option_t>(data->at(i)));
    }

    return properties;
}

std::unique_ptr<IGPDomBuilder> GuitarPro7::createGPDomBuilder() const
{
    return std::make_unique<GP7DomBuilder>();
}
}
