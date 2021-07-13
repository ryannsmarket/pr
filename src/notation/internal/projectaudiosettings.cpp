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
#include "projectaudiosettings.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace mu::notation;

int ProjectAudioSettings::someValue() const
{
    return m_someValue;
}

void ProjectAudioSettings::setSomeValue(int val)
{
    m_someValue = val;
}

mu::Ret ProjectAudioSettings::read(const engraving::MsczReader& reader)
{
    QByteArray json = reader.readAudioSettingsJsonFile();
    QJsonObject rootObj = QJsonDocument::fromJson(json).object();

    QJsonObject exampleObj = rootObj.value("example").toObject();
    m_someValue = exampleObj.value("someValue").toInt();
}

mu::Ret ProjectAudioSettings:: write(engraving::MsczWriter& writer)
{
    QJsonObject exampleObj;
    exampleObj["someValue"] = m_someValue;

    QJsonObject rootObj;
    rootObj["example"] = exampleObj;

    QByteArray json = QJsonDocument(rootObj).toJson();
    writer.writeAudioSettingsJsonFile(json);
}
