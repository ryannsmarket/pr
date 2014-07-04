//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: jackaudio.cpp 5660 2012-05-22 14:17:39Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "jackaudio.h"
#include "musescore.h"
#include "libmscore/mscore.h"
#include "preferences.h"
// #include "msynth/synti.h"
#include "seq.h"
#include "libmscore/score.h"
#include "libmscore/repeatlist.h"
#include "mscore/playpanel.h"

#include <jack/midiport.h>

namespace Ms {

//---------------------------------------------------------
//   JackAudio
//---------------------------------------------------------

JackAudio::JackAudio(Seq* s)
   : Driver(s)
      {
      client = 0;
      }

//---------------------------------------------------------
//   ~JackAudio
//---------------------------------------------------------

JackAudio::~JackAudio()
      {
      if (client) {
            stop();
            if (jack_client_close(client)) {
                  qDebug("jack_client_close() failed: %s",
                     strerror(errno));
                  }
            }
      }

//---------------------------------------------------------
//   registerPort
//---------------------------------------------------------

void JackAudio::registerPort(const QString& name, bool input, bool midi)
      {
      int portFlag         = input ? JackPortIsInput : JackPortIsOutput;
      const char* portType = midi ? JACK_DEFAULT_MIDI_TYPE : JACK_DEFAULT_AUDIO_TYPE;

      jack_port_t* port = jack_port_register(client, qPrintable(name), portType, portFlag, 0);
      if (port == 0) {
            qDebug("JackAudio:registerPort(%s) failed", qPrintable(name));
            return;
            }
      if (midi) {
            if (input)
                  midiInputPorts.append(port);
            else
                  midiOutputPorts.append(port);
            }
      else
            ports.append(port);
      }

//---------------------------------------------------------
//   unregisterPort
//---------------------------------------------------------

void JackAudio::unregisterPort(jack_port_t* port)
      {
      jack_port_unregister(client, port);
      port = 0;
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> JackAudio::inputPorts()
      {
      const char** ports = jack_get_ports(client, 0, 0, 0);
      QList<QString> clientList;
      for (const char** p = ports; p && *p; ++p) {
            jack_port_t* port = jack_port_by_name(client, *p);
            int flags = jack_port_flags(port);
            if (!(flags & JackPortIsInput))
                  continue;
            char buffer[128];
            strncpy(buffer, *p, 128);
            if (strncmp(buffer, "Mscore", 6) == 0)
                  continue;
            clientList.append(QString(buffer));
            }
      return clientList;
      }

//---------------------------------------------------------
//   connect
//---------------------------------------------------------

void JackAudio::connect(void* src, void* dst)
      {
      const char* sn = jack_port_name((jack_port_t*) src);
      const char* dn = jack_port_name((jack_port_t*) dst);

      if (sn == 0 || dn == 0) {
            qDebug("JackAudio::connect: unknown jack ports");
            return;
            }
      if (jack_connect(client, sn, dn)) {
            qDebug("jack connect <%s>%p - <%s>%p failed",
               sn, src, dn, dst);
            }
      }

//---------------------------------------------------------
//   disconnect
//---------------------------------------------------------

void JackAudio::disconnect(void* src, void* dst)
      {
      const char* sn = jack_port_name((jack_port_t*) src);
      const char* dn = jack_port_name((jack_port_t*) dst);
      if (sn == 0 || dn == 0) {
            qDebug("JackAudio::disconnect: unknown jack ports");
            return;
            }
      if (jack_disconnect(client, sn, dn)) {
            qDebug("jack disconnect <%s> - <%s> failed", sn, dn);
            }
      }

//---------------------------------------------------------
//   start
//    return false on error
//---------------------------------------------------------

bool JackAudio::start()
      {
      if (jack_activate(client)) {
            qDebug("JACK: cannot activate client");
            return false;
            }
      // TODO: remember last connections for JACK audio
      if (preferences.useJackAudio) {
            /* connect the ports. Note: you can't do this before
               the client is activated, because we can't allow
               connections to be made to clients that aren't
               running.
             */
            restoreAudioConnections();
            }
      if (preferences.useJackMidi && preferences.rememberLastMidiConnections)
            restoreMidiConnections();
      return true;
      }

//---------------------------------------------------------
//   stop
//    return false on error
//---------------------------------------------------------

bool JackAudio::stop()
      {
      if (preferences.JackTimebaseMaster)
            releaseTimebaseCallback();
      if (preferences.useJackMidi && preferences.rememberLastMidiConnections)
            rememberMidiConnections();

      if (jack_deactivate(client)) {
            qDebug("cannot deactivate client");
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   framePos
//---------------------------------------------------------

int JackAudio::framePos() const
      {
      jack_nframes_t n = jack_frame_time(client);
      return (int)n;
      }

//---------------------------------------------------------
//   freewheel_callback
//---------------------------------------------------------

static void freewheel_callback(int /*starting*/, void*)
      {
      }

//---------------------------------------------------------
//   sampleRateCallback
//---------------------------------------------------------

int sampleRateCallback(jack_nframes_t sampleRate, void*)
      {
      qDebug("JACK: sample rate changed: %d", sampleRate);
      MScore::sampleRate = sampleRate;
      return 0;
      }

//---------------------------------------------------------
//   bufferSizeCallback called if JACK buffer changed
//---------------------------------------------------------

int bufferSizeCallback(jack_nframes_t nframes, void *arg)
      {
      JackAudio* audio = (JackAudio*)arg;
      audio->setBufferSize(nframes);
      return 0;
      }

static void registration_callback(jack_port_id_t, int, void*)
      {
//      qDebug("JACK: registration changed");
      }

static int graph_callback(void*)
      {
//      qDebug("JACK: graph changed");
      return 0;
      }

void JackAudio::timebase(jack_transport_state_t state, jack_nframes_t /*nframes*/, jack_position_t *pos, int /*new_pos*/, void *arg)
      {
      JackAudio* audio = (JackAudio*)arg;
      if (!audio->seq->score()) {
            if (state==JackTransportLooping || state==JackTransportRolling)
                  audio->stopTransport();
            }
      else if (audio->seq->isRunning()) {

            if (!audio->seq->score()->repeatList() || !audio->seq->score()->sigmap())
                  return;

            pos->valid = JackPositionBBT;
            int curTick = audio->seq->score()->repeatList()->utick2tick(audio->seq->getCurTick());
            int bar,beat,tick;
            audio->seq->score()->sigmap()->tickValues(curTick, &bar, &beat, &tick);
            // Providing the final tempo
            pos->beats_per_minute = 60 * audio->seq->curTempo() * audio->seq->score()->tempomap()->relTempo();
            pos->ticks_per_beat   = MScore::division;
            pos->tick             = tick;
            pos->bar              = bar+1;
            pos->beat             = beat+1;

            if (audio->timeSigTempoChanged) {
                  Fraction timeSig = audio->seq->score()->sigmap()->timesig(curTick).nominal();
                  pos->beats_per_bar =  timeSig.numerator();
                  pos->beat_type = timeSig.denominator();
                  audio->timeSigTempoChanged = false;
                  qDebug()<<"Time signature changed: "<< pos->beats_per_minute<<", bar: "<< pos->bar<<",beat: "<<pos->beat<<", tick:"<<pos->tick<<", time sig: "<<pos->beats_per_bar<<"/"<<pos->beat_type;
                  }
            }
      // TODO: Handle new_pos
      }
//---------------------------------------------------------
//   processAudio
//    JACK callback
//---------------------------------------------------------

int JackAudio::processAudio(jack_nframes_t frames, void* p)
      {
      JackAudio* audio = (JackAudio*)p;
      // Prevent from crash if score not opened yet
      if(!audio->seq->score())
            return 0;

      float* l;
      float* r;
      if (preferences.useJackAudio && audio->ports.size() == 2) {
            l = (float*)jack_port_get_buffer(audio->ports[0], frames);
            r = (float*)jack_port_get_buffer(audio->ports[1], frames);
            }
      else {
            l = 0;
            r = 0;
            }
      if (preferences.useJackMidi) {
            foreach(jack_port_t* port, audio->midiOutputPorts) {
                  void* portBuffer = jack_port_get_buffer(port, frames);
                  jack_midi_clear_buffer(portBuffer);
                  }
            foreach(jack_port_t* port, audio->midiInputPorts) {
                  void* portBuffer = jack_port_get_buffer(port, frames);
                  if (portBuffer) {
                        jack_nframes_t n = jack_midi_get_event_count(portBuffer);
                        for (jack_nframes_t i = 0; i < n; ++i) {
                              jack_midi_event_t event;
                              int r = jack_midi_event_get(&event, portBuffer, i);
                              if (r != 0)
                                    continue;
                              int nn = event.size;
                              int type = event.buffer[0];
                              if (nn && (type == ME_CLOCK || type == ME_SENSE))
                                    continue;
                              Event e;
                              e.setType(type);
                              e.setChannel(type & 0xf);
                              type &= 0xf0;
                              if (type == ME_NOTEON || type == ME_NOTEOFF) {
                                    e.setPitch(event.buffer[1]);
                                    e.setVelo(event.buffer[2]);
                                    audio->seq->eventToGui(e);
                                    }
                              else if (type == ME_CONTROLLER) {
                                    e.setController(event.buffer[1]);
                                    e.setValue(event.buffer[2]);
                                    audio->seq->eventToGui(e);
                                    }
                              }
                        }
                  }
            }
      if (l && r) {
            float buffer[frames * 2];
            audio->seq->process((unsigned)frames, buffer);
            float* sp = buffer;
            for (unsigned i = 0; i < frames; ++i) {
                  *l++ = *sp++;
                  *r++ = *sp++;
                  }
            }
      else {
            // JACK MIDI only
            float buffer[frames * 2];
            audio->seq->process((unsigned)frames, buffer);
            }
      return 0;
      }

//---------------------------------------------------------
//   jackError
//---------------------------------------------------------

static void jackError(const char *s)
      {
      qDebug("JACK ERROR: %s", s);
      }

//---------------------------------------------------------
//   noJackError
//---------------------------------------------------------

static void noJackError(const char*  s )
      {
      qDebug("noJACK ERROR: %s", s);
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool JackAudio::init(bool hot)
      {
      if (hot) {
            hotPlug();
            return true;
            }
      jack_set_error_function(noJackError);

      client = 0;
      timeSigTempoChanged = false;
      fakeState = Transport::STOP;
      strcpy(_jackName, "mscore");

      jack_options_t options = (jack_options_t)0;
      jack_status_t status;
      client = jack_client_open(_jackName, options, &status);
      if (client == 0) {
            qDebug("JackAudio()::init(): failed, status 0x%0x", status);
            return false;
            }

      jack_set_error_function(jackError);
      jack_set_process_callback(client, processAudio, this);
      //jack_on_shutdown(client, processShutdown, this);
      jack_set_sample_rate_callback(client, sampleRateCallback, this);
      jack_set_port_registration_callback(client, registration_callback, this);
      jack_set_graph_order_callback(client, graph_callback, this);
      jack_set_freewheel_callback (client, freewheel_callback, this);
      if (preferences.JackTimebaseMaster)
            setTimebaseCallback();
      if (jack_set_buffer_size_callback (client, bufferSizeCallback, this) != 0)
            qDebug("Can not set bufferSizeCallback");
      _segmentSize  = jack_get_buffer_size(client);

      MScore::sampleRate = sampleRate();
      // register mscore left/right output ports
      if (preferences.useJackAudio) {
            registerPort("left", false, false);
            registerPort("right", false, false);

            // connect mscore output ports to jack input ports
            QString lport = preferences.lPort;
            QString rport = preferences.rPort;
            QList<QString> ports = inputPorts();
            QList<QString>::iterator pi = ports.begin();
            if (lport.isEmpty()) {
                  if (pi != ports.end()) {
                        preferences.lPort = *pi;
                        ++pi;
                        }
                  else {
                        qDebug("no jack ports found");
                        jack_client_close(client);
                        client = 0;
                        return false;
                        }
                  }
            if (rport.isEmpty()) {
                  if (pi != ports.end()) {
                        preferences.rPort = *pi;
                        }
                  else {
                        qDebug("no jack port for right channel found!");
                        }
                  }
            }

      if (preferences.useJackMidi) {
            for (int i = 0; i < preferences.midiPorts; ++i)
                  registerPort(QString("mscore-midi-%1").arg(i+1), false, true);
            registerPort(QString("mscore-midiin-1"), true, true);
            }
      return true;
      }

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void JackAudio::startTransport()
      {
      if (preferences.useJackTransport)
            jack_transport_start(client);
      else
            fakeState = Transport::PLAY;
      }

//---------------------------------------------------------
//   stopTrasnport
//---------------------------------------------------------

void JackAudio::stopTransport()
      {
      if (preferences.useJackTransport)
            jack_transport_stop(client);
      else
            fakeState = Transport::STOP;
      }

//---------------------------------------------------------
//   getState
//---------------------------------------------------------

Transport JackAudio::getState()
      {
      if (!preferences.useJackTransport)
            return fakeState;
      int transportState = jack_transport_query(client, NULL);
      switch (transportState) {
            case JackTransportStopped:  return Transport::STOP;
            case JackTransportLooping:
            case JackTransportRolling:  return Transport::PLAY;
            case JackTransportStarting: return seq->isPlaying()?Transport::PLAY:Transport::STOP;// Keep current state
            default:
                  return Transport::STOP;
            }
      }

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void JackAudio::putEvent(const NPlayEvent& e, unsigned framePos)
      {
      if (!preferences.useJackMidi)
            return;

      int portIdx = e.channel() / 16;
      int chan    = e.channel() % 16;

// qDebug("JackAudio::putEvent %d:%d  pos %d(%d)", portIdx, chan, framePos, _segmentSize);

      if (portIdx < 0 || portIdx >= midiOutputPorts.size()) {
            qDebug("JackAudio::putEvent: invalid port %d", portIdx);
            return;
            }
      jack_port_t* port = midiOutputPorts[portIdx];
      if (midiOutputTrace) {
            const char* portName = jack_port_name(port);
            qDebug("MidiOut<%s>: jackMidi: ", portName);
            // e.dump();
            }
      void* pb = jack_port_get_buffer(port, _segmentSize);

      if (framePos >= _segmentSize) {
            qDebug("JackAudio::putEvent: time out of range %d(seg=%d)", framePos, _segmentSize);
            if (framePos > _segmentSize)
                  framePos = _segmentSize - 1;
            }

      switch(e.type()) {
            case ME_NOTEON:
            case ME_NOTEOFF:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
            case ME_PITCHBEND:
                  {
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, 3);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost");
                        return;
                        }
                  p[0] = e.type() | chan;
                  p[1] = e.dataA();
                  p[2] = e.dataB();
                  }
                  break;

            case ME_PROGRAM:
            case ME_AFTERTOUCH:
                  {
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, 2);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost");
                        return;
                        }
                  p[0] = e.type() | chan;
                  p[1] = e.dataA();
                  }
                  break;
          // Do we really need to handle ME_SYSEX?
          /*  case ME_SYSEX:
                  {
                  const unsigned char* data = e.edata();
                  int len = e.len();
                  unsigned char* p = jack_midi_event_reserve(pb, framePos, len+2);
                  if (p == 0) {
                        qDebug("JackMidi: buffer overflow, event lost");
                        return;
                        }
                  p[0]     = 0xf0;
                  p[len+1] = 0xf7;
                  memcpy(p+1, data, len);
                  }
                  break;*/
            case ME_SONGPOS:
            case ME_CLOCK:
            case ME_START:
            case ME_CONTINUE:
            case ME_STOP:
                  qDebug("JackMidi: event type %x not supported", e.type());
                  break;
            }
      }

//---------------------------------------------------------
//   midiRead
//---------------------------------------------------------

void JackAudio::midiRead()
      {
//      midiDriver->read();
      }

//---------------------------------------------------------
//   Called after tempo or time signature
//   changed while playback
//---------------------------------------------------------

void JackAudio::handleTimeSigTempoChanged()
      {
      timeSigTempoChanged = true;
      }

//---------------------------------------------------------
//   checkTransportSeek
//   The opposite of Timebase master:
//   check JACK Transport for a new position or tempo.
//---------------------------------------------------------

void JackAudio::checkTransportSeek(int cur_frame, int nframes)
      {
      if (!preferences.useJackTransport || !seq || !seq->score() || !seq->canStart())
            return;

      // Obtaining the current JACK Transport position
      jack_position_t pos;
      jack_transport_query(client, &pos);

      int cur_utick = seq->score()->utime2utick((qreal)cur_frame / MScore::sampleRate);
      int utick     = seq->score()->utime2utick((qreal)pos.frame / MScore::sampleRate);

      // Conversion is not precise, should check frames and uticks
      if (labs((long int)cur_frame-(long int)pos.frame)>nframes+1 && abs(utick - cur_utick)> seq->score()->utime2utick((qreal)nframes / MScore::sampleRate)+1) {
            qDebug()<<"JACK Transport position changed, cur_frame: "<<cur_frame<<",pos.frame: "<<pos.frame<<", frame diff: "<<labs((long int)cur_frame-(long int)pos.frame)<<"cur utick:"<<cur_utick<<",seek to utick: "<<utick<<", tick diff: "<<abs(utick - cur_utick);
            seq->seek(utick, false);
            }

      // Tempo
      if (!preferences.JackTimebaseMaster  && (pos.valid & JackPositionBBT)) {
            if (!seq->score()->tempomap()) {
                  return;
                  }
            if (int(pos.beats_per_minute) != int(60 * seq->curTempo() * seq->score()->tempomap()->relTempo())) {
                  qDebug()<<"JACK Transport tempo changed! JACK bpm: "<<(int)pos.beats_per_minute<<", current bpm: "<<int(60 * seq->curTempo() * seq->score()->tempomap()->relTempo());
                  qreal newRelTempo = pos.beats_per_minute / (60* seq->curTempo());
                  int utick = seq->getCurTick();
                  seq->score()->tempomap()->setRelTempo(newRelTempo);
                  seq->score()->repeatList()->update();
                  seq->setPlayTime(seq->score()->utick2utime(utick) * MScore::sampleRate);
                  // Update UI
                  if (mscore->getPlayPanel()) {
                        mscore->getPlayPanel()->setRelTempo(newRelTempo);
                        mscore->getPlayPanel()->setTempo(seq->curTempo() * newRelTempo);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   seekTransport
//---------------------------------------------------------

void JackAudio::seekTransport(int utick)
      {
      qDebug()<<"jack locate to utick: "<<utick<<", frame: "<<seq->score()->utick2utime(utick) * MScore::sampleRate;
      jack_transport_locate(client, seq->score()->utick2utime(utick) * MScore::sampleRate);
      }

//---------------------------------------------------------
//   setTimebaseCallback
//---------------------------------------------------------

void JackAudio::setTimebaseCallback()
      {
      int errCode = jack_set_timebase_callback(client, 0, timebase, this); // 0: force set timebase
      if (errCode == 0) {
            qDebug("Registered as JACK Timebase Master.");
            }
      else {
            preferences.JackTimebaseMaster = false;
            qDebug("Unable to take over JACK Timebase, error code: %i",errCode);
            }
      }

//---------------------------------------------------------
//   releaseTimebaseCallback
//---------------------------------------------------------

void JackAudio::releaseTimebaseCallback()
      {
      int errCode = jack_release_timebase(client);
      if (errCode == 0)
            qDebug("Unregistered as JACK Timebase Master");
      else
            qDebug("Unable to unregister as JACK Timebase Master (not a Timebase Master?), error code: %i", errCode);
      }

//---------------------------------------------------------
//   restoreAudioConnections
//   Connect to the ports in Preferences->I/O
//---------------------------------------------------------

void JackAudio::restoreAudioConnections()
      {
      jack_port_disconnect(client, ports[0]);
      jack_port_disconnect(client, ports[1]);

      QString lport = preferences.lPort;
      QString rport = preferences.rPort;

      const char* src = jack_port_name(ports[0]);
      int rv = jack_connect(client, src, qPrintable(lport));
      if (rv)
            qDebug("jack connect lport <%s> - <%s> failed: %d", src, qPrintable(lport), rv);

      src = jack_port_name(ports[1]);
      if (!rport.isEmpty()) {
            rv = jack_connect(client, src, qPrintable(rport));
            if (rv)
                  qDebug("jack connect rport <%s> - <%s> failed: %d", src, qPrintable(rport), rv);
            }
      }

//---------------------------------------------------------
//   rememberMidiConnections
//---------------------------------------------------------

void JackAudio::rememberMidiConnections()
      {
      if (MScore::debugMode)
            qDebug("Saving midi connections...");
      QSettings settings;
      int port = 0;
      foreach(jack_port_t* mp, midiOutputPorts) {
            const char** cc = jack_port_get_connections(mp);
            const char** c = cc;
            int idx = 0;
            while (c) {
                  const char* p = *c++;
                  if (p == 0)
                        break;
                  settings.setValue(QString("midi-%1-%2").arg(port).arg(idx), p);
                  ++idx;
                  }
            settings.setValue(QString("midi-%1-connections").arg(port), idx);
            free((void*)cc);
            ++port;
            }
      // We don't use it now
      port = 0;
      foreach(jack_port_t* mp, midiInputPorts) {
            const char** cc = jack_port_get_connections(mp);
            const char** c = cc;
            int idx = 0;
            while (c) {
                  const char* p = *c++;
                  if (p == 0)
                        break;
                  settings.setValue(QString("midiin-%1-%2").arg(idx).arg(port), p);
                  ++idx;
                  }
            settings.setValue(QString("midiin-%1-connections").arg(port), idx);
            free((void*)cc);
            ++port;
            }
      }

//---------------------------------------------------------
//   restoreMidiConnections
//   Connects to the ports from previous connection
//---------------------------------------------------------

void JackAudio::restoreMidiConnections()
      {
      QSettings settings;
      int nPorts = midiOutputPorts.size();
      for (int i = 0; i < nPorts; ++i) {
            int n = settings.value(QString("midi-%1-connections").arg(i), 0).toInt();
            const char* src = jack_port_name(midiOutputPorts[i]);
            for (int k = 0; k < n; ++k) {
                  QString dst = settings.value(QString("midi-%1-%2").arg(i).arg(k), "").toString();
                  if (!dst.isEmpty()) {
                        if (jack_port_connected_to(midiOutputPorts[i], qPrintable(dst)))
                              continue;
                        int rv = jack_connect(client, src, qPrintable(dst));
                        if (rv)
                              qDebug("jack connect midi output <%s> - <%s> failed: %d", src, qPrintable(dst), rv);
                        }
                  }
            }
      nPorts = midiInputPorts.size();
      for (int i = 0; i < nPorts; ++i) {
            int n = settings.value(QString("midiin-%1-connections").arg(i), 0).toInt();
            const char* dst = jack_port_name(midiInputPorts[i]);
            for (int k = 0; k < n; ++k) {
                  QString src = settings.value(QString("midiin-%1-%2").arg(k).arg(i), "").toString();
                  if (!src.isEmpty()) {
                        if (jack_port_connected_to(midiInputPorts[i], qPrintable(src)))
                              continue;
                        int rv = jack_connect(client, qPrintable(src), dst);
                        if (rv)
                              qDebug("jack connect midi input <%s> - <%s> failed: %d",qPrintable(src), dst, rv);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   hotPlug
//   Change driver settings without unload
//---------------------------------------------------------

void JackAudio::hotPlug()
      {
      // Remember connections before calling jack_deactivate() - it disconnects all ports
      if (preferences.useJackMidi && preferences.rememberLastMidiConnections && midiOutputPorts.size() != 0 && midiInputPorts.size() != 0)
            rememberMidiConnections();

      // We must set callbacks only on inactive client
      if (jack_deactivate(client))
            qDebug("cannot deactivate client");

      // Timebase Master
      if (preferences.JackTimebaseMaster)
            setTimebaseCallback();
      else
            releaseTimebaseCallback();

      // Audio connections
      if (preferences.useJackAudio) {
            if (ports.size() == 0) {
                  registerPort("left", false, false);
                  registerPort("right", false, false);
                  }
            }
      else if (!preferences.useJackAudio) {
            foreach(jack_port_t* p, ports) {
                  unregisterPort(p);
                  ports.removeOne(p);
                  }
            }

      // Midi connections
      if (preferences.useJackMidi) {
            if (midiOutputPorts.size()<preferences.midiPorts) {
                  for (int i = midiOutputPorts.size(); i < preferences.midiPorts; ++i) {
                        registerPort(QString("mscore-midi-%1").arg(i+1), false, true);
                        }
                  }
            else if (midiOutputPorts.size()>preferences.midiPorts) {
                  for(int i = midiOutputPorts.size()-1; i>=preferences.midiPorts; --i) {
                        unregisterPort(midiOutputPorts[i]);
                        midiOutputPorts.removeAt(i);
                        }
                  }

            if (midiInputPorts.size() == 0)
                  registerPort(QString("mscore-midiin-1"), true, true);
            }
      else { // No midi
            foreach(jack_port_t* mp, midiOutputPorts) {
                  unregisterPort(mp);
                  midiOutputPorts.removeOne(mp);
                  }
            if (midiInputPorts.size() != 0) {
                  unregisterPort(midiInputPorts[0]);
                  midiInputPorts.removeOne(midiInputPorts[0]);
                  }
            }
      if (jack_activate(client)) {
            qDebug("JACK: cannot activate client");
            }

      if (preferences.useJackAudio)
            restoreAudioConnections();
      if (preferences.useJackMidi && preferences.rememberLastMidiConnections)
            restoreMidiConnections();
      }
}
