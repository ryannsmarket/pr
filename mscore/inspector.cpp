//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "inspector.h"
#include "inspectorBeam.h"
#include "inspectorImage.h"
#include "inspectorLasso.h"
#include "inspectorGroupElement.h"
#include "inspectorVolta.h"
#include "inspectorOttava.h"
#include "inspectorTrill.h"
#include "inspectorHairpin.h"
#include "inspectorMarker.h"
#include "inspectorJump.h"
#include "inspectorGlissando.h"
#include "inspectorNote.h"
#include "musescore.h"
#include "scoreview.h"

#include "libmscore/element.h"
#include "libmscore/score.h"
#include "libmscore/box.h"
#include "libmscore/undo.h"
#include "libmscore/spacer.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/segment.h"
#include "libmscore/rest.h"
#include "libmscore/beam.h"
#include "libmscore/clef.h"
#include "libmscore/notedot.h"
#include "libmscore/hook.h"
#include "libmscore/stem.h"
#include "libmscore/keysig.h"
#include "libmscore/barline.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"

//---------------------------------------------------------
//   showInspector
//---------------------------------------------------------

void MuseScore::showInspector(bool visible)
      {
      QAction* a = getAction("inspector");
      if (visible) {
            if (!inspector) {
                  inspector = new Inspector();
                  connect(inspector, SIGNAL(inspectorVisible(bool)), a, SLOT(setChecked(bool)));
                  addDockWidget(Qt::RightDockWidgetArea, inspector);
                  }
            updateInspector();
            }
      if (inspector)
            inspector->setVisible(visible);
      a->setChecked(visible);
      }

//---------------------------------------------------------
//   Inspector
//---------------------------------------------------------

