//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 John Pirie
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "importgtp.h"
#include "globals.h"
#include "libmscore/score.h"
#include "libmscore/measurebase.h"
#include "libmscore/text.h"
#include "libmscore/stafftext.h"
#include "libmscore/box.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/measure.h"
#include "libmscore/timesig.h"
#include "libmscore/tremolo.h"
#include "libmscore/rest.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/stringdata.h"
#include "libmscore/clef.h"
#include "libmscore/lyrics.h"
#include "libmscore/tempotext.h"
#include "libmscore/slur.h"
#include "libmscore/tie.h"
#include "libmscore/tuplet.h"
#include "libmscore/barline.h"
#include "libmscore/excerpt.h"
#include "libmscore/stafftype.h"
#include "libmscore/bracket.h"
#include "libmscore/articulation.h"
#include "libmscore/keysig.h"
#include "libmscore/harmony.h"
#include "libmscore/bend.h"
#include "libmscore/tremolobar.h"
#include "libmscore/segment.h"
#include "libmscore/rehearsalmark.h"
#include "libmscore/dynamic.h"
#include "libmscore/arpeggio.h"
#include "libmscore/volta.h"
#include "libmscore/instrtemplate.h"
#include "libmscore/hairpin.h"
#include "libmscore/fingering.h"
#include "libmscore/sym.h"
//#include "libmscore/ottava.h"
#include "libmscore/marker.h"
#include "preferences.h"

