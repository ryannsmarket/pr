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
#include "framework/audio/midiqueue.h"
#include "linuxaudiodriver.h"
#include "../alsa/alsaaudiodriver.h" //FIX: relative path, set path in CMakeLists
#if JACK_AUDIO
#include "../jack/jackaudiodriver.h" //FIX: relative path, set path in CMakeLists
#endif

#include "translation.h"
#include "log.h"
#include "runtime.h"

using namespace muse::audio;

LinuxAudioDriver::LinuxAudioDriver()
{
}

LinuxAudioDriver::~LinuxAudioDriver()
{
}

void LinuxAudioDriver::init()
{
    m_devicesListener.startWithCallback([this]() {
        return availableOutputDevices();
    });

    m_devicesListener.devicesChanged().onNotify(this, [this]() {
        m_availableOutputDevicesChanged.notify();
    });

    // notify driver if when musescore changes play-position or play/pause
    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        isPlayingChanged();
    });

    // HELP: is playbackPositionChanged notified for incremental changes too?
    playbackController()->playbackPositionChanged().onNotify(this, [this]() {
        positionChanged();
    });
}

std::string LinuxAudioDriver::name() const
{
    return m_current_audioDriverState->name();
}

bool LinuxAudioDriver::open(const Spec& spec, Spec* activeSpec)
{
    /**************************************************************************/
    // a bit lazy registering the midi-input-queue here, but midimodule isn't
    // available at audiomodule init, because midimodule starts after audiomodule
    // not sure we got the identity of eventQueue (ie, passed by reference)
#if defined(JACK_AUDIO)
    muse::async::Channel<muse::midi::tick_t, muse::midi::Event> queue = midiInPort()->eventReceived();
    m_current_audioDriverState->registerMidiInputQueue(queue);
#endif
    /**************************************************************************/

    // re-initialize devide
    m_current_audioDriverState->setAudioDelayCompensate(m_audioDelayCompensate);

    if (!m_current_audioDriverState->open(spec, activeSpec)) {
        // FIX: need to carry around the spec because of callback
        m_spec = spec;
        return false;
    }
    m_spec = *activeSpec;
    return true;
}

void LinuxAudioDriver::close()
{
    return m_current_audioDriverState->close();
}

bool LinuxAudioDriver::isOpened() const
{
    return m_current_audioDriverState->isOpened();
}

const LinuxAudioDriver::Spec& LinuxAudioDriver::activeSpec() const
{
    return s_format;
}

AudioDeviceID LinuxAudioDriver::outputDevice() const
{
    if (m_current_audioDriverState != nullptr) {
        return m_current_audioDriverState->m_deviceId;
    } else {
        LOGE() << "device is not opened, deviceId: " << m_deviceId;
        return m_deviceId; // FIX: should return optional type
    }
}

