#include <QCoreApplication>
#include <QTest>
#include "../weapon-system-support-software/events.cpp"

// add necessary includes here

class tst_events : public QObject
{
    Q_OBJECT

private slots:
    void events_constructor();
    void test_addEvent();
    void test_addError();
    void test_freeLinkedLists();
    void test_clearError();
    void test_getNextNodeToPrint();
    void test_nodeToString();
    void test_stringToNode();
};

/**
 * Test case for the events class initialization constructor in events.cpp
 */
void tst_events::events_constructor()
{
    // create a new events class object
    Events *eventObj = new Events();

    // ensure that member variables are 0 upon initialization
    QCOMPARE(eventObj->totalEvents, 0);
    QCOMPARE(eventObj->totalErrors, 0);
    QCOMPARE(eventObj->totalNodes, 0);
    QCOMPARE(eventObj->totalCleared, 0);

    // ensure event pointers are empty/null
    QCOMPARE(eventObj->headEventNode, nullptr);
    QCOMPARE(eventObj->lastEventNode, nullptr);

    // ensure error pointers are empty/null
    QCOMPARE(eventObj->headErrorNode, nullptr);
    QCOMPARE(eventObj->lastErrorNode, nullptr);

    // check default bool value
    QCOMPARE(eventObj->dataLoadedFromLogFile, false);

    // free
    delete eventObj;
}

/**
 * Test case for addEvent() in events.cpp
 */
void tst_events::test_addEvent()
{
    // create a new events class object
    Events *eventObj = new Events();

    // providing a seed value for random values
    srand((unsigned) time(nullptr));

    // set up some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "0:01:15";
    QString eventString = "Sample test message 1";

    // attempt to add the node
    eventObj->addEvent(id, timeStamp, eventString);

    // test the head/last event node values
    QVERIFY(eventObj->headEventNode != nullptr);
    QVERIFY(eventObj->lastEventNode != nullptr);

    // create a working node from the headEventNode
    EventNode *wkgEventNode = eventObj->headEventNode;

    // test the values
    QVERIFY(wkgEventNode != nullptr);
    QCOMPARE(wkgEventNode->id, id);
    QCOMPARE(wkgEventNode->timeStamp, timeStamp);
    QCOMPARE(wkgEventNode->eventString, eventString);
    QCOMPARE(eventObj->totalEvents, 1);
    QCOMPARE(eventObj->totalNodes, 1);

    // free
    delete eventObj;
}

/**
 * Test case for addError() in events.cpp
 */
void tst_events::test_addError()
{
    // create a new events class object
    Events *eventObj = new Events();

    // providing a seed value for random values
    srand((unsigned) time(nullptr));

    // set up some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "0:01:15";
    QString eventString = "Sample test message 1";
    bool cleared = rand() % 1; // randomly determine if this is cleared or not

    // attempt to add the node
    eventObj->addError(id, timeStamp, eventString, cleared);

    // test the head/last event node values
    QVERIFY(eventObj->headErrorNode != nullptr);
    QVERIFY(eventObj->lastErrorNode != nullptr);

    // create a working node from the headErrorNode
    EventNode *wkgErrorNode = eventObj->headErrorNode;

    // test the values
    QVERIFY(wkgErrorNode != nullptr);
    QCOMPARE(wkgErrorNode->id, id);
    QCOMPARE(wkgErrorNode->timeStamp, timeStamp);
    QCOMPARE(wkgErrorNode->eventString, eventString);
    QCOMPARE(wkgErrorNode->cleared, cleared);
    QCOMPARE(wkgErrorNode->error, true);
    QCOMPARE(eventObj->totalErrors, 1);
    QCOMPARE(eventObj->totalNodes, 1);

    // free
    delete eventObj;
}

/**
 * Test case for freeLinkedLists() in events.cpp
 */
void tst_events::test_freeLinkedLists()
{
    // create event object
    Events *eventObj = new Events();

    // providing a seed value for random values
    srand((unsigned) time(nullptr));

    // set up some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "0:01:15";
    QString eventString = "Sample test message 1";
    bool cleared = rand() % 1; // randomly determine if this is cleared or not

    // create nodes to test freeing both linked lists
    eventObj->addEvent(id, timeStamp, eventString);
    eventObj->addError(id + 1, timeStamp, eventString, cleared);

    // show that the head/last nodes are not null
    QVERIFY(eventObj->headEventNode != nullptr);
    QVERIFY(eventObj->lastEventNode != nullptr);
    QVERIFY(eventObj->headErrorNode != nullptr);
    QVERIFY(eventObj->lastErrorNode != nullptr);

    // free and test if head/last node is null
    eventObj->freeLinkedLists();
    QCOMPARE(eventObj->headEventNode, nullptr);
    QCOMPARE(eventObj->lastEventNode, nullptr);
    QCOMPARE(eventObj->headErrorNode, nullptr);
    QCOMPARE(eventObj->lastErrorNode, nullptr);

    // ensure that member variables are reset back to 0
    QCOMPARE(eventObj->totalEvents, 0);
    QCOMPARE(eventObj->totalErrors, 0);
    QCOMPARE(eventObj->totalNodes, 0);
    QCOMPARE(eventObj->totalCleared, 0);

    // free
    delete eventObj;
}

