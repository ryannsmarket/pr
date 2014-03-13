//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "libmscore/score.h"
#include "libmscore/chord.h"
#include "libmscore/note.h"
#include "libmscore/notedot.h"
#include "libmscore/beam.h"
#include "libmscore/stem.h"
#include "libmscore/hook.h"
#include "inspector.h"
#include "inspectorNote.h"

namespace Ms {

//---------------------------------------------------------
//   InspectorNote
//---------------------------------------------------------

InspectorNote::InspectorNote(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());
      s.setupUi(addWidget());
      c.setupUi(addWidget());
      n.setupUi(addWidget());
            
      static const NoteHeadGroup heads[] = {
            NoteHeadGroup::HEAD_NORMAL,
            NoteHeadGroup::HEAD_CROSS,
            NoteHeadGroup::HEAD_DIAMOND,
            NoteHeadGroup::HEAD_TRIANGLE,
            NoteHeadGroup::HEAD_SLASH,
            NoteHeadGroup::HEAD_XCIRCLE,
            NoteHeadGroup::HEAD_DO,
            NoteHeadGroup::HEAD_RE,
            NoteHeadGroup::HEAD_MI,
            NoteHeadGroup::HEAD_FA,
            NoteHeadGroup::HEAD_SOL,
            NoteHeadGroup::HEAD_LA,
            NoteHeadGroup::HEAD_TI,
            NoteHeadGroup::HEAD_BREVIS_ALT
            };

      //
      // fix order of note heads
      //
      for (unsigned i = 0; i < sizeof(heads)/sizeof(*heads); ++i)
            n.noteHeadGroup->setItemData(i, QVariant(int(heads[i])));

      // noteHeadType starts at -1: correct values and count one item more (HEAD_AUTO)
      for (int i = 0; i <= int(NoteHeadType::HEAD_TYPES); ++i)
            n.noteHeadType->setItemData(i, i-1);

      iList = {
            { P_COLOR,          0, 0, b.color,         b.resetColor         },
            { P_VISIBLE,        0, 0, b.visible,       b.resetVisible       },
            { P_USER_OFF,       0, 0, b.offsetX,       b.resetX             },
            { P_USER_OFF,       1, 0, b.offsetY,       b.resetY             },

            { P_SMALL,          0, 0, n.small,         n.resetSmall         },
            { P_HEAD_GROUP,     0, 0, n.noteHeadGroup, n.resetNoteHeadGroup },
            { P_HEAD_TYPE,      0, 0, n.noteHeadType,  n.resetNoteHeadType  },
            { P_MIRROR_HEAD,    0, 0, n.mirrorHead,    n.resetMirrorHead    },
            { P_DOT_POSITION,   0, 0, n.dotPosition,   n.resetDotPosition   },
            { P_PLAY,           0, 0, n.play,          n.resetPlay          },
            { P_TUNING,         0, 0, n.tuning,        n.resetTuning        },
            { P_VELO_TYPE,      0, 0, n.velocityType,  n.resetVelocityType  },
            { P_VELO_OFFSET,    0, 0, n.velocity,      n.resetVelocity      },

            { P_USER_OFF,       0, 1, c.offsetX,       c.resetX             },
            { P_USER_OFF,       1, 1, c.offsetY,       c.resetY             },
            { P_SMALL,          0, 1, c.small,         c.resetSmall         },
            { P_NO_STEM,        0, 1, c.stemless,      c.resetStemless      },
            { P_STEM_DIRECTION, 0, 1, c.stemDirection, c.resetStemDirection },

            { P_LEADING_SPACE,  0, 2, s.leadingSpace,  s.resetLeadingSpace  },
            { P_TRAILING_SPACE, 0, 2, s.trailingSpace, s.resetTrailingSpace }
            };

      mapSignals();

