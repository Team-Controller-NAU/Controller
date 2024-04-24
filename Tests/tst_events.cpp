#include <QCoreApplication>
#include <QTest>
#include "../weapon-system-support-software/events.cpp"
#include "../weapon-system-support-software/constants.h"

class tst_events : public QObject
{
    friend class Events;
    Q_OBJECT

private slots:
    void events_constructor();
    void test_freeLinkedLists();
    void test_clearError();
    void test_clearError_badInput();
    void test_getNextNode();
    void test_nodeToString();
    void test_stringToNode();
    void test_stringToNode_badInput();
    void test_loadEventData();
    void test_loadEventData_badInput_correctDelim();

    void test_loadErrorData();
    void test_loadErrorData_badInput_correctDelim();

    void test_loadEventDump();
    void test_loadErrorDump();
};

/**
 * Test case for the events class initialization constructor in events.cpp
 */
void tst_events::events_constructor()
{
    // create a new events class object
    Events *eventObj = new Events(false, 0);

    // ensure that member variables are 0 upon initialization
    QCOMPARE(eventObj->totalEvents, 0);
    QCOMPARE(eventObj->totalErrors, 0);
    QCOMPARE(eventObj->totalNodes, 0);
    QCOMPARE(eventObj->totalClearedErrors, 0);

    // ensure event pointers are empty/null
    QCOMPARE(eventObj->headEventNode, nullptr);
    QCOMPARE(eventObj->lastEventNode, nullptr);

    // ensure error pointers are empty/null
    QCOMPARE(eventObj->headErrorNode, nullptr);
    QCOMPARE(eventObj->lastErrorNode, nullptr);


    // check default bool value
    //QCOMPARE(eventObj->dataLoadedFromLogFile, false);
    
    // free
    delete eventObj;
}

/**
 * Test case for freeLinkedLists() in events.cpp
 */
void tst_events::test_freeLinkedLists()
{
    // create event object
    Events *eventObj = new Events(false, 0);

    // providing a seed value for random values
    srand((unsigned) time(nullptr));

    // set up some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "01:15:43:237";
    QString eventString = "Sample test message 1";
    int cleared = rand() % 1; // randomly determine if this is cleared or not
    QString eventMessage = QString::number(id) + "," + timeStamp + "," + eventString + ",\n";
    QString errorMessage = QString::number(id + 1) + "," + timeStamp + "," + eventString + "," + QString::number(cleared) + ",\n";

    // create nodes to test freeing both linked lists
    eventObj->loadEventData(eventMessage);
    eventObj->loadErrorData(errorMessage);

    // show that the head/last nodes are not null
    QVERIFY(eventObj->headEventNode != nullptr);
    QVERIFY(eventObj->lastEventNode != nullptr);
    QVERIFY(eventObj->headErrorNode != nullptr);
    QVERIFY(eventObj->lastErrorNode != nullptr);

    // free and test if head/last node is null
    eventObj->freeLinkedLists(true);
    QCOMPARE(eventObj->headEventNode, nullptr);
    QCOMPARE(eventObj->lastEventNode, nullptr);
    QCOMPARE(eventObj->headErrorNode, nullptr);
    QCOMPARE(eventObj->lastErrorNode, nullptr);

    // ensure that member variables are reset back to 0
    QCOMPARE(eventObj->totalEvents, 0);
    QCOMPARE(eventObj->totalErrors, 0);
    QCOMPARE(eventObj->totalNodes, 0);
    QCOMPARE(eventObj->totalClearedErrors, 0);

    // free
    delete eventObj;
}

/**
 * Test case for clearError() in events.cpp
 */
void tst_events::test_clearError()
{
    // create event object
    Events *eventObj = new Events(false, 0);

    // providing a seed value for random values
    srand((unsigned) time(nullptr));

    // set up some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "01:15:43:237";
    QString eventString = "Sample test message 1";
    QString message = QString::number(id) + "," + timeStamp + "," + eventString + ",";

    // create and add node to linked list
    eventObj->loadErrorData(message);
    eventObj->outputToLogFile("../Tests" + TEST_LOG_FILE, false);

    // test the head/last event node values
    QVERIFY(eventObj->headErrorNode != nullptr);
    QVERIFY(eventObj->lastErrorNode != nullptr);

    // create a working node from the headErrorNode
    ErrorNode *wkgErrorNode = eventObj->headErrorNode;

    // verify that this error is not cleared yet
    QCOMPARE(wkgErrorNode->cleared, false);
    QCOMPARE(eventObj->totalClearedErrors, 0);

    // clear the error
    int result = eventObj->clearError(id, "../Tests" + TEST_LOG_FILE);

    // verify that this error is now cleared
    QCOMPARE(result, SUCCESS);
    QCOMPARE(wkgErrorNode->cleared, true);
    QCOMPARE(eventObj->totalClearedErrors, 1);

    // free
    delete eventObj;
}

