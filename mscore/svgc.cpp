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
#include "libmscore/tie.h"
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

/*
void note_row(QTextStream * qts, int tick, float pos, QSet<Note *> * notes, QSet<Note *> * ongoing, TempoMap * tempomap) {
   (*qts) << (notes->isEmpty()?"R ":"N ") << tempomap->tick2time(tick) << ',' << pos;

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
}*/

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

void createAudioTrack(QJsonArray plist, Score * cs, const QString& midiname) {
  	// Mute the parts in the current excerpt
    foreach( Part * part, cs->parts()){
  	  if (!plist.contains(QJsonValue(part->id())))
      	foreach( Channel * channel, part->instrument()->channel())
      		channel->mute = true;
      //else
      //  qWarning() << "TEST RETURNED TRUE!!!" << endl;
    }

    mscore->saveMidi(cs,midiname);

    // Unmute all parts
    foreach( Part * part, cs->parts())
    	foreach( Channel * channel, part->instrument()->channel())
    		channel->mute = false;
}

void addFileToZip(MQZipWriter * uz, const QString& filename, const QString & zippath) {
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    uz->addFile(zippath,&file);
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

void createSvgCollection(MQZipWriter * uz, Score* score, const QString& prefix, const QMap<int,qreal>& t2t, const bool do_linearize);

bool MuseScore::saveSvgCollection(Score * cs, const QString& saveName, const bool do_linearize, const QString& partsName) {

  QJsonObject partsinfo;
  qreal scale_tempo = 1.0;

  if (!partsName.isEmpty()) {
    QFile partsfile(partsName);
    partsfile.open(QIODevice::ReadOnly | QIODevice::Text);
    partsinfo = QJsonDocument::fromJson(partsfile.readAll()).object();
    
    if (partsinfo.contains("scale_tempo"))
      scale_tempo = partsinfo["scale_tempo"].toDouble();
  }

  //qreal rel_tempo = cs->tempomap()->relTempo();
  cs->tempomap()->setRelTempo(scale_tempo);

  // Safety check - done after tempo change just in case. 
	QString safe = checkSafety(cs);
	if (!safe.isEmpty()) {
		qDebug() << safe << endl;
		return false;
	}

  QMap<int,qreal> tick2time; // is bypassed, if empty!

  MQZipWriter uz(saveName);

    Score* thisScore = cs->rootScore();
    if (partsinfo.isEmpty()) {

    	/*
    	// Convert to tab (list of types in stafftype.h)
    	foreach( Staff * staff, cs->staves())
    		staff->setStaffType(StaffType::preset(StaffTypes::TAB_6COMMON));
    	*/

      // Add midifile
      QString tname("1.mid");
      saveMidi(cs,tname);
      addFileToZip(&uz, tname, tname);

    	createSvgCollection(&uz, cs, QString("0/"), tick2time, do_linearize);
    }
    else {

      if (partsinfo.contains("onsets")) {
        QJsonObject onsets = partsinfo["onsets"].toObject(); 
        QJsonArray ticks = onsets["ticks"].toArray();
        QJsonArray times = onsets["times"].toArray();

        for(int i=0;i<ticks.size();i++)
          tick2time[ticks[i].toInt()] = times[i].toDouble();
      }

      // Number parts just the same as exporting metadata
      int pi = 1;
      foreach( Part * part, cs->parts()) {
        part->setId(QString::number(pi++));
      }

      //qWarning() << "JSON HAS " << partsinfo.size() << " PARTS" << endl;

      QJsonObject atracks = partsinfo["audiotracks"].toObject();
      foreach ( QString key, atracks.keys()) {
        // Synthesize the described track
        QJsonObject atobj = atracks[key].toObject();

        if (atobj["synthesize"].toBool()) {
          QString tname = key + ".mid";
          createAudioTrack(atobj["parts"].toArray(),cs,tname);
          addFileToZip(&uz, tname, tname);
        }
      }

      if (partsinfo.contains("excerpts")) {
        int ei = 0;
        QString prefix = QString::number(ei++)+'/';
        createSvgCollection(&uz, cs, prefix, tick2time, do_linearize);

  	    foreach (Excerpt* e, thisScore->excerpts())  {
  	    	Score * tScore = e->partScore();

          prefix = QString::number(ei++)+'/';
  	    	createSvgCollection(&uz, tScore, prefix, tick2time, do_linearize);
  	    }
      }
	}
  uz.close();

  // This causes segfaults on rare occasions
  //cs->tempomap()->setRelTempo(rel_tempo);

	return true;
}

QJsonArray createSvgs(Score* score, MQZipWriter * uz, const QMap<int,qreal>& t2t, QString basename);

void createSvgCollection(MQZipWriter * uz, Score* score, const QString& prefix, const QMap<int,qreal>& t2t, const bool do_linearize) {

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
            return;
         }
      }

      QJsonObject qts = QJsonObject();

      // Basic metadata
      qts["title"] = score->title().trimmed();
      qts["subtitle"] = score->subtitle().trimmed();
      qts["composer"] = score->composer().trimmed();

      // Instruments
      QJsonArray iar;
      foreach( Part * part, score->parts()) {
         QString iname = getInstrumentName(part->instrument());
         if (iname.length()>0)
            iar.push_back(iname);
      }
      qts["instruments"] = iar;

      // Initial time signature and ppm
      Fraction ts = score->firstMeasure()->timesig();
      qreal unit_dur = score->tempomap()->tick2time(1920/ts.denominator())-score->tempomap()->tick2time(0); // 480 ticks per quarter note
      QJsonArray timesig; timesig.push_back(ts.numerator()); timesig.push_back(ts.denominator());
      qts["time_signature"] = timesig;
      qts["ppm"] = round(60.0/unit_dur);

      // Total ticks/time to end.
      Measure* lastm = score->lastMeasure();

      int total_ticks = lastm->tick()+lastm->ticks();
      qts["total_ticks"] = total_ticks;
      qts["total_time"] = t2t.isEmpty()?
                            score->tempomap()->tick2time(total_ticks):
                            t2t.value(total_ticks, // Provide a linear approximation as default (important for old exercises)
                              t2t.first() + (t2t.last()-t2t.first())*total_ticks/(t2t.lastKey()-t2t.firstKey()));

      score->setPrinting(true);

      LayoutMode layout_mode = score->layoutMode();

      // Weird hack for it - but done this way pretty much all over :P
      score->undo(new ChangeLayoutMode(score, LayoutMode::PAGE));
      score->doLayout();
      
      qts["systems"] = createSvgs(score,uz,t2t,prefix+QString("Page"));   

      score->undo(new ChangeLayoutMode(score, LayoutMode::LINE));
      score->doLayout();

      qts["csystem"] = createSvgs(score,uz,t2t,prefix+QString("Line"))[0];

      score->undo(new ChangeLayoutMode(score, layout_mode));
      score->doLayout();

      score->setPrinting(false);

      uz->addFile(prefix+"metainfo.json",QJsonDocument(qts).toJson());
   }

