//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 File handling: loading and saving.
 */

#include "config.h"
#include "globals.h"
#include "musescore.h"
#include "scoreview.h"
#include "exportmidi.h"
#include "libmscore/xml.h"
#include "libmscore/element.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/sig.h"
#include "libmscore/clef.h"
#include "libmscore/key.h"
#include "instrdialog.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/dynamic.h"
#include "file.h"
#include "libmscore/style.h"
#include "libmscore/tempo.h"
#include "libmscore/select.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/utils.h"
#include "libmscore/barline.h"
#include "libmscore/slur.h"
#include "libmscore/hairpin.h"
#include "libmscore/ottava.h"
#include "libmscore/textline.h"
#include "libmscore/pedal.h"
#include "libmscore/trill.h"
#include "libmscore/volta.h"
#include "libmscore/timesig.h"
#include "libmscore/box.h"
#include "libmscore/excerpt.h"
#include "libmscore/system.h"
#include "libmscore/tuplet.h"
#include "libmscore/keysig.h"
#include "libmscore/measure.h"
#include "libmscore/undo.h"
#include "libmscore/repeatlist.h"
#include "libmscore/beam.h"
#include "libmscore/stafftype.h"
#include "libmscore/revisions.h"
#include "libmscore/lyrics.h"
#include "libmscore/segment.h"
#include "libmscore/tempotext.h"
#include "libmscore/sym.h"
#include "libmscore/image.h"
#include "synthesizer/msynthesizer.h"
#include "svggenerator.h"
#include "libmscore/tiemap.h"
#include "libmscore/measurebase.h"

#include "importmidi/importmidi_instrument.h"

#include "libmscore/chordlist.h"
#include "libmscore/mscore.h"
#include "thirdparty/qzip/qzipreader_p.h"
#include "thirdparty/qzip/qzipwriter_p.h"


namespace Ms {
//---------------------------------------------------------
//   saveSvgCollection
//---------------------------------------------------------


QPainter * getSvgPainter(QIODevice * device, qreal width, qreal height, qreal scale) 
   {
      SvgGenerator * printer = new SvgGenerator();
      printer->setResolution(converterDpi);
      printer->setTitle(QString(""));
      printer->setDescription(QString("Generated by MuseScore %1").arg(VERSION));
      printer->setOutputDevice(device);

      qreal w = width; //* MScore::DPI;
      qreal h = height; //* MScore::DPI;
      printer->setSize(QSize(w * scale, h * scale));
      printer->setViewBox(QRectF(0.0, 0.0, w * scale, h * scale));


      QPainter * p = new QPainter(printer);
      p->setRenderHint(QPainter::Antialiasing, true);
      p->setRenderHint(QPainter::TextAntialiasing, true);
      p->scale(scale, scale);

      return p;
   }


void note_row(QTextStream * qts, int tick, float pos, QSet<Note *> * notes, QSet<Note *> * ongoing) {
   (*qts) << (notes->isEmpty()?"R ":"N ") << tick << ',' << pos;

   // Notes still sounding from before
   QSetIterator<Note *> i(*ongoing);
   while (i.hasNext()) {
      Note * cur = i.next();
      int end = cur->chord()->tick() + cur->chord()->actualTicks();
      if (end<=tick) ongoing->remove(cur);
      else (*qts) << ' ' << cur->pitch();
   }

   // New notes 
   if (!notes->isEmpty()) {
      (*qts) << ';';
      QSetIterator<Note *> j(*notes);
      while (j.hasNext()) {
         Note * cur = j.next();
        (*qts) << ' ' << cur->pitch();
        (*ongoing) << cur;
      }
      notes->clear();
   }

   (*qts) << endl;
}

QString getInstrumentName(Instrument * in) {
   QString iname = in->trackName();
   if (!iname.isEmpty())
      return iname;
   
   return MidiInstr::instrumentName(MidiType::GM,in->channel(0)->program,in->useDrumset());
}


/*bool MuseScore::saveMultipartSvgC(Score * cs, const QString& saveName, const bool do_linearize, const QString& partsName) {
      Score* thisScore = cs->rootScore();
      foreach (Excerpt* e, thisScore->excerpts())  {
	      	Score * pScore = e->partScore();
	      	saveSvgCollection(pScore, ???? ,do_linearize, partsName);
	      }
}*/

void createBackingTrack(Excerpt * e, Score * cs, const QString& midiname) {
	// Mute the parts in the current excerpt
	foreach( Part * part, e->parts())
    	foreach( Channel * channel, part->instrument()->channel())
    		channel->mute = true;

    
    mscore->saveMidi(cs,midiname);

    // Unmute the parts in the current excerpt
    foreach( Part * part, e->parts())
    	foreach( Channel * channel, part->instrument()->channel())
    		channel->mute = false;
}

void addFileToZip(MQZipWriter * uz, const QString& filename) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    uz->addFile(filename,&file);
    file.remove();
}