/**
 * Test case for clearError() in events.cpp
 */
void tst_events::test_clearError_badInput()
{
    // create event object
    Events *eventObj = new Events(false, 0);

    // try to clear an error that doesn't exist
    int result = eventObj->clearError(500, "../Tests" + TEST_LOG_FILE);
    QCOMPARE(result, FAILED_TO_CLEAR);

    // set up some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "01:15:43:237";
    QString eventString = "Sample test message 1";
    QString message = QString::number(id) + "," + timeStamp + "," + eventString + ",";

    // create and add node to linked list
    eventObj->loadErrorData(message);

    // try to clear an error that exists in the LL but not the log file
    result = eventObj->clearError(id, "../Tests" + TEST_LOG_FILE);
    QCOMPARE(result, FAILED_TO_CLEAR_FROM_LOGFILE);

    // create and add node to linked list and log file
    message = QString::number(id + 1) + "," + timeStamp + "," + eventString + ",";
    eventObj->loadErrorData(message);
    eventObj->outputToLogFile("../Tests" + TEST_LOG_FILE, false);

    // free the linked list...
    eventObj->freeLinkedLists(true);

    // try to clear an error that exists in the log file but not in the LL
    result = eventObj->clearError(id, "../Tests" + TEST_LOG_FILE);
    QCOMPARE(result, FAILED_TO_CLEAR_FROM_LL);

    // free
    delete eventObj;
}

/**
 * Test case for getNextNode() in events.cpp
 */
void tst_events::test_getNextNode()
{
    // create event object
    Events *eventObj = new Events(false, 0);

    // providing a seed value for random values
    srand((unsigned) time(nullptr));

    // set up some variables
    QString timeStamp = "01:15:43:237";
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

    QString eventMessage = QString::number(eventId) + "," + timeStamp + "," + eventString + ",\n";
    QString errorMessage = QString::number(errorId) + "," + timeStamp + "," + eventString + QString::number(cleared) + ",\n";

    // create nodes
    eventObj->loadEventData(eventMessage);
    eventObj->loadErrorData(errorMessage);

    // create working nodes from the headErrorNode and headEventNode
    ErrorNode *wkgErrorNode = eventObj->headErrorNode;
    EventNode *wkgEventNode = eventObj->headEventNode;

    // get next node to print by ID
    EventNode *nextNode = eventObj->getNextNode(wkgEventNode, wkgErrorNode);

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
    Events *eventObj = new Events(false, 0);
    EventNode *exampleNode = new EventNode();

    // provide a seed value for random values
    srand((unsigned) time(nullptr));

    // create some variables
    int id = 1 + (rand() % 100); // get a random ID value, 1-100
    QString timeStamp = "01:15:43:237";
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
    Events *eventObj = new Events(false, 0);

    // create example string message
    QString exampleString = "ID: 5, 01:15:43:237, Sample test message 1";

    // convert to node
    bool result = eventObj->stringToNode(exampleString);

    // confirm the string was successfully added as a node to the linked list
    QCOMPARE(result, true);
    QVERIFY(eventObj->headEventNode != nullptr);
    QVERIFY(eventObj->lastEventNode != nullptr);

    // get new node that has been created in linked list
    EventNode *wkgEventNode = eventObj->headEventNode;

    // test the values
    if (wkgEventNode != nullptr)
    {
        // only access these values if stringToNode actually worked
        // to avoid dereferencing a null pointer
        QCOMPARE(wkgEventNode->id, 5);
        QCOMPARE(wkgEventNode->timeStamp, "01:15:43:237");
        QCOMPARE(wkgEventNode->eventString, "Sample test message 1");
        QCOMPARE(wkgEventNode->isError(), false);
        QCOMPARE(eventObj->totalEvents, 1);
        QCOMPARE(eventObj->totalNodes, 1);
    }
    else
    {
        // force this test to fail, since we know it is null
        QVERIFY(wkgEventNode != nullptr);
    }

    // confirm the test fails with improper input
    result = eventObj->stringToNode("fail");
    QCOMPARE(result, false);

    // free
    delete eventObj;
}

/**
 * Test case for stringToNode() in events.cpp
 */
