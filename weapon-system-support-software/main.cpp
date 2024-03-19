#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //qt managed operations we can mostly ignore main.cpp
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    a.exec();

    return 0;
}