// Check if the file might be a clever construction that would take ages to parse
QString checkSafety(Score * score) {

	if (score->rootScore()->excerpts().size() > 20) return QString("Too many parts");

	score->repeatList()->unwind();
	if (score->repeatList()->size() > 100) return QString("Too many repeats");

	RepeatSegment * rs = score->repeatList()->last();
	int endTick= rs->tick + rs->len;
	qreal endtime = score->tempomap()->tick2time(endTick);

	if (endtime>60*10) return QString("Piece lasts too long");

	// Empty string to signify 'no complaints'
	return QString();
}

bool createSvgCollection(MQZipWriter * uz, Score* score, const bool do_linearize);

bool MuseScore::saveSvgCollection(Score * cs, const QString& saveName, const bool do_linearize, const QString& partsName) {

	QString safe = checkSafety(cs);
	if (!safe.isEmpty()) {
		qDebug(safe.toLatin1());
		return false;
	}

	int i = '1';

    Score* thisScore = cs->rootScore();
    if (thisScore->excerpts().count()==0) {

		MQZipWriter uz(saveName);

		/*
		// Convert to tab (list of types in stafftype.h)
		foreach( Staff * staff, cs->staves())
			staff->setStaffType(StaffType::preset(StaffTypes::TAB_6COMMON));
		*/

    	createSvgCollection(&uz, cs, do_linearize);

    	// Add midifile
        QString tname("track.mid");
        saveMidi(cs,tname);
        addFileToZip(&uz, tname);

    	uz.close();
    }
    else {
    	/*if (!partsName.isEmpty()) {
		    QFile file(partsName);
		    file.open(QIODevice::ReadOnly | QIODevice::Text);
		    QJsonObject sett2 = QJsonDocument::fromJson(file.readAll()).object();
		    qWarning() << sett2["title"].toString();  // <- print my title
      	}*/
	    foreach (Excerpt* e, thisScore->excerpts())  {
	    	Score * tScore = e->partScore();

	    	MQZipWriter uz(i + saveName);

	    	createSvgCollection(&uz, tScore, do_linearize);

	    	// Add midifile
	        QString tname("track.mid");
	        saveMidi(tScore,tname);
	        addFileToZip(&uz, tname);

	    	QString bname("backing.mid");
	    	createBackingTrack(e,cs,bname);
	    	addFileToZip(&uz, bname);

	    	uz.close();

	    	i++;
	    }
	}

	return true;
}


QJsonArray stavesToJson(Score * score) {
	QJsonArray s_ar = QJsonArray();
    foreach( Staff * staff, score->staves()) {
    	QJsonObject sobj = QJsonObject();

    	sobj["type"] = staff->isPitchedStaff()?"standard":(
    					staff->isDrumStaff()?"percussion":(
    					 staff->isTabStaff()?"tab":"unknown"));

    	s_ar.append(sobj);
    }

    return s_ar;
}

bool MuseScore::getPartsDescriptions(Score* score, const QString& saveName) {

	QString safe = checkSafety(score);
	if (!safe.isEmpty()) {
		qDebug(safe.toLatin1());
		return false;
	}

	QFile file(saveName);
	file.open(QIODevice::WriteOnly | QIODevice::Text);
	
	QJsonObject obj = QJsonObject();

	// List all parts
	QJsonArray p_ar = QJsonArray();
	int pi = 1;
    foreach( Part * part, score->parts().toSet()) {
    	part->setId(QString::number(pi++));

    	QJsonObject pobj = QJsonObject();
	    pobj["id"] = part->id();
	    pobj["instrument"] = getInstrumentName(part->instrument());
	    pobj["name"] = part->partName();
        p_ar.append(pobj);
    }
    obj["parts"] = p_ar;

    // List staves in main score
    obj["staves"] = stavesToJson(score);

    // List all excerpts

    QJsonArray e_ar = QJsonArray();
	foreach (Excerpt* e, score->rootScore()->excerpts())  {
		QJsonObject eobj = QJsonObject();

		QJsonArray ep_ar = QJsonArray();
		foreach(Part * part, e->parts().toSet()) {
			ep_ar.append(part->id());
		}
		eobj["parts"] = ep_ar;

	    eobj["staves"] = stavesToJson(e->partScore());

        e_ar.append(eobj);
    }
    obj["excerpts"] = e_ar;

	file.write(QJsonDocument(obj).toJson());
	file.close();

    return true;
}

