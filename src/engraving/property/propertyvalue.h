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
#include <memory>

#include <QVariant>

#include "types/types.h"
#include "libmscore/types.h"
#include "libmscore/symid.h"
#include "libmscore/pitchvalue.h"
#include "libmscore/fraction.h"
//#include "libmscore/groups.h" can't be included
#include "infrastructure/draw/geometry.h"
#include "infrastructure/draw/painterpath.h"
#include "infrastructure/draw/color.h"

#include "framework/global/logstream.h"

namespace Ms {
class Groups;
class TDuration;
}

namespace mu::engraving {
enum class P_TYPE {
    UNDEFINED = 0,
    // base
    BOOL,           // bool
    INT,            // int
    REAL,           // qreal
    STRING,         // QString

    // geometry
    POINT,
    SIZE,
    PATH,               // mu::PainterPath
    SCALE,
    SPATIUM,

    POINT_SP,           // point units, value saved in (score) spatium units
    POINT_SP_MM,        // point units, value saved as mm or spatium depending on EngravingItem->sizeIsSpatiumDependent()

    SP_REAL,            // real (point) value saved in (score) spatium units

    // draw
    COLOR,          // Color
    ALIGN,
    PLACEMENT,        // ABOVE or BELOW
    HPLACEMENT,       // LEFT, CENTER or RIGHT
    DIRECTION,        // enum class Direction
    DIRECTION_H,      // enum class MScore::DirectionH

    // time
    FRACTION,

    ORNAMENT_STYLE,   // enum class MScore::OrnamentStyle
    TDURATION,
    LAYOUT_BREAK,
    VALUE_TYPE,
    BEAM_MODE,

    TEXT_PLACE,
    TEMPO,
    GROUPS,
    SYMID,
    INT_LIST,
    GLISS_STYLE,
    BARLINE_TYPE,
    HEAD_TYPE,          // enum class Notehead::Type
    HEAD_GROUP,         // enum class Notehead::Group
    ZERO_INT,           // displayed with offset +1

    SUB_STYLE,

    CHANGE_METHOD,      // enum class VeloChangeMethod (for single note dynamics)
    CHANGE_SPEED,       // enum class Dynamic::Speed
    CLEF_TYPE,          // enum class ClefType
    DYNAMIC_TYPE,       // enum class DynamicType
    KEYMODE,            // enum class KeyMode
    ORIENTATION,        // enum class Orientation

    HEAD_SCHEME,        // enum class NoteHead::Scheme

    PITCH_VALUES,
    HOOK_TYPE,
    ACCIDENTAL_ROLE
};

class PropertyValue
{
public:
    PropertyValue();

    // base
    PropertyValue(bool v);
    PropertyValue(int v);
    PropertyValue(qreal v);
    PropertyValue(const char* v);
    PropertyValue(const QString& v);

    // geometry
    PropertyValue(const PointF& v);
    PropertyValue(const SizeF& v);
    PropertyValue(const PainterPath& v);
    PropertyValue(const Spatium& v);

    // draw
    PropertyValue(const draw::Color& v);
    PropertyValue(Ms::Align v);
    PropertyValue(Ms::HPlacement v);
    PropertyValue(Ms::Placement v)
        : m_type(P_TYPE::PLACEMENT), m_val(v) {}
    PropertyValue(Ms::Direction v);

    PropertyValue(Ms::SymId v);
    PropertyValue(Ms::BarLineType v);
    PropertyValue(Ms::HookType v);

    PropertyValue(Ms::DynamicType v)
        : m_type(P_TYPE::DYNAMIC_TYPE), m_val(v) {}

    PropertyValue(const Ms::PitchValues& v);
    PropertyValue(const Ms::Fraction& v);
    PropertyValue(const QList<int>& v); //endings
    PropertyValue(const Ms::AccidentalRole& v)
        : m_type(P_TYPE::ACCIDENTAL_ROLE), m_val(v) {}

    PropertyValue(const Ms::Groups& v);
    PropertyValue(const Ms::TDuration& v);

    bool isValid() const;

    P_TYPE type() const;

