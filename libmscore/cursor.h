//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __CURSOR_H__
#define __CURSOR_H__

namespace Ms {

class Element;
class Score;
class Chord;
class Rest;
class Note;
class Segment;
class RepeatSegment;
class ChordRest;
class StaffText;
class Measure;

//---------------------------------------------------------
//   @@ Cursor
//   @P track     int           current track
//   @P staffIdx  int           current staff (track / 4)
//   @P voice     int           current voice (track % 4)
//   @P filter    enum          segment type filter
//   @P element   Ms::Element*  current element at track, read only
//   @P segment   Ms::Segment*  current segment, read only
//   @P measure   Ms::Measure*  current measure, read only
//   @P tick      int           midi tick position, read only
//   @P time      double        time at tick position, read only
//   @P keySignature int        key signature of current staff at tick pos. (read only)
//   @P score     Ms::Score*    associated score
//---------------------------------------------------------

class Cursor : public QObject {
      Q_OBJECT
      Q_PROPERTY(int track      READ track     WRITE setTrack)
      Q_PROPERTY(int staffIdx   READ staffIdx  WRITE setStaffIdx)
      Q_PROPERTY(int voice      READ voice     WRITE setVoice)
      Q_PROPERTY(int filter     READ filter    WRITE setFilter)

      Q_PROPERTY(Ms::Element* element READ element)
      Q_PROPERTY(Ms::Segment* segment READ segment)
      Q_PROPERTY(Ms::Measure* measure READ measure)

      Q_PROPERTY(int tick         READ tick)
      Q_PROPERTY(double time      READ time)
      Q_PROPERTY(int keySignature READ qmlKeySignature)
      Q_PROPERTY(Ms::Score* score READ score    WRITE setScore)

      Score* _score;
      int _track;
      bool _expandRepeats;

      //state
      Segment* _segment;
      Segment::Type _filter { Segment::Type::ChordRest };

      // utility methods
      void nextInTrack();
      void prevInTrack();

   public:
      Cursor(Score* c = 0);
      Cursor(Score*, bool);

      Score* score() const                    { return _score;    }
      void setScore(Score* s);

      int track() const                       { return _track;    }
      void setTrack(int v);

      int staffIdx() const;
      void setStaffIdx(int v);

      int voice() const;
      void setVoice(int v);

      int filter() const    { return int(_filter); }
      void setFilter(int f) { _filter = Segment::Type(f); }

      Element* element() const;
      Segment* segment() const                { return _segment;  }
      Measure* measure() const;

      int tick();
      double time();

      int qmlKeySignature();

      //@ moves cursor to first element of current track in score (filtered by 'filter')
      Q_INVOKABLE void scoreStart();
      //@ moves cursor to last element of current track in score (filtered by 'filter')
      Q_INVOKABLE void scoreEnd();
      //@ moves cursor to first element of current track in selection (filtered by 'filter');
      //@ if there is no selection, same as scoreStart()
      Q_INVOKABLE void selectionStart();
      //@ moves cursor to last element of current track in selection (filtered by 'filter');
      //@ if there is no selection, same as scoreEnd()
      Q_INVOKABLE void selectionEnd();
      //@ moves cursor to next element of current track (filtered by 'filter')
      Q_INVOKABLE bool next();
      //@ moves cursor to first segment of current track in next measure (filtered by 'filter');
      //@ returns false if end of score is reached
      Q_INVOKABLE bool nextMeasure();

      //@ adds an Element to the current cursor segment in the current track
      Q_INVOKABLE void add(Ms::Element*);
      //@ adds a note of pitch 'pitch' to the chord in current track of current segment;
      //@ chord must already exist
      Q_INVOKABLE void addNote(int pitch);

      // rewind cursor
      //   type=0      rewind to start of score
      //   type=1      rewind to start of selection
      //   type=2      rewind to end of selection
      Q_INVOKABLE void rewind(int type);  // obsolete; replaced by scoreStart/End() and selectionStart/End():

      //@ set duration
      //@   z: numerator
      //@   n: denominator
      //@   Quarter, if n == 0
      Q_INVOKABLE void setDuration(int z, int n);
      };

}     // namespace Ms
#endif

