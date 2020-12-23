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
#include "noteinputbarmodel.h"

#include "log.h"
#include "ui/view/iconcodes.h"
#include "internal/notationactions.h"

using namespace mu::notation;
using namespace mu::actions;
using namespace mu::workspace;
using namespace mu::framework;

static const std::string TOOLBAR_TAG("Toolbar");
static const std::string TOOLBAR_NAME("noteInput");

static const std::string ADD_ACTION_NAME("add");
static const std::string ADD_ACTION_TITLE("Add");
static const IconCode::Code ADD_ACTION_ICON_CODE = IconCode::Code::PLUS;

static QList<mu::actions::ActionName> noteInputModeActionNames = {
    { "note-input" },
    { "note-input-rhythm" },
    { "note-input-repitch" },
    { "note-input-realtime-auto" },
    { "note-input-realtime-manual" },
    { "note-input-timewise" },
};

NoteInputBarModel::NoteInputBarModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QVariant NoteInputBarModel::data(const QModelIndex& index, int role) const
{
    const ActionItem& item = m_items.at(index.row());
    switch (role) {
    case IconRole: return static_cast<int>(item.action.iconCode);
    case SectionRole: return item.section;
    case NameRole: return QString::fromStdString(item.action.name);
    case CheckedRole: return item.checked;
    }
    return QVariant();
}

int NoteInputBarModel::rowCount(const QModelIndex&) const
{
    return m_items.count();
}

QHash<int,QByteArray> NoteInputBarModel::roleNames() const
{
    static const QHash<int, QByteArray> roles = {
        { IconRole, "iconRole" },
        { SectionRole, "sectionRole" },
        { NameRole, "nameRole" },
        { CheckedRole, "checkedRole" }
    };
    return roles;
}

void NoteInputBarModel::load()
{
    m_items.clear();

    beginResetModel();

    std::vector<std::string> noteInputActions = currentWorkspaceActions();

    int section = 0;
    for (const std::string& actionName: noteInputActions) {
        if (actionName.empty()) {
            section++;
            continue;
        }

        m_items << makeActionItem(actionsRegister()->action(actionName), QString::number(section));
    }

    m_items << makeAddItem(QString::number(++section));

    endResetModel();

    emit countChanged(rowCount());

    RetValCh<IWorkspacePtr> workspace = workspaceManager()->currentWorkspace();

    if (workspace.ret) {
        workspace.ch.onReceive(this, [this](IWorkspacePtr) {
            load();
        });

        workspace.val->dataChanged().onReceive(this, [this](const AbstractDataPtr data) {
            if (data->name == TOOLBAR_NAME) {
                load();
            }
        });
    }

    onNotationChanged();

    context()->currentNotationChanged().onNotify(this, [this]() {
        onNotationChanged();
    });

    playbackController()->isPlayingChanged().onNotify(this, [this]() {
        updateState();
    });
}

NoteInputBarModel::ActionItem& NoteInputBarModel::item(const actions::ActionName& actionName)
{
    for (ActionItem& item : m_items) {
        if (item.action.name == actionName) {
            return item;
        }
    }

    static ActionItem null;
    return null;
}

int NoteInputBarModel::findNoteInputModeItemIndex() const
{
    for (int i = 0; i < m_items.size(); i++) {
        if (noteInputModeActionNames.contains(m_items[i].action.name)) {
            return i;
        }
    }

    return -1;
}

void NoteInputBarModel::onNotationChanged()
{
    INotationPtr notation = context()->currentNotation();

    if (notation) {
        noteInput()->stateChanged().onNotify(this, [this]() {
            updateState();
        });

        interaction()->selectionChanged().onNotify(this, [this]() {
            updateState();
        });

        undoStack()->stackChanged().onNotify(this, [this]() {
            updateState();
        });
    }

    updateState();
}

void NoteInputBarModel::toggleNoteInput()
{
    if (!noteInput()) {
        return;
    }

    if (isNoteInputMode()) {
        noteInput()->endNoteInput();
    } else {
        noteInput()->startNoteInput();
    }
}

