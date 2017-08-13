/****************************************************************************
**
** Copyright (C) 2007-2015 Speedovation
** Copyright (C) 2015 creativepulse.gr 
** Contact: Speedovation Lab (info@speedovation.com)
**
** KineticWing IDE CrashHandler
** http:// kineticwing.com
** This file is part of the core classes of the KiWi Editor (IDE)
**
** Author: creativepulse.gr
** License : Apache License 2.0
**
** All rights are reserved.
*/


#include "HttpRequestWorker.h"

#include <QDateTime>
#include <QUrl>
#include <QFileInfo>
#include <QBuffer>


HttpRequestInput::HttpRequestInput() {
    initialize();
}

HttpRequestInput::HttpRequestInput(QString v_url_str, QString v_http_method) {
    initialize();
    url_str = v_url_str;
    http_method = v_http_method;
}

void HttpRequestInput::initialize() {
    var_layout = NOT_SET;
    url_str = "";
    http_method = "GET";
}

void HttpRequestInput::add_var(QString key, QString value) {
    vars[key] = value;
}

void HttpRequestInput::add_file(QString variable_name, QString local_filename, QString request_filename, QString mime_type) {
    HttpRequestInputFileElement file;
    file.variable_name = variable_name;
    file.local_filename = local_filename;
    file.request_filename = request_filename;
    file.mime_type = mime_type;
    files.append(file);
}


HttpRequestWorker::HttpRequestWorker(QObject *parent)
    : QObject(parent), manager(NULL)
{
    qsrand(QDateTime::currentDateTime().toTime_t());

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(on_manager_finished(QNetworkReply*)));
}

QString HttpRequestWorker::http_attribute_encode(QString attribute_name, QString input) {
    // result structure follows RFC 5987
    bool need_utf_encoding = false;
    QString result = "";
    QByteArray input_c = input.toLocal8Bit();
    char c;
    for (int i = 0; i < input_c.length(); i++) {
        c = input_c.at(i);
        if (c == '\\' || c == '/' || c == '\0' || c < ' ' || c > '~') {
            // ignore and request utf-8 version
            need_utf_encoding = true;
        }
        else if (c == '"') {
            result += "\\\"";
        }
        else {
            result += c;
        }
    }

    if (result.length() == 0) {
        need_utf_encoding = true;
    }

    if (!need_utf_encoding) {
        // return simple version
        return QString("%1=\"%2\"").arg(attribute_name, result);
    }

    QString result_utf8 = "";
    for (int i = 0; i < input_c.length(); i++) {
        c = input_c.at(i);
        if (
                (c >= '0' && c <= '9')
                || (c >= 'A' && c <= 'Z')
                || (c >= 'a' && c <= 'z')
                ) {
            result_utf8 += c;
        }
        else {
            result_utf8 += "%" + QString::number(static_cast<unsigned char>(input_c.at(i)), 16).toUpper();
        }
    }

    // return enhanced version with UTF-8 support
    return QString("%1=\"%2\"; %1*=utf-8''%3").arg(attribute_name, result, result_utf8);
}