QSet<Note *> * mark_tie_ends(QList<const Element*> const &elems) {
    QSet<Note*>* res = new QSet<Note*>; 
    foreach(const Element * e, elems) {
      //qDebug()<<e->name();
      if (e->type() == Element::Type::SLUR_SEGMENT) {
        SlurSegment * ss = (SlurSegment *)e;
        if (ss->slurTie()->type() == Element::Type::TIE) {
          //qDebug() << "TIE FOUND";
          res->insert( ((Tie *)ss->slurTie())->endNote() );
        }
      }
    }

    return res;
}


qreal * find_margins(Score * score) {

    qreal max_tm=0.0, max_bm=0.0;

    foreach( Page* page, score->pages() ) {
      foreach( System* sys, *(page->systems()) ) {

        QRectF sys_rect = sys->pageBoundingRect();
        qreal sys_top = sys_rect.top();
        qreal sys_bot = sys_rect.bottom();
        //qDebug() << "SYSTEM: " << sys_top << " " << sys_bot << endl;

        qreal max_top = sys_top, max_bot = sys_bot;

        QList<const Element*> elems;
        foreach(MeasureBase *m, sys->measures())
           m->scanElements(&elems, collectElements, false);
        sys->scanElements(&elems, collectElements, false);

        foreach(const Element * e, elems) {
          QRectF rect = e->pageBoundingRect();
          qreal top = rect.top();
          qreal bot = rect.bottom();

          if (!e->visible()) continue;

          if (top<max_top)  {
            max_top = top;
            //qDebug() << "T" << sys_top-max_top << " " << e->name() << e->height();
          }
          if (bot>max_bot) {
            max_bot = bot;
            //qDebug() << "B" << max_bot-sys_bot << " " << e->name() << e->height();
          }
        }
    
        //qDebug() << sys_top-max_top << " " << max_bot-sys_bot << endl;

        if (sys_top-max_top>max_tm) max_tm = sys_top-max_top;
        if (max_bot-sys_bot>max_bm) max_bm = max_bot-sys_bot;
      }
    }

    //qDebug() << "MARGINS: "<< max_tm << " " << max_bm << " " << score->styleP(StyleIdx::minSystemDistance)/2 << endl;

    qreal * res = new qreal[2];
    res[0] = max_tm; res[1] = max_bm;
    return res;
}

