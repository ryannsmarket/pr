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
#ifndef MU_UI_KEYNAVIGATIONSUBSECTION_H
#define MU_UI_KEYNAVIGATIONSUBSECTION_H

#include <QObject>
#include <QList>

#include "abstractkeynavigation.h"
#include "../ikeynavigation.h"
#include "keynavigationsection.h"

namespace mu::ui {
class KeyNavigationControl;
class KeyNavigationSubSection : public AbstractKeyNavigation, public IKeyNavigationSubSection
{
    Q_OBJECT
    Q_PROPERTY(KeyNavigationSection * section READ section WRITE setSection)

public:
    explicit KeyNavigationSubSection(QObject* parent = nullptr);
    ~KeyNavigationSubSection();

    QString name() const override;
    int order() const override;
    bool enabled() const override;
    bool active() const override;
    void setActive(bool arg) override;
    async::Channel<bool> activeChanged() const override;
    const QList<IKeyNavigationControl*>& controls() const override;

    KeyNavigationSection* section() const;

    void componentComplete() override;

    void addControl(KeyNavigationControl* control);
    void removeControl(KeyNavigationControl* control);

public slots:
    void setSection(KeyNavigationSection* section);

private slots:
    void onSectionDestroyed();

private:
    KeyNavigationSection* m_section = nullptr;
    QList<IKeyNavigationControl*> m_controls;
};
}

#endif // MU_UI_KEYNAVIGATIONSUBSECTION_H
