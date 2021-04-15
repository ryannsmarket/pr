/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/

#ifndef MU_TELEMETRY_TELEMETRYPERMISSIONMODEL_H
#define MU_TELEMETRY_TELEMETRYPERMISSIONMODEL_H

#include <QObject>

#include "modularity/ioc.h"
#include "itelemetryconfiguration.h"

namespace mu::telemetry {
class TelemetryPermissionModel : public QObject
{
    Q_OBJECT

    INJECT(telemetry, ITelemetryConfiguration, configuration)

public:
    explicit TelemetryPermissionModel(QObject* parent = nullptr);

    Q_INVOKABLE void allowUseTelemetry();
    Q_INVOKABLE void forbidUseTelemetry();
};
}

#endif // MU_TELEMETRY_TELEMETRYPERMISSIONMODEL_H
