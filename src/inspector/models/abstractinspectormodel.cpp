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
#include "abstractinspectormodel.h"

#include "libmscore/musescoreCore.h"

#include "types/texttypes.h"

#include "log.h"

using namespace mu::inspector;
using namespace mu::notation;
using namespace mu::engraving;

static const QMap<mu::engraving::ElementType, InspectorModelType> NOTATION_ELEMENT_MODEL_TYPES = {
    { mu::engraving::ElementType::NOTE, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::STEM, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTEDOT, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTEHEAD, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::NOTELINE, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::SHADOW_NOTE, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::HOOK, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::BEAM, InspectorModelType::TYPE_NOTE },
    { mu::engraving::ElementType::GLISSANDO, InspectorModelType::TYPE_GLISSANDO },
    { mu::engraving::ElementType::GLISSANDO_SEGMENT, InspectorModelType::TYPE_GLISSANDO },
    { mu::engraving::ElementType::VIBRATO, InspectorModelType::TYPE_VIBRATO },
    { mu::engraving::ElementType::VIBRATO_SEGMENT, InspectorModelType::TYPE_VIBRATO },
    { mu::engraving::ElementType::SLUR, InspectorModelType::TYPE_SLUR },
    { mu::engraving::ElementType::SLUR_SEGMENT, InspectorModelType::TYPE_SLUR },
    { mu::engraving::ElementType::TIE, InspectorModelType::TYPE_TIE },
    { mu::engraving::ElementType::TIE_SEGMENT, InspectorModelType::TYPE_TIE },
    { mu::engraving::ElementType::TEMPO_TEXT, InspectorModelType::TYPE_TEMPO },
    { mu::engraving::ElementType::FERMATA, InspectorModelType::TYPE_FERMATA },
    { mu::engraving::ElementType::LAYOUT_BREAK, InspectorModelType::TYPE_SECTIONBREAK },
    { mu::engraving::ElementType::BAR_LINE, InspectorModelType::TYPE_BARLINE },
    { mu::engraving::ElementType::MARKER, InspectorModelType::TYPE_MARKER },
    { mu::engraving::ElementType::JUMP, InspectorModelType::TYPE_JUMP },
    { mu::engraving::ElementType::KEYSIG, InspectorModelType::TYPE_KEYSIGNATURE },
    { mu::engraving::ElementType::ACCIDENTAL, InspectorModelType::TYPE_ACCIDENTAL },
    { mu::engraving::ElementType::FRET_DIAGRAM, InspectorModelType::TYPE_FRET_DIAGRAM },
    { mu::engraving::ElementType::PEDAL, InspectorModelType::TYPE_PEDAL },
    { mu::engraving::ElementType::PEDAL_SEGMENT, InspectorModelType::TYPE_PEDAL },
    { mu::engraving::ElementType::SPACER, InspectorModelType::TYPE_SPACER },
    { mu::engraving::ElementType::CLEF, InspectorModelType::TYPE_CLEF },
    { mu::engraving::ElementType::HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::ElementType::HAIRPIN_SEGMENT, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::ElementType::OTTAVA, InspectorModelType::TYPE_OTTAVA },
    { mu::engraving::ElementType::OTTAVA_SEGMENT, InspectorModelType::TYPE_OTTAVA },
    { mu::engraving::ElementType::VOLTA, InspectorModelType::TYPE_VOLTA },
    { mu::engraving::ElementType::VOLTA_SEGMENT, InspectorModelType::TYPE_VOLTA },
    { mu::engraving::ElementType::PALM_MUTE, InspectorModelType::TYPE_PALM_MUTE },
    { mu::engraving::ElementType::PALM_MUTE_SEGMENT, InspectorModelType::TYPE_PALM_MUTE },
    { mu::engraving::ElementType::LET_RING, InspectorModelType::TYPE_LET_RING },
    { mu::engraving::ElementType::LET_RING_SEGMENT, InspectorModelType::TYPE_LET_RING },
    { mu::engraving::ElementType::STAFFTYPE_CHANGE, InspectorModelType::TYPE_STAFF_TYPE_CHANGES },
    { mu::engraving::ElementType::TBOX, InspectorModelType::TYPE_TEXT_FRAME },// text frame
    { mu::engraving::ElementType::VBOX, InspectorModelType::TYPE_VERTICAL_FRAME },// vertical frame
    { mu::engraving::ElementType::HBOX, InspectorModelType::TYPE_HORIZONTAL_FRAME },// horizontal frame
    { mu::engraving::ElementType::ARTICULATION, InspectorModelType::TYPE_ARTICULATION },
    { mu::engraving::ElementType::IMAGE, InspectorModelType::TYPE_IMAGE },
    { mu::engraving::ElementType::HARMONY, InspectorModelType::TYPE_CHORD_SYMBOL },
    { mu::engraving::ElementType::AMBITUS, InspectorModelType::TYPE_AMBITUS },
    { mu::engraving::ElementType::BRACKET, InspectorModelType::TYPE_BRACKET },
    { mu::engraving::ElementType::TIMESIG, InspectorModelType::TYPE_TIME_SIGNATURE },
    { mu::engraving::ElementType::MMREST, InspectorModelType::TYPE_MMREST },
    { mu::engraving::ElementType::BEND, InspectorModelType::TYPE_BEND },
    { mu::engraving::ElementType::TREMOLOBAR, InspectorModelType::TYPE_TREMOLOBAR },
    { mu::engraving::ElementType::TREMOLO, InspectorModelType::TYPE_TREMOLO },
    { mu::engraving::ElementType::MEASURE_REPEAT, InspectorModelType::TYPE_MEASURE_REPEAT },
    { mu::engraving::ElementType::TUPLET, InspectorModelType::TYPE_TUPLET },
    { mu::engraving::ElementType::TEXTLINE, InspectorModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::TEXTLINE_SEGMENT, InspectorModelType::TYPE_TEXT_LINE },
    { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE, InspectorModelType::TYPE_GRADUAL_TEMPO_CHANGE },
    { mu::engraving::ElementType::GRADUAL_TEMPO_CHANGE_SEGMENT, InspectorModelType::TYPE_GRADUAL_TEMPO_CHANGE },
    { mu::engraving::ElementType::INSTRUMENT_NAME, InspectorModelType::TYPE_INSTRUMENT_NAME }
};

