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
#ifndef MU_NOTATION_INOTATIONINTERACTION_H
#define MU_NOTATION_INOTATIONINTERACTION_H

#include <QKeyEvent>
#include <functional>

#include "async/notification.h"

#include "notationtypes.h"
#include "inotationnoteinput.h"
#include "inotationselection.h"
#include "actions/actiontypes.h"

namespace mu::notation {
class INotationInteraction
{
public:
    virtual ~INotationInteraction() = default;

    // Put notes
    virtual INotationNoteInputPtr noteInput() const = 0;

    // Shadow note
    virtual void showShadowNote(const PointF& pos) = 0;
    virtual void hideShadowNote() = 0;

    // Visibility
    virtual void toggleVisible() = 0;

    // Hit
    virtual EngravingItem* hitElement(const PointF& pos, float width) const = 0;
    virtual Staff* hitStaff(const PointF& pos) const = 0;

    struct HitElementContext
    {
        notation::EngravingItem* element = nullptr;
        notation::Staff* staff = nullptr;
    };

    virtual const HitElementContext& hitElementContext() const = 0;
    virtual void setHitElementContext(const HitElementContext& context) = 0;

    // Select
    virtual void moveChordNoteSelection(MoveDirection d) = 0;
    virtual void select(const std::vector<EngravingItem*>& elements, SelectType type = SelectType::REPLACE, int staffIndex = 0) = 0;
    virtual void selectAll() = 0;
    virtual void selectSection() = 0;
    virtual void selectFirstElement(bool frame = true) = 0;
    virtual void selectLastElement() = 0;
    virtual INotationSelectionPtr selection() const = 0;
    virtual void clearSelection() = 0;
    virtual async::Notification selectionChanged() const = 0;
    virtual void selectTopOrBottomOfChord(MoveDirection d) = 0;

    // SelectionFilter
    virtual bool isSelectionTypeFiltered(SelectionFilterType type) const = 0;
    virtual void setSelectionTypeFiltered(SelectionFilterType type, bool filtered) = 0;

    // Drag
    using IsDraggable = std::function<bool (const EngravingItem*)>;
    virtual bool isDragStarted() const = 0;
    virtual void startDrag(const std::vector<EngravingItem*>& elems, const PointF& eoffset, const IsDraggable& isDrag) = 0;
    virtual void drag(const PointF& fromPos, const PointF& toPos, DragMode mode) = 0;
    virtual void endDrag() = 0;
    virtual async::Notification dragChanged() const = 0;

    // Drop
    //! TODO Change KeyboardModifiers to modes
    virtual void startDrop(const QByteArray& edata) = 0;
    virtual bool startDrop(const QUrl& url) = 0;
    virtual bool isDropAccepted(const PointF& pos, Qt::KeyboardModifiers modifiers) = 0; //! NOTE Also may set drop target
    virtual bool drop(const PointF& pos, Qt::KeyboardModifiers modifiers) = 0;
    virtual void endDrop() = 0;
    virtual async::Notification dropChanged() const = 0;

    virtual bool applyPaletteElement(Ms::EngravingItem* element, Qt::KeyboardModifiers modifiers = {}) = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;

    // Change selection
    virtual void moveSelection(MoveDirection d, MoveSelectionType type) = 0;
    virtual void moveLyrics(MoveDirection d) = 0;
    virtual void expandSelection(ExpandSelectionMode mode) = 0;
    virtual void addToSelection(MoveDirection d, MoveSelectionType type) = 0;
    virtual void selectTopStaff() = 0;
    virtual void selectEmptyTrailingMeasure() = 0;
    virtual void moveSegmentSelection(MoveDirection d) = 0;

    // Move/nudge elements
    virtual void movePitch(MoveDirection d, PitchMode mode) = 0;
    virtual void nudge(MoveDirection d, bool quickly) = 0;
    virtual void moveChordRestToStaff(MoveDirection d) = 0;
    virtual void swapChordRest(MoveDirection d) = 0;

    // Text edit
    virtual bool isTextSelected() const = 0;
    virtual bool isTextEditingStarted() const = 0;
    virtual bool textEditingAllowed(const EngravingItem* element) const = 0;
    virtual void startEditText(EngravingItem* element, const PointF& elementPos) = 0;
    virtual void editText(QKeyEvent* event) = 0;
    virtual void endEditText() = 0;
    virtual void changeTextCursorPosition(const PointF& newCursorPos) = 0;
    virtual const TextBase* editedText() const = 0;
    virtual async::Notification textEditingStarted() const = 0;
    virtual async::Notification textEditingChanged() const = 0;

    // Display
    virtual async::Channel<ScoreConfigType> scoreConfigChanged() const = 0;