      //
      // Select
      //
      QLabel* l = new QLabel;
      l->setText(tr("Select"));
      QFont font(l->font());
      font.setBold(true);
      l->setFont(font);
      l->setAlignment(Qt::AlignHCenter);
      _layout->addWidget(l);
      QFrame* f = new QFrame;
      f->setFrameStyle(QFrame::HLine | QFrame::Raised);
      f->setLineWidth(2);
      _layout->addWidget(f);

      QHBoxLayout* hbox = new QHBoxLayout;
      dot1 = new QToolButton(this);
      dot1->setText(tr("Dot1"));
      dot1->setEnabled(false);
      hbox->addWidget(dot1);
      dot2 = new QToolButton(this);
      dot2->setText(tr("Dot2"));
      dot2->setEnabled(false);
      hbox->addWidget(dot2);
      dot3 = new QToolButton(this);
      dot3->setText(tr("Dot3"));
      dot3->setEnabled(false);
      hbox->addWidget(dot3);
      _layout->addLayout(hbox);

      hbox = new QHBoxLayout;
      hook = new QToolButton(this);
      hook->setText(tr("Hook"));
      hook->setEnabled(false);
      hbox->addWidget(hook);
      stem = new QToolButton(this);
      stem->setText(tr("Stem"));
      stem->setEnabled(false);
      hbox->addWidget(stem);
      beam = new QToolButton(this);
      beam->setText(tr("Beam"));
      beam->setEnabled(false);
      hbox->addWidget(beam);
      _layout->addLayout(hbox);

      connect(dot1,     SIGNAL(clicked()),     SLOT(dot1Clicked()));
      connect(dot2,     SIGNAL(clicked()),     SLOT(dot2Clicked()));
      connect(dot3,     SIGNAL(clicked()),     SLOT(dot3Clicked()));
      connect(hook,     SIGNAL(clicked()),     SLOT(hookClicked()));
      connect(stem,     SIGNAL(clicked()),     SLOT(stemClicked()));
      connect(beam,     SIGNAL(clicked()),     SLOT(beamClicked()));
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorNote::setElement()
      {
      Note* note = static_cast<Note*>(inspector->element());
      dot1->setEnabled(note->dot(0));
      dot2->setEnabled(note->dot(1));
      dot3->setEnabled(note->dot(2));
      stem->setEnabled(note->chord()->stem());
      hook->setEnabled(note->chord()->hook());
      beam->setEnabled(note->chord()->beam());
      InspectorBase::setElement();
      }

//---------------------------------------------------------
//   dot1Clicked
//---------------------------------------------------------

void InspectorNote::dot1Clicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      NoteDot* dot = note->dot(0);
      if (dot) {
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->end();
            }
      }

//---------------------------------------------------------
//   dot2Clicked
//---------------------------------------------------------

void InspectorNote::dot2Clicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      NoteDot* dot = note->dot(1);
      if (dot) {
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->end();
            }
      }

//---------------------------------------------------------
//   dot3Clicked
//---------------------------------------------------------

void InspectorNote::dot3Clicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      NoteDot* dot = note->dot(2);
      if (dot) {
            dot->score()->select(dot);
            inspector->setElement(dot);
            dot->score()->end();
            }
      }

//---------------------------------------------------------
//   hookClicked
//---------------------------------------------------------

void InspectorNote::hookClicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      Hook* hook = note->chord()->hook();
      if (hook) {
            note->score()->select(hook);
            inspector->setElement(hook);
            note->score()->end();
            }
      }

//---------------------------------------------------------
//   stemClicked
//---------------------------------------------------------

void InspectorNote::stemClicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      Stem* stem = note->chord()->stem();
      if (stem) {
            note->score()->select(stem);
            inspector->setElement(stem);
            note->score()->end();
            }
      }

//---------------------------------------------------------
//   beamClicked
//---------------------------------------------------------

void InspectorNote::beamClicked()
      {
      Note* note = static_cast<Note*>(inspector->element());
      if (note == 0)
            return;
      Beam* beam = note->chord()->beam();
      if (beam) {
            note->score()->select(beam);
            inspector->setElement(beam);
            note->score()->end();
            }
      }

}

