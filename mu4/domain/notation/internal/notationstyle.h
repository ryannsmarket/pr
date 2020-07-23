#ifndef NOTATIONSTYLE_H
#define NOTATIONSTYLE_H

#include "inotationstyle.h"

#include "igetscore.h"

namespace mu::domain::notation {
class NotationStyle : public INotationStyle
{
public:
    NotationStyle(IGetScore* getScore);

    void updateStyleValue(const StyleId& styleId, const QVariant& newValue) const override;
    QVariant styleValue(const StyleId& styleId) override;

private:
    IGetScore* m_getScore = nullptr;
};
}

#endif // NOTATIONSTYLE_H
