#include <QCoreApplication>
#include <QTest>
#include "../weapon-system-support-software/electrical.cpp"

// add necessary includes here

class tests : public QObject
{
    Q_OBJECT

public:
    tests();
    ~tests();
private:
    electrical *electricalObject;

private slots:
    void electrical_constructor();
    void test_addNode();
};

tests::tests()
{

}

tests::~tests()
{
}

void tests::electrical_constructor()
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

void tests::test_addNode()
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

QTEST_MAIN(tests)
#include "tst_electrical.moc"
