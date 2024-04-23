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
    void test_loadData_badInput_correctDelim();
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
    QString dataMsg = "0,1,0,1,330,90,6,23,1636.14,\n";
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
 * Test case for loadData() in status.cpp
 *
 * - Tests the correct number of delimiters but bad input
 */
void tst_status::test_loadData_badInput_correctDelim()
{
    Status status;

    // check correct input
    QString dataMsg = "0,1,0,1,30,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == true);

    //get values prior to testing bad inputs into string
    QString initialStatus = status.toString();

    //test bad armed data=========================
    dataMsg = "DSL,1,0,1,30,20,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "232,1,0,1,30,20,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //============================================

    //test bad trigger data=======================
    dataMsg = "0,18,4,1,1,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,0,23332,1,1,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,438983479587324987593475938475983475983475983475983475987348597349857439857349857349875498753984759384759348759347593487593475934875934875934875934875934875983475983475394875943875934875934875934875934875934875934875934875934875"
              ",4,1,1,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //============================================

    // test bad controllerState data
    dataMsg = "0,1,0,19,1,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,asds,1,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,-2,1,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //============================================

    // test bad firing mode data
    dataMsg = "0,1,0,1,1,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,1,-3,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,1,adfds,90,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //============================================

    //test bad feed position
    dataMsg = "0,1,0,1,30,20,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,1,30,asldkf,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,1,30,-22,6,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //============================================

    //test bad firing events
    dataMsg = "0,1,0,1,30,90,-1,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,1,30,90,asdf,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,1,30,90,a,23,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //============================================

    //test bad burst length
    dataMsg = "0,1,0,1,30,90,6,-1,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    dataMsg = "0,1,0,1,30,90,6,sadf,1636.14,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //============================================

    //test bad firing rate
    dataMsg = "0,1,0,1,30,90,6,23,-1,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //test bad firing rate
    dataMsg = "0,1,0,1,30,20,6,23, scarp,\n";
    QVERIFY(status.loadData(dataMsg) == false);

    //============================================

    //confirm that none of the bad loads affected any status variables
    QCOMPARE(status.toString(), initialStatus);
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
    QString dataMsg = "00:00:02:000,6.7.2,2F5A1D3E7B9,\n";

    // call loadVersionData
    status.loadVersionData(dataMsg);

    // verify that the elapsed time, version and crc are correct
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