namespace Ms {

//---------------------------------------------------------
//   readBit
//---------------------------------------------------------

int GuitarPro6::readBit() {
      // calculate the byte index by dividing the position in bits by the bits per byte
      int byteIndex = this->position / BITS_IN_BYTE;
      // calculate our offset so we know how much to bit shift
      int byteOffset = ((BITS_IN_BYTE-1) - (this->position % BITS_IN_BYTE));
      // calculate the bit which we want to read
      int bit = ((((*buffer)[byteIndex] & 0xff) >> byteOffset) & 0x01);
      // increment our curent position so we know this bit has been read
      this->position++;
      // return the bit we calculated
      return bit;
      }


//---------------------------------------------------------
//   readBits
//---------------------------------------------------------

int GuitarPro6::readBits(int bitsToRead) {
      int bits = 0;
      for (int i = (bitsToRead-1); i>=0; i--) {
            bits |= (readBit() << i);
            }
      return bits;
      }

//---------------------------------------------------------
//   readBitsReversed
//---------------------------------------------------------

int GuitarPro6::readBitsReversed(int bitsToRead) {
      int bits = 0;
      for( int i = 0 ; i < bitsToRead ; i ++ ) {
            bits |= (readBit() << i );
            }
      return bits;
      }

//---------------------------------------------------------
//   getBytes
//---------------------------------------------------------

QByteArray GuitarPro6::getBytes(QByteArray* buffer, int offset, int length) {
      QByteArray newBytes;
      // compute new bytes from our buffer and return byte array
      for (int i = 0; i < length; i++) {
            if (buffer->length() > offset + i) {
                  newBytes.insert(i, ((*buffer)[offset+i]));
                  }
            }
      return newBytes;
      }

//---------------------------------------------------------
//   readInteger
//---------------------------------------------------------

int GuitarPro6::readInteger(QByteArray* buffer, int offset) {
      // assign four bytes and take them from the buffer
      char bytes[4];
      bytes[0] = (*buffer)[offset+0];
      bytes[1] = (*buffer)[offset+1];
      bytes[2] = (*buffer)[offset+2];
      bytes[3] = (*buffer)[offset+3];
      // increment positioning so we keep track of where we are
      this->position+=sizeof(int)*BITS_IN_BYTE;
      // bit shift in order to compute our integer value and return
      return ((bytes[3] & 0xff) << 24) | ((bytes[2] & 0xff) << 16) | ((bytes[1] & 0xff) << 8) | (bytes[0] & 0xff);
      }

//---------------------------------------------------------
//   readString
//---------------------------------------------------------

QByteArray GuitarPro6::readString(QByteArray* buffer, int offset, int length) {
      QByteArray filename;
      // compute the string by iterating through the buffer
      for (int i = 0; i < length; i++) {
            int charValue = (((*buffer)[offset + i]) & 0xff);
            if (charValue == 0)
                  break;
            filename.push_back((char)charValue);
      }
      return filename;
}

//---------------------------------------------------------
//   readGPX
//---------------------------------------------------------

void GuitarPro6::readGPX(QByteArray* buffer) {
      // start by reading the file header. It will tell us if the byte array is compressed.
      int fileHeader = readInteger(buffer, 0);

      if (fileHeader == GPX_HEADER_COMPRESSED) {
            // this is  a compressed file.
            int length = readInteger(buffer, this->position/BITS_IN_BYTE);
            QByteArray* bcfsBuffer = new QByteArray();
            int positionCounter = 0;
            while(!f->error() && (this->position/this->BITS_IN_BYTE) < length) {
                  // read the bit indicating compression information
                  int flag = this->readBits(1);

                  if (flag) {
                        int bits = this->readBits(4);
                        int offs = this->readBitsReversed(bits);
                        int size = this->readBitsReversed(bits);

                        QByteArray bcfsBufferCopy = *bcfsBuffer;
                        int pos = (bcfsBufferCopy.length() - offs );
                        for( int i = 0; i < (size > offs ? offs : size) ; i ++ ) {
                              bcfsBuffer->insert(positionCounter, bcfsBufferCopy[pos + i] ) ;
                              positionCounter++;
                              }
                        }
                  else  {
                        int size = this->readBitsReversed(2);
                        for(int i = 0; i < size; i++) {
                              bcfsBuffer->insert(positionCounter, this->readBits(8));
                              positionCounter++;
                              }
                        }
                  }
             // recurse on the decompressed file stored as a byte array
             readGPX(bcfsBuffer);
             delete bcfsBuffer;
            }
      else if (fileHeader == GPX_HEADER_UNCOMPRESSED) {
            // this is an uncompressed file - strip the header off
            *buffer = buffer->right(buffer->length()-sizeof(int));
            int sectorSize = 0x1000;
            int offset = 0;
            while ((offset = (offset + sectorSize)) + 3 < buffer->length()) {
                  int newInt = readInteger(buffer,offset);
                  if (newInt == 2) {
                        int indexFileName = (offset + 4);
                        int indexFileSize = (offset + 0x8C);
                        int indexOfBlock = (offset + 0x94);

                        // create a byte array and put information about files found in it
                        int block = 0;
                        int blockCount = 0;
                        QByteArray* fileBytes = new QByteArray();
                        while((block = (readInteger(buffer, (indexOfBlock + (4 * (blockCount ++)))))) != 0 ) {
                              fileBytes->push_back(getBytes(buffer, (offset = (block*sectorSize)), sectorSize));
                              }
                        // get file information and read the file
                        int fileSize = readInteger(buffer, indexFileSize);
                        if (fileBytes->length() >= fileSize) {
                              QByteArray filenameBytes = readString(buffer, indexFileName, 127);
                              char* filename = filenameBytes.data();
                              QByteArray data = getBytes(fileBytes, 0, fileSize);
                              parseFile(filename, &data);
                              }
                        delete fileBytes;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   parseFile
//---------------------------------------------------------

void GuitarPro6::parseFile(char* filename, QByteArray* data)
      {
      // test to check if we are dealing with the score
      if (!strcmp(filename, "score.gpif")) {
            readGpif(data);
            }
      }


//---------------------------------------------------------
//   unhandledNode
//---------------------------------------------------------

void GuitarPro6::unhandledNode(QString nodeName)
      {
      qDebug() << "WARNING: Discovered unhandled node name" << nodeName;
      }

//---------------------------------------------------------
//   readScore
//---------------------------------------------------------

void GuitarPro6::readScore(QDomNode* scoreNode)
      {
      // loop through the score meta-info, grabbing title, artist, etc
      QDomNode currentNode = scoreNode->firstChild();
      while (!currentNode.isNull()) {
            QString nodeName = currentNode.nodeName();
            if (!nodeName.compare("Title"))
                  title = currentNode.toElement().text();
            if (!nodeName.compare("Copyright"))
                  score->setMetaTag("copyright", currentNode.toElement().text());
            else if (!nodeName.compare("Subtitle"))
                  subtitle = currentNode.toElement().text();
            else if (!nodeName.compare("Artist"))
                  artist = currentNode.toElement().text();
            else if (!nodeName.compare("Album"))
                  album = currentNode.toElement().text();
            else if (nodeName == "FirstPageHeader") {}
            else if (nodeName == "FirstPageFooter") {}
            else if (nodeName == "PageHeader") {}
            else if (nodeName == "PageFooter") {}
            else if (nodeName == "ScoreSystemsDefaultLayout") {}
            else if (nodeName == "ScoreSystemsLayout") {}
            currentNode = currentNode.nextSibling();
            }
      }

//---------------------------------------------------------
//   readMasterTracks
//---------------------------------------------------------

void GuitarPro6::readMasterTracks(QDomNode* masterTrack)
      {
      // inspects MasterTrack, gives information applying to start of score such as tempo
      QDomNode currentNode = masterTrack->firstChild();
      while (!currentNode.isNull()) {
            QString nodeName = currentNode.nodeName();
            if (!nodeName.compare("Automations")) {
                  QDomNode currentAutomation = currentNode.firstChild();
                  while (!currentAutomation.isNull()) {
                        if (!currentAutomation.nodeName().compare("Automation")) {
                              if (!currentAutomation.firstChild().nodeName().compare("Tempo"))
                                    tempo = currentAutomation.lastChild().toElement().text().toInt();
                              }
                        currentAutomation = currentAutomation.nextSibling();
                        }
                  }
            currentNode = currentNode.nextSibling();
            }
      }

//---------------------------------------------------------
//   readChord
//---------------------------------------------------------

void GuitarPro6::readChord(QDomNode* diagram, int track)
      {
      // initialise a new fret diagram for our current track
      FretDiagram* fretDiagram = new FretDiagram(score);
      fretDiagram->setTrack(track);

      // get the identifier to set as the domain in the map
      int id = diagram->attributes().namedItem("id").toAttr().value().toInt();
      QDomNode diagramNode = diagram->firstChild();

      // set the number of strings on this part
      int stringCount = diagramNode.attributes().namedItem("stringCount").toAttr().value().toInt();
      fretDiagram->setStrings(stringCount);

      // set the fret offset
      int baseFret = diagramNode.attributes().namedItem("baseFret").toAttr().value().toInt();
      fretDiagram->setOffset(baseFret);

      QDomNode diagramEntity = diagramNode.firstChild();
      int counter = 0;
      while (!diagramEntity.isNull()) {
            QString nodeName = diagramEntity.nodeName();
            // new fret
            if (!nodeName.compare("Fret")) {
                  // get the string and fret numbers from the arguments to the node as integers
                  int string = diagramEntity.attributes().namedItem("string").toAttr().value().toInt();
                  int fret = diagramEntity.attributes().namedItem("fret").toAttr().value().toInt();

                  // if there are unspecified string values, add the X marker to that string
                  while (counter < string) {
                        fretDiagram->setMarker(counter, 'X');
                        counter++;
                        }

                  // look at the specified string/fret and add to diagram
                  if (fret == 0) {
                        fretDiagram->setMarker(string, '0');
                        counter++;
                        }
                  else  {
                        qDebug("Setting fret (string=%d,fret=%d)", string, fret-baseFret);
                        fretDiagram->setDot(string, fret);
                        counter++;
                        }
                  }
            // move to the next string/fret specification
            diagramEntity = diagramEntity.nextSibling();
            }

      // mark any missing strings as 'X'
      while (counter < stringCount) {
            fretDiagram->setMarker(counter, 'X');
            counter++;
            }

      // insert the fret diagram into the map of diagrams
      fretDiagrams.insert(id, fretDiagram);
      }

//---------------------------------------------------------
//   readTracks
//---------------------------------------------------------

void GuitarPro6::readTracks(QDomNode* track)
      {
      QDomNode nextTrack = track->firstChild();
      int trackCounter = 0;
      while (!nextTrack.isNull()) {
            QDomNode currentNode = nextTrack.firstChild();
            Part* part = new Part(score);
            Staff* s = new Staff(score, part, trackCounter);
            while (!currentNode.isNull()) {
                  QString nodeName = currentNode.nodeName();
                  if (nodeName == "Name")
                        part->setLongName(currentNode.toElement().text());
                  else if (nodeName == "Color") {}
                  else if (nodeName == "SystemsLayout") {}
                  // this is a typo is guitar pro - 'defaut' is correct here
                  else if (nodeName == "SystemsDefautLayout") {}
                  else if (nodeName == "RSE") {}
                  else if (nodeName == "GeneralMidi") {}
                  else if (nodeName == "PlaybackState") {}
                  else if (nodeName == "PlayingStyle") {}
                  else if (nodeName == "PageSetup") {}
                  else if (nodeName == "MultiVoice") {}
                  else if (nodeName == "ShortName")
                        part->setPartName(currentNode.toElement().text());
                  else if (nodeName == "Instrument") {
                        QString ref = currentNode.attributes().namedItem("ref").toAttr().value();
                        Instrument* instr = new Instrument;
                        // use an array as a map instead?
                        if (!ref.compare("e-gtr6")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("electric-guitar"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("tnr-s")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("voice"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("s-gtr6")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("guitar-steel"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("snt-lead-ss")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("poly-synth"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("f-bass5")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("bass-guitar"));
                              part->setInstrument(*instr, 0);
                              }
                        else if (!ref.compare("snt-bass-ss")) {
                              *instr = instr->fromTemplate(Ms::searchTemplate("metallic-synth"));
                              part->setInstrument(*instr, 0);
                              }
                        //else if (!ref.compare("mrcs")) {
                              //*instr = instr->fromTemplate(Ms::searchTemplate("maracas"));
                              //part->setInstrument(*instr, 0);
                        //      }
                        // else if (!ref.compare("drmkt")) {
                        //       *instr = instr->fromTemplate(Ms::searchTemplate("drumset"));
                        //       part->setInstrument(*instr, 0);
                        //       }
                        else
                              qDebug() << "WARINNG: unhandled playback instrument" << ref << "- defaulting.";
                        }
                  else if (nodeName == "Properties") {
                        QDomNode currentProperty = currentNode.firstChild();
                        while (!currentProperty.isNull()) {
                              QString propertyName = currentProperty.attributes().namedItem("name").toAttr().value();
                              if (!propertyName.compare("Tuning")) {
                                    // set up the tuning for the part
                                    QString tuningString = currentProperty.firstChild().toElement().text();
                                    QStringList tuningStringList = tuningString.split(" ");
                                    int strings = 0;
                                    int tuning[tuningStringList.length()];
                                    int frets   = 21;
                                    for (auto iter = tuningStringList.begin(); iter != tuningStringList.end(); ++iter) {
                                          int currentString = (*iter).toInt();
                                          tuning[strings] = currentString;
                                          strings++;
                                          }
                                          StringData* stringData = new StringData(frets, strings, tuning);
                                          Instrument* instr = part->instr();
                                          instr->setStringData(*stringData);
                                    }
                              else if (!propertyName.compare("DiagramCollection")) {
                                    QDomNode items = currentProperty.firstChild();
                                    QDomNode currentItem = items.firstChild();
                                    while (!currentItem.isNull()) {
                                          readChord(&currentItem, trackCounter);
                                          currentItem = currentItem.nextSibling();
                                          }
                                    }
                              currentProperty = currentProperty.nextSibling();
                              }
                        }
                  currentNode = currentNode.nextSibling();
                  }

            // add in a new part
            part->insertStaff(s);
            score->staves().push_back(s);
            score->appendPart(part);
            trackCounter++;
            nextTrack = nextTrack.nextSibling();
            }

      previousDynamic = new int[score->staves().length() * VOICES];
      // initialise the dynamics to 0
      for (int i = 0; i < score->staves().length() * VOICES; i++)
           previousDynamic[i] = 0;
      // set the number of staves we need
      staves = score->staves().length();
      }

//---------------------------------------------------------
//   getNode
//---------------------------------------------------------

QDomNode GuitarPro6::getNode(QString id, QDomNode currentNode)
      {
      while (!(currentNode).isNull()) {
            QString currentId = currentNode.attributes().namedItem("id").toAttr().value();
            if (id.compare(currentId) == 0) {
                  return currentNode;
                  }
            currentNode = (currentNode).nextSibling();
            }
      qDebug() << "WARNING: A null node was returned when search for the identifier" << id << ". Your Guitar Pro file may be corrupted.";
      return currentNode;
      }

//---------------------------------------------------------
//   findNumMeasures
//---------------------------------------------------------

int GuitarPro6::findNumMeasures(GPPartInfo* partInfo)
      {
      QDomNode masterBar = partInfo->masterBars.nextSibling();
      QDomNode masterBarElement;
      while (!masterBar.isNull()) {
            GpBar gpBar;
            gpBar.freeTime = false;
            gpBar.direction = "";
            gpBar.directionStyle = "";
            masterBarElement = masterBar.firstChild();
            while (!masterBarElement.isNull()) {
                  if (!masterBarElement.nodeName().compare("Key"))
                        gpBar.keysig = masterBarElement.firstChild().toElement().text().toInt();
                  else if (!masterBarElement.nodeName().compare("Time")) {
                        QString timeSignature = masterBarElement.toElement().text();
                        QList<QString> timeSignatureList = timeSignature.split("/");
                        gpBar.timesig = Fraction(timeSignatureList.first().toInt(), timeSignatureList.last().toInt());
                        }
                  else if (!masterBarElement.nodeName().compare("Directions")) {
                        gpBar.direction = masterBarElement.firstChild().toElement().text();
                        gpBar.directionStyle = masterBarElement.firstChild().nodeName();
                        }
                  else if (!masterBarElement.nodeName().compare("FreeTime")) {
                        gpBar.freeTime = true;
                        gpBar.barLine = BarLineType::DOUBLE;
                        }
                  else if (!masterBarElement.nodeName().compare("DoubleBar"))
                        gpBar.barLine = BarLineType::DOUBLE;
                  masterBarElement = masterBarElement.nextSibling();
                  }
            bars.append(gpBar);
            if (masterBar.nextSibling().isNull())
                  break;
            masterBar = masterBar.nextSibling();
            }
      QString bars = masterBar.lastChildElement("Bars").toElement().text();
      //work out the number of measures (add 1 as couning from 0, and divide by number of parts)
      int numMeasures = (bars.split(" ").last().toInt() + 1) / score->parts().length();
      return numMeasures;
      }

//---------------------------------------------------------
//   fermataToFraction
//---------------------------------------------------------

Fraction GuitarPro6::fermataToFraction(int numerator, int denominator)
      {
      numerator++;
      // minim through to hemidemisemiquaver
      if (denominator == 0)       { denominator = 2; }
      else if (denominator == 1)  { denominator = 4; }
      else if (denominator == 2)  { denominator = 8;  }
      else if (denominator == 3)  { denominator = 12;  }
      else if (denominator == 4)  { denominator = 16;  }
      else if (denominator == 5)  { denominator = 32;  }

      return Fraction(numerator, denominator);
      }

//---------------------------------------------------------
//   rhythmToDuration
//---------------------------------------------------------

Fraction GuitarPro6::rhythmToDuration(QString value)
      {
      Fraction l;
      if (value.compare("Whole") == 0)
            l.set(1, 1);
      else if (value.compare("Half") == 0)
            l.set(1, 2);
      else if (value.compare("Quarter") == 0)
            l.set(1, 4);
      else if (value.compare("Eighth") == 0)
            l.set(1,8);
      else if (value.compare("16th") == 0)
            l.set(1,16);
      else if (value.compare("32nd") == 0)
            l.set(1,32);
      else if (value.compare("64th") == 0)
            l.set(1,64);
      else if (value.compare("128th") == 0)
            l.set(1,128);
      else
            qFatal( "Error - unknown note length: %s", qPrintable(value));
      return l;
      }

//---------------------------------------------------------
//   readDrumNote
//---------------------------------------------------------

void GuitarPro6::readDrumNote(Note* note, int element, int variation)
      {
      int octaveInt;
      int toneInt;
      /* These numbers below were determined by creating all drum
       * notes in a GPX format file and then analyzing the score.gpif
       * file which specifies the score. */
      if (element == 11 && variation == 0) {
            octaveInt = 5;
            toneInt = 0;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 0 && variation == 0) {
            octaveInt = 5;
            toneInt = 5;
      }
      else if (element == 5 && variation == 0) {
            octaveInt = 5;
            toneInt = 7;
      }
      else if (element == 6 && variation == 0) {
            octaveInt = 5;
            toneInt = 9;
      }
      else if (element == 7 && variation == 0) {
            octaveInt = 5;
            toneInt = 11;
      }
      else if (element == 1 && variation == 0) {
            octaveInt = 6;
            toneInt = 0;
      }
      else if (element == 1 && variation == 1) {
            octaveInt = 6;
            toneInt = 0;
            note->setHeadGroup(NoteHead::Group::HEAD_MI);
      }
      else if (element == 1 && variation == 2) {
            octaveInt = 6;
            toneInt = 0;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 8 && variation == 0) {
            octaveInt = 6;
            toneInt = 2;
      }
      else if (element == 9 && variation == 0) {
            octaveInt = 6;
            toneInt = 4;
      }
      else if (element == 2 && variation == 0) {
            octaveInt = 6;
            toneInt = 4;
            note->setHeadGroup(NoteHead::Group::HEAD_TRIANGLE);
      }
      else if (element == 15 && variation == 0) {
            octaveInt = 6;
            toneInt = 5;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 15 && variation == 1) {
            octaveInt = 6;
            toneInt = 5;
            note->setHeadGroup(NoteHead::Group::HEAD_DIAMOND);
      }
      else if (element == 15 && variation == 2) {
            octaveInt = 6;
            toneInt = 5;
            note->setHeadGroup(NoteHead::Group::HEAD_MI);
      }
      else if (element == 3 && variation == 0) {
            octaveInt = 6;
            toneInt = 5;
            note->setHeadGroup(NoteHead::Group::HEAD_TRIANGLE);
      }
      else if (element == 10 && variation == 0) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 10 && variation == 1) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_SLASH);
      }
      else if (element == 10 && variation == 2) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_XCIRCLE);
      }
      else if (element == 12 && variation == 0) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 4 && variation == 0) {
            octaveInt = 6;
            toneInt = 7;
            note->setHeadGroup(NoteHead::Group::HEAD_TRIANGLE);
      }
      else if (element == 14 && variation == 0) {
            octaveInt = 6;
            toneInt = 9;
            note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
      }
      else if (element == 13 && variation == 0) {
            octaveInt = 6;
            toneInt = 9;
            note->setHeadGroup(NoteHead::Group::HEAD_LA);
      }
      else if (element == 16 && variation == 0) {
            octaveInt = 6;
            toneInt = 11;
            note->setHeadGroup(NoteHead::Group::HEAD_NORMAL);
      }
      // multiply octaves by 12 as 12 semitones in octave
      note->setPitch((octaveInt * 12) + toneInt);
      }


void GuitarPro6::makeTie(Note* note) {
      bool found = false;
      Chord* chord     = note->chord();
      Segment* segment = chord->segment();
      segment = segment->prev1(Segment::Type::ChordRest);
      int track        = note->track();
      while (segment) {
            Element* e = segment->element(track);
            if (e) {
                  if (e->type() == Element::Type::CHORD) {
                        Chord* chord2 = static_cast<Chord*>(e);
                        foreach(Note* note2, chord2->notes()) {
                              if (note2->string() == note->string()) {
                                    Tie* tie = new Tie(score);
                                    tie->setEndNote(note);
                                    note2->add(tie);
                                    note->setFret(note2->fret());
                                    note->setPitch(note2->pitch());
                                    found = true;
                                    break;
                              }
                        }
                  }
                  if (found)
                        break;
            }
            segment = segment->prev1(Segment::Type::ChordRest);
      }
      if (!found)
            qDebug("tied note not found, pitch %d fret %d string %d", note->pitch(), note->fret(), note->string());
}

int GuitarPro6::readBeats(QString beats, GPPartInfo* partInfo, Measure* measure, int tick, int staffIdx, int voiceNum, Tuplet* tuplets[], int measureCounter)
      {
            // we must count from the start of the bar, so declare a fraction to track this
            Fraction fermataIndex(0,1);
            int track = staffIdx * VOICES + voiceNum;
            auto currentBeatList = beats.split(" ");
            for (auto currentBeat = currentBeatList.begin(); currentBeat != currentBeatList.end(); currentBeat++) {
                  int slide = -1;
                  if (slides->contains(staffIdx * VOICES + voiceNum))
                        slide = slides->take(staffIdx * VOICES + voiceNum);

                  Fraction l;
                  int dotted = 0;
                  // bool ottavaFound = false;
                  QDomNode beat = getNode(*currentBeat, partInfo->beats);

                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
                  QDomNode currentNode = beat.firstChild();
                  bool noteSpecified = false;
                  ChordRest* cr = segment->cr(track);
                  bool tupletSet = false;
                  Tuplet* tuplet = tuplets[staffIdx * 2 + voiceNum];
                  int whammyOrigin = -1;
                  int whammyMiddle = -1;
                  int whammyEnd = -1;
                  while (!currentNode.isNull()) {
                        if (currentNode.nodeName() == "Notes") {
                              noteSpecified = true;
                              auto notesList = currentNode.toElement().text().split(" ");

                              // this could be set by rhythm if we dealt with a tuplet
                              if (!cr)
                                    cr = new Chord(score);
                              cr->setTrack(track);
                              cr->setDuration(l);
                              TDuration d(l);
                              d.setDots(dotted);
                              cr->setDurationType(d);

                              if(!segment->cr(staffIdx * VOICES + voiceNum))
                                    segment->add(cr);


                              for (auto iter = notesList.begin(); iter != notesList.end(); ++iter) {
                                    // we have found a note
                                    QDomNode note = getNode(*iter, partInfo->notes);
                                    QDomNode currentNote = (note).firstChild();
                                    bool tie = false;
                                    bool trill = false;
                                    // if a <Notes> tag is used but there is no <Note>, then we add a rest. This flag will allow us to check this.
                                    while (!currentNote.isNull()) {
                                          if (currentNote.nodeName() == "Properties") {
                                                QDomNode currentProperty = currentNote.firstChild();
                                                // these should not be in this scope - they may not even exist.
                                                QString stringNum;
                                                QString fretNum;
                                                QString tone;
                                                QString octave;
                                                QString midi;
                                                QString element;
                                                QString variation;

                                                Note* note = new Note(score);
                                                Chord* chord = static_cast<Chord*>(cr);
                                                chord->add(note);

                                                QString harmonicText = "";
                                                bool hasSlur = false;
                                                while (!currentProperty.isNull()) {
                                                      QString argument = currentProperty.attributes().namedItem("name").toAttr().value();
                                                      if (argument == "String")
                                                            stringNum = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Element")
                                                            element = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Slide") {
                                                            int slideKind = currentProperty.firstChild().toElement().text().toInt();
                                                            if (slideKind >= 4)
                                                                  slide = slideKind;
                                                            else
                                                                  slides->insert(staffIdx * VOICES + voiceNum, slideKind);
                                                            }
                                                      else if (!argument.compare("HopoOrigin")) {
                                                            hasSlur = true;
                                                            createSlur(true, staffIdx, cr);
                                                            }
                                                      else if (!argument.compare("HopoDestination") && !hasSlur) {
                                                            createSlur(false, staffIdx, cr);
                                                            }
                                                      else if (argument == "Variation")
                                                            variation = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Fret")
                                                            fretNum = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Tone")
                                                            tone = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Octave")
                                                            octave = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Midi")
                                                            midi = currentProperty.firstChild().toElement().text();
                                                      else if (argument == "Muted") {
                                                            if (!currentProperty.firstChild().nodeName().compare("Enable")) {
                                                                  note->setHeadGroup(NoteHead::Group::HEAD_CROSS);
                                                                  note->setGhost(true);
                                                                  }
                                                            }
                                                      else if (argument == "PalmMuted") {
                                                            if (!currentProperty.firstChild().nodeName().compare("Enable"))
                                                                  addPalmMute(note);
                                                            }
                                                      else if (argument == "Tapped") {
                                                            if (!currentProperty.firstChild().nodeName().compare("Enable"))
                                                                  addTap(note);
                                                            }
                                                      else if (!argument.compare("HarmonicType")) {
                                                            QString type = currentProperty.toElement().text();
                                                            // add the same text to the note that Guitar Pro does
                                                            if (!type.compare("Feedback"))
                                                                  harmonicText = "Fdbk.";
                                                            else if (!type.compare("Semi"))
                                                                  harmonicText = "S.H.";
                                                            else if (!type.compare("Pinch"))
                                                                  harmonicText = "P.H.";
                                                            else if (!type.compare("Tap"))
                                                                  harmonicText = "T.H.";
                                                            else if (!type.compare("Artificial"))
                                                                  harmonicText = "A.H.";
                                                            else
                                                                  harmonicText = "Natural";
                                                            }
                                                      else if (!argument.compare("HarmonicFret")) {
                                                            QString value = currentProperty.toElement().text();
                                                            Note* harmonicNote;

                                                            // natural harmonic = artificial harmonic?
                                                            if (!harmonicText.compare("Natural"))
                                                                  harmonicNote = note;
                                                            else
                                                                  harmonicNote = new Note(score);
                                                            chord->add(harmonicNote);

                                                            Staff* staff = note->staff();
                                                            int harmonicFret = fretNum.toInt();
                                                            int musescoreString = staff->part()->instr()->stringData()->strings() - 1 - stringNum.toInt();
                                                            harmonicNote->setString(musescoreString);
                                                            harmonicNote->setFret(harmonicFret); // add the octave for the harmonic
                                                            harmonicNote->setHeadGroup(NoteHead::Group::HEAD_DIAMOND);
                                                            if (!value.compare("12"))
                                                                  harmonicFret += 12;
                                                            else if (!value.compare("7") || !value.compare("19"))
                                                                  harmonicFret += 19;
                                                            else if (!value.compare("5") || !value.compare("24"))
                                                                  harmonicFret += 24;
                                                            else if (!value.compare("3.9") || !value.compare("4") || !value.compare("9") || !value.compare("16"))
                                                                  harmonicFret += 28;
                                                            else if (!value.compare("3.2"))
                                                                  harmonicFret += 31;
                                                            else if (!value.compare("2.7"))
                                                                  harmonicFret += 34;
                                                            else if (!value.compare("2.3") || !value.compare("2.4"))
                                                                  harmonicFret += 36;
                                                            else if (!value.compare("2"))
                                                                  harmonicFret += 38;
                                                            else if (!value.compare("1.8"))
                                                                  harmonicFret += 40;
                                                            harmonicNote->setFret(harmonicFret);
                                                            harmonicNote->setPitch(staff->part()->instr()->stringData()->getPitch(musescoreString, harmonicFret));
                                                            harmonicNote->setTpcFromPitch();
                                                            if (harmonicText.compare("Natural")) {
                                                                  harmonicNote->setFret(fretNum.toInt());
                                                                  TextStyle textStyle;
                                                                  textStyle.setAlign(AlignmentFlags::CENTER);
                                                                  addTextToNote(harmonicText, textStyle, harmonicNote);
                                                                  }
                                                            }
                                                      else
                                                            qDebug() << "WARNING: Not handling node argument: " << argument << "in node" << currentNote.nodeName();
                                                      currentProperty = currentProperty.nextSibling();
                                                }

                                                if (midi != "")
                                                      note->setPitch(midi.toInt());
                                                else if (element != "")
                                                      readDrumNote(note, element.toInt(), variation.toInt());
                                                else if (stringNum != "" && note->headGroup() != NoteHead::Group::HEAD_DIAMOND) {
                                                      Staff* staff = note->staff();
                                                      int fretNumber = fretNum.toInt();
                                                      int musescoreString = staff->part()->instr()->stringData()->strings() - 1 - stringNum.toInt();
                                                      auto pitch = staff->part()->instr()->stringData()->getPitch(musescoreString, fretNumber);
                                                      note->setFret(fretNumber);
                                                      // we need to turn this string number for GP to the the correct string number for musescore
                                                      note->setString(musescoreString);
                                                      note->setPitch(pitch);
                                                      }
                                                else if (tone != "")
                                                      note->setPitch((octave.toInt() * 12) + tone.toInt()); // multiply octaves by 12 as 12 semitones in octave
                                                QDomNode graceNode = currentNode.parentNode().firstChildElement("GraceNotes");
                                                if (!graceNode.isNull()) {
                                                      if (!graceNode.toElement().text().compare("BeforeBeat")) {
                                                            chord->setDurationType(MScore::division/2);
                                                            chord->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));
                                                            chord->setNoteType(NoteType::ACCIACCATURA);
                                                           }
                                                      if (!graceNode.toElement().text().compare("OnBeat")) {
                                                            chord->setDurationType(MScore::division/2);
                                                            chord->setMag(note->chord()->staff()->mag() * score->styleD(StyleIdx::graceNoteMag));
                                                            chord->setNoteType(NoteType::GRACE4);
                                                           }
                                                      }
                                                if (tie) {
                                                      makeTie(note);
                                                      tie = false;
                                                      }
                                                if (trill) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setArticulationType(ArticulationType::Trill);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                QDomNode tremoloNode = currentNode.parentNode().firstChildElement("Tremolo");
                                                if (!tremoloNode.isNull()) {
                                                      QString value = tremoloNode.toElement().text();
                                                      Tremolo* t = new Tremolo(chord->score());
                                                      if (!value.compare("1/2")) {
                                                            t->setTremoloType(TremoloType::R8);
                                                            chord->add(t);
                                                            }
                                                      else if (!value.compare("1/4")) {
                                                            t->setTremoloType(TremoloType::R16);
                                                            chord->add(t);
                                                            }
                                                      else if (!value.compare("1/8")) {
                                                            t->setTremoloType(TremoloType::R32);
                                                            chord->add(t);
                                                            }
                                                      }
                                                QDomNode wahNode = currentNode.parentNode().firstChildElement("Wah");
                                                if (!wahNode.isNull()) {
                                                      QString value = wahNode.toElement().text();
                                                      if (!value.compare("Open")) {
                                                            Articulation* art = new Articulation(note->score());
                                                            art->setArticulationType(ArticulationType::Ouvert);
                                                            if (!note->score()->addArticulation(note, art))
                                                                  delete art;
                                                            }
                                                      else if (!value.compare("Closed")) {
                                                            Articulation* art = new Articulation(note->score());
                                                            art->setArticulationType(ArticulationType::Plusstop);
                                                            if (!note->score()->addArticulation(note, art))
                                                                  delete art;
                                                            }
                                                      }
                                                QDomNode accentNode = currentNote.parentNode().firstChildElement("Accent");
                                                if (!accentNode.isNull()) {
                                                      int value = accentNode.toElement().text().toInt();
                                                      if (value == 4) {
                                                            Articulation* art = new Articulation(note->score());
                                                            art->setArticulationType(ArticulationType::Marcato);
                                                            if (!note->score()->addArticulation(note, art))
                                                                  delete art;
                                                            }
                                                      else if (value == 8) {
                                                            Articulation* art = new Articulation(note->score());
                                                            art->setArticulationType(ArticulationType::Sforzatoaccent);
                                                            if (!note->score()->addArticulation(note, art))
                                                                  delete art;
                                                            }
                                                }
                                                QDomNode ornamentNode = currentNote.parentNode().firstChildElement("Ornament");
                                                if (!ornamentNode.isNull()) {
                                                      QString value = ornamentNode.toElement().text();
                                                      // guitar pro represents the turns the other way to what we do
                                                      if (!value.compare("InvertedTurn")) {
                                                            Articulation* art = new Articulation(note->score());
                                                            art->setArticulationType(ArticulationType::Turn);
                                                            if (!note->score()->addArticulation(note, art))
                                                                  delete art;
                                                            }
                                                      else if (!value.compare("Turn")) {
                                                            Articulation* art = new Articulation(note->score());
                                                            art->setArticulationType(ArticulationType::Reverseturn);
                                                            if (!note->score()->addArticulation(note, art))
                                                                  delete art;
                                                            }
                                                      else if (!value.compare("LowerMordent")) {
                                                            Articulation* art = new Articulation(note->score());
                                                            art->setArticulationType(ArticulationType::Mordent);
                                                            if (!note->score()->addArticulation(note, art))
                                                                  delete art;
                                                            }
                                                      else if (!value.compare("UpperMordent")) {
                                                            Articulation* art = new Articulation(note->score());
                                                            art->setArticulationType(ArticulationType::Prall);
                                                            if (!note->score()->addArticulation(note, art))
                                                                  delete art;
                                                            }
                                                      }
                                                QDomNode propertiesNode = currentNode.parentNode().firstChildElement("Properties");
                                                if (!propertiesNode.isNull()) {
                                                      QDomNode currentProperty = propertiesNode.firstChild();
                                                      QString barreFret = "";
                                                      bool halfBarre = false;
                                                      while (!currentProperty.isNull()) {
                                                            QString argument = currentProperty.attributes().namedItem("name").toAttr().value();
                                                            if (!argument.compare("PickStroke")) {
                                                                  if (!currentProperty.firstChild().toElement().text().compare("Up")) {
                                                                        Articulation* art = new Articulation(note->score());
                                                                        art->setArticulationType(ArticulationType::Upbow);
                                                                        if (!note->score()->addArticulation(note, art))
                                                                              delete art;
                                                                        }
                                                                  else if (!currentProperty.firstChild().toElement().text().compare("Down")) {
                                                                        Articulation* art = new Articulation(note->score());
                                                                        art->setArticulationType(ArticulationType::Downbow);
                                                                        if (!note->score()->addArticulation(note, art))
                                                                              delete art;
                                                                        }
                                                                  }
                                                            else if (!argument.compare("Brush")) {
                                                                  Arpeggio* a = new Arpeggio(score);
                                                                  // directions in arpeggion type are reversed, they are correct below
                                                                  if (!currentProperty.firstChild().toElement().text().compare("Up"))
                                                                        a->setArpeggioType(ArpeggioType::DOWN_STRAIGHT);
                                                                  else if (!currentProperty.firstChild().toElement().text().compare("Down"))
                                                                        a->setArpeggioType(ArpeggioType::UP_STRAIGHT);
                                                                  chord->add(a);
                                                                  }
                                                            else if (!argument.compare("Slapped")) {
                                                                  if (!currentProperty.firstChild().nodeName().compare("Enable"))
                                                                        addSlap(note);
                                                                  }
                                                            else if (!argument.compare("Popped")) {
                                                                  if (!currentProperty.firstChild().nodeName().compare("Enable"))
                                                                        addPop(note);
                                                                  }
                                                            else if (!argument.compare("VibratoWTremBar")) {
                                                                  Articulation* art = new Articulation(note->score());
                                                                  if (!currentProperty.firstChild().toElement().text().compare("Slight"))
                                                                        art->setArticulationType(ArticulationType::WiggleSawtooth);
                                                                  else
                                                                        art->setArticulationType(ArticulationType::WiggleSawtoothWide);
                                                                  if (!note->score()->addArticulation(note, art))
                                                                        delete art;

                                                                  }
                                                            else if (!argument.compare("BarreFret")) {
                                                                  // target can be anywhere from 1 to 36
                                                                  int target = currentProperty.firstChild().toElement().text().toInt();
                                                                  for (int i = 0; i < (target/10); i++)
                                                                        barreFret += "X";
                                                                  int targetMod10 = target % 10;
                                                                  if (targetMod10 == 9)
                                                                        barreFret += "IX";
                                                                  else if (targetMod10 == 4)
                                                                        barreFret += "IV";
                                                                  else  {
                                                                        if (targetMod10 >= 5) {
                                                                              barreFret += "V";
                                                                              targetMod10-=5;
                                                                              }
                                                                        for (int j = 0; j < targetMod10; j++)
                                                                              barreFret += "I";
                                                                        }
                                                                  }
                                                            else if (!argument.compare("BarreString"))
                                                                  halfBarre = true;
                                                            else if (!argument.compare("WhammyBarOriginValue"))
                                                                  whammyOrigin = currentProperty.firstChild().toElement().text().toInt();
                                                            else if (!argument.compare("WhammyBarMiddleValue"))
                                                                  whammyMiddle = currentProperty.firstChild().toElement().text().toInt();
                                                            else if (!argument.compare("WhammyBarDestinationValue"))
                                                                  whammyEnd = currentProperty.firstChild().toElement().text().toInt();

                                                            currentProperty = currentProperty.nextSibling();
                                                            }

                                                      if (whammyOrigin != -1) {
                                                            // a whammy bar has been detected
                                                            addTremoloBar(segment, track, whammyOrigin, whammyMiddle, whammyEnd);
                                                            }

                                                      // if a barre fret has been specified
                                                      if (barreFret.compare("")) {
                                                            TextStyle textStyle;
                                                            textStyle.setAlign(AlignmentFlags::CENTER);
                                                            if (halfBarre)
                                                                  addTextToNote("1/2B " + barreFret, textStyle, note);
                                                            else
                                                                  addTextToNote("B " + barreFret, textStyle, note);
                                                            }
                                                      }
                                                QDomNode dynamicsNode = currentNode.parentNode().firstChildElement("Dynamic");
                                                if (!dynamicsNode.isNull()) {
                                                      QString dynamicStr = dynamicsNode.toElement().text();
                                                      int dynamic = 0;
                                                      if (!dynamicStr.compare("PPP"))
                                                            dynamic = 1;
                                                      else if (!dynamicStr.compare("PP"))
                                                            dynamic = 2;
                                                      else if (!dynamicStr.compare("P"))
                                                            dynamic = 3;
                                                      else if (!dynamicStr.compare("MP"))
                                                            dynamic = 4;
                                                      else if (!dynamicStr.compare("MF"))
                                                            dynamic = 5;
                                                      else if (!dynamicStr.compare("F"))
                                                            dynamic = 6;
                                                      else if (!dynamicStr.compare("FF"))
                                                            dynamic = 7;
                                                      else if (!dynamicStr.compare("FFF"))
                                                            dynamic = 8;
                                                      if (previousDynamic[track] != dynamic) {
                                                            previousDynamic[track] = dynamic;
                                                            addDynamic(note, dynamic);
                                                            }
                                                      }
                                                /* while left and right fingering nodes have distinct values, they are represented
                                                 * the same way w.r.t. identifying digit/char in the score file. */
                                                QDomNode leftFingeringNode = currentNote.parentNode().firstChildElement("LeftFingering");
                                                QDomNode rightFingeringNode = currentNote.parentNode().firstChildElement("RightFingering");
                                                if (!leftFingeringNode.isNull() || !rightFingeringNode.isNull()) {
                                                      QDomNode fingeringNode = leftFingeringNode.isNull() ? rightFingeringNode : leftFingeringNode;
                                                      QString finger = fingeringNode.toElement().text();
                                                      Text* f = new Fingering(score);
                                                      if (!leftFingeringNode.isNull()) {
                                                            if (!finger.compare("Open"))
                                                                  finger = "O";
                                                            else if (!finger.compare("P"))
                                                                  finger = "t";
                                                            else if (!finger.compare("I"))
                                                                  finger = "1";
                                                            else if (!finger.compare("M"))
                                                                  finger = "2";
                                                            else if (!finger.compare("A"))
                                                                  finger = "3";
                                                            else if (!finger.compare("C"))
                                                                  finger = "4";
                                                            }
                                                      f->setText(finger);
                                                      note->add(f);
                                                      }
                                                QDomNode arpeggioNode = currentNode.parentNode().firstChildElement("Arpeggio");
                                                if (!arpeggioNode.isNull()) {
                                                      QString arpeggioStr = arpeggioNode.toElement().text();
                                                      Arpeggio* a = new Arpeggio(score);
                                                      if (!arpeggioStr.compare("Up"))
                                                            a->setArpeggioType(ArpeggioType::UP);
                                                      else
                                                            a->setArpeggioType(ArpeggioType::DOWN);
                                                      chord->add(a);
                                                      }
                                                QDomNode letRingNode = currentNote.parentNode().firstChildElement("LetRing");
                                                if (!letRingNode.isNull())
                                                      addLetRing(note);
                                                QDomNode timerNode = currentNode.parentNode().firstChildElement("Timer");
                                                if (!timerNode.isNull()) {
                                                      int time = timerNode.toElement().text().toInt();
                                                      TextStyle textStyle;
                                                      textStyle.setAlign(AlignmentFlags::CENTER);
                                                      int minutes = time/60;
                                                      int seconds = time % 60;
                                                      addTextToNote(QString::number(minutes) + ":" + (seconds < 10 ? "0" + QString::number(seconds) : QString::number(seconds)), textStyle, note);
                                                      }
                                                QDomNode textNode = currentNode.parentNode().firstChildElement("FreeText");
                                                if (!textNode.isNull()) {
                                                      TextStyle textStyle;
                                                      textStyle.setAlign(AlignmentFlags::CENTER);
                                                      addTextToNote(textNode.toElement().text(), textStyle, note);
                                                      }
                                                QDomNode ghostNode = currentNote.parentNode().firstChildElement("AntiAccent");
                                                if (!ghostNode.isNull()) {
                                                      Symbol* leftSym = new Symbol(note->score());
                                                      Symbol* rightSym = new Symbol(note->score());
                                                      leftSym->setSym(SymId::noteheadParenthesisLeft);
                                                      rightSym->setSym(SymId::noteheadParenthesisRight);
                                                      leftSym->setParent(note);
                                                      rightSym->setParent(note);
                                                      note->add(leftSym);
                                                      note->add(rightSym);
                                                      }
                                                QDomNode swellNode = currentNode.parentNode().firstChildElement("Fadding");
                                                if (!swellNode.isNull()) {
                                                      Articulation* art = new Articulation(note->score());
                                                      art->setArticulationType(ArticulationType::VolumeSwell);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }
                                                QDomNode noteVibrato = currentNote.parentNode().firstChildElement("Vibrato");
                                                if (!noteVibrato.isNull()) {
                                                      Articulation* art = new Articulation(note->score());
                                                      if (!noteVibrato.toElement().text().compare("Slight"))
                                                            art->setArticulationType(ArticulationType::WiggleVibratoLargeFaster);
                                                      else
                                                            art->setArticulationType(ArticulationType::WiggleVibratoLargeSlowest);
                                                      if (!note->score()->addArticulation(note, art))
                                                            delete art;
                                                      }


                                                createSlide(slide, cr, staffIdx);
                                                note->setTpcFromPitch();

                                                // if (ottava[track]) {
                                                //       int pitch = note->pitch();
                                                //       Ottava::Type type = ottava[track]->ottavaType();
                                                //       if (type == Ottava::Type::OTTAVA_8VA)
                                                //             note->setPitch(pitch-12);
                                                //       else if (type == Ottava::Type::OTTAVA_8VB)
                                                //             note->setPitch(pitch+12);
                                                //       else if (type == Ottava::Type::OTTAVA_15MA)
                                                //             note->setPitch(pitch-24);
                                                //       else if (type == Ottava::Type::OTTAVA_15MB)
                                                //             note->setPitch(pitch+24);
                                                //       }

                                                currentNote = currentNote.nextSibling();
                                                }
                                          else if (!currentNote.nodeName().compare("Trill")) {
                                                trill = true;
                                                }
                                          else if (!currentNote.nodeName().compare("Tie")) {
                                                if (!currentNote.attributes().namedItem("destination").toAttr().value().compare("true"))
                                                      tie = true;
                                                }
                                          currentNote = currentNote.nextSibling();
                                    }
                              }
                        }
                        else if (currentNode.nodeName() == "Dynamic") {
                              }
                        else if (!currentNode.nodeName().compare("Chord")) {
                              int key = currentNode.toElement().text().toInt();
                              segment->add(fretDiagrams[key]);
                              }
                        else if (currentNode.nodeName() == "Rhythm") {
                              // we have found a rhythm
                              QString refString = currentNode.attributes().namedItem("ref").toAttr().value();
                              QDomNode rhythm = getNode(refString, partInfo->rhythms);
                              QDomNode currentNode = (rhythm).firstChild();
                              while (!currentNode.isNull()) {
                                    if (currentNode.nodeName() == "NoteValue") {
                                          l = rhythmToDuration(currentNode.toElement().text());
                                    }
                                    else if (currentNode.nodeName() == "AugmentationDot") {
                                          dotted = currentNode.attributes().namedItem("count").toAttr().value().toInt();
                                          for (int count = 1; count <= dotted; count++)
                                                l = l + (l / pow(2, count));
                                    }
                                    else if (currentNode.nodeName() == "PrimaryTuplet") {
                                          tupletSet = true;
                                          cr = new Chord(score);
                                          cr->setTrack(track);
                                          if ((tuplet == 0) || (tuplet->elementsDuration() == tuplet->baseLen().fraction() * tuplet->ratio().numerator())) {
                                                tuplet = new Tuplet(score);
                                                tuplets[staffIdx * 2 + voiceNum] = tuplet;
                                                tuplet->setParent(measure);
                                          }
                                          tuplet->setTrack(cr->track());
                                          tuplet->setBaseLen(l);
                                          tuplet->setRatio(Fraction(currentNode.attributes().namedItem("num").toAttr().value().toInt(),currentNode.attributes().namedItem("den").toAttr().value().toInt()));
                                          tuplet->add(cr);
                                    }
                                    else
                                          qDebug() << "WARNING: Not handling node: " << currentNode.nodeName();
                                    currentNode = currentNode.nextSibling();
                              }
                              fermataIndex += l;
                        }
                        else if (currentNode.nodeName() == "Hairpin") {
                              Segment* seg = segment->prev1(Segment::Type::ChordRest);
                              bool isCrec = !currentNode.toElement().text().compare("Crescendo");
                              if (seg && hairpins[staffIdx]) {
                                    if (hairpins[staffIdx]->tick2() == seg->tick())
                                          hairpins[staffIdx]->setTick2(tick);
                                    else
                                          createCrecDim(staffIdx, track, tick, isCrec);
                                    }
                              else
                                    createCrecDim(staffIdx, track, tick, isCrec);
                              }
                        else if (!currentNode.nodeName().compare("Properties")) {
                              QDomNode currentProperty = currentNode.firstChild();
                              while (!currentProperty.isNull()) {
                                    QString argument = currentProperty.attributes().namedItem("name").toAttr().value();
                                    if (!argument.compare("Rasgueado")) {
                                          StaffText* st = new StaffText(score);
                                          st->setTextStyleType(TextStyleType::STAFF);
                                          st->setText("rasg.");
                                          st->setParent(segment);
                                          st->setTrack(track);
                                          score->addElement(st);
                                          }
                                    currentProperty = currentProperty.nextSibling();
                                    }
                              }
                        // else if (!currentNode.nodeName().compare("Ottavia")) {
                        //       QString value = currentNode.toElement().text();
                        //       ottavaFound = true;
                        //       int track = track;
                        //       if (ottava[track] == 0) {
                        //             ottava[track] = new Ottava(score);
                        //             ottava[track]->setTrack(track);
                        //             if (!value.compare("8va"))
                        //                   ottava[track]->setOttavaType(Ottava::Type::OTTAVA_8VA);
                        //             else if (!value.compare("8vb"))
                        //                   ottava[track]->setOttavaType(Ottava::Type::OTTAVA_8VB);
                        //             else if (!value.compare("15ma"))
                        //                   ottava[track]->setOttavaType(Ottava::Type::OTTAVA_15MA);
                        //             else if (!value.compare("15mb"))
                        //                   ottava[track]->setOttavaType(Ottava::Type::OTTAVA_15MB);
                        //             ottava[track]->setTick(tick);
                        //             ottava[track]->setTick2(tick);
                        //             }
                        //       else  {
                        //             ottava[track]->setTick2(tick);
                        //             ottava[track]->setProperty(P_ID::LINE_WIDTH,0.1);
                        //             //ottava[track]->staff()->updateOttava(ottava[track]);
                        //             score->addElement(ottava[track]);
                        //             }
                        //       }
                        currentNode = currentNode.nextSibling();
                        dotted = 0;
                  }

