#include "ambitussettingsmodel.h"

AmbitusSettingsModel::AmbitusSettingsModel(QObject* parent, IElementRepositoryService* repository) :
    AbstractInspectorModel(parent, repository)
{
    setModelType(TYPE_AMBITUS);
    setTitle(tr("Ambitus"));
    createProperties();

    setNoteheadGroupsModel(new NoteheadTypesModel(this));
}

void AmbitusSettingsModel::createProperties()
{
    m_noteheadGroup = buildPropertyItem(Ms::Pid::HEAD_GROUP);
    m_noteheadType = buildPropertyItem(Ms::Pid::HEAD_TYPE);

    m_topTpc = buildPropertyItem(Ms::Pid::TPC1);
    m_bottomTpc = buildPropertyItem(Ms::Pid::FBPARENTHESIS1);
    m_topOctave = buildPropertyItem(Ms::Pid::FBPARENTHESIS3);
    m_bottomOctave = buildPropertyItem(Ms::Pid::FBPARENTHESIS4);

    m_direction = buildPropertyItem(Ms::Pid::MIRROR_HEAD);
    m_lineThickness = buildPropertyItem(Ms::Pid::LINE_WIDTH);
}

void AmbitusSettingsModel::requestElements()
{
    m_elementList = m_repository->findElementsByType(Ms::ElementType::AMBITUS);
}

void AmbitusSettingsModel::loadProperties()
{
    loadPropertyItem(m_noteheadGroup);
    loadPropertyItem(m_noteheadType);

    loadPropertyItem(m_topTpc);
    loadPropertyItem(m_bottomTpc);
    loadPropertyItem(m_topOctave);
    loadPropertyItem(m_bottomOctave);

    loadPropertyItem(m_direction);
    loadPropertyItem(m_lineThickness, [] (const QVariant& elementPropertyValue) -> QVariant {
        return QString::number(elementPropertyValue.toDouble(), 'f', 2).toDouble();
    });
}

void AmbitusSettingsModel::resetProperties()
{
    m_noteheadGroup->resetToDefault();
    m_noteheadType->resetToDefault();

    m_topTpc->resetToDefault();
    m_bottomTpc->resetToDefault();
    m_topOctave->resetToDefault();
    m_bottomOctave->resetToDefault();

    m_direction->resetToDefault();
    m_lineThickness->resetToDefault();
}

void AmbitusSettingsModel::matchRangesToStaff()
{
    m_topTpc->resetToDefault();
    m_bottomTpc->resetToDefault();
    m_topOctave->resetToDefault();
    m_bottomOctave->resetToDefault();
}

NoteheadTypesModel* AmbitusSettingsModel::noteheadGroupsModel() const
{
    return m_noteheadGroupsModel;
}

PropertyItem* AmbitusSettingsModel::noteheadGroup() const
{
    return m_noteheadGroup;
}

PropertyItem* AmbitusSettingsModel::noteheadType() const
{
    return m_noteheadType;
}

PropertyItem* AmbitusSettingsModel::topTpc() const
{
    return m_topTpc;
}

PropertyItem* AmbitusSettingsModel::bottomTpc() const
{
    return m_bottomTpc;
}

PropertyItem* AmbitusSettingsModel::topOctave() const
{
    return m_topOctave;
}

PropertyItem* AmbitusSettingsModel::bottomOctave() const
{
    return m_bottomOctave;
}

PropertyItem* AmbitusSettingsModel::direction() const
{
    return m_direction;
}

PropertyItem* AmbitusSettingsModel::lineThickness() const
{
    return m_lineThickness;
}

void AmbitusSettingsModel::setNoteheadGroupsModel(NoteheadTypesModel* noteheadGroupsModel)
{
    if (m_noteheadGroupsModel == noteheadGroupsModel)
        return;

    m_noteheadGroupsModel = noteheadGroupsModel;

    connect(m_noteheadGroupsModel, &NoteheadTypesModel::noteHeadGroupSelected, [this] (const int noteHeadGroup) {
        m_noteheadGroup->setValue(noteHeadGroup);
    });

    connect(m_noteheadGroup, &PropertyItem::valueChanged, [this] (const QVariant& noteHeadGroup) {
        if (m_noteheadGroup->isUndefined()) {
            m_noteheadGroupsModel->init(Ms::NoteHead::Group::HEAD_INVALID);
        } else {
            m_noteheadGroupsModel->init(static_cast<Ms::NoteHead::Group>(noteHeadGroup.toInt()));
        }
    });

    emit noteheadGroupsModelChanged(m_noteheadGroupsModel);
}
