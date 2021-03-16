//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2020 MuseScore BVBA and others
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

#ifndef UICONFIGURATION_H
#define UICONFIGURATION_H

#include "iuiconfiguration.h"
#include "imainwindow.h"
#include "iplatformtheme.h"
#include "iworkspacesettings.h"
#include "val.h"

#include "modularity/ioc.h"

namespace mu::ui {
class UiConfiguration : public IUiConfiguration
{
    INJECT(ui, IMainWindow, mainWindow)
    INJECT(ui, IPlatformTheme, platformTheme)
    INJECT(ui, framework::IWorkspaceSettings, workspaceSettings)

public:
    void init();

    QStringList possibleFontFamilies() const override;
    ThemeList themes() const override;

    ThemeInfo currentTheme() const override;
    void setCurrentThemeType(ThemeType type) override;
    async::Notification currentThemeChanged() const override;

    std::string fontFamily() const override;
    void setFontFamily(const std::string& family) override;
    int fontSize(FontSizeType type) const override;
    void setBodyFontSize(int size) override;
    async::Notification fontChanged() const override;

    std::string iconsFontFamily() const override;
    int iconsFontSize(IconSizeType type) const override;
    async::Notification iconsFontChanged() const override;

    std::string musicalFontFamily() const override;
    int musicalFontSize() const override;
    async::Notification musicalFontChanged() const override;

    float guiScaling() const override;
    float physicalDotsPerInch() const override;

    void setPhysicalDotsPerInch(std::optional<float> dpi) override;

    QByteArray pageState(const std::string& pageName) const override;
    void setPageState(const std::string& pageName, const QByteArray& state) override;
    async::Notification pageStateChanged() const override;

private:
    ThemeType currentThemeType() const;
    ThemeInfo makeTheme(ThemeType type) const;

    QByteArray stringToByteArray(const std::string& string) const;
    std::string byteArrayToString(const QByteArray& byteArray) const;

    async::Notification m_currentThemeChanged;
    async::Notification m_fontChanged;
    async::Notification m_musicalFontChanged;
    async::Notification m_iconsFontChanged;
    async::Notification m_pageStateChanged;

    std::optional<float> m_customDPI;
};
}

#endif // UICONFIGURATION_H
