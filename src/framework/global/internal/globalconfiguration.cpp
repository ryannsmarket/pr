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
#include "globalconfiguration.h"

#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>

#include "config.h"
#include "settings.h"

using namespace mu;
using namespace mu::framework;

static const Settings::Key BACKUP_KEY("global", "application/backup/subfolder");

io::path GlobalConfiguration::appDirPath() const
{
    return io::path(QCoreApplication::applicationDirPath());
}

io::path GlobalConfiguration::sharePath() const
{
    if (m_sharePath.empty()) {
        m_sharePath = getSharePath();
    }

    return m_sharePath;
}

io::path GlobalConfiguration::dataPath() const
{
#if defined(WIN_PORTABLE)
    if (m_dataPath.empty()) {
        m_dataPath = QDir::cleanPath(QString("%1/../../../Data/settings")
                                     .arg(QCoreApplication::applicationDirPath())
                                     .arg(QCoreApplication::applicationName()));
    }
#elif defined(Q_OS_WASM)
    if (m_dataPath.empty()) {
        m_dataPath = std::string("/files/data");
    }
#else
    if (m_dataPath.empty()) {
        m_dataPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    }
#endif
    return m_dataPath;
}

QString GlobalConfiguration::getSharePath() const
{
#ifdef Q_OS_WIN
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../" INSTALL_NAME));
    return dir.absolutePath() + "/";
#elif defined(Q_OS_MAC)
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../Resources"));
    return dir.absolutePath() + "/";
#elif defined(Q_OS_WASM)
    return "/files/share";
#else
    // Try relative path (needed for portable AppImage and non-standard installations)
    QDir dir(QCoreApplication::applicationDirPath() + QString("/../share/" INSTALL_NAME));
    if (dir.exists()) {
        return dir.absolutePath() + "/";
    }
    // Otherwise fall back to default location (e.g. if binary has moved relative to share)
    return QString(INSTPREFIX "/share/" INSTALL_NAME);
#endif
}

io::path GlobalConfiguration::logsPath() const
{
    return dataPath() + "/logs";
}

io::path GlobalConfiguration::backupPath() const
{
    return settings()->value(BACKUP_KEY).toString();
}

bool GlobalConfiguration::useFactorySettings() const
{
    return false;
}

bool GlobalConfiguration::enableExperimental() const
{
    return false;
}
