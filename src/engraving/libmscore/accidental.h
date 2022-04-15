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

#ifndef __ACCIDENTAL_H__
#define __ACCIDENTAL_H__

/**
 \file
 Definition of class Accidental
*/

#include <QString>
#include <vector>
#include <QVariant>

#include "config.h"
#include "engravingitem.h"

namespace mu::engraving {
class Factory;
}

namespace Ms {
class Note;
enum class AccidentalVal : signed char;

//---------------------------------------------------------
//   AccidentalBracket
//---------------------------------------------------------

enum class AccidentalBracket : char {
    NONE,
    PARENTHESIS,
    BRACKET,
    BRACE, //! Deprecated; removed from the UI and kept here only for compatibility purposes
};

//---------------------------------------------------------
//   SymElement
//---------------------------------------------------------

struct SymElement {
    SymId sym;
    qreal x;
    qreal y;
    SymElement(SymId _sym, qreal _x, qreal _y)
        : sym(_sym), x(_x), y(_y) {}
};

//---------------------------------------------------------
//   @@ Accidental
//   @P role        enum  (Accidental.AUTO, .USER) (read only)
//   @P isSmall     bool
//---------------------------------------------------------

class Accidental final : public EngravingItem
{
    std::vector<SymElement> el;
    AccidentalType _accidentalType { AccidentalType::NONE };
    bool m_isSmall                    { false };
    AccidentalBracket _bracket     { AccidentalBracket::NONE };
    AccidentalRole _role           { AccidentalRole::AUTO };

    friend class mu::engraving::Factory;

    Accidental(EngravingItem* parent);

public:

    Accidental* clone() const override { return new Accidental(*this); }

    // Score Tree functions
    EngravingObject* scanParent() const override;
    EngravingObject* scanChild(size_t idx) const override;
    size_t scanChildCount() const override;

    QString subtypeUserName() const;
    void setSubtype(const QString& s);
    void setAccidentalType(AccidentalType t) { _accidentalType = t; }

    AccidentalType accidentalType() const { return _accidentalType; }
    AccidentalRole role() const { return _role; }

    int subtype() const override { return (int)_accidentalType; }
    QString subtypeName() const override { return QString(subtype2name(_accidentalType)); }

    bool acceptDrop(EditData&) const override;
    EngravingItem* drop(EditData&) override;
    void layout() override;
    void layoutMultiGlyphAccidental();
    void layoutSingleGlyphAccidental();
    void draw(mu::draw::Painter*) const override;
    bool isEditable() const override { return true; }
    void startEdit(EditData&) override { setGenerated(false); }

    SymId symbol() const;
    Note* note() const { return (explicitParent() && explicitParent()->isNote()) ? toNote(explicitParent()) : 0; }

    AccidentalBracket bracket() const { return _bracket; }
    void setBracket(AccidentalBracket val) { _bracket = val; }

    void setRole(AccidentalRole r) { _role = r; }

    bool isSmall() const { return m_isSmall; }
    void setSmall(bool val) { m_isSmall = val; }

    void undoSetSmall(bool val);

    void read(XmlReader&) override;
    void write(XmlWriter& xml) const override;

    mu::engraving::PropertyValue getProperty(Pid propertyId) const override;
    bool setProperty(Pid propertyId, const mu::engraving::PropertyValue&) override;
    mu::engraving::PropertyValue propertyDefault(Pid propertyId) const override;
    Pid propertyId(const QStringRef& xmlName) const override;

    static AccidentalVal subtype2value(AccidentalType);               // return effective pitch offset
    static SymId subtype2symbol(AccidentalType);
    static const char* subtype2name(AccidentalType);
    static AccidentalType value2subtype(AccidentalVal);
    static AccidentalType name2subtype(const QString&);
    static bool isMicrotonal(AccidentalType t) { return t > AccidentalType::FLAT3; }

    QString accessibleInfo() const override;
};

extern AccidentalVal sym2accidentalVal(SymId id);
}     // namespace Ms

Q_DECLARE_METATYPE(Ms::AccidentalRole);

#endif