static QMap<mu::engraving::HairpinType, InspectorModelType> HAIRPIN_ELEMENT_MODEL_TYPES = {
    { mu::engraving::HairpinType::CRESC_HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::HairpinType::DECRESC_HAIRPIN, InspectorModelType::TYPE_HAIRPIN },
    { mu::engraving::HairpinType::CRESC_LINE, InspectorModelType::TYPE_CRESCENDO },
    { mu::engraving::HairpinType::DECRESC_LINE, InspectorModelType::TYPE_DIMINUENDO },
};

static QMap<mu::engraving::LayoutBreakType, InspectorModelType> LAYOUT_BREAK_ELEMENT_MODEL_TYPES = {
    { mu::engraving::LayoutBreakType::SECTION, InspectorModelType::TYPE_SECTIONBREAK }
};

AbstractInspectorModel::AbstractInspectorModel(QObject* parent, IElementRepositoryService* repository,
                                               mu::engraving::ElementType elementType)
    : QObject(parent), m_elementType(elementType), m_updatePropertiesAllowed(true)
{
    m_repository = repository;

    if (!m_repository) {
        return;
    }

    setupCurrentNotationChangedConnection();

    connect(m_repository->getQObject(), SIGNAL(elementsUpdated(const QList<mu::engraving::EngravingItem*>&)), this,
            SLOT(updateProperties()));
    connect(this, &AbstractInspectorModel::requestReloadPropertyItems, this, &AbstractInspectorModel::updateProperties);
}

void AbstractInspectorModel::setupCurrentNotationChangedConnection()
{
    auto listenNotationChanged = [this]() {
        INotationPtr notation = currentNotation();
        if (!notation) {
            return;
        }

        notation->notationChanged().onNotify(this, [this]() {
            if (m_updatePropertiesAllowed && !isEmpty()) {
                updatePropertiesOnNotationChanged();
            }

            m_updatePropertiesAllowed = true;
        });

        onStyleChanged();
        notation->style()->styleChanged().onNotify(this, [this]() {
            onStyleChanged();
        });
    };

    listenNotationChanged();

    currentNotationChanged().onNotify(this, [listenNotationChanged]() {
        listenNotationChanged();
    });
}

