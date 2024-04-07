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
#include "playbackconfiguration.h"

#include "settings.h"
#include "types/string.h"

#include "playbacktypes.h"

#include "log.h"

using namespace mu;
using namespace mu::playback;
using namespace muse::audio;

static const std::string moduleName("playback");

static const Settings::Key PLAYBACK_CURSOR_TYPE_KEY(moduleName, "application/playback/cursorType");
static const Settings::Key PLAY_NOTES_WHEN_EDITING(moduleName, "score/note/playOnClick");
static const Settings::Key PLAY_CHORD_WHEN_EDITING(moduleName, "score/chord/playOnAddNote");
static const Settings::Key PLAY_HARMONY_WHEN_EDITING(moduleName, "score/harmony/play/onedit");

static const Settings::Key SOUND_PRESETS_MULTI_SELECTION_KEY(moduleName, "application/playback/soundPresetsMultiSelectionEnabled");

static const Settings::Key MIXER_LABELS_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/labelsSectionVisible");
static const Settings::Key MIXER_SOUND_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/soundSectionVisible");
static const Settings::Key MIXER_AUDIO_FX_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/audioFxSectionVisible");
static const Settings::Key MIXER_BALANCE_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/balanceSectionVisible");
static const Settings::Key MIXER_VOLUME_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/volumeSectionVisible");
static const Settings::Key MIXER_FADER_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/faderSectionVisible");
static const Settings::Key MIXER_MUTE_AND_SOLO_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/muteAndSoloSectionVisible");
static const Settings::Key MIXER_TITLE_SECTION_VISIBLE_KEY(moduleName, "playback/mixer/titleSectionVisible");

static const Settings::Key MIXER_NEED_TO_SHOW_CHANGE_SOUND_WARNING(moduleName, "playback/mixer/needToShowChangeSoundWarning");

static const Settings::Key MUTE_HIDDEN_INSTRUMENTS(moduleName, "playback/mixer/muteHiddenInstruments");

static const Settings::Key DEFAULT_SOUND_PROFILE_FOR_NEW_PROJECTS(moduleName, "playback/profiles/defaultProfileName");
static const SoundProfileName BASIC_PROFILE_NAME(u"MuseScore Basic");
static const SoundProfileName MUSE_PROFILE_NAME(u"Muse Sounds");

static Settings::Key mixerSectionVisibleKey(MixerSectionType sectionType)
{
    switch (sectionType) {
    case MixerSectionType::Labels: return MIXER_LABELS_SECTION_VISIBLE_KEY;
    case MixerSectionType::Sound: return MIXER_SOUND_SECTION_VISIBLE_KEY;
    case MixerSectionType::AudioFX: return MIXER_AUDIO_FX_SECTION_VISIBLE_KEY;
    case MixerSectionType::Balance: return MIXER_BALANCE_SECTION_VISIBLE_KEY;
    case MixerSectionType::Volume: return MIXER_VOLUME_SECTION_VISIBLE_KEY;
    case MixerSectionType::Fader: return MIXER_FADER_SECTION_VISIBLE_KEY;
    case MixerSectionType::MuteAndSolo: return MIXER_MUTE_AND_SOLO_SECTION_VISIBLE_KEY;
    case MixerSectionType::Title: return MIXER_TITLE_SECTION_VISIBLE_KEY;
    case MixerSectionType::Unknown: break;
    }

    return Settings::Key();
}

static Settings::Key auxSendVisibleKey(aux_channel_idx_t index)
{
    return Settings::Key(moduleName, "playback/mixer/auxSend" + std::to_string(index) + "Visible");
}

static Settings::Key auxChannelVisibleKey(aux_channel_idx_t index)
{
    return Settings::Key(moduleName, "playback/mixer/auxChannel" + std::to_string(index) + "Visible");
}

