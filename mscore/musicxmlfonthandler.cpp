//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//
//  Copyright (C) 2014 Werner Schweer and others
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

/**
 MusicXML font handling support.
 */

#include "libmscore/xml.h"
#include "musicxmlfonthandler.h"

namespace Ms {

//---------------------------------------------------------
//   charFormat2QString
//    convert charFormat to QString for debug print
//---------------------------------------------------------

#if 0
static QString charFormat2QString(const CharFormat& f)
      {
      return QString("b %1 i %2 u %3 va %4 fs %5 fam %6")
            .arg(f.bold())
            .arg(f.italic())
            .arg(f.underline())
            .arg(static_cast<int>(f.valign()))
            .arg(f.fontSize())
            .arg(f.fontFamily())
                 ;
      }
#endif

//---------------------------------------------------------
//   MScoreTextToMXML
//    Convert rich text as generated by Text::text()
//    into MusicXML text with or without formatting
//    defTs: default text style (as specified in word-font or lyric-font)
//    actTs: actual text style for this type of text
//    if curfs != deffs, an initial font-size attribute is emitted
//---------------------------------------------------------

MScoreTextToMXML::MScoreTextToMXML(const QString& tag, const QString& attr, const QString& t, const TextStyle& defTs, const TextStyle& actTs)
      : attribs(attr), tagname(tag)
      {
      //qDebug("MScoreTextToMXML('%s')", qPrintable(t));
      // handle difference between style for words / lyric and actual type
      oldFormat.setFontFamily(defTs.family());
      newFormat.setFontFamily(actTs.family());
      oldFormat.setFontSize(defTs.size());
      newFormat.setFontSize(actTs.size());
      oldFormat.setBold(false);
      newFormat.setBold(actTs.bold());
      oldFormat.setItalic(false);
      newFormat.setItalic(actTs.italic());
      oldFormat.setUnderline(false);
      newFormat.setUnderline(actTs.underline());
      // convert text into valid xml by adding dummy start and end tags
      text = "<dummy>" + t + "</dummy>";
      }
      
//---------------------------------------------------------
//   toPlainText
//    convert to plain text
//    naive implementation: simply remove all chars from '<' to '>'
//    typically used to remove formatting info from fields read
//    from MuseScore 1.3 file where they are stored as html, such as
//    part name and shortName
//---------------------------------------------------------

QString MScoreTextToMXML::toPlainText(const QString& text)
      {
      QString res;
      bool inElem = false;
      foreach(QChar ch, text) {
            if (ch == '<')
                  inElem = true;
            else if (ch == '>')
                  inElem = false;
            else {
                  if (!inElem)
                        res += ch;
                  }
            }
      //qDebug("MScoreTextToMXML::toPlainText('%s') res '%s'", qPrintable(text), qPrintable(res));
      return res;
      }

//---------------------------------------------------------
//   MScoreTextToMXML
//    write to xml
//---------------------------------------------------------

void MScoreTextToMXML::write(Xml& xml)
      {
      //qDebug("MScoreTextToMXML::write()");
      QXmlStreamReader r(text);
      bool firstTime = true; // write additional attributes only the first time characters are written
      while (!r.atEnd()) {
            // do processing
            r.readNext();
            if(r.isCharacters()) {
                  //qDebug("old %s", qPrintable(charFormat2QString(oldFormat)));
                  //qDebug("new %s", qPrintable(charFormat2QString(newFormat)));
                  QString formatAttr = updateFormat();
                  //qDebug("old %s", qPrintable(charFormat2QString(oldFormat)));
                  //qDebug("new %s", qPrintable(charFormat2QString(newFormat)));
                  //qDebug("Characters '%s'", qPrintable(r.text().toString()));
                  xml.tag(tagname + (firstTime ? attribs : "") + formatAttr, r.text().toString());
                  firstTime = false;
            }
            else if(r.isEndElement()) {
                  //qDebug("EndElem '%s'", qPrintable(r.name().toString()));
                  handleEndElement(r);
                  }
            else if(r.isStartElement()) {
                  /*
                  qDebug("StartElem '%s'", qPrintable(r.name().toString()));
                  if (r.name() == "font")
                        qDebug("   face='%s' size='%s'",
                               qPrintable(r.attributes().value("face").toString()),
                               qPrintable(r.attributes().value("size").toString()));
                   */
                  handleStartElement(r);
                  }
            }
      if (r.hasError()) {
            // do error handling
            qDebug("Error %s", qPrintable(r.errorString()));
            }
      }

//---------------------------------------------------------
//   MScoreTextToMXML
//    update newFormat for start element
//---------------------------------------------------------

void MScoreTextToMXML::handleStartElement(QXmlStreamReader& r)
{
      if (r.name() == "b")
            newFormat.setBold(true);
      else if (r.name() == "i")
            newFormat.setItalic(true);
      else if (r.name() == "u")
            newFormat.setUnderline(true);
      else if (r.name() == "font" && r.attributes().hasAttribute("size"))
            newFormat.setFontSize(r.attributes().value("size").toFloat());
      else if (r.name() == "font" && r.attributes().hasAttribute("face"))
            newFormat.setFontFamily(r.attributes().value("face").toString());
      else if (r.name() == "sub")
            ; // ignore (not supported in MusicXML)
      else if (r.name() == "sup")
            ; // ignore (not supported in MusicXML)
      else if (r.name() == "sym")
            // ignore (TODO ?)
            r.skipCurrentElement();
      else if (r.name() == "dummy")
            ; // ignore
      else {
            qDebug("handleStartElem '%s' unknown", qPrintable(r.name().toString()));
            r.skipCurrentElement();
      }
}

//---------------------------------------------------------
//   MScoreTextToMXML
//    update newFormat for end element
//---------------------------------------------------------

void MScoreTextToMXML::handleEndElement(QXmlStreamReader& r)
{
      if (r.name() == "b")
            newFormat.setBold(false);
      else if (r.name() == "i")
            newFormat.setItalic(false);
      else if (r.name() == "u")
            newFormat.setUnderline(false);
      else if (r.name() == "font")
            ; // ignore
      else if (r.name() == "sub")
            ; // ignore (not supported in MusicXML)
      else if (r.name() == "sup")
            ; // ignore (not supported in MusicXML)
      else if (r.name() == "sym")
            ; // ignore (TODO ?)
      else if (r.name() == "dummy")
            ; // ignore
      else {
            qDebug("handleEndElem '%s' unknown", qPrintable(r.name().toString()));
            r.skipCurrentElement();
      }
}

//---------------------------------------------------------
//   attribute
//    add one attribute if necessary
//---------------------------------------------------------

static QString attribute(bool needed, bool value, QString trueString, QString falseString)
      {
      QString res;
      if (needed)
            res = value ? trueString : falseString;
      if (res != "")
            res = " " + res;
      return res;
      }

//---------------------------------------------------------
//   updateFormat
//    update the text format by generating attributes
//    corresponding to the difference between old- and newFormat
//    copy newFormat to oldFormat
//---------------------------------------------------------

QString MScoreTextToMXML::updateFormat()
      {
      QString res;
      res += attribute(newFormat.bold() != oldFormat.bold(), newFormat.bold(), "font-weight=\"bold\"", "font-weight=\"normal\"");
      res += attribute(newFormat.italic() != oldFormat.italic(), newFormat.italic(), "font-style=\"italic\"", "font-style=\"normal\"");
      res += attribute(newFormat.underline() != oldFormat.underline(), newFormat.underline(), "underline=\"1\"", "underline=\"0\"");
      res += attribute(newFormat.fontFamily() != oldFormat.fontFamily(), true, QString("font-family=\"%1\"").arg(newFormat.fontFamily()), "");
      bool needSize = newFormat.fontSize() < 0.99 * oldFormat.fontSize() || newFormat.fontSize() > 1.01 * oldFormat.fontSize();
      res += attribute(needSize, true, QString("font-size=\"%1\"").arg(newFormat.fontSize()), "");
      //qDebug("updateFormat() res '%s'", qPrintable(res));
      oldFormat = newFormat;
      return res;
      }

} // namespace Ms
