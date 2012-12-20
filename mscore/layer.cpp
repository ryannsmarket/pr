//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: mscore.cpp 4220 2011-04-22 10:31:26Z wschweer $
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "layer.h"
#include "libmscore/score.h"
#include "musescore.h"
#include "libmscore/mscore.h"

namespace Ms {

//---------------------------------------------------------
//   TagSetManager
//---------------------------------------------------------

TagSetManager::TagSetManager(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      score = s;
      setupUi(this);
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

      for (int i = 0; i < MAX_TAGS; ++i) {
            QTableWidgetItem* item = new QTableWidgetItem(score->tagSetTags()[i+1]);
            tags->setItem(i, 0, item);
            item = new QTableWidgetItem(score->tagSetTagComments()[i+1]);
            tags->setItem(i, 1, item);
            }

      tagSets->setRowCount(score->tagSet().size());
      int row = 0;
      foreach(const TagSet& ts, score->tagSet()) {
            QTableWidgetItem* item = new QTableWidgetItem(ts.name);
            tagSets->setItem(row, 0, item);
            QString tagString;
            for (int i = 0; i < MAX_TAGS-1; ++i) {
                  uint mask = 1 << (i+1);
                  if (mask &  ts.tags) {
                        if (!tagString.isEmpty())
                              tagString += ",";
                        tagString += tags->item(i, 0)->text();
                        }
                  }
            item = new QTableWidgetItem(tagString);
            tagSets->setItem(row, 1, item);
            ++row;
            }
      tagSets->setCurrentCell(score->currentTagSet(), 0);
      connect(createButton, SIGNAL(clicked()), SLOT(createClicked()));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteClicked()));
      connect(addTagButton, SIGNAL(clicked()), SLOT(addTagClicked()));
      connect(deleteTagButton, SIGNAL(clicked()), SLOT(deleteTagClicked()));
      }

//---------------------------------------------------------
//   showTagSetManager
//---------------------------------------------------------

void MuseScore::showTagSetManager()
      {
      TagSetManager am(cs);
      am.exec();
      }

//---------------------------------------------------------
//   createClicked
//---------------------------------------------------------

void TagSetManager::createClicked()
      {
      int row      = tagSets->rowCount();
      QString name = QString("tagSet%1").arg(row + 1);
      QTableWidgetItem* item = new QTableWidgetItem(name);
      tagSets->setRowCount(row+1);
      tagSets->setItem(row, 0, item);
      item = new QTableWidgetItem("");
      tagSets->setItem(row, 1, item);
      }

//---------------------------------------------------------
//   deleteClicked
//---------------------------------------------------------

void TagSetManager::deleteClicked()
      {
      qDebug("TODO\n");
      }

//---------------------------------------------------------
//   addTagClicked
//---------------------------------------------------------

void TagSetManager::addTagClicked()
      {
      int row = tagSets->currentRow();
      if (row == -1)
            return;
      QStringList items;
      for (int i = 0; i < MAX_TAGS; ++i) {
            QString s = score->tagSetTags()[i+1];
            if (!s.isEmpty())
                  items.append(s);
            }
      for (int i = 0; i < MAX_TAGS; ++i) {
            QString tag(tags->item(i, 0)->text());
            if (!tag.isEmpty())
                  items.append(tag);
            }

      if (items.isEmpty()) {
            qDebug("no tags defined\n");
            return;
            }
      bool ok;
      QString item = QInputDialog::getItem(this, tr("MuseScore: select tag"), tr("tag"),
         items, 0, false, &ok);
      if (ok && !item.isEmpty()) {
//            uint tagBits = 0;
            for (int i = 0; i < MAX_TAGS; ++i) {
                  QString s = score->tagSetTags()[i+1];
                  if (s == item) {
//                        tagBits = 1 << (i+1);
                        break;
                        }
                  }
            QTableWidgetItem* wi = tagSets->item(row, 1);
            QString s = wi->text();
            if (!s.isEmpty())
                  s += ",";
            s += item;
            wi->setText(s);
            }
      }

//---------------------------------------------------------
//   deleteTagClicked
//---------------------------------------------------------

void TagSetManager::deleteTagClicked()
      {
      int row = tagSets->currentRow();
      if (row == -1)
            return;
      QTableWidgetItem* item = tagSets->item(row, 1);
      QString s = item->text();
      QStringList items = s.split(",");
      bool ok;
      QString tag = QInputDialog::getItem(this, tr("MuseScore: select tag"), tr("tag"),
         items, 0, false, &ok);
      if (ok && !tag.isEmpty()) {
            items.removeOne(tag);
            item->setText(items.join(","));
            }
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void TagSetManager:: accept()
      {
      score->startCmd();
      for (int i = 0; i < MAX_TAGS; ++i) {
            QString tag(tags->item(i, 0)->text());
            QString comment(tags->item(i, 1)->text());
            score->tagSetTags()[i+1] = tag;
            score->tagSetTagComments()[i+1] = comment;
            }
      int row = tagSets->currentRow();
      if (row != -1)
            score->setCurrentTagSet(row);

      QList<TagSet>& tagSet = score->tagSet();
      tagSet.clear();

      int n = tagSets->rowCount();
      for (int i = 0; i < n; ++i) {
            TagSet ts;
            ts.name          = tagSets->item(i, 0)->text();
            ts.tags           = 1;
            QString tstxt    = tagSets->item(i, 1)->text();
            QStringList tags = tstxt.split(",");
            foreach (QString tag, tags) {
                  for (int idx = 0; idx < MAX_TAGS; ++idx) {
                        if (tag == score->tagSetTags()[idx]) {
                              ts.tags |= 1 << idx;
                              break;
                              }
                        }
                  }
            if (i == 0)             // hardwired default tag
                  ts.tags |= 1;
            tagSet.append(ts);
            }
      score->setDirty(true);
      score->setLayoutAll(true);
      score->endCmd();
      mscore->updateTagSet();
      QDialog::accept();
      }

}