void NoteInputBarModel::updateState()
{
    bool isPlaying = playbackController()->isPlaying();
    if (!notation() || isPlaying) {
        for (ActionItem& item : m_items) {
            item.enabled = false;
            item.checked = false;
        }
    } else {
        for (ActionItem& item : m_items) {
            item.enabled = true;
            item.checked = false;
        }

        updateNoteInputState();
    }

    emit dataChanged(index(0), index(rowCount() - 1));
}

void NoteInputBarModel::updateNoteInputState()
{
    updateNoteInputModeState();

    updateNoteDotState();
    updateNoteDurationState();
    updateNoteAccidentalState();
    updateTieState();
    updateSlurState();
    updateVoicesState();
    updateArticulationsState();
    updateRestState();
}

void NoteInputBarModel::updateNoteInputModeState()
{
    int noteInputModeIndex = findNoteInputModeItemIndex();
    if (noteInputModeIndex == -1) {
        return;
    }

    m_items[noteInputModeIndex].action = currentNoteInputModeAction();
    m_items[noteInputModeIndex].checked = isNoteInputMode();

    emit dataChanged(index(noteInputModeIndex), index(noteInputModeIndex));
}

void NoteInputBarModel::updateNoteDotState()
{
    static std::vector<actions::ActionName> dotActions = {
        "pad-dot",
        "pad-dotdot",
        "pad-dot3",
        "pad-dot4"
    };

    int durationDots = noteInputState().duration.dots();

    for (const actions::ActionName& actionName: dotActions) {
        item(actionName).checked = durationDots == NotationActions::actionDotCount(actionName);
    }
}

void NoteInputBarModel::updateNoteDurationState()
{
    static std::vector<actions::ActionName> noteActions = {
        "note-longa",
        "note-breve",
        "pad-note-1",
        "pad-note-2",
        "pad-note-4",
        "pad-note-8",
        "pad-note-16",
        "pad-note-32",
        "pad-note-64",
        "pad-note-128",
        "pad-note-256",
        "pad-note-512",
        "pad-note-1024"
    };

    DurationType durationType = resolveCurrentDurationType();

    for (const actions::ActionName& actionName: noteActions) {
        item(actionName).checked = durationType == NotationActions::actionDurationType(actionName);
    }
}

void NoteInputBarModel::updateNoteAccidentalState()
{
    static std::vector<actions::ActionName> accidentalActions = {
        "flat2",
        "flat",
        "nat",
        "sharp",
        "sharp2"
    };

    AccidentalType accidentalType = noteInputState().accidentalType;

    for (const actions::ActionName& actionName: accidentalActions) {
        item(actionName).checked = accidentalType == NotationActions::actionAccidentalType(actionName);
    }
}

void NoteInputBarModel::updateTieState()
{
    std::vector<Note*> tiedNotes = selection()->notes(NoteFilter::WithTie);

    bool checked = !tiedNotes.empty();
    for (const Note* note: tiedNotes) {
        if (!note->tieFor()) {
            checked = false;
            break;
        }
    }

    item("tie").checked = checked;
}

void NoteInputBarModel::updateSlurState()
{
    item("add-slur").checked = noteInputState().withSlur;
}

void NoteInputBarModel::updateVoicesState()
{
    static std::vector<actions::ActionName> voiceActions {
        "voice-1",
        "voice-2",
        "voice-3",
        "voice-4"
    };

    int currentVoice = resolveCurrentVoiceIndex();

    for (const actions::ActionName& actionName: voiceActions) {
        item(actionName).checked = currentVoice == NotationActions::actionVoice(actionName);
    }
}