void HttpRequestWorker::execute(HttpRequestInput *input) {

    // reset variables

    QByteArray request_content = "";
    response = "";
    error_type = QNetworkReply::NoError;
    error_str = "";


    // decide on the variable layout

    if (input->files.length() > 0) {
        input->var_layout = MULTIPART;
    }
    if (input->var_layout == NOT_SET) {
        input->var_layout = input->http_method == "GET" || input->http_method == "HEAD" ? ADDRESS : URL_ENCODED;
    }


    // prepare request content

    QString boundary = "";

    if (input->var_layout == ADDRESS || input->var_layout == URL_ENCODED) {
        // variable layout is ADDRESS or URL_ENCODED

        if (input->vars.count() > 0) {
            bool first = true;
            foreach (QString key, input->vars.keys()) {
                if (!first) {
                    request_content.append("&");
                }
                first = false;

                request_content.append(QUrl::toPercentEncoding(key));
                request_content.append("=");
                request_content.append(QUrl::toPercentEncoding(input->vars.value(key)));
            }

            if (input->var_layout == ADDRESS) {
                input->url_str += "?" + request_content;
                request_content = "";
            }
        }
    }
    else {
        // variable layout is MULTIPART

        boundary = "__-----------------------"
                + QString::number(QDateTime::currentDateTime().toTime_t())
                + QString::number(qrand());
        QString boundary_delimiter = "--";
        QString new_line = "\r\n";

        // add variables
        foreach (QString key, input->vars.keys()) {
            // add boundary
            request_content.append(boundary_delimiter);
            request_content.append(boundary);
            request_content.append(new_line);

            // add header
            request_content.append("Content-Disposition: form-data; ");
            request_content.append(http_attribute_encode("name", key));
            request_content.append(new_line);
            request_content.append("Content-Type: text/plain");
            request_content.append(new_line);

            // add header to body splitter
            request_content.append(new_line);

            // add variable content
            request_content.append(input->vars.value(key));
            request_content.append(new_line);
        }

        // add files
        for (QList<HttpRequestInputFileElement>::iterator file_info = input->files.begin(); file_info != input->files.end(); file_info++) {
            QFileInfo fi(file_info->local_filename);

            // ensure necessary variables are available
            if (
                    file_info->local_filename == NULL || file_info->local_filename.isEmpty()
                    || file_info->variable_name == NULL || file_info->variable_name.isEmpty()
                    || !fi.exists() || !fi.isFile() || !fi.isReadable()
                    ) {
                // silent abort for the current file
                continue;
            }

            QFile file(file_info->local_filename);
            if (!file.open(QIODevice::ReadOnly)) {
                // silent abort for the current file
                continue;
            }

            // ensure filename for the request
            if (file_info->request_filename == NULL || file_info->request_filename.isEmpty()) {
                file_info->request_filename = fi.fileName();
                if (file_info->request_filename.isEmpty()) {
                    file_info->request_filename = "file";
                }
            }

            // add boundary
            request_content.append(boundary_delimiter);
            request_content.append(boundary);
            request_content.append(new_line);

            // add header
            request_content.append(QString("Content-Disposition: form-data; %1; %2").arg(
                                       http_attribute_encode("name", file_info->variable_name),
                                       http_attribute_encode("filename", file_info->request_filename)
                                       ));
            request_content.append(new_line);

            if (file_info->mime_type != NULL && !file_info->mime_type.isEmpty()) {
                request_content.append("Content-Type: ");
                request_content.append(file_info->mime_type);
                request_content.append(new_line);
            }

            request_content.append("Content-Transfer-Encoding: binary");
            request_content.append(new_line);

            // add header to body splitter
            request_content.append(new_line);

            // add file content
            request_content.append(file.readAll());
            request_content.append(new_line);

            file.close();
        }

        // add end of body
        request_content.append(boundary_delimiter);
        request_content.append(boundary);
        request_content.append(boundary_delimiter);
    }


    // prepare connection

    QNetworkRequest request = QNetworkRequest(QUrl(input->url_str));
    request.setRawHeader("User-Agent", "Agent name goes here");

    if (input->var_layout == URL_ENCODED) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    }
    else if (input->var_layout == MULTIPART) {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "multipart/form-data; boundary=" + boundary);
    }

    if (input->http_method == "GET") {
        manager->get(request);
    }
    else if (input->http_method == "POST") {
        manager->post(request, request_content);
    }
    else if (input->http_method == "PUT") {
        manager->put(request, request_content);
    }
    else if (input->http_method == "HEAD") {
        manager->head(request);
    }
    else if (input->http_method == "DELETE") {
        manager->deleteResource(request);
    }
    else {
        QBuffer buff(&request_content);
        manager->sendCustomRequest(request, input->http_method.toLatin1(), &buff);
    }

}

void HttpRequestWorker::on_manager_finished(QNetworkReply *reply) {
    error_type = reply->error();
    if (error_type == QNetworkReply::NoError) {
        response = reply->readAll();
    }
    else {
        error_str = reply->errorString();
    }

    reply->deleteLater();

    emit on_execution_finished(this);
}