void PlaybackConfiguration::init()
{
    settings()->setDefaultValue(PLAY_NOTES_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAY_CHORD_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAY_HARMONY_WHEN_EDITING, Val(true));
    settings()->setDefaultValue(PLAYBACK_CURSOR_TYPE_KEY, Val(PlaybackCursorType::STEPPED));
    settings()->setDefaultValue(SOUND_PRESETS_MULTI_SELECTION_KEY, Val(false));
    settings()->setDefaultValue(MIXER_NEED_TO_SHOW_CHANGE_SOUND_WARNING, Val(true));

    for (MixerSectionType sectionType : allMixerSectionTypes()) {
        bool sectionEnabledByDefault = sectionType != MixerSectionType::Volume;
        settings()->setDefaultValue(mixerSectionVisibleKey(sectionType), Val(sectionEnabledByDefault));
        settings()->valueChanged(mixerSectionVisibleKey(sectionType)).onReceive(this, [this, sectionType](const Val& val) {
            m_isMixerSectionVisibleChanged.send(sectionType, val.toBool());
        });
    }

    settings()->setDefaultValue(MUTE_HIDDEN_INSTRUMENTS, Val(true));
    settings()->valueChanged(MUTE_HIDDEN_INSTRUMENTS).onReceive(nullptr, [this](const Val& mute) {
        m_muteHiddenInstrumentsChanged.send(mute.toBool());
    });

    settings()->setDefaultValue(DEFAULT_SOUND_PROFILE_FOR_NEW_PROJECTS, Val(fallbackSoundProfileStr().toStdString()));

    for (aux_channel_idx_t idx = 0; idx < AUX_CHANNEL_NUM; ++idx) {
        Settings::Key auxSendKey = auxSendVisibleKey(idx);
        Settings::Key auxChannelKey = auxChannelVisibleKey(idx);

        settings()->setDefaultValue(auxSendKey, Val(idx == REVERB_CHANNEL_IDX));
        settings()->setDefaultValue(auxChannelKey, Val(false));

        settings()->valueChanged(auxSendKey).onReceive(this, [this, idx](const Val& val) {
            m_isAuxSendVisibleChanged.send(idx, val.toBool());
        });

        settings()->valueChanged(auxChannelKey).onReceive(this, [this, idx](const Val& val) {
            m_isAuxChannelVisibleChanged.send(idx, val.toBool());
        });
    }
}

