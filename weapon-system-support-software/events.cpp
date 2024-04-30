#include "events.h"

/********************************************************************************
** events.cpp
**
** This file implements the logic of the events and error variables. This includes
** creating, updating reading in event/error data into linked lists.
**
** @author Team Controller
********************************************************************************/

/**
 * Initialization constructor for an event/error object.
 *
 * This initializes the parameters into a linked list object
 */
Events::Events(bool EventRAMClearing, int maxDataNodes)
{
    //initialize variables
    totalEvents = 0;
    totalErrors= 0;
    totalNodes= 0;
    totalClearedErrors = 0;
    storedNodes = 0;
    maxNodes = maxDataNodes;
    RAMClearing = EventRAMClearing;
    truncated = false;

    headEventNode= nullptr;
    lastEventNode= nullptr;

    headErrorNode= nullptr;
    lastErrorNode= nullptr;

    int clearedLength = CLEARED_INDICATOR.length();
    int activeLength = ACTIVE_INDICATOR.length();

    // Add spaces to make the lengths equal, and store as byte array
    if (clearedLength > activeLength)
    {
        activeIndicator = ACTIVE_INDICATOR.leftJustified(clearedLength, ' ');
        activeIndicatorBytes = activeIndicator.toUtf8();
        clearedIndicator = CLEARED_INDICATOR;
        clearedIndicatorBytes = clearedIndicator.toUtf8();
    }
    else
    {
        clearedIndicator = CLEARED_INDICATOR.leftJustified(activeLength, ' ');
        clearedIndicatorBytes = clearedIndicator.toUtf8();
        activeIndicator = ACTIVE_INDICATOR;
        activeIndicatorBytes = activeIndicator.toUtf8();
    }
}

/**
 * Deconstructor for the event/error objects
 *
 * This deconstructor calls freeLinkedList, which deletes the event/error nodes
 */
Events::~Events()
{
    qDebug() << "Deleting event and error data";
    freeLinkedLists(true);
}

/**
 * Adds a node to the event linked list
 *
 * This takes in below parameters and adds a node with those values.
 * Either creates a single node, or adds to an existing linked list.
 *
 * @param id The identification number of an event node
 * @param timeStamp The time that the event node was created
 * @param eventString Event data read into the addEvent function
 */
void Events::addEvent(int id, QString timeStamp, QString eventString)
{
    //allocate memory for node
    EventNode *newNode = new EventNode;

    //assign node values
    newNode->timeStamp = timeStamp;
    newNode->eventString = eventString;
    newNode->nextPtr = nullptr;
    newNode->id = id;

    //increment counters
    totalNodes++;
    totalEvents++;

    //check if we have exceeded max nodes and if ram clearing is enabled
    if (RAMClearing && storedNodes > maxNodes)
    {
        //free the linked lists
        freeLinkedLists(false);

        qDebug() << "Events class cleared to reduce RAM usage";

        //notify parent
        emit RAMCleared();

        //set flag indicating ram dump has occurred
        truncated = true;
    }

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

    storedNodes++;

    #if DEV_MODE && EVENTS_DEBUG
    qDebug() << "New event node created. Total nodes: " << totalNodes
             << " total events: " << totalEvents << " stored nodes: " << storedNodes;
    #endif
}

/**
 * Adds a node to the error linked list
 *
 * This takes in below parameters and adds a node with those values.
 * Either creates a single node, or adds to an existing linked list.
 *
 * @param id The identification number of an error node
 * @param timeStamp The time that the error node was created
 * @param eventString Error data read into the addError function
 * @param cleared A boolean that indicates whether or not the error has been cleared
 */
void Events::addError(int id, QString timeStamp, QString eventString, bool cleared)
{
    //allocate memory for node
    ErrorNode *newNode = new ErrorNode;

    //assign node values
    newNode->id = id;
    newNode->timeStamp = timeStamp;
    newNode->eventString = eventString;
    newNode->cleared = cleared;
    newNode->nextPtr = nullptr;
    newNode->logFileIndicator = UNINITIALIZED;

    //increment counters
    totalNodes++;
    totalErrors++;
    if(cleared) totalClearedErrors++;

    //check if we have exceeded max nodes and if ram clearing is enabled
    if (RAMClearing && storedNodes > maxNodes)
    {
        //free the linked lists
        freeLinkedLists(false);

        qDebug() << "Events class cleared to reduce RAM usage";

        //notify parent
        emit RAMCleared();

        //set flag indicating ram dump has occurred
        truncated = true;
    }

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

    storedNodes++;

    #if DEV_MODE && EVENTS_DEBUG
   qDebug() << "New error node created. Total nodes: " << totalNodes << " total errors: " <<
        totalErrors<< " stored nodes: " << storedNodes;
    #endif
}

