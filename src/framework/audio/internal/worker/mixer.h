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
#ifndef MU_AUDIO_MIXER_H
#define MU_AUDIO_MIXER_H

#include <memory>
#include <map>

#include "modularity/ioc.h"
#include "async/asyncable.h"
#include "types/retval.h"

#include "abstractaudiosource.h"
#include "mixerchannel.h"
#include "internal/dsp/limiter.h"
#include "ifxresolver.h"
#include "iaudioconfiguration.h"
#include "iclock.h"

namespace mu::audio {
class Mixer : public AbstractAudioSource, public std::enable_shared_from_this<Mixer>, public async::Asyncable
{
    INJECT(fx::IFxResolver, fxResolver)
    INJECT(IAudioConfiguration, configuration)
public:
    Mixer();
    ~Mixer();

    IAudioSourcePtr mixedSource();

    RetVal<MixerChannelPtr> addChannel(const TrackId trackId, IAudioSourcePtr source);
    RetVal<MixerChannelPtr> addAuxChannel(const TrackId trackId);
    Ret removeChannel(const TrackId trackId);

    void addClock(IClockPtr clock);
    void removeClock(IClockPtr clock);

    AudioOutputParams masterOutputParams() const;
    void setMasterOutputParams(const AudioOutputParams& params);
    void clearMasterOutputParams();
    async::Channel<AudioOutputParams> masterOutputParamsChanged() const;

    async::Channel<audioch_t, AudioSignalVal> masterAudioSignalChanges() const;

    // IAudioSource
    void setSampleRate(unsigned int sampleRate) override;
    unsigned int audioChannelsCount() const override;
    bool setAudioChannelsCount(unsigned int channels) override;

    samples_t process(float* outBuffer, size_t bufferSize, samples_t samplesPerChannel) override;
    void setIsActive(bool arg) override;

private:
    template<typename T>
    struct BufferViewT
    {
        T* const data;
        const size_t size;
        const audioch_t channels;
    };
    using BufferView = BufferViewT<float>;
    using ConstBufferView = BufferViewT<const float>;

    void mixOutputFromChannel(BufferView outBuffer, ConstBufferView inBuffer, unsigned int samplesCount, gain_t signalAmount = 1.f);
    void prepareAuxBuffers(size_t outBufferSize);
    void writeTrackToAuxBuffers(const AuxSendsParams& auxSends, ConstBufferView trackBuffer, samples_t samplesPerChannel);
    void processAuxChannels(BufferView buffer, samples_t samplesPerChannel);

    void completeOutput(BufferView buffer, samples_t samplesPerChannel);
    void notifyAboutAudioSignalChanges(const audioch_t audioChannelNumber, const float linearRms) const;

    std::vector<float> m_writeCacheBuff;
    std::vector<std::vector<float> > m_auxBuffers;

    AudioOutputParams m_masterParams;
    async::Channel<AudioOutputParams> m_masterOutputParamsChanged;
    std::vector<IFxProcessorPtr> m_masterFxProcessors = {};

    std::map<TrackId, MixerChannelPtr> m_trackChannels = {};
    std::vector<MixerChannelPtr> m_auxChannels = {};

    dsp::LimiterPtr m_limiter = nullptr;

    std::set<IClockPtr> m_clocks;
    audioch_t m_audioChannelsCount = 0;

    mutable AudioSignalsNotifier m_audioSignalNotifier;
};

using MixerPtr = std::shared_ptr<Mixer>;
}

#endif // MU_AUDIO_MIXER_H