bool PlaybackConfiguration::playNotesWhenEditing() const
{
    return settings()->value(PLAY_NOTES_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayNotesWhenEditing(bool value)
{
    settings()->setSharedValue(PLAY_NOTES_WHEN_EDITING, Val(value));
}

bool PlaybackConfiguration::playChordWhenEditing() const
{
    return settings()->value(PLAY_CHORD_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayChordWhenEditing(bool value)
{
    settings()->setSharedValue(PLAY_CHORD_WHEN_EDITING, Val(value));
}

bool PlaybackConfiguration::playHarmonyWhenEditing() const
{
    return settings()->value(PLAY_HARMONY_WHEN_EDITING).toBool();
}

void PlaybackConfiguration::setPlayHarmonyWhenEditing(bool value)
{
    settings()->setSharedValue(PLAY_HARMONY_WHEN_EDITING, Val(value));
}

PlaybackCursorType PlaybackConfiguration::cursorType() const
{
    return settings()->value(PLAYBACK_CURSOR_TYPE_KEY).toEnum<PlaybackCursorType>();
}

bool PlaybackConfiguration::isMixerSectionVisible(MixerSectionType sectionType) const
{
    return settings()->value(mixerSectionVisibleKey(sectionType)).toBool();
}

void PlaybackConfiguration::setMixerSectionVisible(MixerSectionType sectionType, bool visible)
{
    settings()->setSharedValue(mixerSectionVisibleKey(sectionType), Val(visible));
}

mu::async::Channel<MixerSectionType, bool> PlaybackConfiguration::isMixerSectionVisibleChanged() const
{
    return m_isMixerSectionVisibleChanged;
}

bool PlaybackConfiguration::isAuxSendVisible(aux_channel_idx_t index) const
{
    return settings()->value(auxSendVisibleKey(index)).toBool();
}

void PlaybackConfiguration::setAuxSendVisible(aux_channel_idx_t index, bool visible)
{
    settings()->setSharedValue(auxSendVisibleKey(index), Val(visible));
}

mu::async::Channel<aux_channel_idx_t, bool> PlaybackConfiguration::isAuxSendVisibleChanged() const
{
    return m_isAuxSendVisibleChanged;
}

bool PlaybackConfiguration::isAuxChannelVisible(aux_channel_idx_t index) const
{
    return settings()->value(auxChannelVisibleKey(index)).toBool();
}

void PlaybackConfiguration::setAuxChannelVisible(aux_channel_idx_t index, bool visible) const
{
    settings()->setSharedValue(auxChannelVisibleKey(index), Val(visible));
}

mu::async::Channel<aux_channel_idx_t, bool> PlaybackConfiguration::isAuxChannelVisibleChanged() const
{
    return m_isAuxChannelVisibleChanged;
}

gain_t PlaybackConfiguration::defaultAuxSendValue(aux_channel_idx_t index, AudioSourceType sourceType,
                                                  const String& instrumentSoundId) const
{
    TRACEFUNC;

    constexpr gain_t DEFAULT_VALUE = 0.30f;

    if (sourceType == AudioSourceType::MuseSampler) {
        if (index == REVERB_CHANNEL_IDX) {
            float lvl = musesamplerInfo()->defaultReverbLevel(instrumentSoundId);
            return mu::RealIsNull(lvl) ? DEFAULT_VALUE : lvl;
        }
    } else if (sourceType == AudioSourceType::Vsti) {
        return 0.f;
    }

    return DEFAULT_VALUE;
}

bool PlaybackConfiguration::muteHiddenInstruments() const
{
    return settings()->value(MUTE_HIDDEN_INSTRUMENTS).toBool();
}

void PlaybackConfiguration::setMuteHiddenInstruments(bool mute)
{
    settings()->setSharedValue(MUTE_HIDDEN_INSTRUMENTS, Val(mute));
}

mu::async::Channel<bool> PlaybackConfiguration::muteHiddenInstrumentsChanged() const
{
    return m_muteHiddenInstrumentsChanged;
}

const SoundProfileName& PlaybackConfiguration::basicSoundProfileName() const
{
    return BASIC_PROFILE_NAME;
}

const SoundProfileName& PlaybackConfiguration::museSoundProfileName() const
{
    return MUSE_PROFILE_NAME;
}

SoundProfileName PlaybackConfiguration::defaultProfileForNewProjects() const
{
    return String::fromStdString(settings()->value(DEFAULT_SOUND_PROFILE_FOR_NEW_PROJECTS).toString());
}

void PlaybackConfiguration::setDefaultProfileForNewProjects(const SoundProfileName& name)
{
    settings()->setSharedValue(DEFAULT_SOUND_PROFILE_FOR_NEW_PROJECTS, Val(name.toStdString()));
}

bool PlaybackConfiguration::soundPresetsMultiSelectionEnabled() const
{
    return settings()->value(SOUND_PRESETS_MULTI_SELECTION_KEY).toBool();
}

void PlaybackConfiguration::setSoundPresetsMultiSelectionEnabled(bool enabled)
{
    settings()->setSharedValue(SOUND_PRESETS_MULTI_SELECTION_KEY, Val(enabled));
}

bool PlaybackConfiguration::needToShowChangeSoundWarning() const
{
    return settings()->value(MIXER_NEED_TO_SHOW_CHANGE_SOUND_WARNING).toBool();
}

void PlaybackConfiguration::setNeedToShowChangeSoundWarning(bool show)
{
    settings()->setSharedValue(MIXER_NEED_TO_SHOW_CHANGE_SOUND_WARNING, Val(show));
}

const SoundProfileName& PlaybackConfiguration::fallbackSoundProfileStr() const
{
    if (musesamplerInfo() && musesamplerInfo()->isInstalled()) {
        return MUSE_PROFILE_NAME;
    }

    return BASIC_PROFILE_NAME;
}
