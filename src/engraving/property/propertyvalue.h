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
#ifndef MU_ENGRAVING_PROPERTYVALUE_H
#define MU_ENGRAVING_PROPERTYVALUE_H

#include <variant>
#include <any>
#include <string>

#include <QVariant>

#include "libmscore/property.h"
#include "libmscore/spatium.h"
#include "libmscore/types.h"
#include "libmscore/symid.h"
#include "libmscore/pitchvalue.h"
#include "libmscore/fraction.h"
//#include "libmscore/groups.h" can't be included
#include "infrastructure/draw/geometry.h"
#include "infrastructure/draw/painterpath.h"
#include "infrastructure/draw/color.h"

namespace Ms {
class Groups;
class TDuration;
}

namespace mu::engraving {
class PropertyValue
{
public:
    PropertyValue();
    PropertyValue(bool v);
    PropertyValue(int v);
    PropertyValue(qreal v);
    PropertyValue(const char* v);
    PropertyValue(const QString& v);
    PropertyValue(const Ms::Spatium& v);
    PropertyValue(const PointF& v);
    PropertyValue(const SizeF& v);
    PropertyValue(const PainterPath& v);
    PropertyValue(const draw::Color& v);
    PropertyValue(Ms::Align v);
    PropertyValue(Ms::Direction v);
    PropertyValue(Ms::SymId v);
    PropertyValue(Ms::BarLineType v);
    PropertyValue(Ms::HookType v);
    PropertyValue(Ms::HPlacement v);
    PropertyValue(Ms::DynamicType v)
        :   m_type(Ms::P_TYPE::DYNAMIC_TYPE), m_val(v) {}
    PropertyValue(const Ms::PitchValues& v);
    PropertyValue(const Ms::Fraction& v);
    PropertyValue(const QList<int>& v); //endings

    PropertyValue(const Ms::Groups& v);
    PropertyValue(const Ms::TDuration& v);

    bool isValid() const;

    Ms::P_TYPE type() const;

    template<typename T>
    T value() const
    {
        if (Ms::P_TYPE::UNDEFINED == m_type) {
            return T();
        }

        const T* pv = std::get_if<T>(&m_val);

        assert(pv);
        if (!pv) {
            return T();
        }

        return *pv;
    }

    bool toBool() const { return value<bool>(); }
    int toInt() const { return value<int>(); }
    qreal toReal() const { return value<qreal>(); }
    QString toString() const { return value<QString>(); }
    Ms::Spatium toSpatium() const { return value<Ms::Spatium>(); }
    Ms::Align toAlign() const { return value<Ms::Align>(); }
    Ms::Direction toDirection() const { return value<Ms::Direction>(); }

    inline bool operator ==(const PropertyValue& v) const { return m_type == v.m_type && m_val == v.m_val; }
    inline bool operator !=(const PropertyValue& v) const { return !this->operator ==(v); }

    template<typename T>
    static PropertyValue fromValue(const T& v) { return PropertyValue(v); }

    //! NOTE compat
    QVariant toQVariant() const;
    static PropertyValue fromQVariant(const QVariant& v);
    operator QVariant() const {
        return toQVariant();
    }

private:

    struct GroupsHolder
    {
        GroupsHolder(const Ms::Groups& v);
        const Ms::Groups& val() const;
    private:
        std::any m_val;
    };

    Ms::P_TYPE m_type = Ms::P_TYPE::UNDEFINED;
    std::variant<bool, int, qreal, QString,
                 Ms::Spatium,
                 PointF, SizeF, PainterPath, draw::Color,
                 Ms::Align, Ms::Direction, Ms::SymId, Ms::BarLineType, Ms::HookType, Ms::HPlacement,
                 Ms::DynamicType,
                 Ms::PitchValues, Ms::Fraction, QList<int>
                 > m_val;
};

//! NOTE compat
inline bool operator ==(const PropertyValue& v1, const QVariant& v2) { return v1 == PropertyValue::fromQVariant(v2); }
inline bool operator !=(const PropertyValue& v1, const QVariant& v2) { return !operator ==(v1, v2); }
inline bool operator ==(const QVariant& v1, const PropertyValue& v2) { return PropertyValue::fromQVariant(v1) == v2; }
inline bool operator !=(const QVariant& v1, const PropertyValue& v2) { return !operator ==(v1, v2); }
}

#endif // MU_ENGRAVING_PROPERTYVALUE_H
