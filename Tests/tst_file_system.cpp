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
    QString dataMsg = "15,0:00:00,Test message on log file";
    QString logfile = userSettings.value("logfileLocation").toString();
    QFile file(logfile + "/tst_outputToLogFile.txt");

    // create event data and add to logfile
    eventObj->loadEventData(dataMsg);

    eventObj->outputToLogFile(logfile + "/tst_outputToLogFile.txt", false);

    // navigate to the

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
