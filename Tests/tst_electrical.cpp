#include <QCoreApplication>
#include <QTest>
#include "../weapon-system-support-software/electrical.cpp"

// add necessary includes here

class tst_electrical : public QObject
{
    Q_OBJECT

private slots:
    void electrical_constructor();
    void test_addNode();
    void test_freeLL();

    void test_loadElecData();
    void test_loadElecData_badInputLess();
    void test_loadElecData_badInputMore();

    void test_loadElecDump();
    void test_loadElecDump_badInputLess();
    void test_loadElecDump_badInputMore();
    void test_loadElecDump_badInputType();
};

void tst_electrical::electrical_constructor()
{
    electrical *elecObj = new electrical();

    // ensure that values are empty string or 0 for constructor
    QCOMPARE(elecObj->name, "");
    QCOMPARE(elecObj->voltage, 0);
    QCOMPARE(elecObj->amps, 0);

    // check the head/last node values
    QCOMPARE(elecObj->headNode, nullptr);
    QCOMPARE(elecObj->lastNode, nullptr);

    delete elecObj;
}

void tst_electrical::test_addNode()
{
    // create electrical object and variables
    electrical *elecObj = new electrical();
    QString nodeName = "testName";
    int voltage = 15;
    int amps = 3;

    elecObj->addNode(nodeName, voltage, amps);

    // test the head/last node values
    QVERIFY(elecObj->headNode != nullptr);
    QVERIFY(elecObj->lastNode != nullptr);

    // create a working node from the headNode
    // should only be one node for test case
    electricalNode *wkgNode = elecObj->headNode;

    // test the values of the wkgnodes
    QVERIFY(wkgNode != nullptr);
    QCOMPARE(wkgNode->name, nodeName);
    QCOMPARE(wkgNode->voltage, voltage);
    QCOMPARE(wkgNode->amps, amps);

    delete elecObj;
}

void tst_electrical::test_freeLL()
{
    // create electrical object and variables
    electrical *elecObj = new electrical();
    QString name1 = "node1";
    QString name2 = "node2";
    int voltage = 14;
    int amps = 3;

    //create node(s) to test free ll
    elecObj->addNode(name1, voltage, amps);
    elecObj->addNode(name2, voltage, amps);

    // show that the head/last node is not null
    QVERIFY(elecObj->headNode != nullptr);
    QVERIFY(elecObj->lastNode != nullptr);

    // free and test if head/last node is null
    elecObj->freeLL();
    QVERIFY(elecObj->headNode == nullptr);
    QVERIFY(elecObj->lastNode == nullptr);

    delete elecObj;
}


void tst_electrical::test_loadElecData()
{
    electrical *elecObj = new electrical();

    QString dataMsg = "name, 17, 38";

    QVERIFY(elecObj->loadElecData(dataMsg) == true);

    electricalNode *wkgNode = elecObj->headNode;

    if(wkgNode != nullptr)
    {
        QCOMPARE(wkgNode->name, "name");
        QCOMPARE(wkgNode->voltage, 17);
        QCOMPARE(wkgNode->amps, 38);
    }
    else
    {
        QVERIFY(wkgNode != nullptr);
    }

    delete elecObj;
}

void tst_electrical::test_loadElecData_badInputLess()
{
    electrical *elecObj = new electrical();

    QString dataMsg = "name, 17";

    QVERIFY(elecObj->loadElecData(dataMsg) == false);

    delete elecObj;
}

void tst_electrical::test_loadElecData_badInputMore()
{
    electrical *elecObj = new electrical();

    QString dataMsg = "name, name, 17, 38";

    QVERIFY(elecObj->loadElecData(dataMsg) == false);

    delete elecObj;
}

void tst_electrical::test_loadElecDump()
{
    electrical *elecObj = new electrical();

    QString dataMsg = "name,16,29,,name_two,14,12";

    QVERIFY(elecObj->loadElecDump(dataMsg) == true);

    electricalNode *wkgNode = elecObj->headNode;

    if(wkgNode != nullptr)
    {
        QCOMPARE(wkgNode->name, "name");
        QCOMPARE(wkgNode->voltage, 16);
        QCOMPARE(wkgNode->amps, 29);

        wkgNode = wkgNode->nextNode;

        QCOMPARE(wkgNode->name, "name_two");
        QCOMPARE(wkgNode->voltage, 14);
        QCOMPARE(wkgNode->amps, 12);
    }
    else
    {
        QVERIFY(wkgNode != nullptr);
    }

    delete elecObj;
}

void tst_electrical::test_loadElecDump_badInputLess()
{
    electrical *elecObj = new electrical();

    QString dataMsg = " ";

    QVERIFY(elecObj->loadElecDump(dataMsg) == false);

    delete elecObj;
}

void tst_electrical::test_loadElecDump_badInputMore()
{
    electrical *elecObj = new electrical();

    QString dataMsg = "name,17,17,17";

    QVERIFY(elecObj->loadElecDump(dataMsg) == false);

    delete elecObj;
}

QTEST_MAIN(tst_electrical)
#include "tst_electrical.moc"