int createSvgs(Score* score, MQZipWriter * uz, QTextStream * qts, QString basename);

bool createSvgCollection(MQZipWriter * uz, Score* score, const bool do_linearize) {

      score->repeatList()->unwind();
      if (score->repeatList()->size()>1) {

         if (do_linearize) {
            Score * nscore = mscore->linearize(score);
            delete score;
            score = nscore;
         }
         else {
            QMessageBox::critical(0, QObject::tr("SVC export Failed"),
               QObject::tr("Score contains repeats. Please linearize!"));
            return false;
         }
      }

      qreal rel_tempo = score->tempomap()->relTempo();
      score->tempomap()->setRelTempo(1.0);

      //if (!metafile.open(QIODevice::WriteOnly))
      //      return false;

      QBuffer metabuf;
      metabuf.open(QIODevice::ReadWrite);
      QTextStream qts(&metabuf);
      //QTextStream qts(stdout, QIODevice::WriteOnly);


      qts << "#TITLE " << score->title().trimmed() << endl;
      qts << "#SUBTITLE " << score->subtitle().trimmed() << endl;
      qts << "#COMPOSER " << score->composer().trimmed() << endl;

      score->setPrinting(true);

      foreach( Part * part, score->parts()) {
         QString iname = getInstrumentName(part->instrument());
         if (iname.length()>0)
            qts << "I " << iname << endl;
      }

      LayoutMode layout_mode = score->layoutMode();

      qts << "#PAGE" << endl;

      score->undo(new ChangeLayoutMode(score, LayoutMode::PAGE));
      score->doLayout();
      
      int ticksFromBeg = createSvgs(score,uz,&qts,QString("Page"));   

      qts << "#LINE" << endl;

      // Weird hack for it - but done this way pretty much all over :P
      score->undo(new ChangeLayoutMode(score, LayoutMode::LINE));
      score->doLayout();

      createSvgs(score,uz,&qts,QString("Line"));

      qDebug("Total ticks: %i. End time: %f",ticksFromBeg,score->tempomap()->tick2time(ticksFromBeg));
      qts << "AT " << score->tempomap()->tick2time(0) << ',' << score->tempomap()->tick2time(ticksFromBeg) << endl;
      qts << "TT " << ticksFromBeg << endl;

      uz->addFile("metainfo.meta",metabuf.data());
      score->setPrinting(false);



      score->tempomap()->setRelTempo(rel_tempo);

      score->undo(new ChangeLayoutMode(score, layout_mode));
      score->doLayout();


      return true;
   }

