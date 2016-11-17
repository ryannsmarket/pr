//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: abstractdialog.h 4220 2011-04-22 10:31:26Z wschweer $
//
//  Copyright (C) 2002-2016 Werner Schweer and others
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
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//=============================================================================

#ifndef __QABSTRACTDIALOG_H__
#define __QABSTRACTDIALOG_H__

namespace Ms {

//---------------------------------------------------------
//   AbstractDialog
//---------------------------------------------------------

class AbstractDialog : public QDialog
      {

   Q_OBJECT

   public:
      AbstractDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);
      virtual ~AbstractDialog();

   protected:
      // change language event
      virtual void changeEvent(QEvent *event);

      // translate all strings
      virtual void retranslate() = 0;
      };

}
#endif

