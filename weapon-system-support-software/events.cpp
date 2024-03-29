#include "events.h"

Events::Events()
{
    //initialize variables
    totalEvents = 0;
    totalErrors= 0;
    totalNodes= 0;
    totalCleared = 0;

    headEventNode= nullptr;
    lastEventNode= nullptr;

    headErrorNode= nullptr;
    lastErrorNode= nullptr;

    dataLoadedFromLogFile = false;
}

Events::~Events()
{
    qDebug() << "Deleting event and error data";
    freeLinkedLists();
}

//add an event node to the end of event ll
void Events::addEvent(int id, QString timeStamp, QString eventString)
{
    //allocate memory for node
    EventNode *newNode = new EventNode;

    //assign node values
    newNode->timeStamp = timeStamp;
    newNode->eventString = eventString;
    newNode->nextPtr = nullptr;
    newNode->id = id;
    newNode->error = false;

    //increment counters
    totalNodes++;
    totalEvents++;

    //log: new event created
    qDebug() << "New event node created. Total nodes: " << totalNodes;

    //check if linked list is currently empty
    if (headEventNode == nullptr)
    {
        //assign new node as head of list
        headEventNode = newNode;

        //assign tail ptr
        lastEventNode = newNode;
    }
    //otherwise, ll is not empty
    else
    {
        //append node to list
        lastEventNode->nextPtr = newNode;

        lastEventNode = newNode;
    }
}

//add and error node to the end of error ll
void Events::addError(int id, QString timeStamp, QString eventString, bool cleared)
{
    //allocate memory for node
    EventNode *newNode = new EventNode;

    //assign node values
    newNode->id = id;
    newNode->timeStamp = timeStamp;
    newNode->eventString = eventString;
    newNode->cleared = cleared;
    newNode->nextPtr = nullptr;
    newNode->error = true;

    //increment counters
    totalNodes++;
    totalErrors++;
    if(cleared) totalCleared++;


    //check if linked list is currently empty
    if (headErrorNode == nullptr)
    {
        //assign new node as head of list
        headErrorNode = newNode;

        //assign tail ptr
        lastErrorNode = newNode;
    }
    //otherwise, ll is not empty
    else
    {
        //append node to list
        lastErrorNode->nextPtr = newNode;

        lastErrorNode = newNode;
    }

    //log: new err created
    qDebug() << "New error node created. Total nodes: " << totalNodes << " total errors: " << totalErrors;
}

//free memory allocated to error and event linked lists
void Events::freeLinkedLists()
{
    // Free memory for the event linked list
    //get ptr to head node
    EventNode *wkgPtr = headEventNode;
    EventNode *wkgPtr2;

    //loop through the list
    while (wkgPtr != nullptr)
    {
        //save location of next node
        wkgPtr2 = wkgPtr->nextPtr;

        //delete current node
        delete wkgPtr;

        //iterate to next node
        wkgPtr = wkgPtr2;
    }
    //ensure head and tail point to null symbolizing empty list
    headEventNode = lastEventNode = nullptr;

    // Free memory for the error linked list
    wkgPtr = headErrorNode;

    //loop through list
    while (wkgPtr != nullptr)
    {
        //save location of next node
        wkgPtr2 = wkgPtr->nextPtr;

        //delete current node
        delete wkgPtr;

        //iterate to next node
        wkgPtr = wkgPtr2;
    }

    //ensure head and tail point to null symbolizing empty list
    headErrorNode = lastErrorNode = nullptr;
    totalErrors = 0;
    totalEvents = 0;
    totalNodes = 0;
    totalCleared = 0;
}

//attempts to find the error node with given id, and update its cleared variable to true
//if the error can not be found, return false.
bool Events::clearError(int id)
{
    //init vars
    EventNode *wkgPtr = headErrorNode;

    //loop through error linked list
    while (wkgPtr != nullptr)
    {
        //if id is found update cleared status and return success
        if (wkgPtr->id == id)
        {
            wkgPtr->cleared = true;

            qDebug() << "Error " << id << " cleared";

            totalCleared++;

            return true;
        }

        //iterate to next error node
        wkgPtr = wkgPtr->nextPtr;
    }

    qDebug() << "Error " << id << " was not found and could not be cleared";

    //no id was found and we reached end of list, return failure
    return false;
}

