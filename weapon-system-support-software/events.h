#ifndef EVENTS_H
#define EVENTS_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include "constants.h"

struct EventNode
{
    int id;
    QString timeStamp;
    QString eventString;
    struct EventNode *nextPtr;

    virtual bool isError() const { return false; }
    virtual void storeIndicatorLoc(qint64 loc) {}
};

//inherits all members of EventNode
struct ErrorNode : public EventNode
{
    bool cleared;
    qint64 logFileIndicator;
    struct ErrorNode *nextPtr;

    bool isError() const override { return true; }
    void storeIndicatorLoc(qint64 loc) override {if(!cleared) logFileIndicator=loc;}
};

class Events : public QObject
{
    Q_OBJECT
public:
    Events();
    ~Events();

    //class variables
    int totalEvents;
    int totalErrors;
    int totalNodes;
    int totalClearedErrors;

    QString clearedIndicator;
    QString activeIndicator;
    QByteArray clearedIndicatorBytes;
    QByteArray activeIndicatorBytes;

    EventNode *headEventNode;
    EventNode *lastEventNode;

    ErrorNode *headErrorNode;
    ErrorNode *lastErrorNode;

    //node creation utils
    void addEvent(int id, QString timeStamp, QString eventString);
    void addError(int id, QString timeStamp, QString eventString, bool cleared);

    //free memory utils
    void freeError(int id);
    void freeLinkedLists();

    //navigation utils
    EventNode* getNextNodeToPrint(EventNode*& eventPtr, ErrorNode*& errorPtr, bool& printErr);

    //load from serial message utils
    bool loadErrorData(QString message);
    bool loadEventData(QString message);
    bool loadEventDump(QString message);
    bool loadErrorDump(QString message);
    int clearError(int id, QString logFileName);

    //log file utils
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
};

#endif // EVENTS_H
