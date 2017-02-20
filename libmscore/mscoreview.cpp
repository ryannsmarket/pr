//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2013 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#include "mscoreview.h"
#include "score.h"
#include "page.h"
#include "measure.h"

namespace Ms {

//---------------------------------------------------------
//   elementLower
//---------------------------------------------------------

static bool elementLower(const Element* e1, const Element* e2)
      {
      if (!e1->selectable())
            return false;
      return e1->z() < e2->z();
      }

//---------------------------------------------------------
//   elementAt
//---------------------------------------------------------

Element* MuseScoreView::elementAt(const QPointF& p)
      {
      QList<Element*> el = elementsAt(p);
#if 0
      qDebug("elementAt");
      foreach(const Element* e, el)
            qDebug("  %s %d", e->name(), e->selected());
#endif
      Element* e = el.value(0);
      if (e && (e->type() == ElementType::PAGE))
            e = el.value(1);
      return e;
      }

//---------------------------------------------------------
//   point2page
//---------------------------------------------------------

Page* MuseScoreView::point2page(const QPointF& p)
      {
      if (score()->layoutMode() == LayoutMode::LINE)
            return score()->pages().isEmpty() ? 0 : score()->pages().front();
      foreach(Page* page, score()->pages()) {
            if (page->bbox().translated(page->pos()).contains(p))
                  return page;
            }
      return 0;
      }

//---------------------------------------------------------
//   elementsAt
//    p is in canvas coordinates
//---------------------------------------------------------

const QList<Element*> MuseScoreView::elementsAt(const QPointF& p)
      {
      QList<Element*> el;

      Page* page = point2page(p);
      if (page) {
            el = page->items(p - page->pos());
            qSort(el.begin(), el.end(), elementLower);
            }
      return el;
      }

//---------------------------------------------------------
//   horizontallyNearestChordRestSegment
//    p is in canvas coordinates
//---------------------------------------------------------

Segment* MuseScoreView::horizontallyNearestChordRestSegment(const QPointF& p)
      {
      Segment* chordRest = 0;
      Page* page = point2page(p);
      if (page) {
            qreal pointHoizontalPosRelativeToPage = p.x() - page->pos().x();

            Measure* m = page->searchMeasure(p);
            if (m) {
                  chordRest = m->segments().firstCRSegment();
                  if (chordRest) {
                        qreal chordRestDX = pointHoizontalPosRelativeToPage - (chordRest->x() + chordRest->bbox().center().x());
                        Segment *nextChordRest = chordRest->nextCR();
                        while (nextChordRest) {
                              qreal nextChordRestDX = pointHoizontalPosRelativeToPage - (nextChordRest->x() + nextChordRest->bbox().center().x());
                              if (nextChordRestDX < 0) {
                                    // now need to compare distance to the chordrest on the left vs to the chordrest on the right
                                    if (-nextChordRestDX < chordRestDX)
                                          chordRest = nextChordRest;
                                    break;
                                    }
                              chordRestDX = nextChordRestDX;
                              chordRest = nextChordRest;
                              nextChordRest = chordRest->nextCR();
                              }
                        }
                  }
            }
      return chordRest;
      }

}

