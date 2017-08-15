//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2017 Nikolaos Hatzopoulos (nickhatz@csu.fullerton.edu)
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

wstring str2wstr(string mystr){
    wstring res(mystr.begin(), mystr.end());
    return res;
}

string wstr2str(wstring mystr){
    string res(mystr.begin(), mystr.end());
    return res;
}

pair<string,string> line2strings(string line){
    string str1, str2;
    string *mystr;
    bool myflag;
    int i;

    myflag = false;
    str1.empty();
    str2.empty();

    mystr = &str1;

    for (i = 0; i < line.size(); i++){
            if ( line[i] == ',' && myflag == false){
                    mystr = &str2;
                    myflag = true;
            }
            else{
                    if (line[i] != '\n'){
                            *mystr += line[i];
                    }

            }
    }

    return pair<string,string>(str1,str2);

}

QMap <QString,QString> read_csv(QString mypath){
        string line;
        pair<string,string> mykeyval;
        ifstream myfile(mypath.toStdString());
        QMap <QString,QString> mymap;

        if (myfile.is_open()){
                while ( getline (myfile,line) ){
                        cout << line << '\n';
                        mykeyval = line2strings(line);
                        //cout << "str1: " << mykeyval.first << " str2: " << mykeyval.second << endl;
                        mymap.insert(mykeyval.first.c_str(),mykeyval.second.c_str());

                }
                myfile.close();

        }
        else{
            cout << "Unable to open file";
        }

        return mymap;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::uploadFinished);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_SSL
    foreach (const QSslError &error, sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

void MainWindow::uploadFinished(QNetworkReply *reply)
{
    if (!reply->error())
    {
        m_file->close();
        m_file->deleteLater();
        reply->deleteLater();
    }
}

void MainWindow::onError(QNetworkReply::NetworkError err)
{
    qDebug() << " SOME ERROR!";
    qDebug() << err;
}

void MainWindow::sendReportQt(QString user_txt){
    QString minidump_path;
    QString metadata_path;

    if ( QCoreApplication::arguments().count() == 3 ){

        minidump_path = QCoreApplication::arguments().at(1);
        metadata_path = QCoreApplication::arguments().at(2);

        QStringList filePathList = minidump_path.split('/');
        QString minidump_filename = filePathList.at(filePathList.count() - 1);

        qDebug() << "minidump file: " << minidump_path;

        QString url = "https://musescore.sp.backtrace.io:6098/post?format=minidump&token=00268871877ba102d69a23a8e713fff9700acf65999b1f043ec09c5c253b9c03";

        QString boundary = "--"
                    + QString::number(
                            qrand() * (90000000000) / (RAND_MAX + 1) + 10000000000, 16);

        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,
                    "multipart/form-data; boundary=" + boundary);

        QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
        multiPart->setBoundary(boundary.toUtf8());

        QMap <QString,QString> metadata;
        metadata = read_csv(metadata_path);

        QMap<QString, QString>::iterator it;
        QHttpPart textToken;

        // metadata input

        for (it = metadata.begin(); it != metadata.end(); ++it) {
            textToken.setHeader(QNetworkRequest::ContentDispositionHeader,
                                     QVariant("form-data; name=\""+it.key()+"\""));
            textToken.setBody(QByteArray(it.value().toUtf8()));
            multiPart->append(textToken);
        }

        // user text input

        textToken.setHeader(QNetworkRequest::ContentDispositionHeader,
                                 QVariant("form-data; name=\"user_text_input\""));
        textToken.setBody(QByteArray(user_txt.toUtf8()));
        multiPart->append(textToken);

        //FILE

        m_file = new QFile(minidump_path);
        m_file->open(QIODevice::ReadOnly);

        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
        QString contentDisposition = QString("form-data; name=\"upload_file_minidump\"; filename=\""+minidump_filename+"\"");
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(contentDisposition));
        filePart.setBodyDevice(m_file);
        m_file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart


        multiPart->append(filePart);

        QNetworkReply *reply = m_manager->post(request,multiPart);

        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(onError(QNetworkReply::NetworkError)));
        //connect(reply, SIGNAL(finished()),this, SLOT(uploadFinished()));

    #ifndef QT_NO_SSL
        connect(reply, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    #endif
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();

        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        QNetworkReply::NetworkError e = reply->error();
        qDebug() << "status code: " << statusCode;
        qDebug() << "reply error: " << e;
        qDebug() << "reply: " << reply->readAll();

    }

}


void MainWindow::on_pushButton_clicked(){

    if ( ui->checkBox->isChecked() ){
        QString user_txt = ui->plainTextEdit->toPlainText();
        sendReportQt(user_txt);

    }

    close();

}

void MainWindow::on_pushButton_2_clicked(){

    if ( ui->checkBox->isChecked() ){
        QString user_txt = ui->plainTextEdit->toPlainText();
        sendReportQt(user_txt);

    }

    // TODO: restart MuseScore

}
