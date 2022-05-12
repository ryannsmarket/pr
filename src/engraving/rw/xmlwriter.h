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
#ifndef MU_ENGRAVING_XMLWRITER_H
#define MU_ENGRAVING_XMLWRITER_H

#include <map>
#include <QXmlStreamReader>
#include <QTextStream>
#include <QFile>

#include <unordered_map>

#include "containers.h"

#include "infrastructure/draw/color.h"
#include "libmscore/connector.h"
#include "libmscore/stafftype.h"
#include "libmscore/interval.h"
#include "libmscore/engravingitem.h"

namespace mu::engraving {
class WriteContext;
}

namespace Ms {
class XmlWriter : public QTextStream
{
    static const int BS = 2048;

    std::list<QString> stack;

    std::vector<std::pair<const EngravingObject*, QString> > _elements;
    bool _recordElements = false;

    mutable mu::engraving::WriteContext* m_context = nullptr;
    mutable bool m_selfContext = false;

    void putLevel();

public:
    XmlWriter();
    XmlWriter(QIODevice* dev);
    ~XmlWriter();

    const std::vector<std::pair<const EngravingObject*, QString> >& elements() const { return _elements; }
    void setRecordElements(bool record) { _recordElements = record; }

    void writeHeader();

    void startObject(const QString&);
    void endObject();

    void startObject(const EngravingObject* se, const QString& attributes = QString());
    void startObject(const QString& name, const EngravingObject* se, const QString& attributes = QString());

    void tagE(const QString&);
    void tagE(const char* format, ...);
    void ntag(const char* name);
    void netag(const char* name);

    void tag(Pid id, const mu::engraving::PropertyValue& data, const mu::engraving::PropertyValue& def = mu::engraving::PropertyValue());
    void tagProperty(const char* name, mu::engraving::P_TYPE type, const mu::engraving::PropertyValue& data);

    void tag(const char* name, QVariant data, QVariant defaultData = QVariant());
    void tag(const QString&, QVariant data);
    void tag(const char* name, const char* s) { tag(name, QVariant(s)); }
    void tag(const char* name, const QString& s) { tag(name, QVariant(s)); }
    void tag(const char* name, const mu::PointF& v);
    void tag(const char* name, const Fraction& v, const Fraction& def = Fraction());
    void tag(const char* name, const CustDef& cd);

    void comment(const QString&);

    void writeXml(const QString&, QString s);
    void dump(int len, const unsigned char* p);

    mu::engraving::WriteContext* context() const;
    void setContext(mu::engraving::WriteContext* context);

    static QString xmlString(const QString&);
    static QString xmlString(ushort c);
};
}

#endif // MU_ENGRAVING_XMLWRITER_H
