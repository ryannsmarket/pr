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
#ifndef MU_PALETTE_SPECIALCHARACTERSDIALOG_H
#define MU_PALETTE_SPECIALCHARACTERSDIALOG_H

#include "ui_specialcharactersdialog.h"

#include "engraving/infrastructure/draw/font.h"
#include "uicomponents/view/topleveldialog.h"

#include "modularity/ioc.h"
#include "context/iglobalcontext.h"

class QListWidget;

namespace mu::palette {
class PaletteWidget;
}

namespace mu::engraving {
class TextBase;
class SpecialCharactersDialog : public mu::uicomponents::TopLevelDialog, public Ui::SpecialCharactersDialog
{
    Q_OBJECT

    INJECT(Ms, mu::context::IGlobalContext, globalContext)

public:
    SpecialCharactersDialog(QWidget* parent = nullptr);
    SpecialCharactersDialog(const SpecialCharactersDialog& other);

    static int static_metaTypeId();

private slots:
    void populateSmufl();
    void populateUnicode();

private:
    void hideEvent(QHideEvent*) override;

    void setFont(const mu::draw::Font& font);
    void populateCommon();

    mu::draw::Font m_font;
    mu::palette::PaletteWidget* m_pCommon = nullptr;
    mu::palette::PaletteWidget* m_pSmufl = nullptr;
    mu::palette::PaletteWidget* m_pUnicode = nullptr;
    QListWidget* m_lws = nullptr;
    QListWidget* m_lwu = nullptr;
};
}

Q_DECLARE_METATYPE(mu::engraving::SpecialCharactersDialog)

#endif // MU_PALETTE_SPECIALCHARACTERSDIALOG_H