bool LinuxAudioDriver::makeDevice(const AudioDeviceID& deviceId)
{
#if defined(JACK_AUDIO)
    if (deviceId == "jack") {
        m_current_audioDriverState = std::make_unique<JackDriverState>(this);
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    } else if (deviceId == "alsa") {
        m_current_audioDriverState = std::make_unique<AlsaDriverState>();
#endif
#else
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    if (deviceId == "alsa") {
        m_current_audioDriverState = std::make_unique<AlsaDriverState>();
#endif
#endif
    } else {
        LOGE() << "Unknown device name: " << deviceId;
        return false;
    }
    return true;
}

// reopens the same device (if m_spec has changed)
bool LinuxAudioDriver::reopen(const AudioDeviceID& deviceId, Spec newSpec)
{
    // close current device if opened
    if (m_current_audioDriverState->isOpened()) {
        m_current_audioDriverState->close();
    }
    // maybe change device, if needed
    if (m_current_audioDriverState->name() != deviceId) {
        // select the new device driver
        if (!makeDevice(deviceId)) {
            return false;
        }
    }

    // re-initialize devide
    m_current_audioDriverState->setAudioDelayCompensate(m_audioDelayCompensate);

    // open the device driver
    if (!m_current_audioDriverState->open(newSpec, &newSpec)) {
        return false;
    }
    return true;
}

bool LinuxAudioDriver::selectOutputDevice(const AudioDeviceID& deviceId)
{
    // When starting, no previously device has been selected
    if (m_current_audioDriverState == nullptr) {
        LOGW() << "no previously opened device";
        return makeDevice(deviceId);
    }
    // If for some reason we select the same device, do nothing
    if (m_current_audioDriverState->name() == deviceId) {
        return true;
    }
    reopen(deviceId, m_spec);
    m_outputDeviceChanged.notify();
    return true;
}

bool LinuxAudioDriver::resetToDefaultOutputDevice()
{
#if defined(JACK_AUDIO)
    return selectOutputDevice("jack"); // FIX:
#else
    return selectOutputDevice("alsa"); // FIX:
#endif
}

async::Notification LinuxAudioDriver::outputDeviceChanged() const
{
    return m_outputDeviceChanged;
}

AudioDeviceList LinuxAudioDriver::availableOutputDevices() const
{
    AudioDeviceList devices;
#if defined(JACK_AUDIO)
    devices.push_back({ "jack", muse::trc("audio", "JACK") });
#endif
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    devices.push_back({ "alsa", muse::trc("audio", "ALSA") });
#endif
    return devices;
}

async::Notification LinuxAudioDriver::availableOutputDevicesChanged() const
{
    return m_availableOutputDevicesChanged;
}

void LinuxAudioDriver::isPlayingChanged()
{
    if (m_current_audioDriverState) {
        m_current_audioDriverState->changedPlaying();
    }
}

void LinuxAudioDriver::positionChanged()
{
    if (m_current_audioDriverState) {
        m_current_audioDriverState->changedPosition();
    }
}

bool LinuxAudioDriver::isPlaying() const
{
    return playbackController()->isPlaying();
}

float LinuxAudioDriver::playbackPositionInSeconds() const
{
    return playbackController()->playbackPositionInSeconds();
}

void LinuxAudioDriver::remotePlayOrStop(bool ps) const
{
    playbackController()->remotePlayOrStop(ps);
}

void LinuxAudioDriver::remoteSeek(msecs_t millis) const
{
    playbackController()->remoteSeek(millis);
}

int LinuxAudioDriver::audioDelayCompensate() const
{
    return m_audioDelayCompensate;
}

void LinuxAudioDriver::setAudioDelayCompensate(const int frames)
{
    m_audioDelayCompensate = frames;
}

unsigned int LinuxAudioDriver::outputDeviceBufferSize() const
{
    return m_current_audioDriverState->m_spec.samples;
}

bool LinuxAudioDriver::setOutputDeviceBufferSize(unsigned int bufferSize)
{
    if (m_spec.samples == (int)bufferSize) {
        return true;
    }
    m_spec.samples = (int)bufferSize;
    if (!reopen(m_current_audioDriverState->name(), m_spec)) {
        return false;
    }
    m_bufferSizeChanged.notify();
    return true;
}

async::Notification LinuxAudioDriver::outputDeviceBufferSizeChanged() const
{
    return m_bufferSizeChanged;
}

std::vector<unsigned int> LinuxAudioDriver::availableOutputDeviceBufferSizes() const
{
    std::vector<unsigned int> result;

    unsigned int n = MAXIMUM_BUFFER_SIZE;
    while (n >= MINIMUM_BUFFER_SIZE) {
        result.push_back(n);
        n /= 2;
    }

    std::sort(result.begin(), result.end());

    return result;
}

// FIX-JACK-20240823: change api callers, WAS:
//      unsigned int LinuxAudioDriver::sampleRate() const
unsigned int LinuxAudioDriver::outputDeviceSampleRate() const
{
    return m_current_audioDriverState->m_spec.sampleRate;
}

// FIX-JACK-20240823: merge this code
bool LinuxAudioDriver::setOutputDeviceSampleRate(unsigned int sampleRate)
{
    LOGE("------ setSamplerate: %u", sampleRate);
    if (m_spec.sampleRate == (int)sampleRate) {
        LOGE("------ SAME setSamplerate, doing nothing ------");
        return true;
    }
    m_spec.sampleRate = (int)sampleRate;
    LOGE("------ CHANGED setSamplerate, doing nothing ------");
    if (!reopen(m_current_audioDriverState->name(), m_spec)) {
        return false;
    }
    m_sampleRateChanged.notify();
    return true;
#if 0
    if (s_format.sampleRate == sampleRate) {
        return true;
    }

    bool reopen = isOpened();
    close();
    s_format.sampleRate = sampleRate;

    bool ok = true;
    if (reopen) {
        ok = open(s_format, &s_format);
    }

    if (ok) {
        m_sampleRateChanged.notify();
    }

    return ok;
#endif
}

async::Notification LinuxAudioDriver::outputDeviceSampleRateChanged() const
{
    return m_sampleRateChanged;
}

std::vector<unsigned int> LinuxAudioDriver::availableOutputDeviceSampleRates() const
{
    // ALSA API is not of any help to get sample rates supported by the driver.
    // (snd_pcm_hw_params_get_rate_[min|max] will return 1 to 384000 Hz)
    // So just returning a sensible hard-coded list.
    return {
        44100,
        48000,
        88200,
        96000,
    };

bool LinuxAudioDriver::pushMidiEvent(muse::midi::Event& e)
{
    if (m_current_audioDriverState) {
        m_current_audioDriverState->pushMidiEvent(e);
        return true;
    }
    return false;
}

std::vector<muse::midi::MidiDevice> LinuxAudioDriver::availableMidiDevices(muse::midi::MidiPortDirection direction) const
{
    if (m_current_audioDriverState) {
        return m_current_audioDriverState->availableMidiDevices(direction);
    }
    std::vector<muse::midi::MidiDevice> x;
    return x;
}

void LinuxAudioDriver::resume()
{
}

void LinuxAudioDriver::suspend()
{
}