void NoteInputBarModel::updateArticulationsState()
{
    static std::vector<actions::ActionName> articulationActions {
        "add-marcato",
        "add-sforzato",
        "add-tenuto",
        "add-staccato"
    };

    std::set<SymbolId> currentArticulations = resolveCurrentArticulations();

    auto isArticulationSelected = [&currentArticulations](SymbolId articulationSymbolId) {
        return std::find(currentArticulations.begin(), currentArticulations.end(),
                         articulationSymbolId) != currentArticulations.end();
    };

    for (const actions::ActionName& actionName: articulationActions) {
        item(actionName).checked = isArticulationSelected(NotationActions::actionArticulationSymbolId(actionName));
    }
}

void NoteInputBarModel::updateRestState()
{
    item("pad-rest").checked = resolveRestSelected();
}

int NoteInputBarModel::resolveCurrentVoiceIndex() const
{
    constexpr int INVALID_VOICE = -1;

    if (!noteInput() || !selection()) {
        return INVALID_VOICE;
    }

    if (isNoteInputMode()) {
        return noteInputState().currentVoiceIndex;
    }

    if (selection()->isNone()) {
        return INVALID_VOICE;
    }

    for (const Element* element: selection()->elements()) {
        return element->voice();
    }

    return INVALID_VOICE;
}

std::set<SymbolId> NoteInputBarModel::resolveCurrentArticulations() const
{
    if (!noteInput() || !selection()) {
        return {};
    }

    if (isNoteInputMode()) {
        return noteInputState().articulationIds;
    }

    if (selection()->isNone()) {
        return {};
    }

    auto chordArticulations = [](const Chord* chord) {
        std::set<SymbolId> result;
        for (Articulation* articulation: chord->articulations()) {
            result.insert(articulation->symId());
        }

        result = Ms::flipArticulations(result, Ms::Placement::ABOVE);
        return Ms::splitArticulations(result);
    };

    std::set<SymbolId> result;
    bool isFirstNote = true;
    for (const Element* element: selection()->elements()) {
        if (!element->isNote()) {
            continue;
        }

        const Note* note = dynamic_cast<const Note*>(element);
        if (isFirstNote) {
            result = chordArticulations(note->chord());
            isFirstNote = false;
        } else {
            std::set<SymbolId> currentNoteArticulations = chordArticulations(note->chord());
            for (auto it = result.begin(); it != result.end();) {
                if (std::find(currentNoteArticulations.begin(), currentNoteArticulations.end(),
                              *it) == currentNoteArticulations.end()) {
                    it = result.erase(it);
                } else {
                    ++it;
                }
            }
        }
    }

    return result;
}

bool NoteInputBarModel::resolveRestSelected() const
{
    if (!noteInput() || !selection()) {
        return false;
    }

    if (isNoteInputMode()) {
        return noteInputState().isRest;
    }

    if (selection()->isNone() || selection()->isRange()) {
        return false;
    }

    for (const Element* element: selection()->elements()) {
        if (!element->isRest()) {
            return false;
        }
    }

    return true;
}

DurationType NoteInputBarModel::resolveCurrentDurationType() const
{
    constexpr DurationType INVALID_DURATION_TYPE = DurationType::V_INVALID;

    if (!noteInput() || !selection()) {
        return INVALID_DURATION_TYPE;
    }

    if (isNoteInputMode()) {
        return noteInputState().duration.type();
    }

    if (selection()->isNone() || selection()->isRange()) {
        return INVALID_DURATION_TYPE;
    }

    if (selection()->elements().empty()) {
        return INVALID_DURATION_TYPE;
    }

    DurationType result = INVALID_DURATION_TYPE;
    bool isFirstElement = true;
    for (const Element* element: selection()->elements()) {
        const ChordRest* chordRest = elementToChordRest(element);
        if (!chordRest) {
            continue;
        }

        if (isFirstElement) {
            result = chordRest->durationType().type();
            isFirstElement = false;
        } else if (result != chordRest->durationType().type()) {
            return INVALID_DURATION_TYPE;
        }
    }

    return result;
}

bool NoteInputBarModel::isNoteInputModeAction(const ActionName& actionName) const
{
    return noteInputModeActionNames.contains(actionName);
}

