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

#include "commonaudioapiconfigurationmodel.h"

#include "audio/audiotypes.h"

#include "translation.h"
#include "log.h"

using namespace mu::appshell;
using namespace mu::audio;

CommonAudioApiConfigurationModel::CommonAudioApiConfigurationModel(QObject* parent)
    : QObject(parent)
{
}

void CommonAudioApiConfigurationModel::load()
{
    audioDriver()->availableOutputDevicesChanged().onNotify(this, [this]() {
        emit deviceListChanged();
    });

    audioDriver()->outputDeviceChanged().onNotify(this, [this]() {
        emit currentDeviceIdChanged();
        emit bufferSizeChanged();
        emit sampleRateChanged();
    });

    audioDriver()->bufferSizeChanged().onNotify(this, [this]() {
        emit bufferSizeChanged();
    });

    audioDriver()->sampleRateChanged().onNotify(this, [this]() {
        emit sampleRateChanged(); // todo
    });
}

QString CommonAudioApiConfigurationModel::currentDeviceId() const
{
    return QString::fromStdString(audioDriver()->outputDevice());
}

QVariantList CommonAudioApiConfigurationModel::deviceList() const
{
    QVariantList result;

    AudioDeviceList devices = audioDriver()->availableOutputDevices();
    for (const AudioDevice& device : devices) {
        QVariantMap obj;
        obj["value"] = QString::fromStdString(device.id);
        obj["text"] = QString::fromStdString(device.name);

        result << obj;
    }

    return result;
}

void CommonAudioApiConfigurationModel::deviceSelected(const QString& deviceId)
{
    audioConfiguration()->setAudioOutputDeviceId(deviceId.toStdString());
}

unsigned int CommonAudioApiConfigurationModel::bufferSize() const
{
    return audioDriver()->bufferSize(currentDeviceId().toStdString());
}

QList<unsigned int> CommonAudioApiConfigurationModel::bufferSizeList() const
{
    QList<unsigned int> result;
    std::pair<unsigned int, unsigned int> bufferSizeRange = audioDriver()->availableBufferSizeRange(currentDeviceId().toStdString());

    unsigned int start = bufferSizeRange.first;
    unsigned int end = bufferSizeRange.second;

    if (start == end) { //todo
        return {};
    }

    for (unsigned int bufferSize = end; bufferSize >= start;) {
        result.prepend(bufferSize);
        bufferSize /= 2;
    }

    return result;
}

void CommonAudioApiConfigurationModel::bufferSizeSelected(const QString& bufferSizeStr)
{
    audioConfiguration()->setDriverBufferSize(bufferSizeStr.toInt());
}

int CommonAudioApiConfigurationModel::sampleRate() const
{
    return audioDriver()->sampleRate(currentDeviceId().toStdString());
}

QVariantList CommonAudioApiConfigurationModel::sampleRateList() const
{
    QVariantList result;

    std::vector<unsigned int> sampleRates = audioDriver()->availableSampleRates(currentDeviceId().toStdString());
    for (unsigned int sampleRate : sampleRates) {
        QVariantMap obj;
        obj["value"] = sampleRate;
        obj["text"] = QString::number(sampleRate) + " " + qtrc("global", "Hz");

        result << obj;
    }

    return result;
}

void CommonAudioApiConfigurationModel::sampleRateSelected(const QString& sampleRateStr)
{
    audioConfiguration()->setSampleRate(sampleRateStr.toInt());
}