                  // we have handled the beat - was there a note?
                  if (!noteSpecified) {
                        // add a rest with length of l
                        cr = new Rest(score);
                        cr->setTrack(track);
                        if (tupletSet)
                              tuplet->add(cr);
                        TDuration d(l);
                        cr->setDuration(l);
                        cr->setDurationType(d);
                        if(!segment->cr(track))
                              segment->add(cr);
                        // if (ottava[track]) {
                        //       ottava[track]->setTick2(tick);
                        //       ottava[track]->setProperty(P_ID::LINE_WIDTH,0.1);
                        //       //ottava[track]->staff()->updateOttava(ottava[track]);
                        //       score->addElement(ottava[track]);
                        //       ottava[track] = 0;
                        //       }
                  }
                  auto fermataList = fermatas.find(measureCounter);
                  if (fermataList != fermatas.end()) {
                        // iterator is a list of GPFermata values
                        for (auto fermataIter = (*fermataList)->begin(); fermataIter != (*fermataList)->end(); fermataIter++) {
                              Fraction targetIndex = fermataToFraction((*fermataIter).index, ((*fermataIter).timeDivision));
                              if (fermataIndex == targetIndex) {
                                    Articulation* art = new Articulation(score);
                                    art->setArticulationType(ArticulationType::Fermata);
                                    art->setUp(true);
                                    art->setAnchor(ArticulationAnchor::TOP_STAFF);
                                    cr->add(art);
                                    }
                              }
                        }
                  // if (!ottavaFound)
                  //       ottava[track] = 0;
                  tick += cr->actualTicks();
            }
            return tick;
      }