Action NoteInputBarModel::currentNoteInputModeAction() const
{
    static QMap<NoteInputMethod, actions::ActionName> noteInputActionNames = {
        { NoteInputMethod::STEPTIME, "note-input" },
        { NoteInputMethod::RHYTHM, "note-input-rhythm" },
        { NoteInputMethod::REPITCH, "note-input-repitch" },
        { NoteInputMethod::REALTIME_AUTO, "note-input-realtime-auto" },
        { NoteInputMethod::REALTIME_MANUAL, "note-input-realtime-manual" },
        { NoteInputMethod::TIMEWISE, "note-input-timewise" },
    };

    NoteInputMethod method = noteInputState().method;
    return actionsRegister()->action(noteInputActionNames[method]);
}

NoteInputBarModel::ActionItem NoteInputBarModel::makeActionItem(const Action& action, const QString& section)
{
    ActionItem item;
    item.action = action;
    item.section = section;
    return item;
}

NoteInputBarModel::ActionItem NoteInputBarModel::makeAddItem(const QString& section)
{
    Action addAction(ADD_ACTION_NAME, ADD_ACTION_TITLE, shortcuts::ShortcutContext::Undefined, ADD_ACTION_ICON_CODE);
    return makeActionItem(addAction, section);
}

std::vector<std::string> NoteInputBarModel::currentWorkspaceActions() const
{
    RetValCh<IWorkspacePtr> workspace = workspaceManager()->currentWorkspace();
    if (!workspace.ret) {
        LOGE() << workspace.ret.toString();
        return {};
    }

    AbstractDataPtr abstractData = workspace.val->data(WorkspaceTag::Toolbar, TOOLBAR_NAME);
    ToolbarDataPtr toolbarData = std::dynamic_pointer_cast<ToolbarData>(abstractData);
    if (!toolbarData) {
        LOGE() << "Failed to get data of actions for " << TOOLBAR_NAME;
        return {};
    }

    return toolbarData->actions;
}

void NoteInputBarModel::handleAction(const QString& action)
{
    if (isNoteInputModeAction(action.toStdString())) {
        toggleNoteInput();
        return;
    }

    dispatcher()->dispatch(actions::namefromQString(action));
}

QVariantMap NoteInputBarModel::get(int index)
{
    QVariantMap result;

    QHash<int,QByteArray> names = roleNames();
    QHashIterator<int, QByteArray> i(names);
    while (i.hasNext()) {
        i.next();
        QModelIndex idx = this->index(index, 0);
        QVariant data = idx.data(i.key());
        result[i.value()] = data;
    }

    return result;
}

INotationPtr NoteInputBarModel::notation() const
{
    return context()->currentNotation();
}

INotationInteractionPtr NoteInputBarModel::interaction() const
{
    return notation() ? notation()->interaction() : nullptr;
}

INotationSelectionPtr NoteInputBarModel::selection() const
{
    return interaction() ? interaction()->selection() : nullptr;
}

INotationUndoStackPtr NoteInputBarModel::undoStack() const
{
    return notation() ? notation()->undoStack() : nullptr;
}

INotationNoteInputPtr NoteInputBarModel::noteInput() const
{
    return interaction() ? interaction()->noteInput() : nullptr;
}

bool NoteInputBarModel::isNoteInputMode() const
{
    return noteInput() ? noteInput()->isNoteInputMode() : false;
}

NoteInputState NoteInputBarModel::noteInputState() const
{
    return noteInput() ? noteInput()->state() : NoteInputState();
}

const ChordRest* NoteInputBarModel::elementToChordRest(const Element* element) const
{
    if (!element) {
        return nullptr;
    }
    if (element->isChordRest()) {
        return toChordRest(element);
    }
    if (element->isNote()) {
        return toNote(element)->chord();
    }
    if (element->isStem()) {
        return toStem(element)->chord();
    }
    if (element->isHook()) {
        return toHook(element)->chord();
    }
    return nullptr;
}
