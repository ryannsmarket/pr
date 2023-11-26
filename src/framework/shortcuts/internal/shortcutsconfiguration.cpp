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
#include "shortcutsconfiguration.h"

#include "settings.h"
#include "io/path.h"

#include "log.h"

using namespace mu::shortcuts;
using namespace mu::framework;

static const std::string MIDIMAPPINGS_FILE_NAME("/midi_mappings.xml");

static const Settings::Key ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE("shortcuts", "io/midi/advanceOnRelease");

void ShortcutsConfiguration::init()
{
    settings()->setDefaultValue(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE, Val(true));
}

QString ShortcutsConfiguration::currentKeyboardLayout() const
{
    NOT_IMPLEMENTED;
    return "US-QWERTY";
}

void ShortcutsConfiguration::setCurrentKeyboardLayout(const QString& layout)
{
    UNUSED(layout);
    NOT_IMPLEMENTED;
    return;
}

QString ShortcutsConfiguration::resolveShortcutsFileName() const
{
#if defined(Q_OS_MACOS)
    return QString("/shortcuts-Mac.xml");
#else
    return QString("/shortcuts.xml");
#endif
}

QString ShortcutsConfiguration::shortcutsFileName() const
{
    if (m_shortcutFileName.isEmpty())
    {
        m_shortcutFileName = resolveShortcutsFileName();
    }
    return m_shortcutFileName;
}

mu::io::path_t ShortcutsConfiguration::shortcutsUserAppDataPath() const
{
    return globalConfiguration()->userAppDataPath() + "/shortcuts.xml";
}

mu::io::path_t ShortcutsConfiguration::shortcutsAppDataPath() const
{
    return ":/data" + shortcutsFileName();
}

mu::io::path_t ShortcutsConfiguration::midiMappingUserAppDataPath() const
{
    return globalConfiguration()->userAppDataPath() + MIDIMAPPINGS_FILE_NAME;
}

bool ShortcutsConfiguration::advanceToNextNoteOnKeyRelease() const
{
    return settings()->value(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE).toBool();
}

void ShortcutsConfiguration::setAdvanceToNextNoteOnKeyRelease(bool value)
{
    settings()->setSharedValue(ADVANCE_TO_NEXT_NOTE_ON_KEY_RELEASE, Val(value));
}