/**
 * Test case for clearError() in events.cpp
 */
void tst_events::test_clearError()
{
    // create event object
    Events *eventObj = new Events();

    // providing a seed value for random values
    srand((unsigned) time(nullptr));

    // set up some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "0:01:15";
    QString eventString = "Sample test message 1";

    // create and add node to linked list
    eventObj->addError(id, timeStamp, eventString, false);

    // test the head/last event node values
    QVERIFY(eventObj->headErrorNode != nullptr);
    QVERIFY(eventObj->lastErrorNode != nullptr);

    // create a working node from the headErrorNode
    EventNode *wkgErrorNode = eventObj->headErrorNode;

    // verify that this error is not cleared yet
    QCOMPARE(wkgErrorNode->cleared, false);
    QCOMPARE(eventObj->totalCleared, 0);

    // clear the error
    eventObj->clearError(id);

    // verify that this error is now cleared
    QCOMPARE(wkgErrorNode->cleared, true);
    QCOMPARE(eventObj->totalCleared, 1);

    // free
    delete eventObj;
}

/**
 * Test case for getNextNodeToPrint() in events.cpp
 */
void tst_events::test_getNextNodeToPrint()
{
    // create event object
    Events *eventObj = new Events();

    // providing a seed value for random values
    srand((unsigned) time(nullptr));

    // set up some variables
    QString timeStamp = "0:01:15";
    QString eventString = "Sample test message 1";
    bool cleared = rand() % 1; // randomly determine if this is cleared or not

    // get random ID values, ensuring they are not ever equal
    int eventId = 1 + (rand() % 100); // get a random ID value, 1-100
    int errorId = eventId;

    // do this until we have two different random numbers
    while (eventId == errorId)
    {
        errorId = 1 + (rand() % 100);
    }

    // create nodes
    eventObj->addEvent(eventId, timeStamp, eventString);
    eventObj->addError(errorId, timeStamp, eventString, cleared);

    // create working nodes from the headErrorNode and headEventNode
    EventNode *wkgErrorNode = eventObj->headErrorNode;
    EventNode *wkgEventNode = eventObj->headEventNode;
    bool printErr; // placeholder bool

    // get next node to print by ID
    EventNode *nextNode = eventObj->getNextNodeToPrint(wkgEventNode, wkgErrorNode, printErr);

    // verify that it captured the proper node
    // we are checking that the next node ID is the smaller of the two IDs in both linked lists
    QCOMPARE(nextNode->id, std::min(eventId, errorId));

    // free
    delete eventObj;
}

/**
 * Test case for nodeToString() in events.cpp
 */
void tst_events::test_nodeToString()
{
    // set up an example node
    Events *eventObj = new Events();
    EventNode *exampleNode = new EventNode();

    // provide a seed value for random values
    srand((unsigned) time(nullptr));

    // create some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "0:01:15";
    QString eventString = "Sample test message 1";

    // input values
    exampleNode->id = id;
    exampleNode->timeStamp = timeStamp;
    exampleNode->eventString = eventString;

    // get string
    QString response = eventObj->nodeToString(exampleNode);
    QString expectedResponse = "ID: " + QString::number(id) + DELIMETER + " " + timeStamp + DELIMETER
                              + " " + eventString;

    // test
    QCOMPARE(response, expectedResponse);

    // free
    delete exampleNode;
    delete eventObj;
}

/**
 * Test case for stringToNode() in events.cpp
 */
void tst_events::test_stringToNode()
{
    // create event object
    Events *eventObj = new Events();

    // create example string message
    QString exampleString = "ID: 5, 0:02:22, Sample test message 1";

    // convert to node
    bool result = eventObj->stringToNode(exampleString);

    // confirm the string was successfully added as a node to the linked list
    QCOMPARE(result, true);
    QVERIFY(eventObj->headEventNode != nullptr);
    QVERIFY(eventObj->lastEventNode != nullptr);

    // get new node that has been created in linked list
    EventNode *wkgEventNode = eventObj->headEventNode;

    // compare stored values with the example string's values
    QCOMPARE(wkgEventNode->id, 5);
    QCOMPARE(wkgEventNode->timeStamp, "0:02:22");
    QCOMPARE(wkgEventNode->eventString, "Sample test message 1");
    QCOMPARE(wkgEventNode->error, false);

    // free
    delete eventObj;
}

QTEST_MAIN(tst_events)
#include "tst_events.moc"
