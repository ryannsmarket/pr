//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2009-2013 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================


#include "pianoview.h"
#include "pianoruler.h"
#include "libmscore/staff.h"
#include "pianokeyboard.h"
#include "libmscore/measure.h"
#include "libmscore/chord.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/slur.h"
#include "libmscore/segment.h"
#include "libmscore/noteevent.h"

namespace Ms {

//static const int MAP_OFFSET = 480;

static const QColor selBoxColor = QColor(255, 128, 0);
static const QColor selBoxColorAlpha = QColor(255, 128, 0, 128);

//const QColor noteDeselected = Qt::blue;
const QColor noteDeselected = QColor(29, 204, 160);
const QColor noteSelected = Qt::yellow;

//const QColor colPianoBg(0x71, 0x8d, 0xbe);
//const QColor colPianoBg(85, 106, 143);
const QColor colPianoBg(54, 54, 54);

const QColor noteDeselectedBlack = noteDeselected.darker(150);
const QColor noteSelectedBlack = noteSelected.darker(150);


static const qreal MIN_DRAG_DIST_SQ = 9;
      
//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

PianoItem::PianoItem(Note* n, NoteEvent* e, PianoView* pianoView)
//   : QGraphicsRectItem(0), _note(n), _event(e), _pianoView(pianoView), isBlack(false)
   : _note(n), _event(e), _pianoView(pianoView), isBlack(false)
      {
//      setFlags(flags() | QGraphicsItem::ItemIsSelectable);
//      setBrush(QBrush());
      updateValues();
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void PianoItem::updateValues()
      {
      Chord* chord = _note->chord();
      int ticks    = chord->duration().ticks();
      int tieLen   = _note->playTicks() - ticks;
      int pitch    = _note->pitch() + _event->pitch();
//      int pitch    = _note->pitch();
      int len      = ticks * _event->len() / 1000 + tieLen;

      int degree = pitch % 12;
      isBlack = degree == 1 || degree == 3 || degree == 6 || degree == 8 || degree == 10;

      //printf("Updating note pitch:%d\n", pitch);
      
      //setSelected(_note->selected());
      
      qreal tix2pix = _pianoView->xZoom();
      int noteHeight = _pianoView->noteHeight();
      
      qreal x1 = _pianoView->tickToPixelX(_note->chord()->tick() + _event->ontime() * ticks / 1000);
//      qreal x1 = (_note->chord()->tick() + _event->ontime() * ticks / 1000 + MAP_OFFSET) * tix2pix;
      qreal y1 = (127 - pitch) * noteHeight;
      
      rect.setRect(x1, y1, len * tix2pix, noteHeight);
            
//      if (isBlack)
//            setRect(x1, y1 + 1, len * tix2pix, noteHeight - 2);
//      else
//            setRect(x1, y1, len * tix2pix, noteHeight);
      
//      setRect(0, 0, len, keyHeight/2);
//
//      setPos(_note->chord()->tick() + _event->ontime() * ticks / 1000 + MAP_OFFSET,
//         pitch2y(pitch) + keyHeight / 4);
//
//      return r | rect().translated(pos());
      }


//---------------------------------------------------------
//   startTick
//---------------------------------------------------------


int PianoItem::startTick()
      {
      Chord* chord = _note->chord();
      int ticks    = chord->duration().ticks();
      return _note->chord()->tick() + _event->ontime() * ticks / 1000;
      }

//---------------------------------------------------------
//   tick length
//---------------------------------------------------------


int PianoItem::tickLength()
      {
      Chord* chord = _note->chord();
      int ticks    = chord->duration().ticks();
      int tieLen   = _note->playTicks() - ticks;
      return ticks * _event->len() / 1000 + tieLen;
      }

//---------------------------------------------------------
//   pitch
//---------------------------------------------------------


int PianoItem::pitch()
      {
      return _note->pitch() + _event->pitch();
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

//void PianoItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
void PianoItem::paint(QPainter* painter)
      {
      painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
//      painter->setRenderHint(QPainter::Antialiasing);
      
      int pitch    = _note->pitch() + _event->pitch();
      int degree = pitch % 12;
      bool isBlack = degree == 1 || degree == 3 || degree == 6 || degree == 8 || degree == 10;
      int roundRadius = 3;
      
//      painter->setPen(pen());
//      painter->setBrush(isSelected() ? Qt::yellow : Qt::blue);
//      QColor outlineColor = isSelected() ? noteSelectedBlack : noteDeselectedBlack;
      
//      if (isBlack)
////            painter->setPen(QPen(outlineColor, 1));
//            painter->setBrush(isSelected() ? noteSelectedBlack : noteDeselectedBlack);
//      else
////            painter->setPen(QPen(outlineColor, 1));
//            painter->setBrush(isSelected() ? noteSelected : noteDeselected);
      

      QColor noteColor = _note->selected() ? noteSelected : noteDeselected;
      painter->setBrush(noteColor);
      
      
      
//      painter->drawRect(boundingRect());
//      painter->setPen(QPen(outlineColor, 1));
//      painter->setPen(isBlack ? QPen(Qt::black, 1, Qt::DotLine) : Qt::NoPen);
      
//      painter->setPen(isBlack ? QPen(Qt::black, 1, Qt::SolidLine) : Qt::NoPen);
      painter->setPen(Qt::NoPen);
      QRectF bounds = rect;
      painter->drawRoundedRect(bounds, roundRadius, roundRadius);
      
//      const QString pitchNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
      const QString pitchNames[] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};

      //Pitch name
      if (bounds.width() >= 20 && bounds.height() >= 11)
            {
            QRectF textRect(bounds.x() + 2, bounds.y() - 1, bounds.width() - 6, bounds.height() + 1);
            QRectF textHiliteRect(bounds.x() + 3, bounds.y(), bounds.width() - 6, bounds.height());
            
            QFont f("FreeSans",  8);
            painter->setFont(f);

            //Note name
            painter->setPen(QPen(noteColor.lighter(130)));
//            painter->drawText(bounds.x() + 3, bounds.y()+1, bounds.width(), bounds.height(),
            painter->drawText(textHiliteRect,
                    Qt::AlignLeft | Qt::AlignTop, pitchNames[pitch % 12]);

            painter->setPen(QPen(noteColor.darker(180)));
            painter->drawText(textRect,
                    Qt::AlignLeft | Qt::AlignTop, pitchNames[pitch % 12]);
            
            
            //Black key hint
            if (isBlack && bounds.width() >= 30)
                  {
                  painter->setPen(QPen(noteColor.lighter(130)));
                  painter->drawText(textHiliteRect,
                          Qt::AlignRight | Qt::AlignTop, QString("#"));
                  
                  painter->setPen(QPen(noteColor.darker(180)));
                  painter->drawText(textRect,
                          Qt::AlignRight | Qt::AlignTop, QString("#"));
                  }
            }
      
//      if (isBlack) {
//            painter->setBrush(isSelected() ? noteSelected : noteDeselected);
//            painter->drawRoundedRect(bounds.x() + 2, 
//                    bounds.y() + 2, 
//                    bounds.width() - 4, 
//                    bounds.height() - 4, 
//                    roundRadius, roundRadius);
//            
//            }
      }


//---------------------------------------------------------
//   PianoView
//---------------------------------------------------------

PianoView::PianoView()
   : QGraphicsView()
      {
      setFrameStyle(QFrame::NoFrame);
      setLineWidth(0);
      setMidLineWidth(0);
      setScene(new QGraphicsScene);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      setResizeAnchor(QGraphicsView::AnchorUnderMouse);
      setMouseTracking(true);
//      setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
//      setDragMode(QGraphicsView::RubberBandDrag);
      _timeType = TType::TICKS;
//      magStep   = 0;
      staff     = 0;
      chord     = 0;
      _noteHeight = DEFAULT_KEY_HEIGHT;
      //Initialize to something that will give us a zoom of around .1
//      _xZoom = (int)(log2(0.1) / log2(X_ZOOM_RATIO));  
      _xZoom = 0.1;
      dragStarted = false;
      mouseDown = false;

      }

//---------------------------------------------------------
//   ~PianoView
//---------------------------------------------------------

PianoView::~PianoView()
      {
      clearNoteData();
      }


//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void PianoView::drawBackground(QPainter* p, const QRectF& r)
      {
      if (staff == 0)
            return;
      Score* _score = staff->score();
      setFrameShape(QFrame::NoFrame);

      QColor colGutter = colPianoBg.darker(150);
      QColor colBlackKeyBg = colPianoBg.darker(130);
      QColor colGridLineMajor = colPianoBg.darker(180);
      QColor colGridLineMinor = colPianoBg.darker(160);
      QPen penLineMajor = QPen(colGridLineMajor, 2.0, Qt::SolidLine);
      QPen penLineMinor = QPen(colGridLineMinor, 1.0, Qt::SolidLine);

      //Ticks to pixels
      //qreal ticks2Pix = _xZoom;
      
      QRectF r1;
      r1.setCoords(-1000000.0, 0.0, tickToPixelX(0), 1000000.0);
      QRectF r2;
      r2.setCoords(tickToPixelX(ticks), 0.0, 1000000.0, 1000000.0);
      
      p->fillRect(r, colPianoBg);
      if (r.intersects(r1))
            p->fillRect(r.intersected(r1), colGutter);
      if (r.intersects(r2))
            p->fillRect(r.intersected(r2), colGutter);

      //
      // draw horizontal grid lines
      //
      qreal y1 = r.y();
      qreal y2 = y1 + r.height();
      qreal x1 = r.x();
      qreal x2 = x1 + r.width();

      int topPitch = ceil((_noteHeight * 128 - y1) / _noteHeight);
      int bmPitch = floor((_noteHeight * 128 - y2) / _noteHeight);
      
      //MIDI notes span [0, 127] and map to pitches starting at C-1
      for (int pitch = bmPitch; pitch <= topPitch; ++pitch) {
            int y = (127 - pitch) * _noteHeight;

            //int octave = pitch / 12;
            int degree = pitch % 12;
            if (degree == 1 || degree == 3 || degree == 6 || degree == 8 || degree == 10)
            {
                  qreal px0 = qMax(r.x(), (qreal)tickToPixelX(0));
                  qreal px1 = qMin(r.x() + r.width(), (qreal)tickToPixelX(ticks));
                  QRectF hbar;
                  
                  hbar.setCoords(px0, y, px1, y + _noteHeight);
                  p->fillRect(hbar, colBlackKeyBg);
            }
            

            //Lines between rows
            p->setPen(degree == 0 ? penLineMajor : penLineMinor);
            p->drawLine(QLineF(x1, y + _noteHeight, x2, y + _noteHeight));
      
            }

      //
      // draw vertical grid lines
      //
      Pos pos1(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(x1), 0), TType::TICKS);
      Pos pos2(_score->tempomap(), _score->sigmap(), qMax(pixelXToTick(x2), 0), TType::TICKS);
      
      int bar1, bar2, beat, tick;
      pos1.mbt(&bar1, &beat, &tick);
//      printf("bar1: %d  beat:%d  tick:%d\n", bar1, beat, tick);
      pos2.mbt(&bar2, &beat, &tick);
//      printf("bar2: %d  beat:%d  tick:%d\n", bar2, beat, tick);
      
      //Draw bar lines
      const int minBeatGap = 20;
      for (int bar = bar1; bar <= bar2; ++bar) {
            Pos barPos(_score->tempomap(), _score->sigmap(), bar, 0, 0);

            //Beat lines
            int beatsInBar = barPos.timesig().timesig().numerator();
            int ticksPerBeat = barPos.timesig().timesig().beatTicks();
            int beatSkip = ceil(minBeatGap / (ticksPerBeat * _xZoom));
            //Round up to next power of 2
            beatSkip = (int)pow(2, ceil(log(beatSkip)/log(2)));
            
            for (int beat = 0; beat < beatsInBar; beat += beatSkip)
                  {
                  Pos beatPos(_score->tempomap(), _score->sigmap(), bar, beat, 0);
                  double x = tickToPixelX(beatPos.time(TType::TICKS));
                  p->setPen(penLineMinor);
                  p->drawLine(x, y1, x, y2);
                  }
            
            
            //Bar line
            double x = tickToPixelX(barPos.time(TType::TICKS));
            p->setPen(x > 0 ? penLineMajor : QPen(Qt::black, 2.0));
            p->drawLine(x, y1, x, y2);
            }
      
      
      //Draw notes
      for (int i = 0; i < noteList.size(); ++i)
      {
            noteList[i]->paint(p);
      }

      
      //Draw drag box
      if (dragStarted)
            {
            int minX = qMin(mouseDownPos.x(), lastMousePos.x());
            int minY = qMin(mouseDownPos.y(), lastMousePos.y());
            int maxX = qMax(mouseDownPos.x(), lastMousePos.x());
            int maxY = qMax(mouseDownPos.y(), lastMousePos.y());
            QRectF rect(minX, minY, maxX - minX + 1, maxY - minY + 1);
            
            p->setPen(QPen(selBoxColor, 2));
            p->setBrush(QBrush(selBoxColorAlpha, Qt::SolidPattern));
            p->drawRect(rect);
            }
      }

//---------------------------------------------------------
//   createLocators
//---------------------------------------------------------

void PianoView::createLocators()
      {
      static const QColor lcColors[3] = { Qt::red, Qt::blue, Qt::blue };
      for (int i = 0; i < 3; ++i) {
            locatorLines[i] = new QGraphicsLineItem(QLineF(0.0, 0.0, 0.0, _noteHeight * 128));
            QPen pen(lcColors[i]);
            pen.setWidth(2);
            locatorLines[i]->setPen(pen);
            locatorLines[i]->setZValue(1000+i);       // set stacking order
            locatorLines[i]->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
            scene()->addItem(locatorLines[i]);
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianoView::moveLocator(int i)
      {
      
      if (_locator[i].valid()) {
            locatorLines[i]->setVisible(true);
//            qreal x = (_locator[i].time(TType::TICKS) + MAP_OFFSET) * _xZoom;
            qreal x = tickToPixelX(_locator[i].time(TType::TICKS));
            locatorLines[i]->setPos(QPointF(x, 0.0));
            }
      else
            locatorLines[i]->setVisible(false);
      }



//---------------------------------------------------------
//   pixelXToTick
//---------------------------------------------------------

int PianoView::pixelXToTick(int pixX) {
      return (int)(pixX / _xZoom) - MAP_OFFSET; 
      }


//---------------------------------------------------------
//   tickToPixelX
//---------------------------------------------------------

int PianoView::tickToPixelX(int tick) { 
      return (int)(tick + MAP_OFFSET) *  _xZoom; 
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void PianoView::wheelEvent(QWheelEvent* event)
      {
      int step = event->delta() / 120;

      if (event->modifiers() == Qt::ControlModifier) 
            {
            //Horizontal zoom
            
            //QPointF mouseScene = mapToScene(event->pos());
            QRectF viewRect = mapToScene(viewport()->geometry()).boundingRect();
//            printf("wheelEvent viewRect %f %f %f %f\n", viewRect.x(), viewRect.y(), viewRect.width(), viewRect.height());
//            printf("mousePos:%d %d\n", event->x(), event->y());
            
//            qreal mouseXTick = event->x() / _xZoom - MAP_OFFSET;
            int mouseXTick = pixelXToTick(event->x() + (int)viewRect.x());
            //int viewportX = event->x() - viewRect.x();
//            printf("mouseTick:%d viewportX:%d\n", mouseXTick, viewportX);
//            printf("mouseTick:%d\n", mouseXTick);
            
            _xZoom *= pow(X_ZOOM_RATIO, step);
            emit xZoomChanged(_xZoom);
            
            updateBoundingSize();
            updateNotes();
            
            int mousePixX = tickToPixelX(mouseXTick);
  //          printf("mousePixX:%d \n", mousePixX);
            horizontalScrollBar()->setValue(mousePixX - event->x());

            scene()->update();
            }
      else if (event->modifiers() == Qt::ShiftModifier) 
            {
            //Horizontal scroll
            QWheelEvent we(event->pos(), event->delta(), event->buttons(), 0, Qt::Horizontal);
            QGraphicsView::wheelEvent(&we);
            }
      else if (event->modifiers() == 0)
            {
            //Vertical scroll
            QGraphicsView::wheelEvent(event);
            }
      else if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier))
            {
            //Vertical zoom
            QRectF viewRect = mapToScene(viewport()->geometry()).boundingRect();
            //QPointF mouseScenePos = mapToScene(event->pos());
            qreal mouseYNote = (event->y() + (int)viewRect.y()) / (qreal)_noteHeight;
            
            _noteHeight = qMax(qMin(_noteHeight + step, MAX_KEY_HEIGHT), MIN_KEY_HEIGHT);
            emit noteHeightChanged(_noteHeight);
            
            updateBoundingSize();
            updateNotes();
            
            int mousePixY = (int)(mouseYNote * _noteHeight);
  //          printf("mousePixX:%d \n", mousePixX);
            verticalScrollBar()->setValue(mousePixY - event->y());
            
            
            scene()->update();
            }
      }


//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void PianoView::mousePressEvent(QMouseEvent* event)
      {
//      qDebug("mousePressEvent %d %d", event->x(), event->y());
      //QGraphicsView::mousePressEvent(event);
      
      mouseDown = true;
      mouseDownPos = mapToScene(event->pos());
      lastMousePos = mouseDownPos;
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void PianoView::mouseReleaseEvent(QMouseEvent* /*event*/)
      {
//      qDebug("mouseReleaseEvent %d %d", event->x(), event->y());
      //QGraphicsView::mouseReleaseEvent(event);
//      bool bnLeft = event->buttons() & Qt::LeftButton;
//      bool bnMid = event->buttons() & Qt::MidButton;
//      bool bnRight = event->buttons() & Qt::RightButton;

      int modifiers = QGuiApplication::keyboardModifiers();
      bool bnShift = modifiers & Qt::ShiftModifier;
      bool bnCtrl = modifiers & Qt::ControlModifier;
//      bool bnAlt = modifiers & Qt::AltModifier;
      
      NoteSelectType selType = bnShift ? (bnCtrl ? NoteSelectType::SUBTRACT : NoteSelectType::XOR)
              : (bnCtrl ? NoteSelectType::ADD : NoteSelectType::REPLACE);

      if (dragStarted)
            {
            //Update selection
            qreal minX = qMin(mouseDownPos.x(), lastMousePos.x());
            qreal minY = qMin(mouseDownPos.y(), lastMousePos.y());
            qreal maxX = qMax(mouseDownPos.x(), lastMousePos.x());
            qreal maxY = qMax(mouseDownPos.y(), lastMousePos.y());

            int startTick = pixelXToTick((int)minX);
            int endTick = pixelXToTick((int)maxX);
            int lowPitch = (int)floor(128 - maxY / noteHeight());
            int highPitch = (int)ceil(128 - minY / noteHeight());

            selectNotes(startTick, endTick, lowPitch, highPitch, selType);
            
            dragStarted = false;
            }
      else
            {
            int startTick = pixelXToTick((int)mouseDownPos.x());
            int lowPitch = (int)floor(128 - mouseDownPos.y() / noteHeight());
            
            selectNotes(startTick, startTick + 1, lowPitch, lowPitch, selType);
            }
      
      
      mouseDown = false;
      }

void PianoView::selectNotes(int startTick, int endTick, int lowPitch, int highPitch, NoteSelectType selType)
      {

      qDebug("Selection bounds t0:%d t1:%d lo:%d hi:%d", startTick, endTick, lowPitch, highPitch);


      qDebug("SelectNoteType type %d", (int)selType);

      Score* score = staff->score();
      Selection selection(score);

      for (int i = 0; i < noteList.size(); ++i)
            {
            PianoItem* pi = noteList[i];
            int ts = pi->startTick();
            int tLen = pi->tickLength();

            int pitch = pi->note()->pitch();


            bool inBounds = ts + tLen >= startTick && ts <= endTick 
                  && pitch >= lowPitch && pitch <= highPitch;

            bool sel;
            switch (selType)
                  {
                  default:
                  case NoteSelectType::REPLACE:
                        sel = inBounds;
                        break;
                  case NoteSelectType::XOR:
                        sel = inBounds != pi->note()->selected();
                        break;
                  case NoteSelectType::ADD:
                        sel = inBounds || pi->note()->selected();
                        break;
                  case NoteSelectType::SUBTRACT:
                        sel = !inBounds && pi->note()->selected();
                        break;
                  }

            printf("Note t0:%d t1:%d pitch:%d inBounds:%d sel:%d\n", ts, ts + tLen, pitch, inBounds, sel);

//                  item->setSelected(pi->note()->selected());
            //item->setSelected(sel);
            //pi->note()->setSelected(sel);
            if (sel)
                  {
                  selection.add(pi->note());
                  }
            }

//
//            for (QGraphicsItem* item : scene()->items())
//                  if (item->type() == PianoItemType)
//                        {
//                        }
//      scene()->items()


//      qreal tix2pix = _pianoView->xZoom();
//      int noteHeight = _pianoView->noteHeight();

//            staff->score()->setUpdateAll();
//            staff->score()->update();
      score->setSelection(selection);
//      score->update();
      for (MuseScoreView* view : score->getViewer())
            view->updateAll();
      
      //updateNotes();
      scene()->update();
      }


//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void PianoView::mouseMoveEvent(QMouseEvent* event)
      {
      lastMousePos = mapToScene(event->pos());
            
      //qDebug("mouseMoveEvent %d %d", event->x(), event->y());
      if (mouseDown && !dragStarted)
            {
            qreal dx = lastMousePos.x() - mouseDownPos.x();
            qreal dy = lastMousePos.y() - mouseDownPos.y();
            if (dx * dx + dy * dy >= MIN_DRAG_DIST_SQ)
                  {
                  dragStarted = true;
                  }
            }

      if (dragStarted)
            {
            
            scene()->update();
            }
      

      //Update mouse tracker      
      QPointF p(mapToScene(event->pos()));
      int pitch = (int)((_noteHeight * 128 - p.y()) / _noteHeight);
//      int pitch = (_noteHeight * 128 - event->y()) / _noteHeight;
      emit pitchChanged(pitch);

      //int tick = event->x() / _xZoom - MAP_OFFSET;
      int tick = pixelXToTick(p.x());
//      int tick = int(p.x()) -480;
      if (tick < 0) {
            tick = 0;
            trackingPos.setTick(tick);
            trackingPos.setInvalid();
            }
      else
            trackingPos.setTick(tick);
      emit trackingPosChanged(trackingPos);
      }
      
//      QGraphicsView::mouseMoveEvent(event);

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void PianoView::leaveEvent(QEvent* event)
      {
      emit pitchChanged(-1);
      trackingPos.setInvalid();
      emit trackingPosChanged(trackingPos);
      QGraphicsView::leaveEvent(event);
      }

//---------------------------------------------------------
//   ensureVisible
//---------------------------------------------------------

void PianoView::scrollToTick(int tick)
      {
      QRectF rect = mapToScene(viewport()->geometry()).boundingRect();
//      printf("scrollToTick tick:%d \n", tick);
//      printf("scrollToTick x:%f y:%f width:%f height:%f \n", rect.x(), rect.y(), rect.width(), rect.height());
      
//      int ypos = verticalScrollBar()->value();
//      printf("ensureVisible tick:%d ypos:%d \n", tick, ypos);
      //QRectF rect = sceneRect();
      //QRectF rect = mapToScene(viewport()->geometry()).boundingRect();
//      printf("ensureVisible x:%f y:%f width:%f height:%f \n", rect.x(), rect.y(), rect.width(), rect.height());
      
      //QPointF pt = mapToScene(0, height() / 2);
      //qreal xpos = (tick + MAP_OFFSET) * _xZoom;
      qreal xpos = tickToPixelX(tick);
//      qreal ypos = matrix().dy();
//      QGraphicsView::ensureVisible(xpos, ypos, 240.0, 1.0);

//      horizontalScrollBar()->setValue(qMax(xpos - rect.width() / 2, 0.0));
      
      
      qreal margin = rect.width() / 2;
      if (xpos < rect.x() + margin)
            horizontalScrollBar()->setValue(qMax(xpos - margin, 0.0));
      else if (xpos >= rect.x() + rect.width() - margin)
            horizontalScrollBar()->setValue(qMax(xpos - rect.width() + margin, 0.0));
            
//      emit xposChanged(xpos);
      
//      horizontalScrollBar()->setValue(qMax(xpos - margin, 0.0));
//      tick += MAP_OFFSET;
//      QPointF pt = mapToScene(0, height() / 2);
//      QGraphicsView::ensureVisible(qreal(tick), pt.y(), 240.0, 1.0);
//      verticalScrollBar()->setValue(ypos);
      }

//---------------------------------------------------------
//   updateBoundingSize
//---------------------------------------------------------
void PianoView::updateBoundingSize()
      {
//      printf("PianoView::updateBoundingSize\n");
//      
//      qreal hbarPos = horizontalScrollBar()->value();
//      qreal vbarPos = verticalScrollBar()->value();
//
//      printf("hbarPos %f vbarPos%f\n", hbarPos, vbarPos);
      
      Measure* lm = staff->score()->lastMeasure();
      ticks       = lm->tick() + lm->ticks();
      scene()->setSceneRect(0.0, 0.0, 
              double((ticks + MAP_OFFSET * 2) * _xZoom),
              _noteHeight * 128);
      
//      qreal hbarPosP = horizontalScrollBar()->value();
//      qreal vbarPosP = verticalScrollBar()->value();
//      printf("post hbarPos %f vbarPos%f\n", hbarPosP, vbarPosP);
//      
//      horizontalScrollBar()->setValue(hbarPos);
//      verticalScrollBar()->setValue(vbarPos);
      }

//---------------------------------------------------------
//   setStaff
//---------------------------------------------------------

void PianoView::setStaff(Staff* s, Pos* l)
      {
//      printf("PianoView::setStaff\n");
      _locator = l;
      
      //bool reentry = false;
      if (staff == s)
            {
            //reentry = true;
            return;
            }
      
      staff    = s;
      setEnabled(staff != nullptr);
      if (!staff) {
            scene()->blockSignals(true);  // block changeSelection()
            scene()->clear();
            clearNoteData();
            scene()->blockSignals(false);
            return;
            }

      trackingPos.setContext(staff->score()->tempomap(), staff->score()->sigmap());
//      Measure* lm = staff->score()->lastMeasure();
//      ticks       = lm->tick() + lm->ticks();
//      scene()->setSceneRect(0.0, 0.0, double(ticks + MAP_OFFSET * 2), keyHeight * 75);
      updateBoundingSize();

      updateNotes();

//      if (reentry)
//            return;
      
      //
      // move to something interesting
      //
//      QList<QGraphicsItem*> items = scene()->items();
      
      QRectF boundingRect;
      bool brInit = false;
      QRectF boundingRectSel;
      bool brsInit = false;
      
//      foreach (QGraphicsItem* item, items) {
//            if (item->type() == PianoItemType)
//                  {
      foreach (PianoItem* item, noteList) {
            if (!brInit)
                  {
                  boundingRect = item->boundingRect();
                  brInit = true;
                  }
            else
                  {
                  boundingRect |= item->boundingRect();
                  }

            if (item->note()->selected())
                  {
                  if (!brsInit)
                        {
                        boundingRectSel = item->boundingRect();
                        brsInit = true;
                        }
                  else
                        {
                        boundingRectSel |= item->boundingRect();
                        }
                  }
                  
                  //PianoItem *pi = (PianoItem)item;
//                  item->isSelected();
//                  boundingRect |= item->mapToScene(item->boundingRect()).boundingRect();
//                  }
            }
      
      if (brsInit)
            {
            horizontalScrollBar()->setValue(boundingRectSel.x());
            verticalScrollBar()->setValue(qMax(boundingRectSel.y() - boundingRectSel.height() / 2, 0.0));
            }
      else if (brInit)
            {
            horizontalScrollBar()->setValue(boundingRect.x());
            verticalScrollBar()->setValue(qMax(boundingRect.y() - boundingRect.height() / 2, 0.0));
            }
      else
            {
            QRectF rect = mapToScene(viewport()->geometry()).boundingRect();
            
            horizontalScrollBar()->setValue(0);
            verticalScrollBar()->setValue(qMax(rect.y() - rect.height() / 2, 0.0));
            }
      //centerOn(boundingRect.center());
//      horizontalScrollBar()->setValue(0);
//      verticalScrollBar()->setValue(qMax(boundingRect.y() - boundingRect.height() / 2, 0.0));
      }

//---------------------------------------------------------
//   addChord
//---------------------------------------------------------

void PianoView::addChord(Chord* chord)
      {
      for (Chord* c : chord->graceNotes())
            addChord(c);
      for (Note* note : chord->notes()) {
            if (note->tieBack())
                  continue;
            for (NoteEvent& e : note->playEvents())
                  {
                  //scene()->addItem(new PianoItem(note, &e, this));
                  noteList.append(new PianoItem(note, &e, this));
                  }
            }
      }

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void PianoView::updateNotes()
      {
//      printf("PianoView::updateNotes()\n");
      
      scene()->blockSignals(true);  // block changeSelection()
      scene()->clearFocus();
      scene()->clear();
      clearNoteData();
      createLocators();

      int staffIdx   = staff->idx();
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      SegmentType st = SegmentType::ChordRest;
      for (Segment* s = staff->score()->firstSegment(st); s; s = s->next1(st)) {
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = s->element(track);
                  if (e && e->isChord())
                        addChord(toChord(e));
                  }
            }
      for (int i = 0; i < 3; ++i)
            moveLocator(i);
      scene()->blockSignals(false);
      
      scene()->update(sceneRect());
      }

//---------------------------------------------------------
//   updateNotes
//---------------------------------------------------------

void PianoView::clearNoteData()
      {
//      printf("PianoView::updateNotes()\n");
      for (int i = 0; i < noteList.size(); ++i)
            {
            delete noteList[i];
            }
      
      noteList.clear();
      }


//---------------------------------------------------------
//   getSelectedItems
//---------------------------------------------------------

QList<PianoItem*> PianoView::getSelectedItems()
      {
      QList<PianoItem*> list;
      for (int i = 0; i < noteList.size(); ++i)
            {
            if (noteList[i]->note()->selected())
                  {
                  list.append(noteList[i]);
                  }
            }
      return list;
      }

//---------------------------------------------------------
//   getItems
//---------------------------------------------------------

QList<PianoItem*> PianoView::getItems()
      {
      QList<PianoItem*> list;
      for (int i = 0; i < noteList.size(); ++i)
            {
            list.append(noteList[i]);
            }
      return list;
      }

}

