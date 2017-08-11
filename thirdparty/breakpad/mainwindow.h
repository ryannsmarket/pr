#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <fstream>
#include <QApplication>
#include "common/windows/http_upload.h"
#include <QMessageBox>

using namespace::std;

wstring str2wstr(string mystr);
string wstr2str(wstring mystr);
pair<wstring,wstring> line2strings(string line);
map <wstring,wstring> read_csv(string mypath);
void sendReport(QString user_txt);

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_2_clicked();

private slots:
    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