    // Grip edit
    virtual bool isGripEditStarted() const = 0;
    virtual bool isHitGrip(const PointF& pos) const = 0;
    virtual void startEditGrip(const PointF& pos) = 0;

    virtual bool isElementEditStarted() const = 0;
    virtual void endEditElement() = 0;
    virtual void startEditElement() = 0;

    virtual void splitSelectedMeasure() = 0;
    virtual void joinSelectedMeasures() = 0;

    virtual void addBoxes(BoxType boxType, int count, int beforeBoxIndex = -1) = 0;

    virtual void copySelection() = 0;
    virtual void copyLyrics() = 0;
    virtual void pasteSelection(const Fraction& scale = Fraction(1, 1)) = 0;
    virtual void swapSelection() = 0;
    virtual void deleteSelection() = 0;
    virtual void flipSelection() = 0;
    virtual void addTieToSelection() = 0;
    virtual void addTiedNoteToChord() = 0;
    virtual void addSlurToSelection() = 0;
    virtual void addOttavaToSelection(OttavaType type) = 0;
    virtual void addHairpinToSelection(HairpinType type) = 0;
    virtual void addAccidentalToSelection(AccidentalType type) = 0;
    virtual void putRestToSelection() = 0;
    virtual void putRest(DurationType duration) = 0;
    virtual void addBracketsToSelection(BracketsType type) = 0;
    virtual void changeSelectedNotesArticulation(SymbolId articulationSymbolId) = 0;
    virtual void addGraceNotesToSelectedNotes(GraceNoteType type) = 0;
    virtual bool canAddTupletToSelecredChordRests() const = 0;
    virtual void addTupletToSelectedChordRests(const TupletOptions& options) = 0;
    virtual void addBeamToSelectedChordRests(BeamMode mode) = 0;

    virtual void increaseDecreaseDuration(int steps, bool stepByDots) = 0;

    virtual void toggleLayoutBreak(LayoutBreakType breakType) = 0;
    virtual void setBreaksSpawnInterval(BreaksSpawnIntervalType intervalType, int interval = 0) = 0;
    virtual bool transpose(const TransposeOptions& options) = 0;
    virtual void swapVoices(int voiceIndex1, int voiceIndex2) = 0;
    virtual void addIntervalToSelectedNotes(int interval) = 0;
    virtual void addFret(int fretIndex) = 0;
    virtual void changeSelectedNotesVoice(int voiceIndex) = 0;
    virtual void addAnchoredLineToSelectedNotes() = 0;

    virtual void addText(TextType type) = 0;
    virtual void addFiguredBass() = 0;

    virtual void addStretch(qreal value) = 0;

    virtual void addTimeSignature(Measure* measure, int staffIndex, TimeSignature* timeSignature) = 0;

    virtual void explodeSelectedStaff() = 0;
    virtual void implodeSelectedStaff() = 0;

    virtual void realizeSelectedChordSymbols() = 0;
    virtual void removeSelectedRange() = 0;
    virtual void removeEmptyTrailingMeasures() = 0;

    virtual void fillSelectionWithSlashes() = 0;
    virtual void replaceSelectedNotesWithSlashes() = 0;

    virtual void repeatSelection() = 0;
    virtual void changeEnharmonicSpelling(bool both) = 0;
    virtual void spellPitches() = 0;
    virtual void regroupNotesAndRests() = 0;
    virtual void resequenceRehearsalMarks() = 0;
    virtual void unrollRepeats() = 0;

    virtual void resetStretch() = 0;
    virtual void resetTextStyleOverrides() = 0;
    virtual void resetBeamMode() = 0;
    virtual void resetShapesAndPosition() = 0;

    virtual ScoreConfig scoreConfig() const = 0;
    virtual void setScoreConfig(ScoreConfig config) = 0;

    virtual void nextLyrics(MoveDirection) = 0;
    virtual void nextLyricsVerse(MoveDirection) = 0;
    virtual void nextSyllable() = 0;
    virtual void addMelisma() = 0;
    virtual void addLyricsVerse() = 0;

    virtual void toggleBold() = 0;
    virtual void toggleItalic() = 0;
    virtual void toggleUnderline() = 0;
    virtual void toggleStrike() = 0;

    virtual void toggleArticulation(Ms::SymId) = 0;
    virtual void insertClef(Ms::ClefType) = 0;
    virtual void changeAccidental(Ms::AccidentalType) = 0;
    virtual void transposeSemitone(int) = 0;
    virtual void toggleGlobalOrLocalInsert() = 0;
    virtual void getLocation() = 0;
    virtual void execute(void (Ms::Score::*)()) = 0;
};

using INotationInteractionPtr = std::shared_ptr<INotationInteraction>;
}

#endif // MU_NOTATION_INOTATIONINTERACTION_H