void AbstractInspectorModel::updatePropertiesOnNotationChanged()
{
}

void AbstractInspectorModel::onStyleChanged()
{
}

void AbstractInspectorModel::requestResetToDefaults()
{
    resetProperties();
}

QString AbstractInspectorModel::title() const
{
    return m_title;
}

int AbstractInspectorModel::icon() const
{
    return static_cast<int>(m_icon);
}

InspectorSectionType AbstractInspectorModel::sectionType() const
{
    return m_sectionType;
}

InspectorModelType AbstractInspectorModel::modelType() const
{
    return m_modelType;
}

InspectorModelType AbstractInspectorModel::modelTypeByElementKey(const ElementKey& elementKey)
{
    if (elementKey.type == mu::engraving::ElementType::HAIRPIN || elementKey.type == mu::engraving::ElementType::HAIRPIN_SEGMENT) {
        return HAIRPIN_ELEMENT_MODEL_TYPES.value(static_cast<mu::engraving::HairpinType>(elementKey.subtype),
                                                 InspectorModelType::TYPE_UNDEFINED);
    }

    if (elementKey.type == mu::engraving::ElementType::LAYOUT_BREAK) {
        return LAYOUT_BREAK_ELEMENT_MODEL_TYPES.value(static_cast<mu::engraving::LayoutBreakType>(elementKey.subtype),
                                                      InspectorModelType::TYPE_UNDEFINED);
    }

    if (elementKey.type == mu::engraving::ElementType::ARTICULATION) {
        if (mu::engraving::Articulation::isOrnament(elementKey.subtype)) {
            return InspectorModelType::TYPE_ORNAMENT;
        }
    }

    return NOTATION_ELEMENT_MODEL_TYPES.value(elementKey.type, InspectorModelType::TYPE_UNDEFINED);
}

InspectorModelTypeSet AbstractInspectorModel::modelTypesByElementKeys(const ElementKeySet& elementKeySet)
{
    InspectorModelTypeSet types;

    for (const ElementKey& key : elementKeySet) {
        types << modelTypeByElementKey(key);
    }

    return types;
}

InspectorSectionTypeSet AbstractInspectorModel::sectionTypesByElementKeys(const ElementKeySet& elementKeySet, bool isRange)
{
    InspectorSectionTypeSet types;

    for (const ElementKey& key : elementKeySet) {
        if (NOTATION_ELEMENT_MODEL_TYPES.keys().contains(key.type)
            && (modelTypeByElementKey(key) != InspectorModelType::TYPE_UNDEFINED)) {
            types << InspectorSectionType::SECTION_NOTATION;
        }

        if (TEXT_ELEMENT_TYPES.contains(key.type)) {
            types << InspectorSectionType::SECTION_TEXT;
        }

        if (key.type != mu::engraving::ElementType::INSTRUMENT_NAME) {
            types << InspectorSectionType::SECTION_GENERAL;
        }
    }

    if (isRange) {
        types << InspectorSectionType::SECTION_MEASURES;
    }

    return types;
}

bool AbstractInspectorModel::isEmpty() const
{
    return m_elementList.isEmpty();
}

void AbstractInspectorModel::setTitle(QString title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;
    emit titleChanged();
}

void AbstractInspectorModel::setIcon(mu::ui::IconCode::Code icon)
{
    m_icon = icon;
}

void AbstractInspectorModel::setSectionType(InspectorSectionType sectionType)
{
    m_sectionType = sectionType;
}

void AbstractInspectorModel::setModelType(InspectorModelType modelType)
{
    m_modelType = modelType;
}

void AbstractInspectorModel::onPropertyValueChanged(const mu::engraving::Pid pid, const QVariant& newValue)
{
    if (isEmpty()) {
        return;
    }

    beginCommand();

    for (mu::engraving::EngravingItem* element : m_elementList) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        mu::engraving::PropertyFlags ps = element->propertyFlags(pid);

        if (ps == mu::engraving::PropertyFlags::STYLED) {
            ps = mu::engraving::PropertyFlags::UNSTYLED;
        }

        PropertyValue propValue = valueToElementUnits(pid, newValue, element);
        element->undoChangeProperty(pid, propValue, ps);
    }

    updateNotation();
    endCommand();
}

