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
#ifndef MU_ENGRAVING_DEFAULTSTYLE_H
#define MU_ENGRAVING_DEFAULTSTYLE_H

#include "style.h"

namespace mu::engraving {
class DefaultStyle
{
public:

    static DefaultStyle* instance();

    void init(const QString& defaultStyleFilePath, const QString& partStyleFilePath);

    static const mu::engraving::MStyle& baseStyle();

    static bool isHasDefaultStyle();
    static const mu::engraving::MStyle& defaultStyle();

    static const mu::engraving::MStyle* defaultStyleForParts();

    static const mu::engraving::MStyle& resolveStyleDefaults(const int defaultsVersion);

private:
    DefaultStyle() = default;

    static bool doLoadStyle(mu::engraving::MStyle* style, const QString& filePath);

    mu::engraving::MStyle m_baseStyle; // builtin initial style
    mu::engraving::MStyle* m_defaultStyle; // builtin modified by preferences
    mu::engraving::MStyle* m_defaultStyleForParts = nullptr;
};
}

#endif // MU_ENGRAVING_DEFAULTSTYLE_H
