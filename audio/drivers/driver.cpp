//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#if (defined (_MSCVER) || defined (_MSC_VER))
// Include stdint.h and #define _STDINT_H to prevent <systemdeps.h> from redefining types
// #undef UNICODE to force LoadLibrary to use the char-based implementation instead of the wchar_t one.
#include <stdint.h>
#define _STDINT_H 1
#endif

#include "config.h"
#include "driver.h"

#include "mscore/preferences.h"

#ifdef USE_JACK
#include "jackaudio.h"
#endif

#ifdef USE_ALSA
#include "alsa.h"
#endif
#ifdef USE_PORTAUDIO
#include "pa.h"
#endif

#ifdef USE_MEDIAKIT
#include "haiku.h"
#endif

namespace Ms {
#ifdef USE_PULSEAUDIO
extern Driver* getPulseAudioDriver(Seq*);
#endif

bool alsaIsUsed = false, jackIsUsed = false, portAudioIsUsed = false, pulseAudioIsUsed = false, mediaKitIsUsed = false;

//---------------------------------------------------------
//   driverFactory
//    driver can be: jack alsa pulse portaudio
//---------------------------------------------------------

Driver* driverFactory(Seq* seq, QString driverName)
{
    Driver* driver = 0;
#if 1 // DEBUG: force "no audio"
    bool useJackFlag
        = (preferences.getBool(PREF_IO_JACK_USEJACKAUDIO) || preferences.getBool(PREF_IO_JACK_USEJACKMIDI));
    bool useAlsaFlag       = preferences.getBool(PREF_IO_ALSA_USEALSAAUDIO);
    bool usePortaudioFlag  = preferences.getBool(PREF_IO_PORTAUDIO_USEPORTAUDIO);
    bool usePulseAudioFlag = preferences.getBool(PREF_IO_PULSEAUDIO_USEPULSEAUDIO);
    bool useMediaKitFlag   = preferences.getBool(PREF_IO_MEDIAKIT_USEMEDIAKITAUDIO);

    if (!driverName.isEmpty()) {
        driverName        = driverName.toLower();
        useJackFlag       = false;
        useAlsaFlag       = false;
        usePortaudioFlag  = false;
        usePulseAudioFlag = false;
        useMediaKitFlag   = false;
        if (driverName == "jack") {
            useJackFlag = true;
        } else if (driverName == "alsa") {
            useAlsaFlag = true;
        } else if (driverName == "pulse") {
            usePulseAudioFlag = true;
        } else if (driverName == "portaudio") {
            usePortaudioFlag = true;
        } else if (driverName == "mediakit" || driverName == "haiku") {
            useMediaKitFlag = true;
        }
    }

    alsaIsUsed       = false;
    jackIsUsed       = false;
    portAudioIsUsed  = false;
    pulseAudioIsUsed = false;
    mediaKitIsUsed   = false;

#ifdef USE_PULSEAUDIO
    if (usePulseAudioFlag) {
        driver = getPulseAudioDriver(seq);
        if (!driver->init()) {
            qDebug("init PulseAudio failed");
            delete driver;
            driver = 0;
        } else {
            pulseAudioIsUsed = true;
        }
    }
#else
    (void)usePulseAudioFlag;   // avoid compiler warning
#endif
#ifdef USE_PORTAUDIO
    if (usePortaudioFlag) {
        driver = new Portaudio(seq);
        if (!driver->init()) {
            qDebug("init PortAudio failed");
            delete driver;
            driver = 0;
        } else {
            portAudioIsUsed = true;
        }
    }
#else
    (void)usePortaudioFlag;   // avoid compiler warning
#endif
#ifdef USE_ALSA
    if (driver == 0 && useAlsaFlag) {
        driver = new AlsaAudio(seq);
        if (!driver->init()) {
            qDebug("init ALSA driver failed");
            delete driver;
            driver = 0;
        } else {
            alsaIsUsed = true;
        }
    }
#else
    (void)useAlsaFlag;   // avoid compiler warning
#endif
#ifdef USE_JACK
    if (useJackFlag) {
        useAlsaFlag      = false;
        usePortaudioFlag = false;
        driver = new JackAudio(seq);
        if (!driver->init()) {
            qDebug("no JACK server found");
            delete driver;
            driver = 0;
        } else {
            jackIsUsed = true;
        }
    }
#else
    (void)useJackFlag;    // avoid compiler warning
#endif
#ifdef USE_MEDIAKIT
    if (useMediaKitFlag) {
        driver = new HaikuMediaKit(seq);
        if (!driver->init()) {
            qDebug("init Haiku Media Kit failed");
            delete driver;
            driver = 0;
        } else {
            mediaKitIsUsed = true;
        }
    }
#else
    (void)useMediaKitFlag; // avoid compiler warning
#endif
#endif
    if (driver == 0) {
        qDebug("no audio driver found");
    }

    return driver;
}
}