/**
 * Frees the memory allocated the linked lists object
 *
 * Deletes all nodes associated with both linked lists
 */
void Events::freeLinkedLists(bool fullClear)
{
    // Free memory for the event linked list
    // get ptr to head node
    EventNode *wkgPtr = headEventNode;
    EventNode *wkgPtr2;

    // loop through the list
    while (wkgPtr != nullptr)
    {
        // save location of next node
        wkgPtr2 = wkgPtr->nextPtr;

        // delete current node
        delete wkgPtr;

        // iterate to next node
        wkgPtr = wkgPtr2;
    }
    // ensure head and tail point to null symbolizing empty list
    headEventNode = lastEventNode = nullptr;

    // Free memory for the error linked list
    ErrorNode *wkgErrPtr = headErrorNode;
    ErrorNode *wkgErrPtr2;

    //loop through list
    while (wkgErrPtr != nullptr)
    {
        //save location of next node
        wkgErrPtr2 = wkgErrPtr->nextPtr;

        //delete current node
        delete wkgErrPtr;

        //iterate to next node
        wkgErrPtr = wkgErrPtr2;
    }

    // ensure head and tail point to null symbolizing empty list
    headErrorNode = lastErrorNode = nullptr;
  
    //reset stored nodes
    storedNodes=0;

    //check for full clear
    if (fullClear)
    {
        //clear counters
        totalNodes=0;
        totalEvents=0;
        totalErrors=0;
        totalClearedErrors=0;
    }

    #if DEV_MODE && EVENTS_DEBUG
    qDebug() << "Event and error linked lists freed";
    #endif
}

/**
 * Attempts to find the error using its id and switches the bool to cleared
 * Both the linked list and the logfile are searched.
 * If error node cannot be found in ll or logfile, the function will return false
 *
 *@param id The identification number of the error node to be cleared
 *@param the logfile to modify
 */
int Events::clearError(int id, QString logFileName)
{
    //check for invalid format
    if (id < 0)
    {
        return FAILED_TO_CLEAR;
    }

    //init vars
    ErrorNode *wkgPtr = headErrorNode;
    int result=SUCCESS;

    //loop through error linked list
    while (wkgPtr != nullptr)
    {
        //if id is found update cleared status
        if (wkgPtr->id == id)
        {
            // Open the log file, check for fail
            QFile logFile(logFileName);
            if (!logFile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                qDebug() << "Error: clearError Failed to open log file:" << logFileName << Qt::endl;
            }
            //logfile opened
            else if (wkgPtr->logFileIndicator > 0)
            {                
                //seek to position of the active indicator
                logFile.seek(wkgPtr->logFileIndicator);

                //overwrite indicator with cleared indicator
                logFile.write(clearedIndicatorBytes);

                //clear value of indicator since its already cleared
                wkgPtr->logFileIndicator = UNINITIALIZED;

                #if DEV_MODE && EVENTS_DEBUG
                qDebug() << "Error " << id << " cleared in log file using preferred method";
                #endif

                logFile.close();
            }
            else
            {
                logFile.close();
                qDebug() << "Error: clearError logFileIndicator is invalid, attempting manual clear for error " << id<< Qt::endl;

                //attempt alternate clear method
                result = clearErrorInLogFile(id, logFileName);
            }

            wkgPtr->cleared = true;

            #if DEV_MODE && EVENTS_DEBUG
            qDebug() << "Error " << id << " cleared in linked list";
            #endif

            totalClearedErrors++;

            //returns success or failed to clear from logfile
            return result;
        }

        //iterate to next error node
        wkgPtr = wkgPtr->nextPtr;
    }
    //error was not found in linked list and could not be cleared
    //this could indicate a RAM dump has been made
    //attempt to find and clear from log file manually
    if (clearErrorInLogFile(id, logFileName) != SUCCESS)
    {
        //we couldnt find the error
        return FAILED_TO_CLEAR;
    }
    else
    {
        #if DEV_MODE && EVENTS_DEBUG
        qDebug() << "Error " << id << " cleared in log file using unpreferred method (likely due to RAM clear)";
        #endif
        return FAILED_TO_CLEAR_FROM_LL;
    }
}

