#ifndef MU_INSPECTOR_BRACESETTINGSMODEL_H
#define MU_INSPECTOR_BRACESETTINGSMODEL_H

#include "models/abstractinspectormodel.h"

namespace mu::inspector {
class BraceSettingsModel : public AbstractInspectorModel
{
    Q_OBJECT

    Q_PROPERTY(PropertyItem * bracketColumnPosition READ bracketColumnPosition CONSTANT)
    Q_PROPERTY(PropertyItem * bracketSpanStaves READ bracketSpanStaves CONSTANT)

public:

    explicit BraceSettingsModel(QObject* parent, IElementRepositoryService* repository);

    void createProperties() override;
    void requestElements() override;
    void loadProperties() override;
    void resetProperties() override;

    PropertyItem* bracketColumnPosition() const;
    PropertyItem* bracketSpanStaves() const;

private:
    PropertyItem* m_bracketColumnPosition = nullptr;
    PropertyItem* m_bracketSpanStaves = nullptr;
};
}

#endif // MU_INSPECTOR_BRACESETTINGSMODEL_H