//---------------------------------------------------------
//   readBars
//---------------------------------------------------------

void GuitarPro6::readBars(QDomNode* barList, Measure* measure, ClefType oldClefId[], GPPartInfo* partInfo, KeySig* /*t*/, int measureCounter)
      {
      // unique bar identifiers are represented as a space separated string of numbers
      QStringList barsString = barList->toElement().text().split(" ");
      int staffIdx = 0;

      // used to keep track of tuplets
      Tuplet* tuplets[staves * 2];
      for (int track = 0; track < staves*2; ++track)
            tuplets[track] = 0;

      // iterate through all the bars that have been specified
      for (auto iter = barsString.begin(); iter != barsString.end(); ++iter) {
            int tick = measure->tick();

            QDomNode barNode = getNode(*iter, partInfo->bars);
            QDomNode currentNode = (barNode).firstChild();
            QDomNode voice;
            while (!currentNode.isNull()) {
                  voice.clear();
                  // get the clef of the bar and apply
                  if (!currentNode.nodeName().compare("Clef")) {
                        QString clefString = currentNode.toElement().text();
                        ClefType clefId;
                        if (!clefString.compare("F4"))
                              clefId = ClefType::F8;
                        else if (!clefString.compare("G2"))
                              clefId = ClefType::G3;
                        else if (!clefString.compare("Neutral"))
                              clefId = ClefType::PERC;
                        else
                              qDebug() << "WARNING: unhandled clef type: " << clefString;
                        Clef* newClef = new Clef(score);
                        newClef->setClefType(clefId);
                        newClef->setTrack(staffIdx * VOICES);
                        Segment* segment = measure->getSegment(Segment::Type::Clef, 0);
                        // only add the clef to the bar if it differs from previous measure
                        if (measure->prevMeasure()) {
                              if (clefId != oldClefId[staffIdx]) {
                                    segment->add(newClef);
                                    oldClefId[staffIdx] = clefId;
                                    }
                              }
                        else  {
                              segment->add(newClef);
                              oldClefId[staffIdx] = clefId;
                              }
                        }
                  // a repeated bar (simile marking)
                  else if (!currentNode.nodeName().compare("SimileMark")) {
                        if (!currentNode.toElement().text().compare("Simple") ||
                            !currentNode.toElement().text().compare("FirstOfDouble") ||
                            !currentNode.toElement().text().compare("SecondOfDouble"))
                              measure->cmdInsertRepeatMeasure(staffIdx);
                        else
                              qDebug() << "WARNING: unhandle similie mark type: " << currentNode.toElement().text();
                        }
                  // new voice specification
                  else if (!currentNode.nodeName().compare("Voices")) {
                        QString voicesString = currentNode.toElement().text();
                        auto currentVoice = voicesString.split(" ").first();
                        // if the voice is not -1 then we set voice
                        if (currentVoice.compare("-1"))
                              voice = getNode(currentVoice, partInfo->voices);
                        }
                  else if (!currentNode.nodeName().compare("XProperties")) {}
                  // go to the next node in the tree
                  currentNode = currentNode.nextSibling();
                  }

            int voiceNum = 0;
            if (voice.isNull()) {
                  Fraction l = Fraction(1,1);
                  // add a rest with length of l
                  ChordRest* cr = new Rest(score);
                  cr->setTrack(staffIdx * VOICES + voiceNum);
                  TDuration d(l);
                  cr->setDuration(l);
                  cr->setDurationType(d);
                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, tick);
                  if(!segment->cr(staffIdx * VOICES + voiceNum))
                        segment->add(cr);
                  tick += cr->actualTicks();
                  staffIdx++;
                  continue;
                  }

            // read the beats that occur in the bar
            tick = readBeats(voice.firstChild().toElement().text(), partInfo, measure, tick, staffIdx, voiceNum, tuplets, measureCounter);
            // increment the counter for parts
            staffIdx++;
            }
      }