// analyzes both linked lists to determine what to output next based on ID
EventNode* Events::getNextNodeToPrint(EventNode*& eventPtr, EventNode*& errorPtr, bool& printErr)
{
    // initialize variables
    EventNode* nextPrintPtr = nullptr;

    // check if both lists have valid ptrs to nodes
    if (eventPtr != nullptr && errorPtr != nullptr)
    {
        // check if next even has lower id than next error
        if (errorPtr->id > eventPtr->id)
        {
            // choose  this event to print
            nextPrintPtr = eventPtr;

            // set no error flag
            printErr = false;

            // get next event
            eventPtr = eventPtr->nextPtr;
        }
        else
        {
            // choose this error to print
            nextPrintPtr = errorPtr;

            // set err flag
            printErr = true;

            // get next error
            errorPtr = errorPtr->nextPtr;
        }
    }
    // check for only events
    else if (eventPtr != nullptr)
    {
        // choose this event to print
        nextPrintPtr = eventPtr;

        // set no err flag
        printErr = false;

        // get next event
        eventPtr = eventPtr->nextPtr;
    }
    // otherwise there are only errors
    else
    {
        // choose this err to print
        nextPrintPtr = errorPtr;

        // set err flag
        printErr = true;

        // get next err
        errorPtr = errorPtr->nextPtr;
    }

    // return next ptr
    return nextPrintPtr;
}

//if log file is valid, overwrite contents of the events class with
//contents loaded from the given log file. otherwise do not alter current
//events class
int Events::loadDataFromLogFile(Events *&events, QString logFileName)
{
    //init file handle
    QFile file(logFileName);

    //check if we cant open logfile for reading
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text ))
    {
        qDebug() << "Error: Could not open log file: " << logFileName;
        return DATA_NOT_FOUND;
    }

    //get log file contents
    QTextStream in(&file);
    Events *newEvents = new Events();

    //loop through log file contents
    while( !in.atEnd() )
    {
        //check if we cant load this line into node
        if ( !newEvents->stringToNode(in.readLine()) )
        {
            delete newEvents;
            return INCORRECT_FORMAT;
        }
    }

    //all log data loaded, we can clear old data
    delete events;

    //assign new events as our events class
    events = newEvents;

    file.close();

    return SUCCESS;
}

//takes in a string of the format generated by nodeToString() and
//creates a node in the events class for the data. Returns false if
//operation fails due to invalid string
bool Events::stringToNode(QString nodeString)
{
    //split the string using delimeter
    QStringList parts = nodeString.split(DELIMETER);

    //ensure the input has at least 3 parts
    if ( parts.size() < 3)
    {
        qDebug() << "string has <3 parts: " << nodeString;
        return false;
    }

    //declare vars
    int id;
    QString timeStamp;
    QString eventStr;
    bool conversionError = false;
    bool cleared;

    //extract ID
    int idStartIndex = parts[0].indexOf(" ") + 1;
    qDebug() << parts[0].mid(idStartIndex);
    id = parts[0].mid(idStartIndex).toInt(&conversionError);

    //check for int conversion error
    if (!conversionError)
    {
        qDebug() << "Int conversion error: " << nodeString;
        return false;
    }

    //extract time stamp
    timeStamp = parts[1].trimmed();

    //extract event string
    eventStr = parts[2].trimmed();

    // Check for error node
    if (parts.size() > 3)
    {
        if (parts[3].trimmed() == "CLEARED")
        {
            cleared = true;
        }
        else if (parts[3].trimmed() == "NOT CLEARED")
        {
            cleared = false;
        }
        else
        {
            // Invalid format for cleared status
            qDebug() << "Cleared conversion error: " << nodeString;
            return false;
        }

        addError(id, timeStamp, eventStr, cleared);
    }
    //otherwise this is an event
    else
    {
        addEvent(id, timeStamp, eventStr);
    }

    //return success
    return true;
}