void AbstractInspectorModel::updateProperties()
{
    requestElements();

    emit isEmptyChanged();

    if (!isEmpty()) {
        loadProperties();
    }
}

void AbstractInspectorModel::requestElements()
{
    if (m_elementType != mu::engraving::ElementType::INVALID) {
        m_elementList = m_repository->findElementsByType(m_elementType);
    }
}

mu::engraving::Sid AbstractInspectorModel::styleIdByPropertyId(const mu::engraving::Pid pid) const
{
    mu::engraving::Sid result = mu::engraving::Sid::NOSTYLE;

    for (const mu::engraving::EngravingItem* element : m_elementList) {
        result = element->getPropertyStyle(pid);

        if (result != mu::engraving::Sid::NOSTYLE) {
            break;
        }
    }

    return result;
}

void AbstractInspectorModel::updateStyleValue(const mu::engraving::Sid& sid, const QVariant& newValue)
{
    PropertyValue newVal = PropertyValue::fromQVariant(newValue, mu::engraving::MStyle::valueType(sid));
    if (style() && style()->styleValue(sid) != newVal) {
        beginCommand();
        style()->setStyleValue(sid, newVal);
        endCommand();
    }
}

QVariant AbstractInspectorModel::styleValue(const mu::engraving::Sid& sid) const
{
    return style() ? style()->styleValue(sid).toQVariant() : QVariant();
}

PropertyValue AbstractInspectorModel::valueToElementUnits(const mu::engraving::Pid& pid, const QVariant& value,
                                                          const mu::engraving::EngravingItem* element) const
{
    if (mu::engraving::Pid::VERSE == pid) {
        return value.toInt() - 1;
    }

    auto toPoint = [](const QVariant& v) {
        return PointF::fromQPointF(v.value<QPointF>());
    };

    P_TYPE type = mu::engraving::propertyType(pid);
    switch (type) {
    case P_TYPE::POINT: {
        if (element->sizeIsSpatiumDependent()) {
            return toPoint(value) * element->spatium();
        } else {
            return toPoint(value) * mu::engraving::DPMM;
        }
    }

    case P_TYPE::MILLIMETRE:
        return Spatium(value.toReal()).toMM(element->spatium());

    case P_TYPE::SPATIUM:
        return Spatium(value.toReal());

    case P_TYPE::TEMPO:
        return BeatsPerSecond::fromBPM(BeatsPerMinute(value.toReal()));

    case P_TYPE::INT_VEC: {
        QList<int> l = value.value<QList<int> >();
        return std::vector<int>(l.begin(), l.end());
    } break;

    case P_TYPE::COLOR:
        return Color::fromQColor(value.value<QColor>());

    default:
        return PropertyValue::fromQVariant(value, type);
    }
}

QVariant AbstractInspectorModel::valueFromElementUnits(const mu::engraving::Pid& pid, const PropertyValue& value,
                                                       const mu::engraving::EngravingItem* element) const
{
    if (mu::engraving::Pid::VERSE == pid) {
        return value.toInt() + 1;
    }

    switch (value.type()) {
    case P_TYPE::POINT: {
        if (element->sizeIsSpatiumDependent()) {
            return value.value<PointF>().toQPointF() / element->spatium();
        } else {
            return value.value<PointF>().toQPointF() / mu::engraving::DPMM;
        }
    }

    case P_TYPE::MILLIMETRE:
        return Spatium::fromMM(value.toReal(), element->spatium()).val();

    case P_TYPE::SPATIUM:
        return value.value<Spatium>().val();

    case P_TYPE::TEMPO:
        return value.value<BeatsPerSecond>().toBPM().val;

    case P_TYPE::DIRECTION_V:
        return static_cast<int>(value.value<mu::engraving::DirectionV>());

    case P_TYPE::INT_VEC: {
        QStringList strList;

        for (const int i : value.value<std::vector<int> >()) {
            strList << QString::number(i);
        }

        return strList.join(",");
    }
    case P_TYPE::COLOR:
        return value.value<mu::draw::Color>().toQColor();
    default:
        return value.toQVariant();
    }
}

void AbstractInspectorModel::setElementType(mu::engraving::ElementType type)
{
    m_elementType = type;
}