QJsonArray createSvgs(Score* score, MQZipWriter * uz, const QMap<int,qreal>& t2t, QString basename) {

      QPainter * p = NULL;
      QBuffer * svgbuf=NULL;

      qreal w=1.0, h=1.0;
      uint count = 1;

      int firstNonRest = 0, lastNonRest = 0;

      qreal mag = converterDpi / MScore::DPI;

      QString svgname = "";

      qreal * margins =  find_margins(score);
      qreal top_margin = margins[0]+0.5;
      qreal bot_margin = margins[1]+0.5;
      qreal h_margin = score->styleP(StyleIdx::staffDistance);
      if (top_margin<bot_margin) bot_margin = top_margin;
      delete [] margins;

      QSet<Note *> * tie_ends = NULL; 

      QJsonArray result;

      foreach( Page* page, score->pages() ) {
         foreach( System* sys, *(page->systems()) ) {
            QJsonObject sobj;

            QRectF sys_rect = sys->pageBoundingRect();
            w = sys_rect.width() + 2*h_margin;
            h = sys_rect.height() + top_margin + bot_margin;
 
            svgname = basename + QString::number(count++)+".svg";
            sobj["img"] = svgname;
            sobj["width"] = w*mag;
            sobj["height"] = h*mag;

            // Staff vertical positions
            QJsonArray staves;
            for(int i=0;i<sys->staves()->size();i++) {
               QRectF bbox = sys->bboxStaff(i);
               QJsonArray vbounds;
               vbounds.push_back((top_margin+bbox.top())/h);
               vbounds.push_back((top_margin+bbox.bottom())/h);
               staves.push_back(vbounds);
            }
            sobj["staves"] = staves;

            svgbuf = new QBuffer();
            svgbuf->open(QIODevice::ReadWrite);

            qreal dx = -(sys_rect.left()-h_margin), 
                  dy = -(sys_rect.top()-top_margin);
            p = getSvgPainter(svgbuf,w,h,mag);
            p->translate(dx,dy);

            // Collect together all elements belonging to this system!
            QList<const Element*> elems;
            foreach(MeasureBase *m, sys->measures())
               m->scanElements(&elems, collectElements, false);
            sys->scanElements(&elems, collectElements, false);

            qreal end_pos = -1.0;
            QJsonArray barlines;
            QMap<int,qreal> tick2pos;
            QMap<int,int> just_tied; // just the end of tied note
            QMap<int,int> is_rest;
            tie_ends = mark_tie_ends(elems);
            foreach(const Element * e, elems) {

               if (!e->visible())
                     continue;

               QPointF pos(e->pagePos());
               p->translate(pos);
               e->draw(p);

               QRectF bb = e->pageBoundingRect();
               qreal lpos = (bb.left()+dx)/w;



               if (e->type() == Element::Type::NOTE || 
                   e->type() == Element::Type::REST) {

                  ChordRest * cr = (e->type()==Element::Type::NOTE?
                                 (ChordRest*)( ((Note*)e)->chord()):(ChordRest*)e);

                  int tick = cr->segment()->tick();

                  if (!tick2pos.contains(tick) || tick2pos[tick]>lpos)
                    tick2pos[tick] = lpos;

                  // NB! ar[17] = !ar.contains(17); would not work as expected...
                  just_tied.insert(tick,just_tied.value(tick,true) && 
                                  (e->type() == Element::Type::NOTE && 
                                    tie_ends->contains((Note*)e)));
                  is_rest.insert(tick,is_rest.value(tick,true) && 
                                  (e->type() == Element::Type::REST));

                  // Update the bounds for actual audio
                  if (e->type() == Element::Type::NOTE) {
                    if (firstNonRest<0 || tick<firstNonRest) firstNonRest = tick;
                    int dur = cr->durationTypeTicks();
                    if (tick+dur > lastNonRest) lastNonRest = tick+dur;
                  }

               }
               else if (e->type() == Element::Type::MEASURE) {
                  barlines.push_back(lpos);
                  end_pos = bb.right();
               }

               p->translate(-pos);
            }

            if (end_pos>0)                     
               barlines.push_back(end_pos);


            p->end();

            svgbuf->seek(0);
            uz->addFile(svgname,svgbuf->data());
            svgbuf->close();
          
            delete p; delete svgbuf; delete tie_ends;

            QJsonArray ticks, times, positions, change, rest;

            bool use_t2t = !t2t.isEmpty();
            TempoMap * tempomap = score->tempomap();

            foreach(int tick, tick2pos.keys()){
              ticks.push_back(tick);
              times.push_back(use_t2t?t2t[tick]:tempomap->tick2time(tick));
              positions.push_back(tick2pos[tick]);

              change.push_back(int(!(just_tied[tick] || (rest.last().toInt() && is_rest[tick]))));
              rest.push_back(int(is_rest[tick]));
            }

            sobj["notes"] = positions;
            sobj["ticks"] = ticks;
            sobj["blines"] = barlines;
            sobj["times"] = times;
            sobj["is_change"] = change;
            sobj["is_rest"] = rest;

            result.push_back(sobj);
         }
      }

      // Print actual audio bounds (i.e. the interval outside which everything is silence)
      //(*qts) << "AA " << tempomap->tick2time(firstNonRest) << ',' << tempomap->tick2time(lastNonRest) << endl;

      return result;
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


  QJsonObject getPartsOnsets(Score* score) {

    // Collect together all elements belonging to this system!
    QList<const Element*> elems;
    score->scanElements(&elems, collectElements, true);

    QMap<QString,int> plt;

    QMap<QString,QList<int>> ponsets;
    QMap<QString,QList<bool>> pisrest;

    QMap<QString,int> firstNonRest, lastNonRest;

    foreach(const Element * e, elems) {
       if (e->type() == Element::Type::NOTE || 
           e->type() == Element::Type::REST) {

          ChordRest * cr = (e->type()==Element::Type::NOTE?
                         (ChordRest*)( ((Note*)e)->chord()):(ChordRest*)e);

          int tick = cr->segment()->tick();

          QString pid = cr->part()->id();

          if (!plt.contains(pid))  {
            ponsets[pid] = QList<int>();
            pisrest[pid] = QList<bool>();
            plt[pid] = -1;
          }


          // Update the bounds for actual audio
          if (e->type() == Element::Type::NOTE) {
            if (!firstNonRest.contains(pid) || tick<firstNonRest[pid]) 
              firstNonRest[pid] = tick;
            int dur = cr->durationTypeTicks();
            if (!lastNonRest.contains(pid) || tick+dur > lastNonRest[pid]) 
              lastNonRest[pid] = tick+dur;
          }

          if (tick > plt[pid]) {
             ponsets[pid].push_back(tick);

             pisrest[pid].push_back(e->type() == Element::Type::REST);

             plt[pid] = tick;
          }
          else if (tick == plt[pid]) {
            pisrest[pid].last() = pisrest[pid].last() && (e->type() == Element::Type::REST);
          } 

       }
    }

    TempoMap * tempomap = score->tempomap();
    QJsonObject jsonobj = QJsonObject();

    foreach(QString key,ponsets.keys()) {
      QJsonObject onset_obj = QJsonObject();

      QJsonArray tar, ar, nrar;

      QList<int> consets = ponsets[key];
      QList<bool> cisrest = pisrest[key]; 

      for(int i=0;i<consets.size();i++) {
        int tick = consets[i];
        tar.push_back(tick);
        ar.push_back(tempomap->tick2time(tick));
        if (!cisrest[i])
          nrar.push_back(tempomap->tick2time(tick));
      }

      onset_obj["ticks"] = tar;
      onset_obj["times"] = ar;
      onset_obj["nonrest_times"] = nrar;
      onset_obj["beg_time"] = tempomap->tick2time(firstNonRest[key]);
      onset_obj["end_time"] = tempomap->tick2time(lastNonRest[key]);

      jsonobj[key] = onset_obj;
    }

    return jsonobj;
  }

  bool MuseScore::getPartsDescriptions(Score* score, const QString& saveName) {

      qreal rel_tempo = score->tempomap()->relTempo();
      score->tempomap()->setRelTempo(1.0);

      QString safe = checkSafety(score);
      if (!safe.isEmpty()) {
        qDebug() << safe << endl;
        return false;
      }

      // Linearize the score (for getting all the onsets)
      Score * nscore = mscore->linearize(score);
      delete score;
      score = nscore;

      QFile file(saveName);
      file.open(QIODevice::WriteOnly | QIODevice::Text);
      
      QJsonObject obj = QJsonObject();

      // List all parts
      QJsonArray p_ar;
      int pi = 1;
      foreach( Part * part, score->parts()) {
        part->setId(QString::number(pi++));

        QJsonObject pobj = QJsonObject();
        pobj["id"] = part->id();
        pobj["instrument"] = getInstrumentName(part->instrument());
        pobj["name"] = part->partName();
          p_ar.append(pobj);
      }
      obj["parts"] = p_ar;


      // List all excerpts

      QJsonArray e_ar;
      int ei = 0;

      // Create the "Full" excerpt

      QJsonObject eobj = QJsonObject();

      eobj["id"] = QString::number(ei++);
      eobj["title"] = "Full";
      eobj["staves"] = stavesToJson(score);

      QJsonArray ep_ar;
      foreach(Part * part, score->parts()) {
        ep_ar.append(part->id());
      }
      eobj["parts"] = ep_ar;

      e_ar.append(eobj);

      // Create the other excerpt objects

      foreach (Excerpt* e, score->rootScore()->excerpts())  {
        eobj = QJsonObject();

        eobj["id"] = QString::number(ei++);
        eobj["title"] = e->title();
        eobj["staves"] = stavesToJson(e->partScore());

        ep_ar = QJsonArray();
        foreach(Part * part, e->parts().toSet()) {
          ep_ar.append(part->id());
        }
        eobj["parts"] = ep_ar;

        e_ar.append(eobj);
      }
      obj["excerpts"] = e_ar;

      obj["onsets"] = getPartsOnsets(score);

      Measure* lastm = score->lastMeasure();
      obj["total_ticks"] = lastm->tick()+lastm->ticks();
      obj["total_time"] = score->tempomap()->tick2time(lastm->tick()+lastm->ticks());

      // Time Signature
      QJsonObject tso = QJsonObject();
      Fraction ts = score->firstMeasure()->timesig();
      tso["numerator"] = ts.numerator();
      tso["denominator"] = ts.denominator();
      tso["unit_duration"] = score->tempomap()->tick2time(1920/ts.denominator())-score->tempomap()->tick2time(0); // 480 ticks per quarter note
      obj["timesig"] = tso;

      file.write(QJsonDocument(obj).toJson());
      file.close();

      score->tempomap()->setRelTempo(rel_tempo);

      return true;
    }
}