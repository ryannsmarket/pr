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
#include "audioplayer.h"

#include "log.h"

#include <QElapsedTimer>

using namespace mu;
using namespace mu::audio;
using namespace mu::audio::engine;

template<class T>
static const T& clamp(const T& v, const T& lo, const T& hi)
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

AudioPlayer::AudioPlayer()
{
    m_status.val = PlayStatus::UNDEFINED;
}

PlayStatus AudioPlayer::status() const
{
    return m_status.val;
}

async::Channel<PlayStatus> AudioPlayer::statusChanged() const
{
    return m_status.ch;
}

async::Channel<uint32_t> AudioPlayer::midiTickPlayed() const
{
    return m_midiTickPlayed;
}

void AudioPlayer::setMidiStream(const std::shared_ptr<midi::MidiStream>& stream)
{
    if (stream) {
        m_midiSource = std::make_shared<RpcMidiStream>();
        m_midiSource->loadMIDI(stream);

        m_tracks.clear();
        for (size_t num = 0; num < stream->initData.tracks.size(); ++num) {
            m_tracks[num] = std::make_shared<Track>();
        }

        m_status.set(PlayStatus::STOPED);
    } else {
        m_tracks.clear();
    }
}

bool AudioPlayer::play()
{
    LOGD() << "try play \n";
    if (m_status.val == PlayStatus::PLAYING) {
        LOGW() << "already playing \n";
        return true;
    }

    if (!doPlay()) {
        LOGE() << "failed do play \n";
        return false;
    }

    m_status.set(PlayStatus::PLAYING);

    return true;
}

void AudioPlayer::pause()
{
    doPause();
    m_status.set(PlayStatus::PAUSED);
}

void AudioPlayer::stop()
{
    doStop();
    //! NOTE The status will be changed in `onStop`
}

void AudioPlayer::rewind()
{
    doStop();
    //! NOTE The status will be changed in `onStop`
}

bool AudioPlayer::init()
{
    if (m_inited && audioEngine()->isInited()) {
        return true;
    }

    m_inited = audioEngine()->init();
    if (m_inited) {
        if (m_midiSource) {
            float samplerate = audioEngine()->sampleRate();
            m_midiSource->init(samplerate);
        }
    }
    return m_inited;
}

bool AudioPlayer::isInited() const
{
    return m_inited;
}

bool AudioPlayer::doPlay()
{
    if (!init()) {
        return false;
    }

    if (!hasTracks()) {
        return false;
    }

    IF_ASSERT_FAILED(m_midiSource) {
        return false;
    }

    if (!m_midiHandle) {
        m_midiHandle = audioEngine()->play(m_midiSource, -1, 0, true); // paused

        auto ctxCh = audioEngine()->playContextChanged(m_midiHandle);
        ctxCh.onReceive(this, [this](const engine::Context& ctx) { onMidiPlayContextChanged(ctx); });

        auto statusCh = audioEngine()->statusChanged(m_midiHandle);
        statusCh.onReceive(this, [this](const IAudioEngine::Status& status) { onMidiStatusChanged(status); });
    }

    audioEngine()->seek(m_midiHandle, m_beginPlayPosition);
    audioEngine()->setPause(m_midiHandle, false);

    return true;
}

void AudioPlayer::doPause()
{
    m_beginPlayPosition = currentPlayPosition();
    if (m_midiHandle) {
        audioEngine()->setPause(m_midiHandle, true);
    }
}

void AudioPlayer::doStop()
{
    audioEngine()->stop(m_midiHandle);
}

void AudioPlayer::onStop()
{
    m_beginPlayPosition = 0;
    m_midiHandle = 0;
    m_status.set(PlayStatus::STOPED);
}

float AudioPlayer::currentPlayPosition() const
{
    return audioEngine()->position(m_midiHandle);
}

float AudioPlayer::playbackPosition() const
{
    if (m_status.val == PlayStatus::PLAYING) {
        return currentPlayPosition();
    }
    return m_beginPlayPosition;
}

void AudioPlayer::setPlaybackPosition(float sec)
{
    sec = std::max(sec, 0.f);

    m_beginPlayPosition = sec;

    if (m_status.val == PlayStatus::PLAYING) {
        audioEngine()->seek(m_midiHandle, sec);
    }
}

float AudioPlayer::generalVolume() const
{
    return m_generalVolume;
}

void AudioPlayer::setGeneralVolume(float v)
{
    m_generalVolume = clamp(v, 0.f, 1.27f); //! NOTE 127 - midi limitation

    if (!isInited()) {
        return;
    }

    applyCurrentVolume();
}

float AudioPlayer::generalBalance() const
{
    return m_generalBalance;
}

void AudioPlayer::setGeneralBalance(float b)
{
    m_generalBalance = clamp(b, -1.f, 1.f);

    if (!isInited()) {
        return;
    }

    applyCurrentBalance();
}

float AudioPlayer::normalizedVolume(float volume) const
{
    return clamp(m_generalVolume * volume, 0.f, 1.27f); //! NOTE 127 - midi limitation
}

float AudioPlayer::normalizedBalance(float balance) const
{
    return clamp(m_generalBalance + balance, -1.f, 1.f);
}

void AudioPlayer::applyCurrentVolume()
{
    for (const auto& p : m_tracks) {
        m_midiSource->setTrackVolume(p.first, normalizedVolume(p.second->volume));
    }
}

void AudioPlayer::applyCurrentBalance()
{
    for (const auto& p : m_tracks) {
        m_midiSource->setTrackBalance(p.first, normalizedBalance(p.second->balance));
    }
}

bool AudioPlayer::hasTracks() const
{
    return m_tracks.size() > 0;
}

void AudioPlayer::onMidiPlayContextChanged(const engine::Context& ctx)
{
    //LOGI() << ctx.dump();

    if (ctx.hasVal(CtxKey::PlayTick)) {
        uint32_t tick = ctx.get<uint32_t>(CtxKey::PlayTick);
        if (tick != m_lastMidiPlayTick) {
            m_lastMidiPlayTick = tick;
            m_midiTickPlayed.send(tick);
        }
    }
}

void AudioPlayer::onMidiStatusChanged(engine::IAudioEngine::Status status)
{
    if (status == engine::IAudioEngine::Status::Stoped) {
        onStop();
    }
}
