//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2021 MuseScore BVBA and others
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
#include "keynavigationsection.h"

#include "log.h"

using namespace mu::ui;

KeyNavigationSection::KeyNavigationSection(QObject* parent)
    : QObject(parent)
{
}

KeyNavigationSection::~KeyNavigationSection()
{
    keyNavigationController()->unreg(this);
}

void KeyNavigationSection::setName(QString name)
{
    if (m_name == name) {
        return;
    }

    m_name = name;
    emit nameChanged(m_name);
}

QString KeyNavigationSection::name() const
{
    return m_name;
}

void KeyNavigationSection::setOrder(int order)
{
    if (m_order == order) {
        return;
    }

    m_order = order;
    emit orderChanged(m_order);
}

int KeyNavigationSection::order() const
{
    return m_order;
}

void KeyNavigationSection::setEnabled(bool enabled)
{
    if (m_enabled == enabled) {
        return;
    }

    m_enabled = enabled;
    emit enabledChanged(m_enabled);
}

bool KeyNavigationSection::enabled() const
{
    return m_enabled;
}

void KeyNavigationSection::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    emit activeChanged(m_active);
}

bool KeyNavigationSection::active() const
{
    return m_active;
}

void KeyNavigationSection::classBegin()
{
}

void KeyNavigationSection::componentComplete()
{
    //! NOTE Reg after set properties.
    LOGD() << "Completed: " << m_name << ", order: " << m_order;
    keyNavigationController()->reg(this);
}
