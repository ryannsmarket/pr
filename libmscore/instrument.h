//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include "stringdata.h"
#include "mscore.h"
#include "synthesizer/event.h"
#include "interval.h"
#include "clef.h"
#include <QtGlobal>
#include <QString>
#include <QPointer>
#include <QtPlugin>

namespace Ms {

class InstrumentTemplate;
class XmlWriter;
class XmlReader;
class Drumset;
class StringData;
class ChannelListener;

//---------------------------------------------------------
//   StaffName
//---------------------------------------------------------

class StaffName {
      QString _name;    // html string
      int _pos;         // even number -> between staves

   public:
      StaffName() {}
      StaffName(const QString& s, int p=0);

      bool operator==(const StaffName&) const;
      void read(XmlReader&);
      void write(XmlWriter& xml, const char* name) const;
      int pos() const { return _pos; }
      QString name() const { return _name; }
      };

//---------------------------------------------------------
//   StaffNameList
//---------------------------------------------------------

class StaffNameList : public QList<StaffName> {

   public:
      void write(XmlWriter& xml, const char* name) const;
      };

//---------------------------------------------------------
//   NamedEventList
//---------------------------------------------------------

struct NamedEventList {
      QString name;
      QString descr;
      std::vector<MidiCoreEvent> events;

      void write(XmlWriter&, const QString& name) const;
      void read(XmlReader&);
      bool operator==(const NamedEventList& i) const { return i.name == name && i.events == events; }
      };

//---------------------------------------------------------
//   MidiArticulation
//---------------------------------------------------------

struct MidiArticulation {
      QString name;
      QString descr;
      int velocity;           // velocity change: -100% - +100%
      int gateTime;           // gate time change: -100% - +100%
      void write(XmlWriter&) const;
      void read(XmlReader&);

      MidiArticulation() {}
      MidiArticulation(const QString& n, const QString& d, int v, int g) : name(n), descr(d), velocity(v), gateTime(g) {}
      bool operator==(const MidiArticulation& i) const;
      };


//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

class Channel {
      // this are the indexes of controllers which are always present in
      // Channel init EventList (maybe zero)
      QString _name;
      QString _descr;

      static const int DEFAULT_COLOR = 0x3399ff;
      int _color;  //rgb

      QString _synti;

      char _volume;
      char _pan;

      char _chorus;
      char _reverb;

      int _program;     // current values as shown in mixer
      int _bank;        // initialized from "init"
      int _channel { 0 };      // mscore channel number, mapped to midi port/channel

      bool _soloMute;
      bool _mute;
      bool _solo;

//      QList<ChannelListener *> listeners;
      QList<QPointer<QObject>> listeners;

public:
      static const char* DEFAULT_NAME;

      mutable std::vector<MidiCoreEvent> init;

      enum class A : char {
            HBANK, LBANK, PROGRAM, VOLUME, PAN, CHORUS, REVERB,
            INIT_COUNT
            };

      enum class Prop : char {
            VOLUME, PAN, CHORUS, REVERB, NAME, DESCR, PROGRAM, BANK, COLOR,
            SOLOMUTE, SOLO, MUTE, SYNTI, CHANNEL
            };


public:
      Channel();
      ~Channel();

      QString name() const { return _name; }
      void setName(const QString& value);
      QString descr() const { return _descr; }
      void setDescr(const QString& value);
      QString synti() const { return _synti; }
      void setSynti(const QString& value);
      int color() const { return _color; }
      void setColor(int value);

      char volume() const { return _volume; }
      void setVolume(char value);
      char pan() const { return _pan; }
      void setPan(char value);
      char chorus() const { return _chorus; }
      void setChorus(char value);
      char reverb() const { return _reverb; }
      void setReverb(char value);

      int program() const { return _program; }
      void setProgram(int value);
      int bank() const { return _bank; }
      void setBank(int value);
      int channel() const { return _channel; }
      void setChannel(int value);

      bool soloMute() const { return _soloMute; }
      void setSoloMute(bool value);
      bool mute() const { return _mute; }
      void setMute(bool value);
      bool solo() const { return _solo; }
      void setSolo(bool value);

      QList<NamedEventList> midiActions;
      QList<MidiArticulation> articulation;

      void write(XmlWriter&, Part *part) const;
      void read(XmlReader&, Part *part);
      void updateInitList() const;
      bool operator==(const Channel& c) { return (_name == c._name) && (_channel == c._channel); }

