#ifndef EVENTS_H
#define EVENTS_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include "constants.h"

/**
 * @brief The EventNode linked list
 * Stores significant information about an EVENT message that has been received from the
 * controller.
 */
struct EventNode
{

    int id; // id is a unique identifier for this specific node, it could be any number
    QString timeStamp; // the timestamp from the controller of when this message was received
    QString eventString; // the actual string message from the controller
    struct EventNode *nextPtr; // pointer to next node in linked list

    virtual bool isError() const { return false; }
    virtual void storeIndicatorLoc(qint64 loc) {}
};

/**
 * @brief The ErrorNode linked list
 * Stores significant information about an ERROR message that has been received from the
 * controller.
 *
 * NOTE: This struct inherits all member variables from the EventNode defined above
 */
struct ErrorNode : public EventNode
{
    bool cleared; // status of whether or not this error has been cleared yet
    qint64 logFileIndicator; // location of where this error is stored in the logfile
    struct ErrorNode *nextPtr; // pointer to next node in linked list

    bool isError() const override { return true; }
    void storeIndicatorLoc(qint64 loc) override {if(!cleared) logFileIndicator=loc;}
};

class Events : public QObject
{
    Q_OBJECT
private:
    /**
     * These methods are helper functions that are NOT safeguarded with error checks.
     * Therefore, they should not be used outside of the Events data structure
     */
    // node creation helper methods
    void addEvent(int id, QString timeStamp, QString eventString);
    void addError(int id, QString timeStamp, QString eventString, bool cleared);
public:
    // initialization constructor
    Events(bool EventRAMClearing, int maxDataNodes);
    ~Events(); // destructor

    // member variables variables
    int totalEvents; // stores the total amount of events per session
    int totalErrors; // stores the total amount of errors per session
    int totalNodes; // stores the total amount of nodes (events + errors) per session
    int totalClearedErrors; // stores the total amount of cleared errors per session
    int storedNodes; // stores the total amount of nodes in a session, even after truncation
    int maxNodes; // the max number of nodes allowed (to improve CPU performance), defined in user settings
    bool RAMClearing; // boolean stating whether or not to clear the program's ram usage, defined in user settings
    QString clearedIndicator;
    QString activeIndicator;
    QByteArray clearedIndicatorBytes;
    QByteArray activeIndicatorBytes;
    EventNode *headEventNode; // stores the top node in the Events linked list
    EventNode *lastEventNode; // stores the bottom node in the EVents linked list
    ErrorNode *headErrorNode; // stores the top node in the Errors linked list
    ErrorNode *lastErrorNode; // stores the bottom node in the Errors linked list

    // free memory utils
    void freeError(int id);
    void freeLinkedLists(bool fullClear);

    // navigation utils
    EventNode* getNextNode(EventNode*& eventPtr, ErrorNode*& errorPtr);

    // load from serial message utils
    bool loadErrorData(QString message);
    bool loadEventData(QString message);
    bool loadEventDump(QString message);
    bool loadErrorDump(QString message);
    int clearError(int id, QString logFileName);

    // log file utils
    bool outputToLogFile(QString logFileName, bool advancedLogFile);
    int loadDataFromLogFile(Events *&events, QString logFileName);
    void appendToLogfile(QString logfilePath, EventNode *event);
    QString nodeToString(EventNode *event);
    bool stringToNode(QString nodeString);

    //======================================================================================
    //DEV_MODE exclusive methods
    //======================================================================================
    #if DEV_MODE
        //serial message generation utils
        QString generateNodeMessage(EventNode *event);
        QString generateDataDump(EventNode *headPtr);
        QString generateClearedMessage(EventNode *error);

        //debug utils
        void displayErrorLL();

        int getErrorIdByPosition(int pos);
    #endif
signals:
    void RAMCleared();
private:
    //called as last resort when clearError cant find error node, if this
    //returns fail, we dont recognize the node
    int clearErrorInLogFile(int id, QString logFileName);
};

#endif // EVENTS_H
