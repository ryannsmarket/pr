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
#include "playbackcursor.h"

using namespace mu::notation;

void PlaybackCursor::paint(mu::draw::Painter* painter)
{
    if (!m_visible) {
        return;
    }

    painter->fillRect(m_rect, color());
}

const mu::draw::RectF& PlaybackCursor::rect() const
{
    return m_rect;
}

void PlaybackCursor::setRect(const draw::RectF& rect)
{
    m_rect = rect;
}

void PlaybackCursor::setVisible(bool arg)
{
    m_visible = arg;
}

QColor PlaybackCursor::color()
{
    QColor color = configuration()->playbackCursorColor();
    color.setAlpha(configuration()->cursorOpacity());
    return color;
}
