//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010-2011 Werner Schweer
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

#include "pattern.h"
#include "utils.h"
#include "libmscore/sym.h"
#include "omr.h"

namespace Ms {

//---------------------------------------------------------
//   Pattern
//---------------------------------------------------------

Pattern::Pattern()
      {
      }

Pattern::~Pattern()
      {
<<<<<<< HEAD
//          for(int i = 0; i < rows; ++i)
//              delete []model[i];
//          delete []model;
=======
>>>>>>> 0e4c6b6... add pattern source files
      }

//---------------------------------------------------------
//   patternMatch
//    compare two patterns for similarity
//    return:
//          1.0   - identical
//          0.5   - 50% of all pixel match
//          0.0   - no match
//---------------------------------------------------------

double Pattern::match(const Pattern* a) const
      {
      int n = image()->byteCount();
      if (n != a->image()->byteCount())
            return 0.0;
      int k = 0;
      const uchar* p1 = image()->bits();
      const uchar* p2 = a->image()->bits();
      for (int i = 0; i < n; ++i) {
            uchar v = (*(p1++)) ^ (*(p2++));
            k += Omr::bitsSetTable[v];
            }
      return 1.0 - (double(k) / (h() * w()));
      }
<<<<<<< HEAD
    
    double Pattern::match(const QImage* img, int col, int row) const
    {
        return 0.0;
    }

double Pattern::match(const QImage* img, int col, int row, double bg_parm) const
      {
//      int rows      = h();
//      int bytes     = ((w() + 7) / 8) - 1;
//      int shift     = col & 7;
//      int k         = 0;
//      int eshift    = (col + w()) & 7;
          double k = 0;
          if(bg_parm == 0) bg_parm = 1e-10;
          if(bg_parm == 1) bg_parm = 1-1e-10;

      for (int y = 0; y < rows; ++y) {
            //const uchar* p1 = image()->scanLine(y);
            //const uchar* p2 = img->scanLine(row + y) + (col/8);
          
          
          
          for(int x = 0; x < cols; x++){
              //const uchar* p = img->scanLine(row + y) + ((col+x) / 32);
              //bool black = (*p) & (0x1 << ((col+x) % 32));
              if(col+x >= img->size().width() || row+y >= img->size().height()) continue;
              QRgb c = img->pixel(col+x, row+y);
              bool black = (qGray(c) < 100);
              //if(black)
                  //printf("here");
              k += black?(log(model[y][x]) - log(bg_parm)):(log(1.0 - model[y][x]) - log(1-bg_parm));
          }
      }
//            for (int x = 0; x < bytes; ++x) {
//                  uchar a = *p1++;
//                  uchar b1 = *p2;
//                  uchar b2 = *(p2 + 1);
//                  p2++;
//                  uchar b  = (b1 >> shift) | (b2 << (7 - shift));
//                  uchar v = a ^ b;
//                  k += Omr::bitsSetTable[v];
//                  }
//            uchar a = *p1++;
//            uchar b1 = *p2;
//            uchar b2 = *(p2 + 1) & (0xff << eshift);
//            uchar b  = (b1 >> shift) | (b2 << (7 - shift));
//            uchar v = a ^ b;
//            k += Omr::bitsSetTable[v];
//            }

          return k;
=======

double Pattern::match(const QImage* img, int col, int row) const
      {
      int rows      = h();
      int bytes     = ((w() + 7) / 8) - 1;
      int shift     = col & 7;
      int k         = 0;
      int eshift    = (col + w()) & 7;

      for (int y = 0; y < rows; ++y) {
            const uchar* p1 = image()->scanLine(y);
            const uchar* p2 = img->scanLine(row + y) + (col/8);
            for (int x = 0; x < bytes; ++x) {
                  uchar a = *p1++;
                  uchar b1 = *p2;
                  uchar b2 = *(p2 + 1);
                  p2++;
                  uchar b  = (b1 >> shift) | (b2 << (7 - shift));
                  uchar v = a ^ b;
                  k += Omr::bitsSetTable[v];
                  }
            uchar a = *p1++;
            uchar b1 = *p2;
            uchar b2 = *(p2 + 1) & (0xff << eshift);
            uchar b  = (b1 >> shift) | (b2 << (7 - shift));
            uchar v = a ^ b;
            k += Omr::bitsSetTable[v];
            }
      return 1.0 - (double(k) / (h() * w()));
>>>>>>> 0e4c6b6... add pattern source files
      }

//---------------------------------------------------------
//   Pattern
//    create a Pattern from symbol
//---------------------------------------------------------

<<<<<<< HEAD
<<<<<<< HEAD
Pattern::Pattern(Score *s, SymId id, double spatium)
      {
          _score = s;
          _id = id;

          QFont f("Bravura");
          f.setPixelSize(lrint(spatium * 4));
          QFontMetrics fm(f);
          QRectF r = _score->scoreFont()->bbox(id, 9.0);
          int _w = r.right() - r.left() + 2;
          int _h = ((r.height() + 1) / 2) * 2;
          _base = QPoint(-r.left(), -r.top());
          
          _image = QImage(_w, _h, QImage::Format_MonoLSB);
          QVector<QRgb> ct(2);
          ct[0] = qRgb(255, 255, 255);
          ct[1] = qRgb(0, 0, 0);
          _image.setColorTable(ct);
          _image.fill(0);
          
          QPainter painter;
          painter.begin(&_image);
          painter.setFont(f);
          painter.drawText(-r.left() + 1, -r.y(), _score->scoreFont()->toString(id));
          painter.end();
          
          int ww = _w % 32;
          if (ww == 0)
              return;
          uint mask = 0xffffffff << ww;
          int n = ((_w + 31) / 32) - 1;
          for (int i = 0; i < _h; ++i) {
              uint* p = ((uint*)_image.scanLine(i)) + n;
              *p = ((*p) & ~mask);
          }
          
      }
    
    //---------------------------------------------------------
    //   Pattern
    //    create a Pattern from symbol name
    //---------------------------------------------------------
    
    Pattern::Pattern(Score *s, QString name)
    {
        _score = s;
        QString filename = "/Users/Hipapa/Projects/Git/MuseScore/omr/solid_note_head.dat";//name + ".dat";
        
        QFile f(filename);
        if(!f.open(QFile::ReadOnly| QIODevice::Text)){
            rows = 0;
            cols = 0;
        }
        else{
            QTextStream in(&f);
            
            in>>rows>>cols;
            
            model = new float*[rows];
            for(int i = 0; i < rows; i++){
                model[i] = new float[cols];
            }
            for(int i = 0; i < rows; i++){
                for(int j = 0; j < cols; j++){
                    in>>model[i][j];
                }
            }
        }
        f.close();
    }

    
=======
Pattern::Pattern(int id, Sym* symbol, double spatium)
=======
Pattern::Pattern(Score *s, SymId id, double spatium)
>>>>>>> 27d1d6c... debug pattern match
      {
          _score = s;
          _id = id;

          QFont f("Bravura");
          f.setPixelSize(lrint(spatium * 4));
          QFontMetrics fm(f);
          QRectF r = _score->scoreFont()->bbox(id, 9.0);
          int _w = r.right() - r.left() + 2;
          int _h = ((r.height() + 1) / 2) * 2;
          _base = QPoint(-r.left(), -r.top());
          
          _image = QImage(_w, _h, QImage::Format_MonoLSB);
          QVector<QRgb> ct(2);
          ct[0] = qRgb(255, 255, 255);
          ct[1] = qRgb(0, 0, 0);
          _image.setColorTable(ct);
          _image.fill(0);
          
          QPainter painter;
          painter.begin(&_image);
          painter.setFont(f);
          painter.drawText(-r.left() + 1, -r.y(), _score->scoreFont()->toString(id));
          painter.end();
          
          int ww = _w % 32;
          if (ww == 0)
              return;
          uint mask = 0xffffffff << ww;
          int n = ((_w + 31) / 32) - 1;
          for (int i = 0; i < _h; ++i) {
              uint* p = ((uint*)_image.scanLine(i)) + n;
              *p = ((*p) & ~mask);
          }
          
      }

>>>>>>> 0e4c6b6... add pattern source files
//---------------------------------------------------------
//   Pattern
//    create a Pattern from image
//---------------------------------------------------------

Pattern::Pattern(QImage* img, int x, int y, int w, int h)
      {
      _image = img->copy(x, y, w, h);
      int ww = w % 32;
      if (ww == 0)
            return;
      uint mask = 0xffffffff << ww;
      int n = ((w + 31) / 32) - 1;
      for (int i = 0; i < h; ++i) {
            uint* p = ((uint*)_image.scanLine(i)) + n;
            *p     &= ~mask;
            }
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Pattern::dump() const
      {
      printf("pattern %d x %d\n", _image.width(), _image.height());
      for (int y = 0; y < _image.height(); ++y) {
            for (int x = 0; x < _image.width(); ++x) {
                  QRgb pixel = _image.pixel(x, y);
                  printf("%c", pixel & 0xffffff ? '-' : '*');
                  }
            printf("\n");
            }
      }

//---------------------------------------------------------
//   dot
//---------------------------------------------------------

bool Pattern::dot(int x, int y) const
      {
      const uint* p = (const uint*)_image.scanLine(y) + (x / 32);
      return (*p) & (0x1 << (x % 32));
      }
}

