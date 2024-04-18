#include <QCoreApplication>
#include <QTest>
#include <QSettings>
#include "../weapon-system-support-software/events.cpp"
#include "../weapon-system-support-software/constants.h"

class tst_file_system : public QObject
{
    Q_OBJECT

private slots:
    void tst_outputToLogFile();

    void tst_appendToLogfile();

    void tst_loadDataFromLogFile();
};

//TODO: add documentation
//TODO: add negative tests

void tst_file_system::tst_outputToLogFile()
{
    // initialize variables
    QSettings userSettings("Team Controller", "WSSS");
    Events *eventObj = new Events(false, 50);
    QString dataMsg = "15,0:00:00:150,Test message on log file";
    QString logfile = userSettings.value("logfileLocation").toString();
    QFile file(logfile + "/tst_outputToLogFile.txt");

    // create event data and add to logfile
    eventObj->loadEventData(dataMsg);

    eventObj->outputToLogFile(logfile + "/tst_outputToLogFile.txt", false);

    // check if the file can be opened in read mode
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    QTextStream in(&file);

    // verify that the text stream is not at the end of the file
    QVERIFY(!in.atEnd());

    // capture the first line in the file
    QString firstLine = in.readLine();

    // move the text stream to the next line
    QVERIFY(!in.atEnd());

    // capture the second line (contains event data)
    QString secondLine = in.readLine();
    QStringList values = secondLine.split(", ");

    // check for the correctly inputed values
    QCOMPARE(values[0], "ID: 15");
    QCOMPARE(values[1], "0:00:00:150");
    QCOMPARE(values[2], "Test message on log file");

    // remove the test file
    QVERIFY(file.remove());

    // remove event object
    delete eventObj;
}

void tst_file_system::tst_appendToLogfile()
{
    QSettings userSettings("Team Controller", "WSSS");
    Events *eventObj = new Events(false, 50);
    QString dataMsg = "15,0:00:00:011,Test message on log file";
    QString dataMsg2 = "16,0:00:11:123,Second test message on log";
    QString logfile = userSettings.value("logfileLocation").toString();
    QString logfileName = "/tst_appendToLogFile.txt";
    QFile file(logfile + logfileName);

    // load the first data msg into event obj
    eventObj->loadEventData(dataMsg);

    // create and add data msg into logfile
    eventObj->outputToLogFile(logfile + logfileName, false);

    // load second data msg
    eventObj->loadEventData(dataMsg2);

    EventNode *wkgNode = eventObj->lastEventNode;

    // call append
    eventObj->appendToLogfile(logfile + logfileName, wkgNode);

    // open file in readonly
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

    // create a text stream and capture the data lines inside the file
    QTextStream in(&file);
    QVERIFY(!in.atEnd());

    // capture first line (***ADVANCED LOG FILE DISABLED/ENABLED)
    QString firstLine = in.readLine();
    qDebug() << "First line of logfile: " << firstLine;

    // capture the first data msg in the file
    QVERIFY(!in.atEnd());
    QString dataMsginFile = in.readLine();
    QStringList values = dataMsginFile.split(", ");

    // check for the correctly inputed values
    QCOMPARE(values[0], "ID: 15");
    QCOMPARE(values[1], "0:00:00:011");
    QCOMPARE(values[2], "Test message on log file");

    // capture the second data msg in file
    QVERIFY(!in.atEnd());
    QString dataMsg2inFile = in.readLine();
    values = dataMsg2inFile.split(", ");

    // check for correct values
    QCOMPARE(values[0], "ID: 16");
    QCOMPARE(values[1], "0:00:11:123");
    QCOMPARE(values[2], "Second test message on log");

    // remove file
    QVERIFY(file.remove());

    // delete event obj
    delete eventObj;
}

#if DEV_MODE
void tst_file_system::tst_loadDataFromLogFile()
{
    QSettings userSettings("Team Controller", "WSSS");
    Events *eventObj = new Events(false, 50);
    QString dataMsg = "15,0:00:00:150,Test message on log file";
    QString logfile = userSettings.value("logfileLocation").toString();
    QString logfileName = "loadDataFromLogFile.txt";
    QFile file(logfile + logfileName);

    // create a log file to load in
    eventObj->loadEventData(dataMsg);
    eventObj->outputToLogFile(logfile + logfileName, false);

    // create an event object to store loaded data in
    Events *wkgNode = new Events(false, 50);

    // check the return of loadDataFromLogFile
    QCOMPARE(eventObj->loadDataFromLogFile(wkgNode, logfile + logfileName), SUCCESS);

    // create a secondary wkgnode for comparrison
    EventNode *wkgEventNode = wkgNode->headEventNode;

    // check for valid values
    QCOMPARE(wkgEventNode->id, 15);
    QCOMPARE(wkgEventNode->timeStamp, "0:00:00:150");
    QCOMPARE(wkgEventNode->eventString, "Test message on log file");

    // delete the file
    QVERIFY(file.remove());

    // delete the created objects
    delete eventObj;
    delete wkgNode;
}
#endif
QTEST_MAIN(tst_file_system)
#include "tst_file_system.moc"
