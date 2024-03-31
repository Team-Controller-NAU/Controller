#include <QCoreApplication>
#include <QTest>
//#include "constants.h"
#include "../weapon-system-support-software/constants.h"
#include "../weapon-system-support-software/status.cpp"
// #include "../weapon-system-support-software/mainwindow.h"

// add necessary includes here

class tst_status : public QObject
{
    Q_OBJECT

public:

private:

private slots:
    void test_loadData();
    void test_loadVersionData();
    // void test_generateMessage();
};


/**
 * Test case for loadData() in status.cpp
 */
void tst_status::test_loadData()
{
    /* the statusMessage contains csv data in the following order
     *
        bool armed;
        TriggerStatus trigger1; -> disengaged = 1, engaged = 0
        TriggerStatus trigger2; -> disengaged = 1, engaged = 0
        ControllerState controllerState; -> RUNNING=0, BLOCKED=1, TERMINATED=2, SUSPENDED=3
        FiringMode firingMode; -> SAFE=0, SINGLE=1, BURST=2, FULL_AUTO=3
        FeedPosition feedPosition;
        //to find total events including errors, add total errors and events
        int totalFiringEvents;
        int burstLength;
        double firingRate;
     *
     */

    Status status;
    QString dataMsg = "0,1,0,1,1,90,6,23,1636.14,\n";
    QStringList values = dataMsg.split(DELIMETER);
    int totalFiringEvents_testData = values[6].toInt();
    int burstLength_testData = values[7].toInt();
    double firingRate_testData = values[8].toDouble();

    // call loadData function
    status.loadData(dataMsg);

    // verify the dataMsg passed tests
    QVERIFY(status.armed == false);
    QCOMPARE(status.trigger1, ENGAGED);
    QCOMPARE(status.trigger2, DISENGAGED);
    QCOMPARE(status.controllerState, BLOCKED);
    QCOMPARE(status.firingMode, SINGLE);
    QCOMPARE(status.feedPosition, FIRING);
    QCOMPARE(status.totalFiringEvents, totalFiringEvents_testData);
    QCOMPARE(status.burstLength, burstLength_testData);
    QCOMPARE(status.firingRate, firingRate_testData);
}

/**
 * Test case for loadVersionData() in status.cpp
 */
void tst_status::test_loadVersionData()
{
    Status status;
    QString dataMsg = "0:00:00:03,6.7.2,2F5A1D3E7B9";

    status.loadVersionData(dataMsg);

    // verify that the controller and crc versions are correct
    // QCOMPARE(status.version, CONTROLLER_VERSION);
    // QCOMPARE(status.crc, CRC_VERSION);
}

//======================================================================================
//DEV_MODE exclusive test cases
//======================================================================================



// #if DEV_MODE
// /**
//  * Test case for generateMessage() in status.cpp if we are in dev mode
//  */

// void tst_status::test_generateMessage()
// {
//     Status status;

//     QString msg = status.generateMessage();

//     qDebug() << msg;

// }
// #endif


QTEST_MAIN(tst_status)
#include "tst_status.moc"