      void addListener(QPointer<QObject> l) { listeners.append(l); }
      void removeListener(QPointer<QObject> l) { listeners.removeOne(l); }

private:
      void firePropertyChanged(Channel::Prop prop);
      };

//---------------------------------------------------------
//   ChannelListener
//---------------------------------------------------------

class ChannelListener {
public:
      virtual ~ChannelListener() {}

      virtual void propertyChanged(Channel::Prop property) = 0;
      virtual void disconnectChannelListener() = 0;
      };

//Q_DECLARE_INTERFACE(ChannelListener, "com.musecore/iface/ChannelListener")

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

class Instrument {
      StaffNameList _longNames;
      StaffNameList _shortNames;
      QString _trackName;

      char _minPitchA, _maxPitchA, _minPitchP, _maxPitchP;
      Interval _transpose;
      QString _instrumentId;

      bool _useDrumset;
      Drumset* _drumset;
      StringData  _stringData;

      QList<NamedEventList>   _midiActions;
      QList<MidiArticulation> _articulation;
      QList<Channel*> _channel;      // at least one entry
      QList<ClefTypeList> _clefType;

   public:
      Instrument();
      Instrument(const Instrument&);
      void operator=(const Instrument&);
      ~Instrument();

      void read(XmlReader&, Part *part);
      bool readProperties(XmlReader&, Part* , bool* customDrumset);
      void write(XmlWriter& xml, Part *part) const;
      NamedEventList* midiAction(const QString& s, int channel) const;
      int channelIdx(const QString& s) const;
      void updateVelocity(int* velocity, int channel, const QString& name);
      void updateGateTime(int* gateTime, int channelIdx, const QString& name);

      bool operator==(const Instrument&) const;

      void setMinPitchP(int v)                               { _minPitchP = v;     }
      void setMaxPitchP(int v)                               { _maxPitchP = v;     }
      void setMinPitchA(int v)                               { _minPitchA = v;     }
      void setMaxPitchA(int v)                               { _maxPitchA = v;     }
      Interval transpose() const                             { return _transpose; }
      void setTranspose(const Interval& v)                   { _transpose = v; }
      QString instrumentId()                                 { return _instrumentId; }
      void setInstrumentId(const QString& instrumentId)      { _instrumentId = instrumentId; }

      void setDrumset(const Drumset* ds);
      const Drumset* drumset() const                         { return _drumset;    }
      Drumset* drumset()                                     { return _drumset;    }
      bool useDrumset() const                                { return _useDrumset; }
      void setUseDrumset(bool val);
      void setAmateurPitchRange(int a, int b)                { _minPitchA = a; _maxPitchA = b; }
      void setProfessionalPitchRange(int a, int b)           { _minPitchP = a; _maxPitchP = b; }
      Channel* channel(int idx)                              { return _channel[idx];  }
      const Channel* channel(int idx) const                  { return _channel[idx];  }
      ClefTypeList clefType(int staffIdx) const;
      void setClefType(int staffIdx, const ClefTypeList& c);

      const QList<NamedEventList>& midiActions() const       { return _midiActions; }
      const QList<MidiArticulation>& articulation() const    { return _articulation; }

      const QList<Channel*>& channel() const                 { return _channel; }
      void appendChannel(Channel* c)                         { _channel.append(c); }
      void clearChannels()                                   { _channel.clear(); }

      void setMidiActions(const QList<NamedEventList>& l)    { _midiActions = l;  }
      void setArticulation(const QList<MidiArticulation>& l) { _articulation = l; }
      const StringData* stringData() const                   { return &_stringData; }
      void setStringData(const StringData& d)                { _stringData = d;     }

      void setLongName(const QString& f);
      void setShortName(const QString& f);

      void addLongName(const StaffName& f);
      void addShortName(const StaffName& f);

      int minPitchP() const;
      int maxPitchP() const;
      int minPitchA() const;
      int maxPitchA() const;
      QString instrumentId() const;

      const QList<StaffName>& longNames() const;
      const QList<StaffName>& shortNames() const;
      QList<StaffName>& longNames();

      QList<StaffName>& shortNames();
      QString trackName() const;
      void setTrackName(const QString& s);
      static Instrument fromTemplate(const InstrumentTemplate* t);
      };

//---------------------------------------------------------
//   InstrumentList
//---------------------------------------------------------

class InstrumentList : public std::map<const int, Instrument*> {
      static Instrument defaultInstrument;

   public:
      InstrumentList() {}
      const Instrument* instrument(int tick) const;
      Instrument* instrument(int tick);
      void setInstrument(Instrument*, int tick);
      };

}     // namespace Ms
#endif

