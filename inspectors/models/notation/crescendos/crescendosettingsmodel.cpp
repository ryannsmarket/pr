#include "crescendosettingsmodel.h"

#include "hairpin.h"
#include "types/crescendotypes.h"

CrescendoSettingsModel::CrescendoSettingsModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_CRESCENDO);
    setTitle(tr("Crescendo"));
    createProperties();
}

void CrescendoSettingsModel::createProperties()
{
    m_isLineVisible = buildPropertyItem(Ms::Pid::LINE_VISIBLE, [this] (const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        updateLinePropertiesAvailability();
    });

    m_startHookType = buildPropertyItem(Ms::Pid::BEGIN_HOOK_TYPE);
    m_endHookType = buildPropertyItem(Ms::Pid::END_HOOK_TYPE);
    m_thickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);

    m_hookHeight = buildPropertyItem(Ms::Pid::END_HOOK_HEIGHT, [this] (const int, const QVariant& newValue) {
        onPropertyValueChanged(Ms::Pid::END_HOOK_HEIGHT, newValue);
        onPropertyValueChanged(Ms::Pid::BEGIN_HOOK_HEIGHT, newValue);
    });

    m_lineStyle = buildPropertyItem(Ms::Pid::LINE_STYLE, [this] (const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), newValue);

        updateLinePropertiesAvailability();
    });

    m_dashLineLength = buildPropertyItem(Ms::Pid::DASH_LINE_LEN);
    m_dashGapLength = buildPropertyItem(Ms::Pid::DASH_GAP_LEN);
    m_placement = buildPropertyItem(Ms::Pid::PLACEMENT);

    m_continiousText = buildPropertyItem(Ms::Pid::CONTINUE_TEXT);
    m_continiousTextHorizontalOffset = buildPropertyItem(Ms::Pid::CONTINUE_TEXT_OFFSET, [this] (const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(newValue.toDouble(), m_continiousTextVerticalOffset->value().toDouble()));
    });

    m_continiousTextVerticalOffset = buildPropertyItem(Ms::Pid::CONTINUE_TEXT_OFFSET, [this] (const int pid, const QVariant& newValue) {
        onPropertyValueChanged(static_cast<Ms::Pid>(pid), QPointF(m_continiousTextHorizontalOffset->value().toDouble(), newValue.toDouble()));
    });
}

void CrescendoSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::HAIRPIN, [] (const Ms::Element* element) -> bool {
        const Ms::Hairpin* hairpin = Ms::toHairpin(element);

        if (!hairpin) {
            return false;
        }

        return hairpin->hairpinType() == Ms::HairpinType::CRESC_LINE || hairpin->hairpinType() == Ms::HairpinType::DECRESC_LINE;
    });
}

void CrescendoSettingsModel::loadProperties()
{
    loadPropertyItem(m_isLineVisible);
    loadPropertyItem(m_startHookType);
    loadPropertyItem(m_endHookType);

    auto formatDoubleFunc = [] (const QVariant& elementPropertyValue) -> QVariant {
        return QString::number(elementPropertyValue.toDouble(), 'f', 2).toDouble();
    };

    loadPropertyItem(m_thickness, formatDoubleFunc);
    loadPropertyItem(m_hookHeight, formatDoubleFunc);
    loadPropertyItem(m_lineStyle);
    loadPropertyItem(m_dashLineLength, formatDoubleFunc);
    loadPropertyItem(m_dashGapLength, formatDoubleFunc);
    loadPropertyItem(m_placement);

    loadPropertyItem(m_continiousText);
    loadPropertyItem(m_continiousTextHorizontalOffset, [] (const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toPointF().x();
    });
    loadPropertyItem(m_continiousTextVerticalOffset, [] (const QVariant& elementPropertyValue) -> QVariant {
        return elementPropertyValue.toPointF().x();
    });

    updateLinePropertiesAvailability();
}

void CrescendoSettingsModel::resetProperties()
{
    m_isLineVisible->resetToDefault();
    m_startHookType->resetToDefault();
    m_endHookType->resetToDefault();
    m_thickness->resetToDefault();
    m_hookHeight->resetToDefault();
    m_lineStyle->resetToDefault();
    m_dashLineLength->resetToDefault();
    m_dashGapLength->resetToDefault();
    m_placement->resetToDefault();
}

PropertyItem* CrescendoSettingsModel::isLineVisible() const
{
    return m_isLineVisible;
}

PropertyItem* CrescendoSettingsModel::startHookType() const
{
    return m_startHookType;
}

PropertyItem* CrescendoSettingsModel::endHookType() const
{
    return m_endHookType;
}

PropertyItem* CrescendoSettingsModel::thickness() const
{
    return m_thickness;
}

PropertyItem* CrescendoSettingsModel::hookHeight() const
{
    return m_hookHeight;
}

PropertyItem* CrescendoSettingsModel::lineStyle() const
{
    return m_lineStyle;
}

PropertyItem* CrescendoSettingsModel::dashLineLength() const
{
    return m_dashLineLength;
}

PropertyItem* CrescendoSettingsModel::dashGapLength() const
{
    return m_dashGapLength;
}

PropertyItem* CrescendoSettingsModel::placement() const
{
    return m_placement;
}

PropertyItem* CrescendoSettingsModel::continiousText() const
{
    return m_continiousText;
}

PropertyItem* CrescendoSettingsModel::continiousTextHorizontalOffset() const
{
    return m_continiousTextHorizontalOffset;
}

PropertyItem* CrescendoSettingsModel::continiousTextVerticalOffset() const
{
    return m_continiousTextVerticalOffset;
}

void CrescendoSettingsModel::updateLinePropertiesAvailability()
{
    bool isLineAvailable = m_isLineVisible->value().toBool();

    m_startHookType->setIsEnabled(isLineAvailable);
    m_endHookType->setIsEnabled(isLineAvailable);
    m_thickness->setIsEnabled(isLineAvailable);
    m_hookHeight->setIsEnabled(isLineAvailable);
    m_lineStyle->setIsEnabled(isLineAvailable);
    m_placement->setIsEnabled(isLineAvailable);

    CrescendoTypes::LineStyle currentStyle = static_cast<CrescendoTypes::LineStyle>(m_lineStyle->value().toInt());

    bool areDashPropertiesAvailable = currentStyle == CrescendoTypes::LineStyle::LINE_STYLE_CUSTOM;

    m_dashLineLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
    m_dashGapLength->setIsEnabled(isLineAvailable && areDashPropertiesAvailable);
}
