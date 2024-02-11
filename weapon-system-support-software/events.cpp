#include "events.h"
#include "constants.h"

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

void Events::outputToLogFile(std::string logFileName)
{
    // get the path to the users temp folder
    QString tempPath = QDir::tempPath() + LOGFILE_FOLDER_NAME;
    QDir path(tempPath);

    // check if the path does not lead to anything
    if(!path.exists())
    {
        // check if can make path successfully
        if(path.mkpath("."))
        {
            qDebug() << "log file directory has been successfully made in the temp folder";
        }

        else
        {
            qDebug() << "The log file directory failed to create";
        }
    }

    else
    {
        qDebug() << "Log file directory already exists";
    }

    // check number of autosaved files
    QStringList numberAutosaves = path.entryList(QStringList("*-A*"), QDir::Files);

    if(numberAutosaves.count() > 4)
    {
        // assume too many files, initialize variables
        qint64 oldestTime = QDateTime::currentSecsSinceEpoch();
        QString fileToDelete;

        // loop through the autosaved files
        for(const QString &fileName:numberAutosaves)
        {
            QFileInfo fileInformation(path.filePath(fileName));

            // bool is just used to check the name split operation
            bool good;
            qint64 timestamp = fileInformation.baseName().split('-').first().toLongLong(&good);

            //check for older file and good opeation
            if(good && timestamp < oldestTime)
            {
                //set oldest and file to delete
                oldestTime = timestamp;
                fileToDelete = fileName;
            }

        }

        qDebug() << numberAutosaves.count() << " autosaved log files detected, deleting: " << fileToDelete;

        // check if file exists
        if(!fileToDelete.isEmpty())
        {
            // file is there, get path
            QString deletionPath = path.filePath(fileToDelete);

            if(QFile::remove(deletionPath))
            {
                qDebug() << "Deletion successful";
            }

            else
            {
                qDebug() << "Deletion failed";
            }
        }

        // otherwise, files does not exist
        else
        {
            qDebug() << "File does not exist";
        }

    }

    // Open the log file in overwrite mode
    std::ofstream logFile(tempPath.toStdString() + '/' + logFileName, std::ios::out);

    if (logFile.is_open())
    {
        // declare variables
        EventNode* wkgErrPtr = headErrorNode;
        EventNode* wkgEventPtr = headEventNode;
        bool printErr;

        // get today's date
        QString currentDateString = QDateTime::currentDateTime().date().toString("MM/dd/yyyy");

        // print log file header
        logFile << "====== " << currentDateString.toStdString() << " ======" << std::endl << std::endl;

        // loop through events and errors
        while (wkgErrPtr != nullptr || wkgEventPtr != nullptr)
        {
            EventNode* nextPrintPtr = getNextNodeToPrint(wkgEventPtr, wkgErrPtr, printErr);

            // print data of chosen node
            logFile << "ID: " << nextPrintPtr->id << " " << nextPrintPtr->timeStamp.toStdString() << " " << nextPrintPtr->eventString.toStdString();

            // check if chosen node is an error node
            if (printErr)
            {
                // print cleared status
                logFile << (nextPrintPtr->cleared ? ", CLEARED" : ", NOT CLEARED");
            }

            // end the current line
            logFile << std::endl;
        }

        logFile.close();

        qDebug() << "Log file written successfully";
    }
    else
    {
        //the log file could not be opened
        qDebug() << "Unable to open the log file";
    }
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

    //errorSet[errorSet.size() - 1].append(",");

    qDebug() << errorSet;

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

// takes message and appends it to log file
void Events::appendToLogfile(QString logfileName, QString message, bool dump)
{
    // initialize method
    QFile file(logfileName);
    QStringList messageSplit;
    QString timeStamp;
    QString eventString;
    EventNode* wkgErrPtr;
    EventNode* wkgEventPtr;
    int id;
    bool printErr;

    // check if the file does not exist
    if (!file.exists())
    {
        // test if possible to append
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            // failed to create the file
            qDebug() << "Unable to create running logfile";
            return;
        }
        file.close();
    }

    // check if the app can open the file
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        // failed to open the file
        qDebug() << "Unable to open running logfile";
        return;
    }

    // check for dump
    // assuming this can only happen after a new "session" begins
    if (dump)
    {
        // initialize ptrs
        wkgErrPtr = headErrorNode;
        wkgEventPtr = headEventNode;

        // loop through all errors and events so the log file is in order..
        while(wkgErrPtr != nullptr || wkgEventPtr != nullptr)
        {
            // get next to print by ID
            EventNode* nextPrintPtr = getNextNodeToPrint(wkgEventPtr, wkgErrPtr, printErr);

            // get parts
            id = nextPrintPtr->id;
            timeStamp = nextPrintPtr->timeStamp;
            eventString = nextPrintPtr-> eventString;

            // write to file
            QTextStream out(&file);
            out << "ID: " << id << " " << timeStamp << " " << eventString;

            // check for error
            if (printErr)
            {
                // check for cleared
                out << (nextPrintPtr->cleared ? ", CLEARED" : ", NOT CLEARED");
            }

            // end the current line
            out << Qt::endl;
        }
    }
    // assume not a dump
    else
    {
        // split the message into its parts
        messageSplit = message.split(DELIMETER);

        // check for real event/error
        if (messageSplit.length() > NUM_EVENT_DELIMETERS)
        {
            // get parts
            id = messageSplit[0].toInt();
            timeStamp = messageSplit[1];
            eventString = messageSplit[2];

            // write to file
            QTextStream out(&file);
            out << "ID: " << id << " " << timeStamp << " " << eventString;

            // check for error
            if (messageSplit.length() > NUM_ERROR_DELIMETERS)
            {
                // check for cleared
                if (messageSplit[3] == "1")
                {
                    out << ", CLEARED";
                }
                else
                {
                    out << ", NOT CLEARED";
                }
            }

            // end the current line
            out << Qt::endl;
        }
    }

    // Close the file
    file.close();
}
