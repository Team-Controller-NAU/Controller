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
    void test_loadData_badInputLess();
    void test_loadData_badInputGreater();
    void test_loadData_badInputType();

    void test_loadVersionData();
    void test_loadVersionData_badInputLess();
    void test_loadVersionData_badInputGreater();
    void test_loadVersionData_badInputType();
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
     */
    Status status;
    QString dataMsg = "0,1,0,1,1,135,6,23,1636.14,\n";
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
    QCOMPARE(status.feedPosition, UNLOCKING);
    QCOMPARE(status.totalFiringEvents, totalFiringEvents_testData);
    QCOMPARE(status.burstLength, burstLength_testData);
    QCOMPARE(status.firingRate, firingRate_testData);
}

/**
 * Test case for loadData() in status.cpp
 *
 * - tests the input having less items than the desired input
 */
void tst_status::test_loadData_badInputLess()
{
    // initialize variables
    Status status;
    QString dataMsg = "0,1,0,1,1,135,6,23\n";

    // make sure loadData returns false
    QVERIFY(status.loadData(dataMsg) == false);
}

/**
 * Test case for loadData() in status.cpp
 *
 * - tests the input having more items than the desired input
 */
void tst_status::test_loadData_badInputGreater()
{
    // initialize variables
    Status status;
    QString dataMsg = "0,1,0,1,1,135,6,23,1636.14,100,\n";

    // make sure loadData returns false
    QVERIFY(status.loadData(dataMsg) == false);
}

/**
 * Test case for loadData() in status.cpp
 *
 * - tests the input having the wrong type
 */
void tst_status::test_loadData_badInputType()
{
    // initialize variables
    Status status;
    QString dataMsg = "0, 1.0 ,0,1,1,135, 6.2 ,23,1636.14,\n";

    status.loadData(dataMsg);

    // trigger1 and totalfiringEvents output results in 0
    QCOMPARE(status.trigger1, 0);
    QCOMPARE(status.totalFiringEvents, 0);

}

//======================================================================================
//DEV_MODE exclusive test cases
//======================================================================================
#if DEV_MODE

/**
 * Test case for loadVersionData() in status.cpp
 */
void tst_status::test_loadVersionData()
{
    // initialize variables
    Status status;
    QString dataMsg = "00:00:02,6.7.2,2F5A1D3E7B9,\n";
    QStringList values = dataMsg.split(DELIMETER);
    QTime controllerTime = QTime::fromString(values[0]);

    // call loadVersionData
    status.loadVersionData(dataMsg);

    // verify that the elapsed time, version and crc are correct
    QCOMPARE(status.elapsedControllerTime, controllerTime);
    QCOMPARE(status.version, CONTROLLER_VERSION);
    QCOMPARE(status.crc, CRC_VERSION);
}

/**
 * Test case for loadVersionData() in status.cpp
 *
 * - tests input have less items
 */
void tst_status::test_loadVersionData_badInputLess()
{
    //initialize variables
    Status status;
    QString dataMsg = "6.7.2,2F5A1D3E7B9,\n";

    // make sure loadVersionData returns false
    QVERIFY(status.loadVersionData(dataMsg) == false);
}

/**
 * Test case for loadVersionData() in status.cpp
 *
 * - tests input have more items
 */
void tst_status::test_loadVersionData_badInputGreater()
{
    //initialize variables
    Status status;
    QString dataMsg = "00:00:02,6.7.2,2F5A1D3E7B9,7,\n";

    // make sure loadVersionData returns false
    QVERIFY(status.loadVersionData(dataMsg) == false);
}

/**
 * Test case for loadVersionData() in status.cpp
 *
 * - tests input having improper type for time
 */
void tst_status::test_loadVersionData_badInputType()
{
    //initialize variables
    Status status;
    QString dataMsg = " 00.00.02 ,6.7.2,2F5A1D3E7B9,\n";

    // make sure loadVersionData returns false
    QVERIFY(status.loadVersionData(dataMsg) == false);
}
#endif


QTEST_MAIN(tst_status)
#include "tst_status.moc"