// opens log file in overwrite mode and outputs currently saved events and errors in order of id
void Events::outputToLogFile(QString logFileName)
{
    // retreive given file
    QFile file(logFileName);

    // check if we can open in overwrite mode
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "Error: Could not open " << logFileName << " for writing: " << file.errorString();
        return;
    }

    // Create a QTextStream for writing to the file
    QTextStream out(&file);

    //init vars for looping through events class
    EventNode *nextPrintPtr;
    EventNode *errPtr = headErrorNode;
    EventNode *eventPtr = headEventNode;
    bool error;

    //loop through the events struct to print all nodes in order
    while(errPtr!= nullptr || eventPtr != nullptr)
    {
        //get next node to print
        nextPrintPtr = getNextNodeToPrint(eventPtr, errPtr, error);

        //print node to log file
        out << nodeToString(nextPrintPtr) << "\n";
    }

    //close log file
    file.close();
}

//searches for error with given id, removes it from error linked list (intended for CSim use only)
void Events::freeError(int id)
{
    EventNode *wkgPtr = headErrorNode;

    //check if this error is head node
    if (headErrorNode->id == id)
    {
        //assign new head
        headErrorNode = headErrorNode->nextPtr;

        //free old head
        free(wkgPtr);

        //decriment total errors
        totalErrors--;

        return;
    }

    //head node is not the error we need to delete, get next
    wkgPtr = wkgPtr->nextPtr;

    //assign wkgPtr2 to trail wkgPtr
    EventNode *wkgPtr2 = headErrorNode;

    //loop until list ends
    while (wkgPtr != nullptr)
    {
        //check if current node is to be freed
        if (wkgPtr->id == id)
        {
            //link list around deleted node
            wkgPtr2->nextPtr = wkgPtr->nextPtr;

            //check if the deleted node is the last node in list
            if (wkgPtr->nextPtr == nullptr)
            {
                //update last node ptr
                lastErrorNode = wkgPtr2;
            }

            // update total cleared (error does not exist, therefore it can not be cleared)
            if(wkgPtr->cleared) totalCleared--;

            //delete the node
            free(wkgPtr);

            //update total errors
            totalErrors--;

            //return success
            return;
        }
       //otherwise, get next node
        else
        {
            //update trailing ptr
            wkgPtr2 = wkgPtr;
            //get next node
            wkgPtr = wkgPtr->nextPtr;
        }
    }

    //no node with given id was found
    qDebug() << "[CSIM] No node with id " << id << " was found, no deletions made";
}

//function developed to handle reading a node message and create a new error node with given data
void Events::loadErrorData(QString message)
{
    // parse message
    QStringList values = message.split(DELIMETER);

    // check for real error
    if(values.length() > NUM_ERROR_DELIMETERS)
    {
        // get values
        int id = values[0].toInt();
        QString timeStamp = values[1];
        QString eventString = values[2];
        bool cleared = (values[3] == "1");

        //using extracted data, add an error to the end of the error linked list
        addError(id, timeStamp, eventString, cleared);
    }
    else
    {
        qDebug() << "Invalid input to load error data: " << message << "\n";
    }
}

//function developed to handle reading a node message and create a new event node with given data
void Events::loadEventData(QString message)
{
    // parse data
    QStringList values = message.split(DELIMETER);

    // check for real event
    if(values.length() > NUM_EVENT_DELIMETERS)
    {
        // get values
        int id = values[0].toInt();
        QString timeStamp = values[1];
        QString eventString = values[2];

        // using extracted data add a new event to the end of the events linked list
        addEvent( id, timeStamp, eventString);
    }
    else
    {
        qDebug() << "Invalid input to load event data: " << message << "\n";
    }
}

