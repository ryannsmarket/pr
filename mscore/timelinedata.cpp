#include "timelinedata.h"
#include "musescore.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/system.h"
#include "libmscore/part.h"

namespace Ms {

//---------------------------------------------------------
//   TimelineDataLabel
//---------------------------------------------------------

TimelineDataLabel::TimelineDataLabel(TimelineDataLabels *view, QString text, int nMeta)
      : TimelineLabel(view, text, view->getParent()->getParent()->getFont(), nMeta, view->getParent()->getParent()->cellHeight())
      {

      }

//---------------------------------------------------------
//   updateLabelWidths
//---------------------------------------------------------

void TimelineDataLabels::updateLabelWidths(int newWidth)
      {
      for (TimelineDataLabel* label : _labels)
            label->updateWidth(newWidth);

      // -1 makes sure the rect border is within view
      // -2 makes sure the sDcene rect is always smaller than the view rect, thus no scrollbar is displayed
      setSceneRect(-1, -1, newWidth - 2, getParent()->getParent()->cellHeight() * _labels.length());
      }

//---------------------------------------------------------
//   TimelineDataLabels
//---------------------------------------------------------

TimelineDataLabels::TimelineDataLabels(TimelineData *parent)
      : QGraphicsView(parent)
      {
      setScene(new QGraphicsScene);
      scene()->setBackgroundBrush(Qt::lightGray);
      setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

      connect(parent, SIGNAL(splitterMoved(int, int)), this, SLOT(updateLabelWidths(int)));
      }

//---------------------------------------------------------
//   getParent
//---------------------------------------------------------

TimelineData* TimelineDataLabels::getParent()
      {
      return static_cast<TimelineData*>(parent());
      }

//---------------------------------------------------------
//   updateLabels
//---------------------------------------------------------

void TimelineDataLabels::updateLabels()
      {
      _labels.clear();
      scene()->clear();

      if (!score())
            return;

      QList<Part*> parts = score()->parts();

      int nMeta = 0;
      for (Part* part : parts) {
            for (Staff* staff : *(part->staves())) {
                  TimelineDataLabel* instrumentLabel = new TimelineDataLabel(this, staff->partName(), nMeta);
                  _labels.append(instrumentLabel);
                  scene()->addItem(instrumentLabel);
                  nMeta++;
                  }
            }

      updateLabelWidths(getParent()->sizes()[0]);
      }

//---------------------------------------------------------
//   score
//---------------------------------------------------------

Score* TimelineDataLabels::score()
      {
      return getParent()->score();
      }

//---------------------------------------------------------
//   TimelineDataCell
//---------------------------------------------------------

TimelineDataGridCell::TimelineDataGridCell(Measure* measure, int staffIdx, int measureIdx)
      : _measure(measure), _staffIdx(staffIdx), _measureIdx(measureIdx)
      {
      setPen(QPen(QColor(Qt::black)));

      if (isFilled())
            setBrush(QBrush(QColor(Qt::gray)));
      else
            setBrush(QBrush(QColor(Qt::lightGray)));
      }

//---------------------------------------------------------
//   infoPair
//---------------------------------------------------------

QPair<Measure*, int> TimelineDataGridCell::infoPair()
      {
      QPair<Measure*, int> info(_measure, _staffIdx);
      return info;
      }

//---------------------------------------------------------
//   getMeasureToSelect
//---------------------------------------------------------

Measure* TimelineDataGridCell::getMeasureToSelect()
      {
      if (_measure->mmRest())
            return _measure->mmRest();
      else if (_measure->mmRestCount() == -1) // Inside an MMRest
            return _measure->prevMeasureMM();
      else
            return _measure;
      }

//---------------------------------------------------------
//   TimelineDataGrid
//---------------------------------------------------------

TimelineDataGrid::TimelineDataGrid(TimelineData *parent)
      : QGraphicsView(parent)
      {
      setScene(new QGraphicsScene);
      scene()->setBackgroundBrush(Qt::lightGray);
      setAlignment(Qt::Alignment((Qt::AlignLeft | Qt::AlignTop)));

      setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
      setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

      setMouseTracking(true);
      }

//---------------------------------------------------------
//   getCell
//---------------------------------------------------------

TimelineDataGridCell* TimelineDataGrid::getCell(int row, int column)
      {
      int gridIdx = column * _nStaves + row;
//      qDebug() << gridIdx;
      if (_grid.length() >= gridIdx + 1)
            return _grid[gridIdx];
      else
            return nullptr;
      }

//---------------------------------------------------------
//   populateGrid
//---------------------------------------------------------

void TimelineDataGrid::populateGrid()
      {
      _grid.clear();
      Score* localScore = score();

      if (localScore == nullptr)
            return;

      _nStaves = localScore->nstaves();
      int measureIdx = 0;
      for (Measure* measure = score()->firstMeasure(); measure; measure = measure->nextMeasure()) {
            for (int staffIdx = 0; staffIdx < _nStaves; staffIdx++) {
                  TimelineDataGridCell* newCell = new TimelineDataGridCell(measure, staffIdx, measureIdx);
                  newCell->setRect(cellWidth() * measureIdx, cellHeight() * staffIdx, cellWidth(), cellHeight());
                  newCell->setZValue(ZValues::CELL);

                  _grid.append(newCell);

                  scene()->addItem(newCell);
                  }
            measureIdx++;
            }

      // Use 1 to make sure rect borders are in the sceneRect
      setSceneRect(-1, -1, cellWidth() * measureIdx + 1, cellHeight() * _nStaves + 1);
      }

//---------------------------------------------------------
//   updateView
//---------------------------------------------------------

void TimelineDataGrid::updateView()
      {
      if (!scoreView() || !score())
            return;

      if (_visibleCellsPathItem)
            scene()->removeItem(_visibleCellsPathItem);
      if (_nonVisibleCellsPathItem)
            scene()->removeItem(_nonVisibleCellsPathItem);

      QList<TimelineDataGridCell*> visibleCells = getVisibleCells();
      QPainterPath visiblePath = createCellPath(visibleCells);

      QPainterPath nonVisiblePath = QPainterPath();
      nonVisiblePath.setFillRule(Qt::WindingFill);
      nonVisiblePath.addRect(gridBounds());
      nonVisiblePath -= visiblePath;

      _visibleCellsPathItem = new QGraphicsPathItem(visiblePath.simplified());
      _nonVisibleCellsPathItem = new QGraphicsPathItem(nonVisiblePath.simplified());

      _visibleCellsPathItem->setPen(QPen(Qt::yellow));
      _visibleCellsPathItem->setBrush(Qt::NoBrush);
      _visibleCellsPathItem->setZValue(ZValues::VIEW);
      QColor transparentColor = QColor(Qt::transparent);
      transparentColor.setAlpha(100);
      _nonVisibleCellsPathItem->setPen(QPen(transparentColor));
      _nonVisibleCellsPathItem->setBrush(QBrush(transparentColor));
      _visibleCellsPathItem->setZValue(ZValues::VIEW);

      scene()->addItem(_nonVisibleCellsPathItem);
      scene()->addItem(_visibleCellsPathItem);
      }

//---------------------------------------------------------
//   getVisibleCells
//---------------------------------------------------------

QList<TimelineDataGridCell*> TimelineDataGrid::getVisibleCells()
      {
      QList<TimelineDataGridCell*> visibleCells;
      // Converts the viewable area into a rect mapped to the scoreview
      QRectF canvas = QRectF(scoreView()->matrix().inverted().mapRect(scoreView()->geometry()));

      QSet<QPair<Measure*, int>> visibleMeasures; // QPair<measure, staffIdx> used for _grid lookup

      Score* localScore = score();

      // Fill visibleMeasures
      Measure* currMeasure = localScore->firstMeasure();
      for (; currMeasure; currMeasure = currMeasure->nextMeasure()) {

            if (currMeasure->mmRest() && localScore->styleB(Sid::createMultiMeasureRests)) {
                  for (int staffIdx = 0; staffIdx < localScore->nstaves(); staffIdx++) {
                        QRectF bounds = staffMeasureBounds(currMeasure->mmRest(), staffIdx, localScore);
                        if (!canvas.intersects(bounds))
                              continue;

                        int measuresToAdd = currMeasure->mmRest()->mmRestCount();
                        for (int i = 0; i < measuresToAdd; i++, currMeasure = currMeasure->nextMeasure()) {
                              QPair<Measure*, int> visibleMeasure(currMeasure, staffIdx);
                              visibleMeasures.insert(visibleMeasure);
                              }
                        }

                  // mmrest goes to the end of the score
                  if (!currMeasure)
                        break;
                  }
            else {
                  for (int staffIdx = 0; staffIdx < localScore->nstaves(); staffIdx++) {
                        QRectF bounds = staffMeasureBounds(currMeasure, staffIdx, localScore);

                        if (canvas.intersects(bounds)) {
                              QPair<Measure*, int> visibleMeasure(currMeasure, staffIdx);
                              visibleMeasures.insert(visibleMeasure);
                              }
                        }
                  }
            }

      // Use the visibleMeasures list to generate a list of visibleCells--subset of _grid
      for (TimelineDataGridCell* cell : _grid) {
            if (visibleMeasures.contains(cell->infoPair()))
                  visibleCells.append(cell);
            }

      return visibleCells;
      }

//---------------------------------------------------------
//   gridBounds
//---------------------------------------------------------

QRectF TimelineDataGrid::gridBounds() {
      return QRectF(0, 0, score()->nmeasures() * cellWidth(), score()->nstaves() * cellHeight());
      }

//---------------------------------------------------------
//   staffMeasureBounds
//   Returns QRect(0, 0, 0, 0) if no system is found for the measure
//   or if the staff doesn't show
//---------------------------------------------------------

QRectF TimelineDataGrid::staffMeasureBounds(Measure* measure, int staffIdx, Score* localScore) {
      System* system = measure->system();
      if (!system || !localScore->staff(staffIdx)->show())
            return QRectF();

      QRectF staffRect = QRectF(system->canvasBoundingRect().left(),
                                system->staffCanvasYpage(staffIdx),
                                system->width(),
                                system->staff(staffIdx)->bbox().height());

      // Staves with one line return bbox height of 0. Set to 1 for intersection
      if (localScore->staff(staffIdx)->lines(measure->tick()) < 2)
            staffRect.setHeight(1);

      return measure->canvasBoundingRect().intersected(staffRect);
      }

//---------------------------------------------------------
//   updateSelection
//---------------------------------------------------------

void TimelineDataGrid::updateSelection()
      {
      if (!score())
            return;

      if (_selectionPathItem) {
            _oldSelectionPath = _selectionPathItem->path();
            scene()->removeItem(_selectionPathItem);
            }
      else
            _oldSelectionPath = QPainterPath();

      QList<TimelineDataGridCell*> selectedCells = getSelectedCells();
      QPainterPath selectionPath = createCellPath(selectedCells);
      QGraphicsPathItem* selectionPathItem = new QGraphicsPathItem(selectionPath.simplified());

      const Selection& scoreSelection = score()->selection();
      if (scoreSelection.isRange())
            selectionPathItem->setPen(QPen(Qt::blue, 3));
      else
            selectionPathItem->setPen(QPen(Qt::blue, 1));

      selectionPathItem->setBrush(Qt::NoBrush);
      selectionPathItem->setZValue(ZValues::SELECTION);
      scene()->addItem(selectionPathItem);

      _selectionPathItem = selectionPathItem;

      keepSelectionChangeInView();
      }

//---------------------------------------------------------
//   keepSelectionChangeInView
//---------------------------------------------------------

void TimelineDataGrid::keepSelectionChangeInView()
      {
      if (_oldSelectionPath == _selectionPathItem->path() || _selectionPathItem->path().isEmpty())
            return;

      // TODO: If subset, the view jumps to the alternative side of _selectionPath instead of the one closest to it
      //       On the next subset, It jumps back.
      QPainterPath commonSelection = _oldSelectionPath.intersected(_selectionPathItem->path());
      QPainterPath newSelection = _selectionPathItem->path().subtracted(commonSelection);
      if (newSelection.isEmpty()) // _selectionPath is a subset of the _oldSelectionPath, move to _selectionPath
            newSelection = _selectionPathItem->path();
      QGraphicsPathItem* changedSelectionItem = new QGraphicsPathItem(newSelection);

      ensureVisible(changedSelectionItem, 0, 0);
      }

//---------------------------------------------------------
//   getSelectedCells
//---------------------------------------------------------

QList<TimelineDataGridCell*> TimelineDataGrid::getSelectedCells()
      {
      QList<TimelineDataGridCell*> selectedCells;
      const Selection& scoreSelection = score()->selection();
      QSet<QPair<Measure*, int>> measuresToSelect; // QPair<measure, staffIdx> used for _grid lookup

      // Fill measuresToSelect
      for (Element* element : scoreSelection.elements()) {
            if (element->tick() == -1)
                  continue;
            else {
                  // Selecting these are ignored in the timeline
                  switch (element->type()) {
                        case ElementType::INSTRUMENT_NAME:
                        case ElementType::VBOX:
                        case ElementType::HBOX:
                        case ElementType::TEXT:
                        case ElementType::TIE_SEGMENT:
                        case ElementType::SLUR_SEGMENT:
                        case ElementType::TIE:
                        case ElementType::SLUR:
                              continue;
                              break;
                        default: break;
                        }
                  }

            Measure* containingMeasure = score()->tick2measure(element->tick());
            int staffIdx = element->staffIdx();
            if (!element->staff() || !element->staff()->show())
                  continue;

            // If has a mmrest, find the count and add each measure to it
            // ws: if style flag Sid::createMultiMeasureRests is not set, then
            // measure->mmRest() is not valid
            if (containingMeasure->mmRest() && containingMeasure->score()->styleB(Sid::createMultiMeasureRests)) {
                  int mmRestCount = containingMeasure->mmRest()->mmRestCount();
                  Measure* currMeasure = containingMeasure;

                  for (int i = 0; i < mmRestCount; i++) {
                        QPair<Measure*, int> measurePair(currMeasure, staffIdx);
                        measuresToSelect << measurePair;
                        currMeasure = currMeasure->nextMeasure();
                        }
                  }
            else {
                  QPair<Measure*, int> measurePair(containingMeasure, staffIdx);
                  measuresToSelect << measurePair;
                  }
            }

      // Use the measuresToSelect list to generate a list of selectedCells--subset of _grid
      for (TimelineDataGridCell* cell : _grid) {
            if (measuresToSelect.contains(cell->infoPair()))
                  selectedCells.append(cell);
            }

      return selectedCells;
      }

//---------------------------------------------------------
//   createCellPath
//---------------------------------------------------------

QPainterPath TimelineDataGrid::createCellPath(QList<TimelineDataGridCell*> selectedCells)
      {
      QPainterPath selectionPath = QPainterPath();
      selectionPath.setFillRule(Qt::WindingFill);

      for (TimelineDataGridCell* cell : selectedCells)
            selectionPath.addRect(cell->rect());

      return selectionPath;
      }

//---------------------------------------------------------
//   updateGrid
//---------------------------------------------------------

void TimelineDataGrid::updateGrid()
      {
      scene()->clear();
      _visibleCellsPathItem = nullptr;
      _nonVisibleCellsPathItem = nullptr;
      _selectionPathItem = nullptr;

      populateGrid();
      updateView();
      updateSelection();
      }

//---------------------------------------------------------
//   redrawGrid
//---------------------------------------------------------

void TimelineDataGrid::redrawGrid()
      {
      int nMeasures = score()->nmeasures();
      int newX = 0;
      int gridIdx = 0;
      int newWidth = cellWidth();

      for (int row = 0; row < nMeasures; row++) {
            for (int column = 0; column < _nStaves; column++) {
                  TimelineDataGridCell* cell = _grid[gridIdx];

                  QRectF newRect = cell->rect();
                  newRect.setX(newX);
                  newRect.setWidth(newWidth);
                  cell->setRect(newRect);

                  gridIdx++;
                  }
            newX += newWidth;
            }

      updateView();
      updateSelection();
      setSceneRect(-1, -1, newWidth * nMeasures + 1, cellHeight() * _nStaves + 1);
      }


//---------------------------------------------------------
//   adjustScrollBar
//   attempts to keep the mouse in the same position
//   on the grid while zooming in and out
//---------------------------------------------------------

void TimelineDataGrid::adjustScrollBar(int originalScrollValue, QPoint globalCursorPos, int originalGridWidth)
      {
      QPoint gridStartPoint = mapFromScene(0, 0);
      gridStartPoint = mapToGlobal(gridStartPoint);

      qreal originalCursorPos = globalCursorPos.x() - gridStartPoint.x();
      qreal ratio = originalCursorPos / qreal(originalGridWidth);

      qreal newCursorPos = qreal(gridBounds().width()) * ratio;
      int offset = newCursorPos - originalCursorPos;
      setScrollBarValue(originalScrollValue + offset);
      }

//---------------------------------------------------------
//   score
//---------------------------------------------------------

Score* TimelineDataGrid::score()
      {
      return getParent()->score();
      }

//---------------------------------------------------------
//   scoreView
//---------------------------------------------------------

ScoreView* TimelineDataGrid::scoreView()
      {
      return getParent()->scoreView();
      }

//---------------------------------------------------------
//   cellWidth
//---------------------------------------------------------

int TimelineDataGrid::cellWidth()
      {
      return getParent()->getParent()->cellWidth();
      }

//---------------------------------------------------------
//   cellHeight
//---------------------------------------------------------

int TimelineDataGrid::cellHeight()
      {
      return getParent()->getParent()->cellHeight();
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void TimelineDataGrid::mousePressEvent(QMouseEvent* event)
      {
      _oldMousePos = mapToScene(event->pos());
      if (!score())
            return;

      QList<QGraphicsItem*> itemsList = items(event->pos());

      if (!itemsList.isEmpty()) {
            TimelineDataGridCell* cell = static_cast<TimelineDataGridCell*>(itemsList.last());
            if (!cell || cell->zValue() != TimelineDataGrid::CELL)
                  return; // Clicks outside edge of selection path, ignore

            if (!cell->measure()->system()->staff(cell->staffIdx())->show())
                  return; // Ignore clicks on non-visible cells

            Measure* measureToSelect = cell->getMeasureToSelect();

            // TODO: Decide behavior when clicking hidden instruments
            if (event->modifiers() == Qt::ShiftModifier)
                  score()->select(measureToSelect, SelectType::RANGE, cell->staffIdx());
            else if (event->modifiers() == Qt::ControlModifier) {
                  if (score()->selection().isNone()) {
                        score()->select(measureToSelect, SelectType::RANGE, 0);
                        score()->select(measureToSelect, SelectType::RANGE, score()->nstaves() - 1);
                        }
                  else
                        score()->deselectAll();
                  }
            else
                  score()->select(measureToSelect, SelectType::SINGLE, cell->staffIdx());

            scoreView()->adjustCanvasPosition(measureToSelect, false, cell->staffIdx());
            scoreView()->updateAll(); // Repaints the score for occasional selection bugs

            updateSelection();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void TimelineDataGrid::mouseMoveEvent(QMouseEvent* event)
      {
      if (event->buttons() != Qt::LeftButton) {
            setMouseCursor(event);
            return;
            }

      QPointF newMousePos = mapToScene(event->pos());
      QPointF offset = _oldMousePos - newMousePos;

      if (_lassoSelection) {
            QRectF adjustedLasso = QRectF(_oldMousePos, newMousePos);
            _lassoSelection->setRect(adjustedLasso.normalized());
            setMouseCursor(event);
            }
      else if (event->modifiers() == Qt::ShiftModifier && !_draggingGrid) {
            setMouseCursor(event);
            if (offset.manhattanLength() <= 4)
                  return; // Allows for some wiggle room to mimic scoreview

            score()->deselectAll();
            updateSelection();

            _lassoSelection = new QGraphicsRectItem(QRectF(_oldMousePos, _oldMousePos));
            _lassoSelection->setPen(QPen(Qt::blue, 2));
            QColor transparent = QColor(Qt::blue);
            transparent.setAlpha(50);
            _lassoSelection->setBrush(QBrush(transparent));
            _lassoSelection->setZValue(TimelineDataGrid::LASSO);

            scene()->addItem(_lassoSelection);
            }
      else {
            _draggingGrid = true;

            this->setCursor(Qt::SizeAllCursor);
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + offset.x());
            verticalScrollBar()->setValue(verticalScrollBar()->value() + offset.y());
            }
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void TimelineDataGrid::mouseReleaseEvent(QMouseEvent* event)
      {
      if (_lassoSelection) {
            QRect lassoRect = _lassoSelection->rect().toRect();
            selectLassoItems(items(lassoRect));

            scene()->removeItem(_lassoSelection);
            _lassoSelection = nullptr;

            mscore->endCmd();
            }
      else if (!_draggingGrid) {
            QList<QGraphicsItem*> itemsList = items(event->pos());
            if (itemsList.isEmpty()) {
                  score()->deselectAll();
                  updateSelection();
                  }
            }

      _draggingGrid = false;
      setMouseCursor(event);
      }

//---------------------------------------------------------
//   setMouseCursor
//---------------------------------------------------------

void TimelineDataGrid::setMouseCursor(QMouseEvent* event)
      {
      QList<QGraphicsItem*> itemsList = items(event->pos());
      if (itemsList.isEmpty()) {
            this->setCursor(Qt::ArrowCursor);
            return;
            }

      TimelineDataGridCell* cell = static_cast<TimelineDataGridCell*>(itemsList.last());
      if (!cell || cell->zValue() != TimelineDataGrid::CELL) {
            this->setCursor(Qt::ArrowCursor);
            return; // Clicks outside edge of selection path, ignore
            }

      if (!cell->measure()->system()->staff(cell->staffIdx())->show())
            this->setCursor(Qt::ForbiddenCursor);
      else
            this->setCursor(Qt::ArrowCursor);
      }

//---------------------------------------------------------
//   selectLassoItems
//   Locate top left and bottom right cells. Perform a range seletion.
//---------------------------------------------------------

void TimelineDataGrid::selectLassoItems(QList<QGraphicsItem*> items)
      {
      if (items.isEmpty())
            return;

      TimelineDataGridCell* topLeft = nullptr;
      TimelineDataGridCell* bottomRight = nullptr;

      for (QGraphicsItem* item : items) {
            // Use dynamic cast because TimelineDataGridCell inherits QGraphicsRectItem causing bad casting
            TimelineDataGridCell* currCell = dynamic_cast<TimelineDataGridCell*>(item);
            if (!currCell)
                  continue;

            if (!topLeft ||
                (currCell->measureIdx() <= topLeft->measureIdx() &&
                 currCell->staffIdx() <= topLeft->staffIdx()))
                  topLeft = currCell;

            if (!bottomRight ||
                (currCell->measureIdx() >= bottomRight->measureIdx() &&
                 currCell->staffIdx() >= bottomRight->staffIdx()))
                  bottomRight = currCell;
            }

      // No cell was selected
      if (!topLeft)
            return;

      score()->select(topLeft->getMeasureToSelect(), SelectType::SINGLE, topLeft->staffIdx());
      score()->select(bottomRight->getMeasureToSelect(), SelectType::RANGE, bottomRight->staffIdx());
      scoreView()->adjustCanvasPosition(topLeft->getMeasureToSelect(), false, topLeft->staffIdx());
      }

//---------------------------------------------------------
//   TimelineData
//---------------------------------------------------------

TimelineData::TimelineData(Timeline* parent)
   : QSplitter(parent)
      {
      addWidget(new TimelineDataLabels(this));
      addWidget(new TimelineDataGrid(this));

      setCollapsible(1, false);

      connect(labelView()->verticalScrollBar(), SIGNAL(valueChanged(int)),
              gridView()->verticalScrollBar(), SLOT(setValue(int)));
      connect(gridView()->verticalScrollBar(), SIGNAL(valueChanged(int)),
              labelView()->verticalScrollBar(), SLOT(setValue(int)));
      }

//---------------------------------------------------------
//   metaSplitterMoved
//---------------------------------------------------------

void TimelineData::metaSplitterMoved()
      {
      QSplitter* metaSplitter = getParent()->metaWidget();
      setSizes(metaSplitter->sizes());
      labelView()->updateLabelWidths(metaSplitter->sizes()[0]);
      }

//---------------------------------------------------------
//   score
//---------------------------------------------------------

Score* TimelineData::score()
      {
      return getParent()->score();
      }

//---------------------------------------------------------
//   scoreView
//---------------------------------------------------------

ScoreView* TimelineData::scoreView()
      {
      return getParent()->scoreView();
      }

//---------------------------------------------------------
//   getParent
//---------------------------------------------------------

Timeline* TimelineData::getParent()
      {
      QObject* localParent = parent()->parent(); // Needed to get Timeline from QSplitter
      return static_cast<Timeline*>(localParent);
      }

}