    template<typename T>
    T value() const
    {
        if (P_TYPE::UNDEFINED == m_type) {
            return T();
        }

        const T* pv = std::get_if<T>(&m_val);
        if (pv) {
            return *pv;
        }

        //! HACK Temporary hack for int to enum
        if constexpr (std::is_enum<T>::value) {
            if (P_TYPE::INT == m_type) {
                const int* pi = std::get_if<int>(&m_val);
                assert(pi);
                return pi ? static_cast<T>(*pi) : T();
            }
        }

        //! HACK Temporary hack for enum to int
        if constexpr (std::is_same<T, int>::value) {
            switch (m_type) {
            case P_TYPE::ALIGN:      return static_cast<int>(value<Ms::Align>());
            case P_TYPE::HPLACEMENT: return static_cast<int>(value<Ms::HPlacement>());
            case P_TYPE::PLACEMENT:  return static_cast<int>(value<Ms::Placement>());
            case P_TYPE::DIRECTION:  return static_cast<int>(value<Ms::Direction>());
            case P_TYPE::SYMID:      return static_cast<int>(value<Ms::SymId>());
            case P_TYPE::BARLINE_TYPE: return static_cast<int>(value<Ms::BarLineType>());
            case P_TYPE::HOOK_TYPE:  return static_cast<int>(value<Ms::HookType>());
            case P_TYPE::DYNAMIC_TYPE: return static_cast<int>(value<Ms::DynamicType>());
            case P_TYPE::ACCIDENTAL_ROLE: return static_cast<int>(value<Ms::AccidentalRole>());
            default:
                break;
            }
        }

        //! HACK Temporary hack for bool to int
        if constexpr (std::is_same<T, int>::value) {
            if (P_TYPE::BOOL == m_type) {
                const bool* pb = std::get_if<bool>(&m_val);
                assert(pb);
                return pb ? static_cast<T>(*pb) : T();
            }
        }

        //! HACK Temporary hack for int to bool
        if constexpr (std::is_same<T, bool>::value) {
            const int* pi = std::get_if<int>(&m_val);
            assert(pi);
            return pi ? static_cast<T>(*pi) : T();
        }

        //! HACK Temporary hack for real to Spatium
        if constexpr (std::is_same<T, Spatium>::value) {
            if (P_TYPE::SP_REAL == m_type || P_TYPE::REAL == m_type) {
                const qreal* pr = std::get_if<qreal>(&m_val);
                assert(pr);
                return pr ? Spatium(*pr) : Spatium();
            }
        }

        //! HACK Temporary hack for Spatium to real
        if constexpr (std::is_same<T, qreal>::value) {
            if (P_TYPE::SPATIUM == m_type) {
                const Spatium* ps = std::get_if<Spatium>(&m_val);
                assert(ps);
                return ps ? ps->val() : T();
            }
        }

        //! HACK Temporary hack for Fraction to String
        if constexpr (std::is_same<T, QString>::value) {
            if (P_TYPE::FRACTION == m_type) {
                const Ms::Fraction* pf = std::get_if<Ms::Fraction>(&m_val);
                assert(pf);
                return pf ? pf->toString() : T();
            }
        }

        assert(pv);
        return T();
    }

    bool toBool() const { return value<bool>(); }
    int toInt() const { return value<int>(); }
    qreal toReal() const { return value<qreal>(); }
    double toDouble() const { return value<qreal>(); }
    QString toString() const { return value<QString>(); }
    Ms::Align toAlign() const { return value<Ms::Align>(); }
    Ms::Direction toDirection() const { return value<Ms::Direction>(); }

    const Ms::Groups& toGroups() const;
    const Ms::TDuration& toTDuration() const;

    bool operator ==(const PropertyValue& v) const;
    inline bool operator !=(const PropertyValue& v) const { return !this->operator ==(v); }

    template<typename T>
    static PropertyValue fromValue(const T& v) { return PropertyValue(v); }

    //! NOTE compat
    QVariant toQVariant() const;
    static PropertyValue fromQVariant(const QVariant& v, P_TYPE type);

private:
    P_TYPE m_type = P_TYPE::UNDEFINED;
    std::variant<
        //base
        bool, int, qreal, QString,
        // geometry
        PointF, SizeF, PainterPath, Spatium,
        // draw
        draw::Color, Ms::Align, Ms::HPlacement, Ms::Placement, Ms::Direction,

        // time
        Ms::Fraction,

        Ms::SymId, Ms::BarLineType, Ms::HookType,
        Ms::DynamicType,
        Ms::PitchValues, QList<int>,
        Ms::AccidentalRole
        > m_val;

    //! HACK Temporary solution for some types
    std::any m_any;
};
}

inline mu::logger::Stream& operator<<(mu::logger::Stream& s, const mu::engraving::PropertyValue&)
{
    s << "property(not implemented log output)";
    return s;
}

#endif // MU_ENGRAVING_PROPERTYVALUE_H