//---------------------------------------------------------
//   checkForHeld
//---------------------------------------------------------

bool checkForHold(Segment* segment, QList<PitchValue> points)
      {
      bool same = false;
      Segment* prevSeg = segment->prev1(Segment::Type::ChordRest);
      if (!prevSeg)
            return false;
      foreach (Element* e, prevSeg->annotations()) {
            if (e->type() == Element::Type::TREMOLOBAR) {
                  QList<PitchValue> prevPoints = ((TremoloBar*)e)->points();
                  if (prevPoints.length() != points.length())
                        break;

                  auto iter = points.begin();
                  for (auto prevIter = prevPoints.begin(); prevIter != prevPoints.end(); ++prevIter) {
                        if (*prevIter == *iter)
                              same = true;
                        else  {
                              same = false;
                              break;
                              }
                        ++iter;
                        }
                  }
            }
      return same;
      }


//---------------------------------------------------------
//   addTremoloBar
//---------------------------------------------------------

void GuitarPro6::addTremoloBar(Segment* segment, int track, int whammyOrigin, int whammyMiddle, int whammyEnd)
      {
      if ((whammyOrigin == whammyEnd) && (whammyOrigin != whammyMiddle) && whammyMiddle != -1) {
            /* we are dealing with a dip. We need the chek for whammy middle
             * to be set as a predive has the same characteristics. */
            QList<PitchValue> points;
            points.append(PitchValue(0, whammyOrigin, false));
            points.append(PitchValue(50, whammyMiddle, false));
            points.append(PitchValue(100, whammyEnd, false));
            TremoloBar* b = new TremoloBar(score);
            b->setPoints(points);
            b->setTrack(track);
            segment->add(b);
            }
      else if (whammyOrigin == 0) {
            // we're dealing with a dive that does not continue from a previous marking
            QList<PitchValue> points;
            points.append(PitchValue(0, whammyOrigin, false));
            points.append(PitchValue(50, whammyMiddle, false));
            points.append(PitchValue(100, whammyEnd, false));
            TremoloBar* b = new TremoloBar(score);
            b->setPoints(points);
            b->setTrack(track);
            segment->add(b);
            }
      else if (whammyOrigin == whammyEnd && whammyMiddle == -1) {
            // this deals with a pre-dive
            QList<PitchValue> points;
            points.append(PitchValue(0, 0, false));
            points.append(PitchValue(50, whammyOrigin, false));
            points.append(PitchValue(100, whammyEnd, false));
            TremoloBar* b = new TremoloBar(score);
            b->setPoints(points);
            b->setTrack(track);
            segment->add(b);
            }
      else if (whammyMiddle != -1) {
            // dive starting from pre-existing point
            Segment* prevSeg = segment->prev1(Segment::Type::ChordRest);
            if (!prevSeg)
                  return;
            foreach (Element* e, prevSeg->annotations()) {
                  if (e->type() == Element::Type::TREMOLOBAR) {
                        QList<PitchValue> prevPoints = ((TremoloBar*)e)->points();
                        QList<PitchValue> points;
                        points.append(PitchValue(0, prevPoints[prevPoints.length()-1].pitch, false));
                        points.append(PitchValue(50, whammyOrigin, false));
                        points.append(PitchValue(100, whammyEnd, false));
                        TremoloBar* b = new TremoloBar(score);
                        b->setPoints(points);
                        b->setTrack(track);
                        segment->add(b);
                        }
                  }
            }
      else  {
            // a predive/dive
            QList<PitchValue> points;
            points.append(PitchValue(0, 0, false));
            points.append(PitchValue(50, whammyOrigin, false));
            points.append(PitchValue(100, whammyEnd, false));

            if (checkForHold(segment, points)) {
                  points.clear();
                  points.append(PitchValue(0, whammyEnd, false));
                  points.append(PitchValue(100, whammyEnd, false));
                  }

            TremoloBar* b = new TremoloBar(score);
            b->setPoints(points);
            b->setTrack(track);
            segment->add(b);
            }
      }