void tst_events::test_stringToNode_badInput()
{
    // create event object
    Events *eventObj = new Events(false, 0);

    // create example string message with <3 parts
    QString exampleString = "01:15:43:237, Sample test message 1";

    // convert to node
    bool result = eventObj->stringToNode(exampleString);

    // confirm the test fails
    QCOMPARE(result, false);

    // create example string message with invalid ID
    exampleString = "ID: six, 01:15:43:237, Sample test message 2";

    // convert to node
    result = eventObj->stringToNode(exampleString);

    // confirm the test fails
    QCOMPARE(result, false);

    // create example string message with invalid cleared/active indicator
    exampleString = "ID: 5, 01:15:43:237, Sample test message 2,false";

    // convert to node
    result = eventObj->stringToNode(exampleString);

    // confirm the test fails
    QCOMPARE(result, false);

    delete eventObj;
}

/**
 * Test case for loadEventData() in events.cpp
 */
void tst_events::test_loadEventData()
{
    // create a new events class object
    Events *eventObj = new Events(false, 0);

    // set up some variables
    QString exampleMsg = "30,01:15:43:237,Sample Test message 1,\n";

    // attempt to add the node
    bool result = eventObj->loadEventData(exampleMsg);

    // test the head/last event node values
    QCOMPARE(result, true);
    QVERIFY(eventObj->headEventNode != nullptr);
    QVERIFY(eventObj->lastEventNode != nullptr);

    // create a working node from the headEventNode
    EventNode *wkgEventNode = eventObj->headEventNode;

    // test the values
    if (wkgEventNode != nullptr)
    {
        // only access these values if loadEventData actually worked
        // to avoid dereferencing a null pointer
        QCOMPARE(wkgEventNode->id, 30);
        QCOMPARE(wkgEventNode->timeStamp, "01:15:43:237");
        QCOMPARE(wkgEventNode->eventString, "Sample Test message 1");
        QCOMPARE(eventObj->totalEvents, 1);
        QCOMPARE(eventObj->totalNodes, 1);
    }
    else
    {
        // force this test to fail, since we know it is null
        QVERIFY(wkgEventNode != nullptr);
    }

    // confirm the test fails with improper input
    result = eventObj->loadEventData("fail");
    QCOMPARE(result, false);

    // free
    delete eventObj;
}

void tst_events::test_loadEventData_badInput_correctDelim()
{
    Events *eventObj = new Events(false, 0);

    // check out of bounds id
    QString dataMsg = "-30,0:01:15:43,Sample Test message 1,\n";
    QVERIFY(eventObj->loadEventData(dataMsg) == false);

    //check invalid time
    dataMsg = "30,-2:01:15,Sample Test message 1,\n";
    QVERIFY(eventObj->loadEventData(dataMsg) == false);
    dataMsg = "30,0:-01:15,Sample Test message 1,\n";
    QVERIFY(eventObj->loadEventData(dataMsg) == false);
    dataMsg = "30,2:01:-15,Sample Test message 1,\n";
    QVERIFY(eventObj->loadEventData(dataMsg) == false);

    dataMsg = "30,0:01:15,,\n";
    QVERIFY(eventObj->loadEventData(dataMsg) == false);

    delete eventObj;
}

/**
 * Test case for loadErrorData() in events.cpp
 */
void tst_events::test_loadErrorData()
{
    // create a new events class object
    Events *eventObj = new Events(false, 0);

    // set up some variables
    QString exampleMsg = "30,01:15:43:237,Sample Test message 1,1,\n";

    // attempt to add the node
    bool result = eventObj->loadErrorData(exampleMsg);

    // test the head/last event node values
    QCOMPARE(result, true);
    QVERIFY(eventObj->headErrorNode != nullptr);
    QVERIFY(eventObj->lastErrorNode != nullptr);

    // create a working node from the headEventNode
    ErrorNode *wkgErrorNode = eventObj->headErrorNode;

    // test the values
    if (wkgErrorNode != nullptr)
    {
        // only access these values if loadEventData actually worked
        // to avoid dereferencing a null pointer
        QCOMPARE(wkgErrorNode->id, 30);
        QCOMPARE(wkgErrorNode->timeStamp, "01:15:43:237");
        QCOMPARE(wkgErrorNode->eventString, "Sample Test message 1");
        QCOMPARE(wkgErrorNode->cleared, 1);
        QCOMPARE(eventObj->totalErrors, 1);
        QCOMPARE(eventObj->totalNodes, 1);
        QCOMPARE(eventObj->totalClearedErrors, 1);
    }
    else
    {
        // force this test to fail, since we know it is null
        QVERIFY(wkgErrorNode != nullptr);
    }

    // confirm the test fails with improper input
    result = eventObj->loadErrorData("fail");
    QCOMPARE(result, false);

    // free
    delete eventObj;
}

