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
#ifndef MU_PLAYBACK_PLAYBACKCONFIGURATIONSTUB_H
#define MU_PLAYBACK_PLAYBACKCONFIGURATIONSTUB_H

#include "playback/iplaybackconfiguration.h"

namespace mu::playback {
class PlaybackConfigurationStub : public IPlaybackConfiguration
{
public:
    bool playNotesWhenEditing() const override;
    void setPlayNotesWhenEditing(bool value) override;

    bool playChordWhenEditing() const override;
    void setPlayChordWhenEditing(bool value) override;

    bool playHarmonyWhenEditing() const override;
    void setPlayHarmonyWhenEditing(bool value) override;

    PlaybackCursorType cursorType() const override;

    bool isMixerSectionVisible(MixerSectionType sectionType) const override;
    void setMixerSectionVisible(MixerSectionType sectionType, bool visible) override;
    mu::async::Channel<MixerSectionType, bool> isMixerSectionVisibleChanged() const override;

    bool isAuxSendVisible(muse::audio::aux_channel_idx_t index) const override;
    void setAuxSendVisible(muse::audio::aux_channel_idx_t index, bool visible) override;
    async::Channel<muse::audio::aux_channel_idx_t, bool> isAuxSendVisibleChanged() const override;

    bool isAuxChannelVisible(muse::audio::aux_channel_idx_t index) const override;
    void setAuxChannelVisible(muse::audio::aux_channel_idx_t index, bool visible) const override;
    async::Channel<muse::audio::aux_channel_idx_t, bool> isAuxChannelVisibleChanged() const override;

    muse::audio::gain_t defaultAuxSendValue(muse::audio::aux_channel_idx_t index, muse::audio::AudioSourceType sourceType,
                                            const String& instrumentSoundId) const override;

    bool muteHiddenInstruments() const override;
    void setMuteHiddenInstruments(bool mute) override;
    async::Channel<bool> muteHiddenInstrumentsChanged() const override;

    const SoundProfileName& basicSoundProfileName() const override;
    const SoundProfileName& museSoundProfileName() const override;

    SoundProfileName defaultProfileForNewProjects() const override;
    void setDefaultProfileForNewProjects(const SoundProfileName& name) override;

    bool soundPresetsMultiSelectionEnabled() const override;
    void setSoundPresetsMultiSelectionEnabled(bool enabled) override;

    bool needToShowChangeSoundWarning() const override;
    void setNeedToShowChangeSoundWarning(bool show) override;
};
}

#endif // MU_PLAYBACK_PLAYBACKCONFIGURATIONSTUB_H
