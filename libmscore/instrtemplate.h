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

#ifndef __INSTRTEMPLATE_H__
#define __INSTRTEMPLATE_H__

#include "mscore.h"
#include "instrument.h"
#include "clef.h"

namespace Ms {

class Xml;
class Part;
class Staff;
class Tablature;

//---------------------------------------------------------
//   InstrumentGenre
//---------------------------------------------------------

class InstrumentGenre {
   public:
      QString id;
      InstrumentTemplate* instrumentTemplate;

      InstrumentGenre() { id = ""; }
      };

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

class InstrumentTemplate {
      int staves;             // 1 <= MAX_STAVES

   public:
      QString id;
      QString trackName;
//      QString groupId;
      QList<StaffName> longNames;      ///< shown on first system
      QList<StaffName> shortNames;     ///< shown on followup systems
      QString musicXMLid;              ///< used in MusicXML 3.0
      QString description;             ///< a longer description of the instrument

      char minPitchA;         // pitch range playable by an amateur
      char maxPitchA;
      char minPitchP;         // pitch range playable by professional
      char maxPitchP;

      Interval transpose;     // for transposing instruments

      StaffGroup  staffGroup;
      int         staffTypePreset;
      bool useDrumset;
      Drumset* drumset;

      Tablature* tablature;

      QList<NamedEventList>   midiActions;
      QList<MidiArticulation> articulation;
      QList<Channel>          channel;

      ClefTypeList clefTypes[MAX_STAVES];
      int staffLines[MAX_STAVES];
      BracketType bracket[MAX_STAVES];            // bracket type (NO_BRACKET)
      int bracketSpan[MAX_STAVES];
      int barlineSpan[MAX_STAVES];
      bool smallStaff[MAX_STAVES];

      bool extended;          // belongs to extended instrument set if true

//      InstrumentGenre instrumentGenres;

      InstrumentTemplate();
      InstrumentTemplate(const InstrumentTemplate&);
      ~InstrumentTemplate();
      void init(const InstrumentTemplate&);
      void linkGenre(const QString &);
      void addGenre(QList<InstrumentGenre *>);

      void setPitchRange(const QString& s, char* a, char* b) const;
      void write(Xml& xml) const;
      void write1(Xml& xml) const;
      void read(XmlReader&);
      int nstaves() const { return staves; }
      void setStaves(int val) { staves = val; }
      };

//---------------------------------------------------------
//   InstrumentGroup - Extends InstrumentSection
//---------------------------------------------------------

struct InstrumentGroup {
      QString id;
      QString name;
      QString groupId;
      bool extended;          // belongs to extended instruments set if true
      QList<InstrumentTemplate*> instrumentTemplates;

      void read(XmlReader&);
//      InstrumentGroup * searchInstrumentGroup(const QString& name);

      InstrumentGroup() { extended = false; }
      };

//---------------------------------------------------------
//   InstrumentSection - Extends InstrumentGroup
//                       Can contain a list of instrument groups,
//                       & can contain instruments
//---------------------------------------------------------

struct InstrumentSection {      // : public InstrumentGroup {
      QString id;
      QString name;
      QString sectionId;
      bool extended;          // belongs to extended instruments set if true
      QList<InstrumentGroup*> instrumentGroups;

      void read(XmlReader&);

      InstrumentSection * searchInstrumentSection(const QString& name);
      InstrumentSection() { extended = false; }
      };

extern QList<InstrumentGenre*> instrumentGenres;
extern QList<InstrumentSection*> instrumentSections;
extern QList<InstrumentGroup*> instrumentGroups;
extern bool loadInstrumentTemplates(const QString& instrTemplates);
extern bool saveInstrumentTemplates(const QString& instrTemplates);
extern InstrumentTemplate* searchTemplate(const QString& name);

}     // namespace Ms

extern InstrumentType* searchInstrumentType(const QString& name);
extern InstrumentGroup * searchInstrumentGroup(const QString& name);


// extern InstrumentGroup * searchInstrumentGroup(const QString& name);
//extern InstrumentSection* searchInstrumentSection(const QString& name);

#endif