/**
 * Searches log file for error with given id, attempts to clear
 * and returns result. This method is inefficient and only used
 * when standard clearError fails
 *
 * @param id The identification number of the error node to be cleared
 * @param the logfile to modify
 */
int Events::clearErrorInLogFile(int id, QString logFileName)
{
    // Open the log file
    QFile logFile(logFileName);
    if (!logFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Failed to open log file:" << logFile.errorString();
        return FAILED_TO_CLEAR_FROM_LOGFILE;
    }

    // Read the entire log file into memory
    QByteArray fileData = logFile.readAll();
    QString fileContent(fileData);

    // Find and replace the line we are searching for
    QString searchString = "ID: " + QString::number(id) + DELIMETER;
    int index = fileContent.indexOf(searchString);
    if (index != -1)
    {
        // Find the end of the line
        int endIndex = fileContent.indexOf("\n", index);
        if (endIndex != -1)
        {
            // Extract the line
            QString line = fileContent.mid(index, endIndex - index + 1);

            // Replace the substring if found
            int indicatorIndex = line.indexOf(activeIndicator);
            if (indicatorIndex != -1)
            {
                line.replace(indicatorIndex, activeIndicator.length(), clearedIndicator);

                // Replace the modified line in the file content
                fileContent.replace(index, line.length(), line);
            }
        }
    }

    // Clear the file and write the modified content
    logFile.resize(0);
    QTextStream out(&logFile);
    out << fileContent;

    // Close the file
    logFile.close();

    return (index != -1) ? SUCCESS : FAILED_TO_CLEAR_FROM_LOGFILE;
}

/**
 * Goes through both linked lists and returns a pointer to the node which
 * has the earliest time stamp
 *
 * @param eventPtr Pointer to the event linked lists
 * @param errorPtr Pointer to the error linked lists
 * @param printErr Boolean determining if an error occured
 */
EventNode* Events::getNextNode(EventNode*& eventPtr, ErrorNode*& errorPtr)
{
    // initialize variables
    EventNode* nextPtr = nullptr;

    // check if both lists have valid ptrs to nodes
    if (eventPtr != nullptr && errorPtr != nullptr)
    {
        // check if event occurred before error
        if (errorPtr->timeStamp > eventPtr->timeStamp)
        {
            // choose this event to print
            nextPtr = eventPtr;
            eventPtr = eventPtr->nextPtr;
        }
        //check if error occurred first
        else if (errorPtr->timeStamp < eventPtr->timeStamp)
        {
            // choose this error to print
            nextPtr = errorPtr;
            errorPtr = errorPtr->nextPtr;
        }
        //otherwise they occurred at the same time
        else
        {
            //select node with lowest id
            if (eventPtr->id < errorPtr->id)
            {
                nextPtr = eventPtr;
                eventPtr = eventPtr->nextPtr;
            }
            else
            {
                nextPtr = errorPtr;
                errorPtr= errorPtr->nextPtr;
            }
        }
    }
    // check for only events
    else if (eventPtr != nullptr)
    {
        // choose this event to print
        nextPtr = eventPtr;
        eventPtr = eventPtr->nextPtr;
    }
    // otherwise there are only errors
    else
    {
        // choose this err to print
        nextPtr = errorPtr;
        errorPtr = errorPtr->nextPtr;
    }

    // return next ptr
    return nextPtr;
}

/**
 * Creates linked lists based on an imported logfile
 *
 * If the log file is valid, overwrites the contents of an already
 * made events with the log file contents. If invalid, dont alter
 * current events class.
 *
 * @param events pointer to the event linked list
 * @param logFileName The name of the log file that will be read in
 */
