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
#include "musicxmlconfiguration.h"

#include "settings.h"
#include "translation.h"

using namespace mu::framework;
using namespace mu::iex::musicxml;

static const std::string module_name("iex_musicxml");

static const Settings::Key MUSICXML_IMPORT_BREAKS_KEY(module_name, "import/musicXML/importBreaks");
static const Settings::Key MUSICXML_IMPORT_LAYOUT_KEY(module_name, "import/musicXML/importLayout");
static const Settings::Key MUSICXML_EXPORT_LAYOUT_KEY(module_name, "export/musicXML/exportLayout");
static const Settings::Key MUSICXML_EXPORT_MU3_COMPAT_KEY(module_name, "export/musicXML/exportMu3Compat");
static const Settings::Key MUSICXML_EXPORT_BREAKS_TYPE_KEY(module_name, "export/musicXML/exportBreaks");
static const Settings::Key MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY(module_name, "export/musicXML/exportInvisibleElements");
static const Settings::Key MIGRATION_APPLY_EDWIN_FOR_XML(module_name, "import/compatibility/apply_edwin_for_xml");
static const Settings::Key MIGRATION_NOT_ASK_AGAIN_KEY(module_name, "import/compatibility/do_not_ask_me_again");
static const Settings::Key MUSICXML_IMPORT_INFER_TEXT_TYPE(module_name, "import/musicXML/importInferTextType");

void MusicXmlConfiguration::init()
{
    settings()->setDefaultValue(MUSICXML_IMPORT_BREAKS_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_IMPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_LAYOUT_KEY, Val(true));
    settings()->setDefaultValue(MUSICXML_EXPORT_MU3_COMPAT_KEY, Val(false));
    settings()->setDescription(MUSICXML_EXPORT_MU3_COMPAT_KEY,
                               //: Means that less information will be included in exported MusicXML files,
                               //: to prevent errors when importing them into MuseScore 3
                               mu::trc("iex_musicxml", "Limit MusicXML export for compatibility with MuseScore 3"));
    settings()->setCanBeManuallyEdited(MUSICXML_EXPORT_MU3_COMPAT_KEY, true);
    settings()->setDefaultValue(MUSICXML_EXPORT_BREAKS_TYPE_KEY, Val(MusicxmlExportBreaksType::All));
    settings()->setDefaultValue(MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY, Val(false));
    settings()->setDefaultValue(MIGRATION_APPLY_EDWIN_FOR_XML, Val(false));
    settings()->setDefaultValue(MIGRATION_NOT_ASK_AGAIN_KEY, Val(false));
    settings()->setDefaultValue(MUSICXML_IMPORT_INFER_TEXT_TYPE, Val(false));
}

bool MusicXmlConfiguration::musicxmlImportBreaks() const
{
    return settings()->value(MUSICXML_IMPORT_BREAKS_KEY).toBool();
}

void MusicXmlConfiguration::setMusicxmlImportBreaks(bool value)
{
    settings()->setSharedValue(MUSICXML_IMPORT_BREAKS_KEY, Val(value));
}

bool MusicXmlConfiguration::musicxmlImportLayout() const
{
    return settings()->value(MUSICXML_IMPORT_LAYOUT_KEY).toBool();
}

void MusicXmlConfiguration::setMusicxmlImportLayout(bool value)
{
    settings()->setSharedValue(MUSICXML_IMPORT_LAYOUT_KEY, Val(value));
}

bool MusicXmlConfiguration::musicxmlExportLayout() const
{
    return settings()->value(MUSICXML_EXPORT_LAYOUT_KEY).toBool();
}

void MusicXmlConfiguration::setMusicxmlExportLayout(bool value)
{
    settings()->setSharedValue(MUSICXML_EXPORT_LAYOUT_KEY, Val(value));
}

bool MusicXmlConfiguration::musicxmlExportMu3Compat() const
{
    return settings()->value(MUSICXML_EXPORT_MU3_COMPAT_KEY).toBool();
}

void MusicXmlConfiguration::setMusicxmlExportMu3Compat(bool value)
{
    settings()->setSharedValue(MUSICXML_EXPORT_MU3_COMPAT_KEY, Val(value));
}

MusicXmlConfiguration::MusicxmlExportBreaksType MusicXmlConfiguration::musicxmlExportBreaksType() const
{
    return settings()->value(MUSICXML_EXPORT_BREAKS_TYPE_KEY).toEnum<MusicxmlExportBreaksType>();
}

void MusicXmlConfiguration::setMusicxmlExportBreaksType(MusicxmlExportBreaksType breaksType)
{
    settings()->setSharedValue(MUSICXML_EXPORT_BREAKS_TYPE_KEY, Val(breaksType));
}

bool MusicXmlConfiguration::musicxmlExportInvisibleElements() const
{
    return settings()->value(MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY).toBool();
}

void MusicXmlConfiguration::setMusicxmlExportInvisibleElements(bool value)
{
    settings()->setSharedValue(MUSICXML_EXPORT_INVISIBLE_ELEMENTS_KEY, Val(value));
}

bool MusicXmlConfiguration::needUseDefaultFont() const
{
    if (m_needUseDefaultFontOverride.has_value()) {
        return m_needUseDefaultFontOverride.value();
    }

    return settings()->value(MIGRATION_APPLY_EDWIN_FOR_XML).toBool();
}

void MusicXmlConfiguration::setNeedUseDefaultFont(bool value)
{
    settings()->setSharedValue(MIGRATION_APPLY_EDWIN_FOR_XML, Val(value));
}

void MusicXmlConfiguration::setNeedUseDefaultFontOverride(std::optional<bool> value)
{
    m_needUseDefaultFontOverride = value;
}

bool MusicXmlConfiguration::needAskAboutApplyingNewStyle() const
{
    return !settings()->value(MIGRATION_NOT_ASK_AGAIN_KEY).toBool();
}

void MusicXmlConfiguration::setNeedAskAboutApplyingNewStyle(bool value)
{
    settings()->setSharedValue(MIGRATION_NOT_ASK_AGAIN_KEY, Val(!value));
}

bool MusicXmlConfiguration::inferTextType() const
{
    if (m_inferTextTypeOverride.has_value()) {
        return m_inferTextTypeOverride.value();
    }

    return settings()->value(MUSICXML_IMPORT_INFER_TEXT_TYPE).toBool();
}

void MusicXmlConfiguration::setInferTextType(bool value)
{
    settings()->setSharedValue(MUSICXML_IMPORT_INFER_TEXT_TYPE, Val(value));
}

void MusicXmlConfiguration::setInferTextTypeOverride(std::optional<bool> value)
{
    m_inferTextTypeOverride = value;
}