int createSvgs(Score* score, MQZipWriter * uz, QTextStream * qts, QString basename) {

      Measure * measure = NULL;
      QPainter * p = NULL;
      TimeSig * timesig = NULL;
      QBuffer * svgbuf=NULL;

      qreal w=1.0, h=1.0;
      uint count = 1;

      int ticksFromBeg = 0;

      int firstNonRest = 0, lastNonRest = 0;

      double mag = converterDpi / MScore::DPI;

      QString svgname = "";

      foreach( Page* page, score->pages() ) {

      	 qreal staff_dist = score->styleP(StyleIdx::minSystemDistance);

         foreach( System* sys, *(page->systems()) ) {

         	// These are values you can manually edit under style->general->Page
            qreal top_margin = score->styleP(StyleIdx::staffUpperBorder);
            qreal bot_margin = score->styleP(StyleIdx::staffLowerBorder);
            qreal h_margin = score->styleP(StyleIdx::staffDistance); 

            if (sys->isVbox()) { // Non-staff things - heading, for instance
               //qDebug("VB %f %f, %f %f %f %f", mtop, mbot, top_margin,sys->vbox()->topGap(),bot_margin, sys->vbox()->bottomGap());
               top_margin  = sys->vbox()->topGap();
               bot_margin = sys->vbox()->bottomGap();
            }

            //qDebug("TOP %f BOT %f FALLBACK %f",top_margin,bot_margin,staff_dist/2);//score->styleP(StyleIdx::minSystemDistance));

            top_margin = qMax(staff_dist/2,top_margin);
            bot_margin = qMax(staff_dist/2,bot_margin);

            //qDebug("Margins: %f %f",top_margin,bot_margin);

            w = sys->width() + 2*h_margin;
            h = sys->height() + top_margin + bot_margin;
 

            svgname = basename + QString::number(count++)+".svg";
            (*qts) << "F " << svgname << ' ' << w*mag << ',' << h*mag << endl;

            // Staff vertical positions
            for(int i=0;i<sys->staves()->size();i++) {
               QRectF bbox = sys->bboxStaff(i);
               (*qts) << "S " << (top_margin+bbox.top())/h << "," << (top_margin+bbox.bottom())/h << endl;
            }

            //(*qts) << ticksFromBeg << ',' << 0.0 << '\n';

            svgbuf = new QBuffer();
            svgbuf->open(QIODevice::ReadWrite);

            p = getSvgPainter(svgbuf,w,h,mag);
            p->translate(-(sys->pagePos().rx()-h_margin), -(sys->staffYpage(0)-top_margin) );

            // Collect together all elements belonging to this system!
            QList<const Element*> elems;
            foreach(MeasureBase *m, sys->measures())
               m->scanElements(&elems, collectElements, false);
            sys->scanElements(&elems, collectElements, false);


            qreal end_pos = -1.0;

            int last_tick = -1;
            float last_pos = 0.0;

            QSet<Note *> notes;
            QSet<Note *> ongoing;

            foreach(const Element * e, elems) {

            //qDebug("%s", qPrintable(e->userName()) );

            //auto drawElementOnP = [&] (void * v, Element * e) {

               if (e->type() == Element::Type::MEASURE) {
                  if (measure!=NULL) ticksFromBeg+=measure->ticks();
                  measure = (Measure *)e;
               }

               /*if (e->visible()) { 
                  qDebug("ELEMENT %s %f", qPrintable(e->userName()),e->pagePos().ry());
                  if (e->parent()) 
                     qDebug(" PARENT %s", qPrintable(e->parent()->userName()));
               }*/

               if (!e->visible())
                     continue;

               QPointF pos(e->pagePos());
               p->translate(pos);
               e->draw(p);

               QTransform world = p->worldTransform(); // Get global translation i.e. page+element

               if (e->type() == Element::Type::NOTE || 
                   e->type() == Element::Type::REST) {

                  ChordRest * cr = (e->type()==Element::Type::NOTE?
                                 (ChordRest*)( ((Note*)e)->chord()):(ChordRest*)e);

                  int tick = cr->segment()->tick();

                  if (tick > last_tick) {
                     //tick != last_tick

                     if (last_tick>=0) {
                        note_row(qts,last_tick,last_pos,&notes,&ongoing);
                     }

                     // Update the bounds for actual audio
                     if (e->type() == Element::Type::NOTE) {
                      if (firstNonRest<0 || tick<firstNonRest) firstNonRest = tick;
                      int dur = cr->durationTypeTicks();
                      if (tick+dur > lastNonRest) lastNonRest = tick+dur;
                     }

                     //qDebug("%i (%i) - %f",cr->segment()->tick(),ticksFromBeg,world.m31());
                     last_tick = tick;

                     last_pos = world.m31()/(w*mag);
                  }
                  else if (tick!=last_tick) { // SHOULD NOT HAPPEN
                     (*qts) << "# Omitted " << tick << ',' << last_tick << ' ';
                     (*qts) << last_pos << ' ' << world.m31()/(w*mag) << endl; 
                  }
                  else {
                     if (world.m31()/(w*mag)<last_pos) // correct for long rests
                       last_pos = world.m31()/(w*mag);
                  }

                  if (e->type() == Element::Type::NOTE) {
                     notes << ((Note*)e);
                  }

               }
               else if (e->type() == Element::Type::MEASURE) {
                  
                  if (last_tick>=0) {
                     note_row(qts,last_tick,last_pos,&notes,&ongoing);

                     last_tick = -1;
                  }

                  last_pos = world.m31()/(w*mag);
                  (*qts) << "B " << world.m31()/(w*mag) << ',' << ((Measure*)e)->ticks() << endl;
                  end_pos = (world.m31() + mag*e->bbox().width())/(w*mag);
               }
               else if (e->type() == Element::Type::TIMESIG) {

                  TimeSig * cur_ts =  ((TimeSig*)e);

                  if (timesig==NULL || 
                      timesig->numerator()!=cur_ts->numerator() || 
                      timesig->denominator()!=cur_ts->denominator()) {
                     (*qts) << "TS " << cur_ts->numerator() << ',' << cur_ts->denominator() << endl;
                     timesig = cur_ts;
                  }
               }

               p->translate(-pos);

            //};
            //sys->scanElements(NULL, drawElementOnP,true);
            }

            if (end_pos>0) {
               if (last_tick>=0) {
                  note_row(qts,last_tick,last_pos,&notes,&ongoing);
               }
                     
               (*qts) << "B " << end_pos << endl;
            }


            p->end();

            svgbuf->seek(0);
            uz->addFile(svgname,svgbuf->data());
            svgbuf->close();
            
            delete p; delete svgbuf;
         }
      }

      // Print actual audio bounds (i.e. the interval outside which everything is silence)
      (*qts) << "AA " << score->tempomap()->tick2time(firstNonRest) << ',' << score->tempomap()->tick2time(lastNonRest) << endl;

      if (measure!=NULL) ticksFromBeg+=measure->ticks();

      return ticksFromBeg;
}