int Events::loadDataFromLogFile(Events *&events, QString logFileName)
{
    //init file handle
    QFile file(logFileName);

    //check if we cant open logfile for reading
    if ( !file.open(QIODevice::ReadOnly | QIODevice::Text ))
    {
        qDebug() << "Error: loadDataFromLogFile could not open log file: " << logFileName<< Qt::endl;
        return DATA_NOT_FOUND;
    }

    //get log file contents
    QTextStream in(&file);
    Events *newEvents = new Events(false, 0);
    QString currentLine;

    //loop through log file contents
    while( !in.atEnd() )
    {
        currentLine = in.readLine();

        //check for advanced log file line
        if (currentLine.startsWith(ADVANCED_LOG_FILE_INDICATOR))
        {
            //do nothing
        }
        //otherwise attempt to load current node, return if fail
        else if ( !newEvents->stringToNode(currentLine) )
        {
            delete newEvents;
            file.close();
            qDebug() << "Error: loadDataFromLogFile, corrupt line: " << currentLine<< Qt::endl;
            return INCORRECT_FORMAT;
        }
    }

    file.close();

    //transfer ram clearing settings to new class in case user starts a new session
    newEvents->RAMClearing = events->RAMClearing;
    newEvents->maxNodes = events->maxNodes;

    //we can clear old data
    delete events;

    //assign new events as our events class
    events = newEvents;

    #if DEV_MODE && EVENTS_DEBUG
    qDebug() << "New events class allocated for loaded data";
    #endif

    return SUCCESS;
}

/**
 * Creates a node based on an inputed string
 *
 * Takes a string of the format generated by nodeToString()
 * and creates a node in events calss. Returns false if invalid string
 *
 * @param nodeString String holding event data
 */