PropertyItem* AbstractInspectorModel::buildPropertyItem(const mu::engraving::Pid& propertyId,
                                                        std::function<void(const mu::engraving::Pid propertyId,
                                                                           const QVariant& newValue)> onPropertyChangedCallBack)
{
    PropertyItem* newPropertyItem = new PropertyItem(propertyId, this);

    auto callback = onPropertyChangedCallBack;

    if (!callback) {
        callback = [this](const mu::engraving::Pid propertyId, const QVariant& newValue) {
            onPropertyValueChanged(propertyId, newValue);
        };
    }

    connect(newPropertyItem, &PropertyItem::propertyModified, this, callback);
    connect(newPropertyItem, &PropertyItem::applyToStyleRequested, this, [this](const mu::engraving::Sid sid, const QVariant& newStyleValue) {
        updateStyleValue(sid,
                         newStyleValue);

        emit requestReloadPropertyItems();
    });

    return newPropertyItem;
}

void AbstractInspectorModel::loadPropertyItem(PropertyItem* propertyItem,
                                              std::function<QVariant(const QVariant&)> convertElementPropertyValueFunc)
{
    if (!propertyItem || m_elementList.isEmpty()) {
        return;
    }

    mu::engraving::Pid pid = propertyItem->propertyId();

    mu::engraving::Sid styleId = styleIdByPropertyId(pid);
    propertyItem->setStyleId(styleId);

    QVariant propertyValue;
    QVariant defaultPropertyValue;

    bool isUndefined = false;

    for (const mu::engraving::EngravingItem* element : m_elementList) {
        IF_ASSERT_FAILED(element) {
            continue;
        }

        QVariant elementCurrentValue = valueFromElementUnits(pid, element->getProperty(pid), element);
        QVariant elementDefaultValue = valueFromElementUnits(pid, element->propertyDefault(pid), element);

        bool isPropertySupportedByElement = elementCurrentValue.isValid();

        if (!isPropertySupportedByElement) {
            continue;
        }

        if (convertElementPropertyValueFunc) {
            elementCurrentValue = convertElementPropertyValueFunc(elementCurrentValue);
            elementDefaultValue = convertElementPropertyValueFunc(elementDefaultValue);
        }

        if (!(propertyValue.isValid() && defaultPropertyValue.isValid())) {
            propertyValue = elementCurrentValue;
            defaultPropertyValue = elementDefaultValue;
        }

        isUndefined = propertyValue != elementCurrentValue;

        if (isUndefined) {
            break;
        }
    }

    //@note Some elements may support the property, some don't. If element doesn't support property it'll return invalid value.
    //      So we use that knowledge here
    propertyItem->setIsEnabled(propertyValue.isValid());

    if (isUndefined) {
        propertyValue = QVariant();
    }

    propertyItem->fillValues(propertyValue, defaultPropertyValue);
}

bool AbstractInspectorModel::isNotationExisting() const
{
    return context()->currentProject() != nullptr;
}

INotationStylePtr AbstractInspectorModel::style() const
{
    return currentNotation() ? currentNotation()->style() : nullptr;
}

INotationUndoStackPtr AbstractInspectorModel::undoStack() const
{
    return currentNotation() ? currentNotation()->undoStack() : nullptr;
}

void AbstractInspectorModel::beginCommand()
{
    if (undoStack()) {
        undoStack()->prepareChanges();
    }

    //! NOTE prevents unnecessary updating of properties
    //! after changing their values in the inspector
    m_updatePropertiesAllowed = false;
}

void AbstractInspectorModel::endCommand()
{
    if (undoStack()) {
        undoStack()->commitChanges();
    }
}

INotationPtr AbstractInspectorModel::currentNotation() const
{
    return context()->currentNotation();
}

mu::async::Notification AbstractInspectorModel::currentNotationChanged() const
{
    return context()->currentNotationChanged();
}

INotationSelectionPtr AbstractInspectorModel::selection() const
{
    INotationPtr notation = currentNotation();
    return notation ? notation->interaction()->selection() : nullptr;
}

void AbstractInspectorModel::updateNotation()
{
    if (!currentNotation()) {
        return;
    }

    currentNotation()->notationChanged().notify();
}