//---------------------------------------------------------
//   readMasterBars
//---------------------------------------------------------

void GuitarPro6::readMasterBars(GPPartInfo* partInfo)
      {
      Measure* measure = score->firstMeasure();
      int bar = 0;
      QDomNode nextMasterBar = partInfo->masterBars;
      nextMasterBar = nextMasterBar.nextSibling();
      int measureCounter = 0;
      ClefType oldClefId[staves];
      hairpins = new Hairpin*[staves];
      for (int i = 0; i < staves; i++)
            hairpins[i] = 0;
      do    {
            const GpBar& gpbar = bars[bar];

            if (!gpbar.marker.isEmpty()) {
                  Text* s = new RehearsalMark(score);
                  s->setText(gpbar.marker.trimmed());
                  s->setTrack(0);
                  Segment* segment = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                  segment->add(s);
                  }

            QDomNode masterBarElement = nextMasterBar.firstChild();
            while (!masterBarElement.isNull()) {
                  if (bars[measureCounter].freeTime) {
                        TimeSig* ts = new TimeSig(score);
                        ts->setSig(bars[measureCounter].timesig);
                        ts->setTrack(0);
                        Measure* m = score->getCreateMeasure(measure->tick());
                        Segment* s = m->getSegment(Segment::Type::TimeSig, measure->tick());
                        ts->setFreeTime(true);
                        s->add(ts);
                        StaffText* st = new StaffText(score);
                        st->setTextStyleType(TextStyleType::STAFF);
                        st->setText("Free time");
                        st->setParent(s);
                        st->setTrack(0);
                        score->addElement(st);
                        }
                  else if (measureCounter > 0 && bars[measureCounter - 1].freeTime) {
                        TimeSig* ts = new TimeSig(score);
                        ts->setSig(bars[measureCounter].timesig);
                        ts->setTrack(0);
                        Measure* m = score->getCreateMeasure(measure->tick());
                        Segment* s = m->getSegment(Segment::Type::TimeSig, measure->tick());
                        ts->setFreeTime(false);
                        s->add(ts);
                        }
                  else
                        measure->setTimesig(bars[measureCounter].timesig);
                  measure->setLen(bars[measureCounter].timesig);

                  if (!bars[measureCounter].direction.compare("Fine") || (bars[measureCounter].direction.compare("") && !bars[measureCounter].directionStyle.compare("Jump"))) {
                        Segment* s = measure->getSegment(Segment::Type::KeySig, measure->tick());
                        StaffText* st = new StaffText(score);
                        st->setTextStyleType(TextStyleType::STAFF);
                        if (!bars[measureCounter].direction.compare("Fine"))
                              st->setText("fine");
                        else if (!bars[measureCounter].direction.compare("DaCapo"))
                              st->setText("Da Capo");
                        else if (!bars[measureCounter].direction.compare("DaCapoAlCoda"))
                              st->setText("D.C. al Coda");
                        else if (!bars[measureCounter].direction.compare("DaCapoAlDoubleCoda"))
                              st->setText("D.C. al Double Coda");
                        else if (!bars[measureCounter].direction.compare("DaCapoAlFine"))
                              st->setText("D.C. al Fine");
                        else if (!bars[measureCounter].direction.compare("DaSegnoAlCoda"))
                              st->setText("D.S. al Coda");
                        else if (!bars[measureCounter].direction.compare("DaSegnoAlDoubleCoda"))
                              st->setText("D.S. al Double Coda");
                        else if (!bars[measureCounter].direction.compare("DaSegnoAlFine"))
                              st->setText("D.S. al Fine");
                        else if (!bars[measureCounter].direction.compare("DaSegnoSegno"))
                              st->setText("Da Segno Segno");
                        else if (!bars[measureCounter].direction.compare("DaSegnoSegnoAlCoda"))
                              st->setText("D.S.S. al Coda");
                        else if (!bars[measureCounter].direction.compare("DaSegnoSegnoAlDoubleCoda"))
                              st->setText("D.S.S. al Double Coda");
                        else if (!bars[measureCounter].direction.compare("DaSegnoSegnoAlFine"))
                              st->setText("D.S.S. al Fine");
                        else if (!bars[measureCounter].direction.compare("DaCoda"))
                              st->setText("Da Coda");
                        else if (!bars[measureCounter].direction.compare("DaDoubleCoda"))
                              st->setText("Da Double Coda");
                        st->setParent(s);
                        st->setTrack(0);
                        score->addElement(st);
                        bars[measureCounter].direction = "";
                  }
                  else if (bars[measureCounter].direction.compare("") && !bars[measureCounter].directionStyle.compare("Target")) {
                        Segment* s = measure->getSegment(Segment::Type::BarLine, measure->tick());
                        Symbol* sym = new Symbol(score);
                        if (!bars[measureCounter].direction.compare("Segno"))
                              sym->setSym(SymId::segno);
                        else if (!bars[measureCounter].direction.compare("SegnoSegno")) {
                              sym->setSym(SymId::segno);
                              Segment* s2 = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                              Symbol* sym2 = new Symbol(score);
                              sym2->setSym(SymId::segno);
                              sym2->setParent(measure);
                              sym2->setTrack(0);
                              s2->add(sym2);
                              }
                        else if (!bars[measureCounter].direction.compare("Coda"))
                              sym->setSym(SymId::coda);
                        else if (!bars[measureCounter].direction.compare("DoubleCoda")) {
                              sym->setSym(SymId::coda);
                              Segment* s2 = measure->getSegment(Segment::Type::ChordRest, measure->tick());
                              Symbol* sym2 = new Symbol(score);
                              sym2->setSym(SymId::coda);
                              sym2->setParent(measure);
                              sym2->setTrack(0);
                              s2->add(sym2);
                              }
                        sym->setParent(measure);
                        sym->setTrack(0);
                        s->add(sym);
                        bars[measureCounter].direction = "";
                        }

                  KeySig* t = new KeySig(score);
                  if (!masterBarElement.nodeName().compare("Key")) {
                        t->setKey(Key(masterBarElement.firstChild().toElement().text().toInt()));
                        t->setTrack(0);
                        measure->getSegment(Segment::Type::KeySig, measure->tick())->add(t);
                        }
                  else if (!masterBarElement.nodeName().compare("Fermatas")) {
                        QDomNode currentFermata = masterBarElement.firstChild();
                        while (!currentFermata.isNull()) {
                              QString fermata = currentFermata.lastChildElement("Offset").toElement().text();
                              currentFermata = currentFermata.nextSibling();

                              // get the fermata information and construct a gpFermata from them
                              QStringList fermataComponents = fermata.split("/", QString::SkipEmptyParts);
                              GPFermata gpFermata;
                              gpFermata.index = fermataComponents.at(0).toInt();
                              gpFermata.timeDivision = fermataComponents.at(1).toInt();

                              if (fermatas.contains(measureCounter)) {
                                    QList<GPFermata>* fermataList = fermatas.value(measureCounter);
                                    fermataList->push_back(gpFermata);
                                    }
                              else  {
                                    QList<GPFermata>* fermataList = new QList<GPFermata>;
                                    fermataList->push_back(gpFermata);
                                    fermatas.insert(measureCounter, fermataList);
                                    }
                              }
                        }
                  else if (!masterBarElement.nodeName().compare("Repeat")) {
                        bool start = !masterBarElement.attributes().namedItem("start").toAttr().value().compare("true");
                        int count = masterBarElement.attributes().namedItem("count").toAttr().value().toInt();
                        if (start)
                              measure->setRepeatFlags(Repeat::START);
                        else
                              measure->setRepeatFlags(Repeat::END);
                        measure->setRepeatCount(count);
                        }
                  else if (!masterBarElement.nodeName().compare("AlternateEndings")) {
                        QString endNumbers = masterBarElement.toElement().text();
                        Ms::Volta* volta = new Ms::Volta(score);
                        volta->endings().clear();
                        volta->setText(endNumbers.replace(" ",","));
                        volta->setTick(measure->tick());
                        volta->setTick2(measure->tick() + measure->ticks());
                        score->addElement(volta);
                        }
                  else if (!masterBarElement.nodeName().compare("Bars")) {
                        readBars(&masterBarElement, measure, oldClefId, partInfo, t, measureCounter);
                        }
                  masterBarElement = masterBarElement.nextSibling();
                  }
            measureCounter++;
            nextMasterBar = nextMasterBar.nextSibling();
            measure = measure->nextMeasure();
            bar++;
            } while (!nextMasterBar.isNull());
      }

