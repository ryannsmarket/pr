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
#ifndef MUSE_EXTENSIONS_EXTENSIONSLISTMODEL_H
#define MUSE_EXTENSIONS_EXTENSIONSLISTMODEL_H

#include <QAbstractListModel>
#include <QList>

#include "async/asyncable.h"

#include "modularity/ioc.h"
#include "iinteractive.h"
#include "extensions/iextensionsconfiguration.h"
#include "extensions/iextensionsprovider.h"

namespace muse::extensions {
class ExtensionsListModel : public QAbstractListModel, public Injectable, public async::Asyncable
{
    Q_OBJECT

    Inject<IInteractive> interactive = { this };
    Inject<IExtensionsProvider> provider = { this };
    Inject<IExtensionsConfiguration> configuration = { this };

public:
    explicit ExtensionsListModel(QObject* parent = nullptr);

    QVariant data(const QModelIndex& index, int role) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void load();

    Q_INVOKABLE int currentExecPointIndex(const QString& uri) const;
    Q_INVOKABLE QVariantList execPointsModel(const QString& uri) const;
    Q_INVOKABLE void selectExecPoint(const QString& uri, int index);

    Q_INVOKABLE void editShortcut(QString codeKey);
    Q_INVOKABLE void reloadPlugins();

    Q_INVOKABLE QVariantList categories() const;

signals:
    void finished();

private:
    enum Roles {
        rCode = Qt::UserRole + 1,
        rName,
        rDescription,
        rThumbnailUrl,
        rEnabled,
        rCategory,
        rVersion,
        rShortcuts
    };

    struct ExecPoints {
        QString uri;
        std::vector<ExecPoint> points;
    };

    void updatePlugin(const Manifest& plugin);
    int itemIndexByCodeKey(const QString& uri) const;

    const std::vector<ExecPoint>& execPoints(const QString& uri) const;

    QHash<int, QByteArray> m_roles;
    ManifestList m_plugins;
    mutable ExecPoints m_execPointsCache;
};
}

#endif // MUSE_EXTENSIONS_EXTENSIONSLISTMODEL_H
