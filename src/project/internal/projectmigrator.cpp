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
#include "projectmigrator.h"

#include "engraving/libmscore/undo.h"

#include "log.h"

using namespace mu::project;

static const Uri MIGRATION_DIALOG_URI("musescore://project/migration");

Ret ProjectMigrator::migrateEngravingProjectIfNeed(engraving::EngravingProjectPtr project)
{
    if (!(project->mscVersion() < Ms::MSCVERSION)) {
        return true;
    }

    MigrationOptions migrationOptions = configuration()->migrationOptions();
    if (migrationOptions.isAskAgain) {
        Ret ret = askAboutMigration(migrationOptions, project);
        if (!ret) {
            return ret;
        }

        migrationOptions.appVersion = Ms::MSCVERSION;
        configuration()->setMigrationOptions(migrationOptions);
    }

    if (!migrationOptions.isApplyMigration) {
        return true;
    }

    Ret ret = migrateProject(project, migrationOptions);
    return ret;
}

Ret ProjectMigrator::askAboutMigration(MigrationOptions& out, const engraving::EngravingProjectPtr project)
{
    UriQuery query(MIGRATION_DIALOG_URI);
    query.addParam("title", Val(project->title()));
    query.addParam("version", Val(QString::number(project->mscVersion())));
    RetVal<Val> rv = interactive()->open(query);
    if (!rv.ret) {
        return rv.ret;
    }

    QVariantMap vals = rv.val.toQVariant().toMap();
    out.isApplyMigration = vals.value("isApplyMigration").toBool();
    out.isAskAgain = vals.value("isAskAgain").toBool();
    out.isApplyLeland = vals.value("isApplyLeland").toBool();
    out.isApplyEdwin = vals.value("isApplyEdwin").toBool();

    return true;
}

Ret ProjectMigrator::migrateProject(engraving::EngravingProjectPtr project, const MigrationOptions& opt)
{
    Ms::MasterScore* score = project->masterScore();
    IF_ASSERT_FAILED(score) {
        return make_ret(Ret::Code::InternalError);
    }

    score->startCmd();

    bool ok = applyStyleDefaults(score);

    if (ok && opt.isApplyLeland) {
        ok = applyLelandStyle(score);
    }

    if (ok && opt.isApplyEdwin) {
        ok = applyEdwinStyle(score);
    }

    if (ok) {
        ok = resetAllElementsPositions(score);
    }

    if (ok && score->mscVersion() != Ms::MSCVERSION) {
        score->undo(new Ms::ChangeMetaText(score, "mscVersion", MSC_VERSION));
    }

    score->endCmd();

    return make_ret(Ret::Code::Ok);
}

bool ProjectMigrator::applyStyleDefaults(Ms::MasterScore* score)
{
}

bool ProjectMigrator::applyLelandStyle(Ms::MasterScore* score)
{
}

bool ProjectMigrator::applyEdwinStyle(Ms::MasterScore* score)
{
}

bool ProjectMigrator::resetAllElementsPositions(Ms::MasterScore* score)
{
}
