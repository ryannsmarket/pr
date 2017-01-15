//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: file.h 5009 2011-11-18 21:46:17Z lasconic $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __FILE_H__
#define __FILE_H__

namespace Ms {

//---------------------------------------------------------
//   WallpaperPreview
//---------------------------------------------------------

class WallpaperPreview : public QFrame {
      Q_OBJECT
      QPixmap* _pixmap;

      virtual void paintEvent(QPaintEvent*);
      virtual QSize sizeHint() const { return QSize(100, 100); }

   public slots:
      void setImage(const QString& path);

   public:
      WallpaperPreview(QWidget* parent = 0);
      };


extern QStringList getSoundFont(const QString&);

} // namespace Ms
#endif

