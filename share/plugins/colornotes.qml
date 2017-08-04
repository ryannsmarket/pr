//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2012 Werner Schweer
//  Copyright (C) 2013-2015 Nicolas Froment, Joachim Schmitz
//  Copyright (C) 2014 Jörn Eichler
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

import QtQuick 2.0
import MuseScore 3.0

MuseScore {
      version:  "1.0"
      description: qsTr("This plugin colors notes in the selection depending on their pitch as per the Boomwhackers convention")
      menuPath: "Plugins.Notes.Color Notes"

      property variant colors : [ // "#rrggbb" with rr, gg, and bb being the hex values for red, green, and blue, respectively
               "#e21c48", // C
               "#f26622", // C#/Db
               "#f99d1c", // D
               "#ffcc33", // D#/Eb
               "#fff32b", // E
               "#bcd85f", // F
               "#62bc47", // F#/Gb
               "#009c95", // G
               "#0071bb", // G#/Ab
               "#5e50a1", // A
               "#8d5ba6", // A#/Bb
               "#cf3e96"  // B
               ]
      property variant black : "#000000"

      // Apply the given function to all notes in selection
      // or, if nothing is selected, in the entire score

      function applyToNotesInSelection(func) {
            var cursor = curScore.newCursor();
            cursor.rewind(1);
            var startStaff;
            var endStaff;
            var endTick;
            var fullScore = false;
            if (!cursor.segment()) { // no selection
                  fullScore = true;
                  startStaff = 0; // start with 1st staff
                  endStaff = curScore.nstaves - 1; // and end with last
            } else {
                  startStaff = cursor.staffIdx;
                  cursor.rewind(2);
                  if (cursor.tick == 0) {
                        // this happens when the selection includes
                        // the last measure of the score.
                        // rewind(2) goes behind the last segment (where
                        // there's none) and sets tick=0
                        endTick = curScore.lastSegment.tick + 1;
                  } else {
                        endTick = cursor.tick;
                  }
                  endStaff = cursor.staffIdx;
            }
            console.log(startStaff + " - " + endStaff + " - " + endTick)
            for (var staff = startStaff; staff <= endStaff; staff++) {
                  for (var voice = 0; voice < 4; voice++) {
                        cursor.rewind(1); // sets voice to 0
                        cursor.voice = voice; //voice has to be set after goTo
                        cursor.staffIdx = staff;

                        if (fullScore)
                              cursor.rewind(0) // if no selection, beginning of score
                        console.log("voice="+voice+" segment="+cursor.segment()+" fullscore="+fullScore);
                        while (cursor.segment() && (fullScore || cursor.tick < endTick)) {
                              console.log("Element="+cursor.element());
                              if (cursor.element() && cursor.element().type == Ms.CHORD) {
                                    var graceChords = cursor.element().graceNotes;
                                    for (var i = 0; i < graceChords.length; i++) {
                                          // iterate through all grace chords
                                          var notes = graceChords[i].notes;
                                          for (var j = 0; j < notes.length; j++)
                                                func(notes[j]);
                                    }
                                    var notes = cursor.element().notes;
                                    for (var i = 0; i < notes.length; i++) {
                                          var note = notes[i];
                                          func(note);
                                    }
                              }
                              cursor.next();
                        }
                  }
            }
      }

      function colorNote(note) {
            var pitch = note.get("pitch");
            var color = note.get("color");
            var newcolor;
            console.log("Pitch="+pitch+" color="+color);
            if (color == black) 
                  newcolor = colors[pitch % 12];
            else
                  newcolor = black;

            note.set("color", newcolor);
            if (note.accidental) note.accidental.set("color",newcolor);
            for (var i = 0; i < note.dots.length; i++) {
                  if (note.dots[i]) {
                        note.dots[i].set("color",newcolor);
                        }
                  }
         }

      onRun: {
            console.log("hello colornotes");

            if (typeof curScore === 'undefined')
                  Qt.quit();

            applyToNotesInSelection(colorNote)

            Qt.quit();
         }
}
