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

#include "notationlistmodel.h"

using namespace mu::scene::notation;
using namespace mu::domain::notation;
using namespace mu::framework;

NotationListModel::NotationListModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles.insert(RoleTitle, "title");
}

void NotationListModel::load()
{
    updateNotations();

    globalContext()->currentMasterNotationChanged().onNotify(this, [this]() {
         updateNotations();
    });
}

void NotationListModel::updateNotations()
{
    IMasterNotationPtr masterNotation = globalContext()->currentMasterNotation();

    beginResetModel();
    m_notations.clear();

    if (masterNotation) {
        std::vector<INotationPtr> parts = masterNotation->parts();
        m_notations.push_back(masterNotation);
        m_notations.insert(m_notations.end(), parts.begin(), parts.end());
    }

    endResetModel();
}

QVariant NotationListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    INotationPtr notation = m_notations[index.row()];
    Meta meta = notation->metaInfo();

    switch (role) {
    case RoleTitle: return QVariant::fromValue(meta.title);
    }

    return QVariant();
}

int NotationListModel::rowCount(const QModelIndex&) const
{
    return m_notations.size();
}

QHash<int, QByteArray> NotationListModel::roleNames() const
{
    return m_roles;
}

void NotationListModel::setCurrentNotation(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    globalContext()->setCurrentNotation(m_notations[index]);
}

void NotationListModel::closeNotation(int index)
{
    if (!isIndexValid(index)) {
        return;
    }

    if (globalContext()->currentNotation() == m_notations[index]) {
        globalContext()->setCurrentNotation(nullptr);
    }

    beginRemoveRows(QModelIndex(), index, index);

    m_notations.erase(m_notations.begin() + index);

    endRemoveRows();
}

bool NotationListModel::isIndexValid(int index) const
{
    return index >= 0 && index < static_cast<int>(m_notations.size());
}
