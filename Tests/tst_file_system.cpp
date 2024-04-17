#include <QCoreApplication>
#include <QTest>
#include <QSettings>
#include "../weapon-system-support-software/events.cpp"

class tst_file_system : public QObject
{
    Q_OBJECT

private slots:
    void tst_outputToLogFile();
};

void tst_file_system::tst_outputToLogFile()
{
    // initialize variables
    QSettings userSettings("Team Controller", "WSSS");
    Events *eventObj = new Events();
    int id = 15;
    QString timeStamp = "0:00:00";
    QString eventString = "tst_outputToLogFile";
    QString logfile = userSettings.value("logfileLocation").toString();
    QFile file(logfile + "/tst_outputToLogFile.txt");

    // create event data and add to logfile
    eventObj->addEvent(id, timeStamp, eventString);

    eventObj->outputToLogFile(logfile + "/tst_outputToLogFile.txt", false);


    // remove test file
    if(file.remove())
    {
        qDebug() << "file removed";
    }
    else
    {
        qDebug() << "file not removed";
    }

    delete eventObj;
}

QTEST_MAIN(tst_file_system)
#include "tst_file_system.moc"