void appendCopiesOfMeasures(Score * score,Measure * fm,Measure * lm) {

      Score * fscore = fm->score();

      fscore->select(fm,SelectType::SINGLE,0);
      fscore->select(lm,SelectType::RANGE,score->nstaves()-1);
      QString mimeType = fscore->selection().mimeType();
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, fscore->selection().mimeData());
      fscore->deselectAll();


      Measure * last = 0;
      last = static_cast<Measure*>(score->insertMeasure(Element::Type::MEASURE,0,false));

      score->select(last);
      score->startCmd();
      score->cmdPaste(mimeData,0);
      score->endCmd();
      score->deselectAll();
   }

   Score * MuseScore::linearize(Score* old_score)
      {

      Score* score = old_score->clone();
      
      //old_score->deselectAll(); 
      // Figure out repeat structure and traverse it
      old_score->repeatList()->unwind();
      old_score->setPlaylistDirty();

      bool copy=false;
      foreach (const RepeatSegment* rs, *(old_score->repeatList()) ) {
         int startTick  = rs->tick;
         int endTick    = startTick + rs->len;

         qDebug("Segment %i-%i",startTick,endTick);

         Measure * mf = old_score->tick2measure(startTick);
         Measure * ml = ml;


         if (!copy && startTick==0) ml = score->tick2measure(startTick);

         for (ml=mf; ml; ml = ml->nextMeasure()) {
            if (ml->tick() + ml->ticks() >= endTick) break;
         }

         // First segment can be done in-place
         if (!copy) {
            if (startTick==0) // keep first segment in place
               ml = ml?ml->nextMeasure():ml;
            else { // remove everything and copy things over
               copy=true;
               ml = score->firstMeasure();
            }

            // Remove all measures past the first jump
            if (ml) {  
               score->select(ml,SelectType::SINGLE);
               score->select(score->lastMeasure(),SelectType::RANGE);
               score->startCmd();
               score->cmdDeleteSelectedMeasures();
               score->endCmd(); 
            }
         }
         
         if (copy) appendCopiesOfMeasures(score,mf,ml);

         copy = true;;
      }

      
      // Remove volta markers
      for (const std::pair<int,Spanner*>& p : score->spannerMap().map()) {
         Spanner* s = p.second;
         if (s->type() != Element::Type::VOLTA) continue;
         //qDebug("VOLTA!");
         score->removeSpanner(s);
      }

      for(Measure * m = score->firstMeasure(); m; m=m->nextMeasure()) {
         // Remove repeats
         if (m->repeatFlags()!=Repeat::NONE) {


            m->setRepeatFlags(Repeat::NONE);
            m->setRepeatCount(0);
         }
         // Remove coda/fine labels and jumps
         for (auto e : m->el())
            if (e->type() == Element::Type::MARKER || 
               e->type() == Element::Type::JUMP) {
               //qDebug("JUMP? %s",qPrintable(e->userName()));
               score->deleteItem(e);
            }
      }

      score->lastMeasure()->setEndBarLineType(BarLineType::END, false);
      
      // score->deselectAll();
      //old_score->deselectAll();

      // Postprocessing stuff
      score->setLayoutAll(true);
      score->fixTicks();
      score->doLayout();

      return score;
   }

   bool MuseScore::newLinearized(Score* old_score)
   {
      Score * score = linearize(old_score);
      setCurrentScoreView(appendScore(score));

      return true;
      }
}