//---------------------------------------------------------
//   readGpif
//---------------------------------------------------------

void GuitarPro6::readGpif(QByteArray* data)
      {
      QDomDocument qdomDoc;
      qdomDoc.setContent(*data);
      QDomElement qdomElem = qdomDoc.documentElement();
      // GPRevision node
      QDomNode revision = qdomElem.firstChild();
      // Score node
      QDomNode scoreNode = revision.nextSibling();
      readScore(&scoreNode);
      // MasterTrack node
      QDomNode masterTrack = scoreNode.nextSibling();
      readMasterTracks(&masterTrack);
      // Tracks node
      QDomNode eachTrack = masterTrack.nextSibling();
      readTracks(&eachTrack);

      // now we know how many staves there are from readTracks, we can initialise slurs
      slurs = new Slur*[staves];
      for (int i = 0; i < staves; ++i)
            slurs[i] = 0;
      // ottava = new Ottava*[staves];
      // for (int staffIdx = 0; staffIdx < staves; ++staffIdx)
      //       ottava[staffIdx] = 0;

      // MasterBars node
      GPPartInfo partInfo;
      QDomNode masterBars = eachTrack.nextSibling();
      QDomNode bars = masterBars.nextSibling();
      QDomNode voices = bars.nextSibling();
      QDomNode beats = voices.nextSibling();
      QDomNode notes = beats.nextSibling();
      QDomNode rhythms = notes.nextSibling();

      // set up the partInfo struct to contain information from the file
      partInfo.masterBars = masterBars.firstChild();
      partInfo.bars = bars.firstChild();
      partInfo.voices = voices.firstChild();
      partInfo.beats = beats.firstChild();
      partInfo.notes = notes.firstChild();
      partInfo.rhythms = rhythms.firstChild();

      measures = findNumMeasures(&partInfo);

      createMeasures();
      fermatas.clear();
      readMasterBars(&partInfo);
      // set the starting tempo of the score
      setTempo(/*tempo*/120, score->firstMeasure());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void GuitarPro6::read(QFile* fp)
      {
      qDebug("reading guitar pro v6 file (.gpx)...");
      f = fp;

      slides = new QMap<int,int>();

      previousTempo = -1;
      this->buffer = new QByteArray();
      *(this->buffer) = fp->readAll();

      // decompress and read files contained within GPX file
      readGPX(this->buffer);
      delete this->buffer;
      }

//---------------------------------------------------------
//   readBeatEffects
//---------------------------------------------------------

int GuitarPro6::readBeatEffects(int, Segment*)
      {
      qDebug("reading beat effects (.gpx)...\n");
      return 0;
      }

}
