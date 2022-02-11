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
#ifndef MU_PROJECT_PROJECTTYPES_H
#define MU_PROJECT_PROJECTTYPES_H

#include <QString>

#include "io/path.h"
#include "log.h"

#include "notation/notationtypes.h"

namespace mu::project {
struct ProjectCreateOptions
{
    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;

    io::path templatePath;

    notation::ScoreCreateOptions scoreOptions;
};

struct MigrationOptions
{
    // common
    int appVersion = 0;
    bool isApplyMigration = false;
    bool isAskAgain = true;

    bool isApplyLeland = true;
    bool isApplyEdwin = true;
    bool isApplyAutoSpacing = true;

    bool isValid() const { return appVersion != 0; }
};

enum class SaveMode
{
    Save,
    SaveAs,
    SaveCopy,
    SaveSelection,
    AutoSave
};

enum class SaveLocationType
{
    None,
    Local,
    Cloud
};

struct SaveLocation {
    struct UnsavedInfo {
        io::path pathOrNameHint;

        QString userFriendlyName() const
        {
            if (pathOrNameHint.empty()) {
                return qtrc("project", "Untitled");
            }

            return io::filename(pathOrNameHint, false).toQString();
        }

        bool operator ==(const UnsavedInfo& other) const
        {
            return pathOrNameHint == other.pathOrNameHint;
        }
    };

    struct LocalInfo {
        io::path path;

        io::path fileName(bool includingExtension = true) const
        {
            return io::filename(path, includingExtension);
        }

        QString userFriendlyName() const
        {
            if (path.empty()) {
                return qtrc("project", "Untitled");
            }

            std::string suffix = io::suffix(path);
            bool isExtensionInteresting = suffix != engraving::MSCZ && !suffix.empty();
            return io::filename(path, isExtensionInteresting).toQString();
        }

        bool operator ==(const LocalInfo& other) const
        {
            return path == other.path;
        }
    };

    struct CloudInfo {
        // TODO

        bool operator ==(const CloudInfo& /*other*/) const
        {
            return true;
        }
    };

    SaveLocationType type = SaveLocationType::None;
    std::variant<UnsavedInfo, LocalInfo, CloudInfo> info;

    bool isUnsaved() const
    {
        return type == SaveLocationType::None
               && std::holds_alternative<UnsavedInfo>(info);
    }

    bool isLocal() const
    {
        return type == SaveLocationType::Local
               && std::holds_alternative<LocalInfo>(info);
    }

    bool isCloud() const
    {
        return type == SaveLocationType::Cloud
               && std::holds_alternative<CloudInfo>(info);
    }

    bool isValid() const
    {
        return isUnsaved() || isLocal() || isCloud();
    }

    const UnsavedInfo& unsavedInfo() const
    {
        IF_ASSERT_FAILED(isUnsaved()) {
            static UnsavedInfo null;
            return null;
        }

        return std::get<UnsavedInfo>(info);
    }

    const LocalInfo& localInfo() const
    {
        IF_ASSERT_FAILED(isLocal()) {
            static LocalInfo null;
            return null;
        }

        return std::get<LocalInfo>(info);
    }

    const CloudInfo& cloudInfo() const
    {
        IF_ASSERT_FAILED(isCloud()) {
            static CloudInfo null;
            return null;
        }

        return std::get<CloudInfo>(info);
    }

    QString userFriendlyName() const
    {
        if (isUnsaved()) {
            return unsavedInfo().userFriendlyName();
        }

        if (isLocal()) {
            return localInfo().userFriendlyName();
        }

        if (isCloud()) {
            return ""; // TODO
        }

        UNREACHABLE;
        return {};
    }

    bool operator ==(const SaveLocation& other) const
    {
        return type == other.type
               && info == other.info;
    }

    static SaveLocation makeUnsaved(const io::path& pathOrNameHint = {})
    {
        return { SaveLocationType::None, UnsavedInfo { pathOrNameHint } };
    }

    static SaveLocation makeLocal(const io::path& path)
    {
        return { SaveLocationType::Local, LocalInfo { path } };
    }

    static SaveLocation makeCloud()
    {
        return { SaveLocationType::Cloud, CloudInfo {} };
    }
};

struct ProjectMeta
{
    SaveLocation saveLocation;

    QString title;
    QString subtitle;
    QString composer;
    QString lyricist;
    QString copyright;
    QString translator;
    QString arranger;
    size_t partsCount = 0;
    QPixmap thumbnail;
    QDate creationDate;

    QString source;
    QString platform;
    QString musescoreVersion;
    int musescoreRevision = 0;
    int mscVersion = 0;

    QVariantMap additionalTags;
};

using ProjectMetaList = QList<ProjectMeta>;

struct Template
{
    QString categoryTitle;
    ProjectMeta meta;
};

using Templates = QList<Template>;

class Migration
{
    Q_GADGET

public:
    enum class Type
    {
        Unknown,
        Pre300,
        Post300AndPre362,
        Ver362
    };
    Q_ENUM(Type)
};

using MigrationType = Migration::Type;

inline std::vector<MigrationType> allMigrationTypes()
{
    static const std::vector<MigrationType> types {
        MigrationType::Pre300,
        MigrationType::Post300AndPre362,
        MigrationType::Ver362
    };

    return types;
}
}

#endif // MU_PROJECT_PROJECTTYPES_H
