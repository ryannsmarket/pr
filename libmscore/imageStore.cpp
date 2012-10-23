//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtCore/QCryptographicHash>
#include "imageStore.h"
#include "score.h"
#include "image.h"

ImageStore imageStore;  // the global image store

//---------------------------------------------------------
//   ImageStoreItem
//---------------------------------------------------------

ImageStoreItem::ImageStoreItem(const QString& p)
      {
      setPath(p);
      }

//---------------------------------------------------------
//   dereference
//    decrement usage count of image in score
//---------------------------------------------------------

void ImageStoreItem::dereference(Image* image)
      {
      _references.removeOne(image);
      }

//---------------------------------------------------------
//   reference
//    increment usage count of image in score
//---------------------------------------------------------

void ImageStoreItem::reference(Image* image)
      {
      _references.append(image);
      }

//---------------------------------------------------------
//   isUsed
//    check if item is used in score
//---------------------------------------------------------

bool ImageStoreItem::isUsed(Score* score) const
      {
      foreach(Image* image, _references) {
            if (image->score() == score)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void ImageStoreItem::load()
      {
      if (!_buffer.isEmpty())
            return;
      QFile inFile(_path);
      if (!inFile.open(QIODevice::ReadOnly)) {
            qDebug("cannot open picture file");
            return;
            }
      _buffer = inFile.readAll();
      inFile.close();
      QCryptographicHash h(QCryptographicHash::Md4);
      h.addData(_buffer);
      _hash = h.result();
      }

//---------------------------------------------------------
//   hashName
//---------------------------------------------------------

QString ImageStoreItem::hashName() const
      {
      const char hex[17] = "0123456789abcdef";
      char p[33];
      for (int i = 0; i < 16; ++i) {
            p[i * 2]     = hex[(_hash[i] >> 4) & 0xf];
            p[i * 2 + 1] = hex[_hash[i] & 0xf];
            }
      p[32] = 0;
      return QString(p) + "." + _type;
      }

//---------------------------------------------------------
//   setPath
//---------------------------------------------------------

void ImageStoreItem::setPath(const QString& val)
      {
      _path = val;
      QFileInfo fi(_path);
      _type = fi.suffix();
      }


//---------------------------------------------------------
//   toInt
//---------------------------------------------------------

inline static int toInt(char c)
      {
      if (c >= '0' && c <= '9')
            return c - '0';
      return c - 'a' + 10;
      }

#if 0
//---------------------------------------------------------
//   dumpHash
//---------------------------------------------------------

static void dumpHash(const QByteArray& _hash)
      {
      const char hex[17] = "0123456789abcdef";
      char p[33];
      for (int i = 0; i < 16; ++i) {
            p[i * 2]     = hex[(_hash[i] >> 4) & 0xf];
            p[i * 2 + 1] = hex[_hash[i] & 0xf];
            }
      p[32] = 0;
      printf("   <%s>\n", p);
      }
#endif

//---------------------------------------------------------
//   getImage
//---------------------------------------------------------

ImageStoreItem* ImageStore::getImage(const QString& path) const
      {
      QString s = QFileInfo(path).baseName();
      if (s.size() != 32) {
            //
            // some limited support for backward compatibility
            //
            foreach(ImageStoreItem* item, *this) {
                  if (item->path() == path)
                        return item;
                  }
            qDebug("ImageStore::getImage(%s): bad base name <%s>",
               qPrintable(path), qPrintable(s));
            return 0;
            }
      QByteArray hash(16, 0);
      for (int i = 0; i < 16; ++i) {
            hash[i] = toInt(s[i * 2].toAscii()) * 16 + toInt(s[i * 2 + 1].toAscii());
            }
      foreach(ImageStoreItem* item, *this) {
            if (item->hash() == hash)
                  return item;
            }
      qDebug("ImageStore::getImage(): not found <%s>", qPrintable(path));
      return 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

ImageStoreItem* ImageStore::add(const QString& path, const QByteArray& ba)
      {
      QCryptographicHash h(QCryptographicHash::Md4);
      h.addData(ba);
      QByteArray hash = h.result();
      foreach(ImageStoreItem* item, *this) {
            if (item->hash() == hash)
                  return item;
            }
      ImageStoreItem* item = new ImageStoreItem(path);
      item->set(ba, hash);
      append(item);
      return item;
      }