bool Events::stringToNode(QString nodeString)
{
    //split the string using delimeter
    QStringList parts = nodeString.split(DELIMETER);

    //ensure the input has at least 3 parts
    if ( parts.size() < 3)
    {
        qDebug()<< "Error: stringToNode failed because string has < 3 parts: " << nodeString<< Qt::endl;
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
    id = parts[0].mid(idStartIndex).toInt(&conversionError);

    //check for int conversion error
    if (!conversionError)
    {
        qDebug() << "Error: stringToNode Int conversion error: " << nodeString<< Qt::endl;
        return false;
    }

    //extract time stamp
    timeStamp = parts[1].trimmed();

    //extract event string
    eventStr = parts[2].trimmed();

    // Check for error node
    if (parts.size() > 3)
    {
        if (parts[3].trimmed() == clearedIndicator)
        {
            cleared = true;
        }
        else if (parts[3].trimmed() == activeIndicator.trimmed())
        {
            cleared = false;
        }
        else
        {
            // Invalid format for cleared status
            qDebug() << "Error: stringToNode cleared status conversion error: " << nodeString;
            qDebug() << "Expected: '" << clearedIndicator.trimmed() << "' or '" << activeIndicator.trimmed()
                     << "'. data received: '" << parts[3].trimmed() << "'"<< Qt::endl;
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

/**
 * Outputs event/error data to a log file
 *
 * Opens a .txt log file in overwrite mode and outputs all saved events and
 * errors in order of id
 *
 * @param logFileName The name of the logfile to write to
 * @param advancedLogFile setting which toggles verbose log file outputs
 */
bool Events::outputToLogFile(QString logFileName, bool advancedLogFile)
{
    // Retrieve given file
    QFile file(logFileName);
    if (!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        qDebug() << "Error: outputToLogFile could not open " << logFileName << " for writing: " << file.errorString()<< Qt::endl;
        return false;
    }

    //clear file of existing contents
    file.resize(0);

    // Create a QTextStream for writing to the file
    QTextStream out(&file);

    // Initialize vars for looping through events class
    EventNode *nextPrintPtr;
    ErrorNode *errPtr = headErrorNode;
    EventNode *eventPtr = headEventNode;

    // Display advanced log file value
    if (advancedLogFile)
    {
        out << ADVANCED_LOG_FILE_INDICATOR + "ADVANCED LOG FILE ENABLED" << Qt::endl;
    }
    else
    {
        out << ADVANCED_LOG_FILE_INDICATOR + "ADVANCED LOG FILE DISABLED" << Qt::endl;
    }

    //display if the data is truncated
    if (truncated)
    {
        out << ADVANCED_LOG_FILE_INDICATOR + "This log file is truncated, view the auto save log file for the complete data set." << Qt::endl;
    }

    // Loop through the events struct to print all nodes in order
    while (errPtr != nullptr || eventPtr != nullptr)
    {
        // Get next node to print
        nextPrintPtr = getNextNode(eventPtr, errPtr);

        // Print node to log file
        out << nodeToString(nextPrintPtr) << Qt::endl;

        if (nextPrintPtr->isError())
        {
            //update the file obj with changes made to this point
            out.flush();

            nextPrintPtr->storeIndicatorLoc(file.size() - (activeIndicatorBytes.size()+NEW_LINE_SIZE));
        }
    }

    file.close();
    #if DEV_MODE && EVENTS_DEBUG
    qDebug() << "Output to log file complete";
    #endif
    return true;
}

/**
 * Creates a new error node from a string
 *
 * Checks for correct message format and parses the the string to create an error node
 *
 * @param message The error data that will be parsed
 */
bool Events::loadErrorData(QString message)
{
    // parse message
    QStringList values = message.split(DELIMETER);

    // check for real error (allows new line at the end or no new line at the end)
    if(values.length()-1 == NUM_ERROR_ELEMENTS || values.length() == NUM_ERROR_ELEMENTS)
    {
        // get values and check for validity
        int id = values[0].toInt();
        if(id <= -1)
        {
            qDebug() << "Error: loadErrorData invalid id: " << values[0]<< Qt::endl;
            return false;
        }

        QString timeStamp = values[1];
        QStringList timeValues = timeStamp.split(':');

        if (timeValues.length() != 4)
        {
            qDebug() << "Error: loadErrorData invalid time stamp: " << timeStamp<< Qt::endl;
            return false;
        }
        if(timeValues[0].toInt() <= -1 || timeValues[1].toInt() <= -1
            || timeValues[2].toInt() <= -1 || timeValues[3].toInt()<= -1)
        {
            qDebug()<< "Error: loadErrorData invalid time stamp: " << timeStamp<< Qt::endl;
            return false;
        }

        QString eventString = values[2];
        if(eventString == "")
        {
            qDebug()<< "Error: loadErrorData empty event string"<< Qt::endl;
            return false;
        }

        bool cleared = (values[3] == "1");

        //using extracted data, add an error to the end of the error linked list
        addError(id, timeStamp, eventString, cleared);
        return true;
    }
    else
    {
        qDebug()<< "Error: Invalid input to load error data: " << message << Qt::endl;
        return false;
    }
}

/**
 * Creates an event node based on a string
 *
 * Checks string for proper format and parses the string to create
 * an event data
 *
 * @param message Event data message to be parsed
 */
bool Events::loadEventData(QString message)
{
    // parse data
    QStringList values = message.split(DELIMETER);

    // check for real event (allows new line at the end or no new line at the end)
    if(values.length()-1 == NUM_EVENT_ELEMENTS || values.length() == NUM_EVENT_ELEMENTS)
    {
        // get values and check for validity
        int id = values[0].toInt();
        if(id <= -1)
        {
            qDebug()<< "Error: loadEventData invalid id: " << values[0]<< Qt::endl;
            return false;
        }

        QString timeStamp = values[1];
        QStringList timeValues = timeStamp.split(':');
        if (timeValues.length() != 4)
        {
            qDebug()<< "Error: loadEventData invalid time stamp: " << timeStamp << Qt::endl;
            return false;
        }
        else if (timeValues[0].toInt() <= -1 || timeValues[1].toInt() <= -1
            || timeValues[2].toInt() <= -1 || timeValues[3].toInt()<= -1)
        {
            qDebug() << "Error: loadEventData invalid time stamp: " << timeStamp << Qt::endl;
            return false;
        }

        QString eventString = values[2];
        if(eventString == "")
        {
            qDebug()<< "Error: loadEventData empty event string"<< Qt::endl;
            return false;
        }

        // using extracted data add a new event to the end of the events linked list
        addEvent( id, timeStamp, eventString);
        return true;
    }
    else
    {
        qDebug()<< "Error: Invalid input to load event data: " << message << Qt::endl;
        return false;
    }
}

/**
 * Creates linked lists based on a dump message (multiple errors)
 *
 * If dump message is valid, it parses through and creates
 * error objects
 *
 * @param message The error dump message
 */
bool Events::loadErrorDump(QString message)
{
    bool successfulLoad = true;

    //temporarily disable ram clearing
    bool prevRAMClearing = RAMClearing;
    RAMClearing = false;

    // Split the dump messages into individual error sets
    QStringList errorSet = message.split(",,", Qt::SkipEmptyParts);

    #if DEV_MODE && EVENTS_DEBUG
    qDebug() << "num errors in error dump: " << errorSet.length();
    qDebug() << "Error dump: " << errorSet;
    #endif

    //check for empty dump
    if (errorSet[0] =="\n")
    {
        RAMClearing = prevRAMClearing;
        return false;
    }

    // Iterate through the error sets and call loadErrorData for each
    for (const QString &error : errorSet)
    {
        // check for empty
        if(!errorSet.isEmpty() && error != "\n")
        {
            // Call loadErrorData for each individual error set, check if fail
            if ( !loadErrorData(error) )
            {
                successfulLoad = false;
            }
        }
    }
    RAMClearing = prevRAMClearing;
    return successfulLoad;
}

/**
 * Creates linked lists based on a dump message (multiple events)
 *
 * If dump message is valid, it parses through and creates
 * events objects
 *
 * @param message The event dump message
 */
bool Events::loadEventDump(QString message)
{
    bool successfulLoad = true;

    //temporarily disable ram clearing
    bool prevRAMClearing = RAMClearing;
    RAMClearing = false;

    // Split the dump messages into individual event sets
    QStringList eventSet = message.split(",,", Qt::SkipEmptyParts);

    #if DEV_MODE && EVENTS_DEBUG
    qDebug() << "num events in event dump" << eventSet.length();
    qDebug() << "Event dump: " << eventSet;
    #endif

    //check for empty dump
    if (eventSet[0] =="\n")
    {
        RAMClearing = prevRAMClearing;
        return false;
    }

    // Iterate through the event sets and call loadEventData for each
    for (const QString &event : eventSet)
    {
        // check for empty
        if(!eventSet.isEmpty() && event != "\n")
        {
            // Call loadEventData for each individual event set, check if fail
            if (!loadEventData(event))
            {
                successfulLoad = false;
            }
        }
    }
    RAMClearing = prevRAMClearing;
    return successfulLoad;
}

/**
 * Adds event/error data to an existing log file
 *
 * Given a log file path, append event/error data to the end of a log file
 *
 * @param logfilePath Path of the log file to append to
 * @param *event The event/error node to be appended
 */
void Events::appendToLogfile(QString logfilePath, EventNode *event)
{
    //retreive the given file
    QFile file(logfilePath);

    //attempt to open in append mode
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        qDebug()<<  "Error: appendToLogfile could not open log file for appending: " << logfilePath << Qt::endl;
        return;
    }

    //append the node to the log file
    QTextStream out(&file);
    out << nodeToString(event) << Qt::endl;

    if (event->isError())
    {
        //update the file obj with changes made to this point
        out.flush();

        event->storeIndicatorLoc(file.size() - (activeIndicatorBytes.size()+NEW_LINE_SIZE));
    }

    //close the file
    file.close();
}

/**
 * Translates node data into a QString
 *
 * Given a valid node, construct a string to be displayed on GUI
 *
 * Note: changes here will require equivalent changes to the Events::clearError
 * and void MainWindow::clearErrorFromEventsOutput functions
 *
 * @param *event Event/error Node to be translated to a string
 */
QString Events::nodeToString(EventNode *event)
{
    QString nodeString;

    // construct string (changes here must be made to clearErrorInLogFile as well)
    nodeString = "ID: " + QString::number(event->id) + DELIMETER + " " + event->timeStamp + DELIMETER
                 + " " + event->eventString;

    // Check if chosen node is an error node
    if (event->isError())
    {
        // Print cleared status
        nodeString += (static_cast<ErrorNode*>(event)->cleared ? DELIMETER + " " + clearedIndicator : DELIMETER + " " + activeIndicator);
    }

    return nodeString;
}

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================
#if DEV_MODE
//generate message containing data from given event or error, csim will attach an identifier to the
//begining of message to tell ddm if it is an error or event

//not currently operational
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

    //message += QString::number(event->cleared) + DELIMETER;

    return message + '\n';
}

//retrieves the error in position i in the linked list
int Events::getErrorIdByPosition(int pos)
{
    ErrorNode *wkgPtr = headErrorNode;
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
    ErrorNode *wkgPtr = headErrorNode;

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
        message += generateNodeMessage(wkgPtr);

        wkgPtr = wkgPtr->nextPtr;
    }

    //add new line for data parsing and return
    return message + '\n';
}

/**
 * CSim function: searches for an error by id and removes it from the linked list
 *
 * @param id The identification number of the error to be removed
 */
void Events::freeError(int id)
{
    ErrorNode *wkgPtr = headErrorNode;

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
    ErrorNode *wkgPtr2 = headErrorNode;

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
            if(wkgPtr->cleared) totalClearedErrors--;

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
#endif