void Events::loadErrorDump(QString message)
{
    // Split the dump messages into individual error sets
    QStringList errorSet = message.split(",,", Qt::SkipEmptyParts);

    // Iterate through the error sets and call loadErrorData for each
    for (const QString &error : errorSet)
    {
        // check for empty
        if(!errorSet.isEmpty() && error != "\n")
        {
            // Call loadErrorData for each individual error set
            loadErrorData(error);
        }
    }
}

void Events::loadEventDump(QString message)
{
    // Split the dump messages into individual event sets
    QStringList eventSet = message.split(",,", Qt::SkipEmptyParts);

    // Iterate through the event sets and call loadEventData for each
    for (const QString &event : eventSet)
    {
        // check for empty
        if(!eventSet.isEmpty() && event != "\n")
        {
            // Call loadEventData for each individual event set
            loadEventData(event);
        }
    }
}

// appends the given node to the log file
void Events::appendToLogfile(QString logfilePath, EventNode *event)
{
    //retreive the given file
    QFile file(logfilePath);

    //attempt to open in append mode
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        qDebug() <<  "Could not open log file for appending: " << logfilePath;
        return;
    }

    //append the node to the log file
    QTextStream out(&file);
    out << nodeToString(event) + "\n";

    //close the file
    file.close();
}


QString Events::nodeToString(EventNode *event)
{
    QString nodeString;

    // construct string
    nodeString = "ID: " + QString::number(event->id) + DELIMETER + " " + event->timeStamp + DELIMETER
                 + " " + event->eventString;

    // Check if chosen node is an error node
    if (event->error)
    {
        // Print cleared status
        nodeString += (event->cleared ? DELIMETER + " CLEARED" : DELIMETER + " NOT CLEARED");
    }

    return nodeString; // Return the concatenated QString
}

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================
#if DEV_MODE
//generate message containing data from given event or error, csim will attach an identifier to the
//begining of message to tell ddm if it is an error or event
QString Events::generateNodeMessage(EventNode *event)
{
    /*the message will format data in this order
    int id;
    QString timeStamp;
    QString eventString;
    double param1;
    double param2;
    double param3;
    bool cleared;*/

    QString message = QString::number(event->id) + DELIMETER;

    message += event->timeStamp + DELIMETER;

    message += event->eventString + DELIMETER;

    message += QString::number(event->cleared) + DELIMETER;

    return message + '\n';
}

//retrieves the error in position i in the linked list
int Events::getErrorIdByPosition(int pos)
{
    EventNode *wkgPtr = headErrorNode;
    int i = 0;

    //loop until we reach the position, or ll ends
    while(i != pos && wkgPtr != nullptr)
    {
        //get next error
        wkgPtr = wkgPtr->nextPtr;
        i++;
    }

    //check if we got to correct position
    if (i == pos)
    {
        return wkgPtr->id;
    }

    //otherwise data not found
    return DATA_NOT_FOUND;
}

void Events::displayErrorLL()
{
    EventNode *wkgPtr = headErrorNode;

    qDebug() << "====Printing Error Linked List ====";
    qDebug() << "Total Errors " << totalErrors;

    while (wkgPtr != nullptr)
    {
        qDebug() << "Error " << wkgPtr->id << wkgPtr->eventString << wkgPtr->cleared;

        wkgPtr = wkgPtr->nextPtr;
    }

    qDebug() << "===================================";
}

//given the head to either the error linked list or the event linked list, a message will be created containing
//data from all nodes in the list (data dump is used when the controller has been running prior to the ddm being connected
//the data dump will catch the ddm up to any events and errors that have occured before connection)
QString Events::generateDataDump(EventNode *headPtr)
{
    QString message = "";
    EventNode *wkgPtr = headPtr;

    while (wkgPtr != nullptr)
    {
        message += QString::number(wkgPtr->id) + DELIMETER;

        message += wkgPtr->timeStamp + DELIMETER;

        message += wkgPtr->eventString + DELIMETER;

        message += QString::number(wkgPtr->cleared) + DELIMETER;

        wkgPtr = wkgPtr->nextPtr;
    }

    //add new line for data parsing and return
    return message + '\n';
}
#endif
