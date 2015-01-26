//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2014 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __STARTCENTER_H__
#define __STARTCENTER_H__

#include "ui_startcenter.h"
#include "thirdparty/qtwaitingspinner/QtWaitingSpinner.h"

namespace Ms {

//---------------------------------------------------------
//   MyNetworkAccessManager
//---------------------------------------------------------

class MyNetworkAccessManager: public QNetworkAccessManager
      {
      Q_OBJECT

   public:
      MyNetworkAccessManager(QObject *parent) : QNetworkAccessManager(parent) {}

   protected:
      QNetworkReply * createRequest(Operation op,
                                    const QNetworkRequest & req,
                                    QIODevice * outgoingData = 0);
      };

//---------------------------------------------------------
//   MyWebView
//---------------------------------------------------------

class MyWebView: public QWebView
      {
      Q_OBJECT
      QtWaitingSpinner* _loadingSpinner;

   public slots:
      void link(const QUrl& url);
      void setBusy();
      void stopBusy(bool val);
      void addToJavascript();

#ifndef QT_NO_OPENSSL
      void ignoreSSLErrors(QNetworkReply *reply, QList<QSslError> sslErrors);
#endif
     
   public:
      MyWebView(QWidget *parent = 0);
      ~MyWebView();
      virtual QSize sizeHint () const;
      };

//---------------------------------------------------------
//   CookieJar
//---------------------------------------------------------

class CookieJar : public QNetworkCookieJar
      {
      Q_OBJECT

    public:
      CookieJar(QString path, QObject *parent = 0);  //load cookie
      ~CookieJar();  //save cookies

    private:
      QString file; // where to save cookies
      };

//---------------------------------------------------------
//   Startcenter
//---------------------------------------------------------

class Startcenter : public QDialog, public Ui::Startcenter
      {
      Q_OBJECT
      MyWebView* _webView;
      virtual void closeEvent(QCloseEvent*);

   private slots:
      void loadScore(QString);
      void newScore();
      void openScoreClicked(const QString &);

   signals:
      void closed(bool);

   public:
      Startcenter();
      ~Startcenter();
      void updateRecentScores();
      void writeSettings(QSettings&);
      void readSettings(QSettings&);
      };

}
#endif

