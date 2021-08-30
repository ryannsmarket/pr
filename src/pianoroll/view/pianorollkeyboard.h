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

#ifndef MU_PIANOROLL_PIANOROLLKEYBOARD_H
#define MU_PIANOROLL_PIANOROLLKEYBOARD_H


#include <QQuickPaintedItem>
#include <QColor>

#include "async/asyncable.h"
#include "context/iglobalcontext.h"
#include "pianoroll/ipianorollcontroller.h"

namespace mu::pianoroll {

class PianorollKeyboard : public QQuickPaintedItem, public async::Asyncable
{
    Q_OBJECT

    INJECT(pianoroll, context::IGlobalContext, globalContext)
    INJECT(pianoroll, IPianorollController, controller)

    Q_PROPERTY(double noteHeight READ noteHeight WRITE setNoteHeight NOTIFY noteHeightChanged)
    Q_PROPERTY(double centerY READ centerY WRITE setCenterY NOTIFY centerYChanged)
    Q_PROPERTY(double displayObjectHeight READ displayObjectHeight WRITE setDisplayObjectHeight NOTIFY displayObjectHeightChanged)

public:
    PianorollKeyboard(QQuickItem* parent = nullptr);

    Q_INVOKABLE void load();

    double noteHeight() const { return m_noteHeight; }
    void setNoteHeight(double value);
    double centerY() const { return m_centerY; }
    void setCenterY(double value);
    double displayObjectHeight() const { return m_displayObjectHeight; }
    void setDisplayObjectHeight(double value);

    void paint(QPainter*) override;

    int pitchToPixelY(double pitch) const;
    double pixelYToPitch(int tick) const;

signals:
    void noteHeightChanged();
    void centerYChanged();
    void displayObjectHeightChanged();

private:
    void onNotationChanged();
    void onCurrentNotationChanged();
    void updateBoundingSize();

    double m_centerY = 0;  //fraction of note grid camera is focused on
    double m_displayObjectHeight = 0;  //Set to note grid in pixels
    double m_noteHeight;

    QColor m_colorBackground = Qt::lightGray;

};

}

#endif // MU_PIANOROLL_PIANOROLLKEYBOARD_H
