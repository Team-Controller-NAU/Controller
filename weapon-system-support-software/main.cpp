#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    //qt managed operations we can mostly ignore main.cpp
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //set constant style sheet for tool tips
    qApp->setStyleSheet("QToolTip { background-color: #000000; color: #FFFFFF; border: 1px solid black; } "
                        "QPushButton:disabled {"
                        "color: #666666; }");

    a.exec();

    return 0;
}
