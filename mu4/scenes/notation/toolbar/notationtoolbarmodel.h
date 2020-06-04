//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================
#ifndef MU_NOTATIONSCENE_NOTATIONTOOLBARMODEL_H
#define MU_NOTATIONSCENE_NOTATIONTOOLBARMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "modularity/ioc.h"
#include "actions/iactionsdispatcher.h"

namespace mu {
namespace scene {
namespace notation {
class NotationToolBarModel : public QAbstractListModel
{
    Q_OBJECT
    INJECT(notation_scene, actions::IActionsDispatcher, dispatcher)

public:
    explicit NotationToolBarModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;

    Q_INVOKABLE void load();
    Q_INVOKABLE void click(const QString& action);

signals:

private:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        TitleRole
    };

    QList<actions::Action> m_actions;
};
}
}
}

#endif // MU_NOTATIONSCENE_NOTATIONTOOLBARMODEL_H