void tst_events::test_loadErrorData_badInput_correctDelim()
{
    Events *eventObj = new Events(false, 0);

    //check for invalid id
    QString dataMsg = "-30,0:01:15:43,Sample Test message 1,1,\n";
    QVERIFY(eventObj->loadErrorData(dataMsg) == false);

    dataMsg = "30,-2:01:15,Sample Test message 1,1,\n";
    QVERIFY(eventObj->loadErrorData(dataMsg) == false);

    dataMsg = "30,0:-01:15,Sample Test message 1,1,\n";
    QVERIFY(eventObj->loadErrorData(dataMsg) == false);

    dataMsg = "30,0:01:-15,Sample Test message 1,1,\n";
    QVERIFY(eventObj->loadErrorData(dataMsg) == false);

    dataMsg = "30,0:01:15,,1,\n";
    QVERIFY(eventObj->loadErrorData(dataMsg) == false);

    delete eventObj;
}

/**
 * Test case for loadEventDump() in events.cpp
 */
void tst_events::test_loadEventDump()
{
    // create a new events class object
    Events *eventObj = new Events(false, 0);

    // set up some variables
    QString exampleMsg = "30,01:15:43:237,Sample Test message 1,,31,01:15:43:437,Sample Test message 2";

    // attempt to add the two nodes
    bool result = eventObj->loadEventDump(exampleMsg);

    // test the head/last event node values
    QCOMPARE(result, true);
    QVERIFY(eventObj->headEventNode != nullptr);
    QVERIFY(eventObj->lastEventNode != nullptr);

    // create a working node from the headEventNode
    EventNode *wkgEventNode = eventObj->headEventNode;

    // test the values
    if (wkgEventNode != nullptr && wkgEventNode->nextPtr != nullptr)
    {
        // only access these values if loadEventData actually worked
        // to avoid dereferencing a null pointer
        QCOMPARE(wkgEventNode->id, 30);
        QCOMPARE(wkgEventNode->timeStamp, "01:15:43:237");
        QCOMPARE(wkgEventNode->eventString, "Sample Test message 1");

        wkgEventNode = wkgEventNode->nextPtr;
        QCOMPARE(wkgEventNode->id, 31);
        QCOMPARE(wkgEventNode->timeStamp, "01:15:43:437");
        QCOMPARE(wkgEventNode->eventString, "Sample Test message 2");

        QCOMPARE(eventObj->totalEvents, 2);
        QCOMPARE(eventObj->totalNodes, 2);
    }
    else
    {
        // force this test to fail, since we know it is null
        QVERIFY(wkgEventNode != nullptr);
    }

    // confirm the test fails with improper input
    result = eventObj->loadEventDump("fail");
    QCOMPARE(result, false);

    // free
    delete eventObj;
}

/**
 * Test case for loadErrorDump() in events.cpp
 */
void tst_events::test_loadErrorDump()
{
    // create a new events class object
    Events *eventObj = new Events(false, 0);

    // set up some variables
    QString exampleMsg = "30,01:15:43:237,Sample Test message 1,1,,31,01:15:43:632,Sample Test message 2,0";

    // attempt to add the two nodes
    bool result = eventObj->loadErrorDump(exampleMsg);

    // test the head/last event node values
    QCOMPARE(result, true);
    QVERIFY(eventObj->headErrorNode != nullptr);
    QVERIFY(eventObj->lastErrorNode != nullptr);

    // create a working node from the headEventNode
    ErrorNode *wkgErrorNode = eventObj->headErrorNode;

    // test the values
    if (wkgErrorNode != nullptr && wkgErrorNode->nextPtr != nullptr)
    {
        // only access these values if loadEventData actually worked
        // to avoid dereferencing a null pointer
        QCOMPARE(wkgErrorNode->id, 30);
        QCOMPARE(wkgErrorNode->timeStamp, "01:15:43:237");
        QCOMPARE(wkgErrorNode->eventString, "Sample Test message 1");
        QCOMPARE(wkgErrorNode->cleared, 1);

        wkgErrorNode = wkgErrorNode->nextPtr;
        QCOMPARE(wkgErrorNode->id, 31);
        QCOMPARE(wkgErrorNode->timeStamp, "01:15:43:632");
        QCOMPARE(wkgErrorNode->eventString, "Sample Test message 2");
        QCOMPARE(wkgErrorNode->cleared, 0);

        QCOMPARE(eventObj->totalErrors, 2);
        QCOMPARE(eventObj->totalNodes, 2);
        QCOMPARE(eventObj->totalClearedErrors, 1);
    }
    else
    {
        // force this test to fail, since we know it is null
        QVERIFY(wkgErrorNode != nullptr);
    }

    // confirm the test fails with improper input
    result = eventObj->loadErrorDump("fail");
    QCOMPARE(result, false);

    // free
    delete eventObj;
}

QTEST_MAIN(tst_events)
#include "tst_events.moc"