Inspector::Inspector(QWidget* parent)
   : QDockWidget(tr("Inspector"), parent)
      {
      setObjectName("inspector");
      setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
      sa = new QScrollArea;
      setWidget(sa);

      _inspectorEdit = false;
      ie             = 0;
      _element       = 0;
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Inspector::closeEvent(QCloseEvent* ev)
      {
      emit inspectorVisible(false);
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Inspector::reset()
      {
      if (ie)
            ie->setElement();
      }

//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void Inspector::setElement(Element* e)
      {
      QList<Element*> el;
      if (e)
            el.append(e);
      setElements(el);
      }

//---------------------------------------------------------
//   setElements
//---------------------------------------------------------

void Inspector::setElements(const QList<Element*>& l)
      {
      if (_inspectorEdit) {          // if within an inspector-originated edit
            _inspectorEdit = false;  // reset flag
            if (_el == l)            // if element is not changing...
                  return;            // ...do nothing
            }
      Element* e = l.isEmpty() ? 0 : l[0];
      if (e == 0 || _element == 0 || (_el != l)) {
            _el = l;
            if (ie)
                  ie->deleteLater();
            ie = 0;
            _element = e;

            if (_element == 0)
                  return;

            bool sameTypes = true;
            foreach(Element* ee, _el) {
                  if (_element->type() != ee->type())
                        sameTypes = false;
                  }
            if (!sameTypes)
                  ie = new InspectorGroupElement(this);
            else {
                  switch(_element->type()) {
                        case Element::FBOX:
                        case Element::TBOX:
                        case Element::VBOX:
                              ie = new InspectorVBox(this);
                              break;
                        case Element::HBOX:
                              ie = new InspectorHBox(this);
                              break;
                        case Element::ARTICULATION:
                              ie = new InspectorArticulation(this);
                              break;
                        case Element::SPACER:
                              ie = new InspectorSpacer(this);
                              break;
                        case Element::NOTE:
                              ie = new InspectorNote(this);
                              break;
                        case Element::REST:
                              ie = new InspectorRest(this);
                              break;
                        case Element::CLEF:
                              ie = new InspectorClef(this);
                              break;
                        case Element::TIMESIG:
                              ie = new InspectorTimeSig(this);
                              break;
                        case Element::KEYSIG:
                              ie = new InspectorKeySig(this);
                              break;
                        case Element::BEAM:
                              ie = new InspectorBeam(this);
                              break;
                        case Element::IMAGE:
                              ie = new InspectorImage(this);
                              break;
                        case Element::LASSO:
                              ie = new InspectorLasso(this);
                              break;
                        case Element::VOLTA_SEGMENT:
                              ie = new InspectorVolta(this);
                              break;
                        case Element::OTTAVA_SEGMENT:
                              ie = new InspectorOttava(this);
                              break;
                        case Element::TRILL_SEGMENT:
                              ie = new InspectorTrill(this);
                              break;
                        case Element::HAIRPIN_SEGMENT:
                              ie = new InspectorHairpin(this);
                              break;
                        case Element::BAR_LINE:
                              ie = new InspectorBarLine(this);
                              break;
                        case Element::JUMP:
                              ie = new InspectorJump(this);
                              break;
                        case Element::MARKER:
                              ie = new InspectorMarker(this);
                              break;
                        case Element::GLISSANDO:
                              ie = new InspectorGlissando(this);
                              break;
                        default:
                              ie = new InspectorElement(this);
                              break;
                        }
                  }
            sa->setWidget(ie);
            setMinimumWidth(ie->width() + sa->frameWidth() * 2 + (width() - sa->width()) + 3);
            }
      _element = e;
      ie->setElement();
      }

//---------------------------------------------------------
//   InspectorElement
//---------------------------------------------------------

InspectorElement::InspectorElement(QWidget* parent)
   : InspectorBase(parent)
      {
      b.setupUi(addWidget());

      iList = {
            { P_COLOR,    0, false, b.color,      b.resetColor   },
            { P_VISIBLE,  0, false, b.visible,    b.resetVisible },
            { P_USER_OFF, 0, false, b.offsetX,    b.resetX       },
            { P_USER_OFF, 1, false, b.offsetY,    b.resetY       }
            };

      mapSignals();
      }

//---------------------------------------------------------
//   InspectorVBox
//---------------------------------------------------------

InspectorVBox::InspectorVBox(QWidget* parent)
   : InspectorBase(parent)
      {
      vb.setupUi(addWidget());

      iList = {
            { P_TOP_GAP,       0, false, vb.topGap,       vb.resetTopGap       },
            { P_BOTTOM_GAP,    0, false, vb.bottomGap,    vb.resetBottomGap    },
            { P_LEFT_MARGIN,   0, false, vb.leftMargin,   vb.resetLeftMargin   },
            { P_RIGHT_MARGIN,  0, false, vb.rightMargin,  vb.resetRightMargin  },
            { P_TOP_MARGIN,    0, false, vb.topMargin,    vb.resetTopMargin    },
            { P_BOTTOM_MARGIN, 0, false, vb.bottomMargin, vb.resetBottomMargin },
            { P_BOX_HEIGHT,    0, false, vb.height,       0                    }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorHBox
//---------------------------------------------------------

InspectorHBox::InspectorHBox(QWidget* parent)
   : InspectorBase(parent)
      {
      hb.setupUi(addWidget());

      iList = {
            { P_TOP_GAP,    0, false, hb.leftGap,  hb.resetLeftGap  },
            { P_BOTTOM_GAP, 0, false, hb.rightGap, hb.resetRightGap },
            { P_BOX_WIDTH,  0, false, hb.width,    0                }
            };

      mapSignals();
      }

//---------------------------------------------------------
//   InspectorArticulation
//---------------------------------------------------------

InspectorArticulation::InspectorArticulation(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      ar.setupUi(addWidget());

      iList = {
            { P_COLOR,               0, false, e.color,        e.resetColor      },
            { P_VISIBLE,             0, false, e.visible,      e.resetVisible    },
            { P_USER_OFF,            0, false, e.offsetX,      e.resetX          },
            { P_USER_OFF,            1, false, e.offsetY,      e.resetY          },
            { P_ARTICULATION_ANCHOR, 0, false, ar.anchor,      ar.resetAnchor    },
            { P_DIRECTION,           0, false, ar.direction,   ar.resetDirection }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorSpacer
//---------------------------------------------------------

InspectorSpacer::InspectorSpacer(QWidget* parent)
   : InspectorBase(parent)
      {
      sp.setupUi(addWidget());

      iList = {
            { P_SPACE, 0, false, sp.height, sp.resetHeight  }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorRest
//---------------------------------------------------------

InspectorRest::InspectorRest(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());
      r.setupUi(addWidget());

      iList = {
            { P_COLOR,          0, false, e.color,         e.resetColor         },
            { P_VISIBLE,        0, false, e.visible,       e.resetVisible       },
            { P_USER_OFF,       0, false, e.offsetX,       e.resetX             },
            { P_USER_OFF,       1, false, e.offsetY,       e.resetY             },
            { P_SMALL,          0, false, r.small,         r.resetSmall         },
            { P_LEADING_SPACE,  0, true,  s.leadingSpace,  s.resetLeadingSpace  },
            { P_TRAILING_SPACE, 0, true,  s.trailingSpace, s.resetTrailingSpace }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorTimeSig
//---------------------------------------------------------

InspectorTimeSig::InspectorTimeSig(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());
      t.setupUi(addWidget());

      iList = {
            { P_COLOR,          0, false, e.color,         e.resetColor         },
            { P_VISIBLE,        0, false, e.visible,       e.resetVisible       },
            { P_USER_OFF,       0, false, e.offsetX,       e.resetX             },
            { P_USER_OFF,       1, false, e.offsetY,       e.resetY             },
            { P_LEADING_SPACE,  0, true,  s.leadingSpace,  s.resetLeadingSpace  },
            { P_TRAILING_SPACE, 0, true,  s.trailingSpace, s.resetTrailingSpace },
            { P_SHOW_COURTESY,  0, false, t.showCourtesy,  t.resetShowCourtesy  }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorKeySig
//---------------------------------------------------------

InspectorKeySig::InspectorKeySig(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());
      k.setupUi(addWidget());

      iList = {
            { P_COLOR,          0, false, e.color,         e.resetColor         },
            { P_VISIBLE,        0, false, e.visible,       e.resetVisible       },
            { P_USER_OFF,       0, false, e.offsetX,       e.resetX             },
            { P_USER_OFF,       1, false, e.offsetY,       e.resetY             },
            { P_LEADING_SPACE,  0, true,  s.leadingSpace,  s.resetLeadingSpace  },
            { P_TRAILING_SPACE, 0, true,  s.trailingSpace, s.resetTrailingSpace },
            { P_SHOW_COURTESY,  0, false, k.showCourtesy,  k.resetShowCourtesy  },
            { P_SHOW_NATURALS,  0, false, k.showNaturals,  k.resetShowNaturals  }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorClef
//---------------------------------------------------------

InspectorClef::InspectorClef(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      s.setupUi(addWidget());
      c.setupUi(addWidget());

      iList = {
            { P_COLOR,          0, false, e.color,         e.resetColor         },
            { P_VISIBLE,        0, false, e.visible,       e.resetVisible       },
            { P_USER_OFF,       0, false, e.offsetX,       e.resetX             },
            { P_USER_OFF,       1, false, e.offsetY,       e.resetY             },
            { P_LEADING_SPACE,  0, true,  s.leadingSpace,  s.resetLeadingSpace  },
            { P_TRAILING_SPACE, 0, true,  s.trailingSpace, s.resetTrailingSpace },
            { P_SHOW_COURTESY,  0, false, c.showCourtesy,  c.resetShowCourtesy  }
            };
      mapSignals();
      }

//---------------------------------------------------------
//   InspectorBarLine
//---------------------------------------------------------

#define BARLINE_TYPE_DEFAULT  -1

QString InspectorBarLine::builtinSpanNames[BARLINE_BUILTIN_SPANS] =
{
      tr("Staff default"), tr("Tick"), tr("Tick alt."), tr("Short"), tr("Short alt.")
};

int InspectorBarLine::builtinSpans[BARLINE_BUILTIN_SPANS][3] =
{//   span From To
      { 0,  0,  0},           // = staff defalt
      { 1, -2,  2},           // tick 1
      { 1, -1,  1},           // tick 2
      { 1,  2,  0},           // short 1 (To depends on staff num. of lines)
      { 1,  1,  0}            // short 2 (To depends on staff num. of lines)
};

InspectorBarLine::InspectorBarLine(QWidget* parent)
   : InspectorBase(parent)
      {
      e.setupUi(addWidget());
      b.setupUi(addWidget());

      iList = {
            { P_COLOR,          0, 0, e.color,    e.resetColor    },
            { P_VISIBLE,        0, 0, e.visible,  e.resetVisible  },
            { P_USER_OFF,       0, 0, e.offsetX,  e.resetX        },
            { P_USER_OFF,       1, 0, e.offsetY,  e.resetY        },
            { P_SUBTYPE,        0, 0, b.type,     b.resetType     },
            { P_BARLINE_SPAN,   0, 0, b.span,     b.resetSpan     },
            };
      mapSignals();
      }

#if 0
//---------------------------------------------------------
//   setElement
//---------------------------------------------------------

void InspectorBarLine::setElement()
      {
      BarLine* bl = static_cast<BarLine*>(inspector->element());
      Measure* m = static_cast<Segment*>(bl->parent())->measure();
      measureBarLineType = m->endBarLineType();

      iElement->setElement(bl);
      type->blockSignals(true);
      span->blockSignals(true);

      type->setEnabled(true);
      // set type: if measure bar line is a repeat, no other type is possible; disable combo
      if (measureBarLineType == START_REPEAT || measureBarLineType == END_REPEAT || measureBarLineType == END_START_REPEAT) {
            type->setEnabled(false);
            type->setCurrentIndex(0);
            }
      // if same as parent measure, set combo to Measure default
      else if (bl->barLineType() == measureBarLineType) {
            type->setCurrentIndex(0);
            }
      // if custom type, set combo to item corresponding to bar line type
      else
            for (int i = 1; i < type->count(); i++)
                  if (type->itemData(i) == bl->barLineType()) {
                        type->setCurrentIndex(i);
                        break;
                        }

      // set span: fix spanTo values depending from staff number of lines
      if(bl->staff()) {
            Staff* st = bl->staff();
            int maxSpanTo = (st->lines()-1) * 2;
            builtinSpans[3][2] = maxSpanTo-2;         // short
            builtinSpans[4][2] = maxSpanTo-1;         // short alt
      }
      else {
            builtinSpans[3][2] = DEFAULT_BARLINE_TO-2;
            builtinSpans[4][2] = DEFAULT_BARLINE_TO-1;
            }
      // if bar line span is same as staff, set to "Staff default"
      if (!bl->customSpan())
            span->setCurrentIndex(0);
      // if custom span, look for corresponding item in combo box
      else {
            int i;
            for(i=1; i < BARLINE_BUILTIN_SPANS; i++)
                  if(bl->span() == builtinSpans[i][0]
                              && bl->spanFrom() == builtinSpans[i][1]
                              && bl->spanTo() == builtinSpans[i][2])
                        break;
            // if no match found among combo items, will set to "Custom"
            span->setCurrentIndex(i);
            }

      type->blockSignals(false);
      span->blockSignals(false);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void InspectorBarLine::apply()
      {
      BarLine*    bl = static_cast<BarLine*>(inspector->element());
      Score*      score = bl->score();

      mscore->getInspector()->setInspectorEdit(true); // this edit is coming from within the inspector itself:
                                                      // do not set element values again
      score->startCmd();

      // type
      int currType = type->itemData(type->currentIndex()).toInt();
      if(currType == BARLINE_TYPE_DEFAULT)
            currType = measureBarLineType;
      if (currType != bl->barLineType())
            score->undoChangeProperty(bl, P_SUBTYPE, currType);
      // if value reverted to measure default, update combo box
      if(!bl->customSubtype())
            type->setCurrentIndex(0);

      // span: determine span, spanFrom and spanTo values for current combo box item
      int currSpan = span->currentIndex();
      int spanStaves, spanFrom, spanTo;
      if(currSpan == 0) {                 // staff default selected
            if(bl->staff()) {                               // if there is a staff
                  Staff* st = bl->staff();                  // use its span values as selected values
                  spanStaves  = st->barLineSpan();
                  spanFrom    = st->barLineFrom();
                  spanTo      = st->barLineTo();
                  }
            else {                                          // if no staff
                  spanStaves  = 1;                          // use values for a 'standard' staff
                  spanFrom    = 0;
                  spanTo      = DEFAULT_BARLINE_TO;
                  }
            }
      else {                              // specific value selected
            if (currSpan == BARLINE_BUILTIN_SPANS+1) {      // selecting custom has no effect:
                  spanStaves  = bl->span();                 // use values from bar line itself
                  spanFrom    = bl->spanFrom();
                  spanTo      = bl->spanTo();
                  }
            else {
                  spanStaves  = builtinSpans[currSpan][0];  // use values from selected combo item
                  spanFrom    = builtinSpans[currSpan][1];
                  spanTo      = builtinSpans[currSpan][2];
                  }
            }
      // if combo values different from bar line's, set them
      if(bl->span() != spanStaves || bl->spanFrom() != spanFrom || bl->spanTo() != spanTo)
            score->undoChangeSingleBarLineSpan(bl, spanStaves, spanFrom, spanTo);
      // if value reverted to staff default, update combo box
      if(!bl->customSpan())
            span->setCurrentIndex(0);

      score->endCmd();
      mscore->endCmd();
      }
#endif

