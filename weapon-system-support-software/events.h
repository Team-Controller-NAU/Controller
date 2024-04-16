#ifndef EVENTS_H
#define EVENTS_H

#include <QObject>
#include <QDebug>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>
#include "constants.h"

struct EventNode {
    int id;
    QString timeStamp;
    QString eventString;
    bool error;
    bool cleared;
    struct EventNode *nextPtr;
};

class Events : public QObject
{
    Q_OBJECT
public:
    Events(bool EventRAMClearing, int maxDataNodes);
    ~Events();

    //class variables
    int totalEvents;
    int totalErrors;
    int totalNodes;
    int totalClearedErrors;
    bool RAMClearing;
    int maxNodes;
    int storedNodes;
    QString clearedIndicator;
    QString activeIndicator;
    QByteArray clearedIndicatorBytes;
    QByteArray activeIndicatorBytes;

    EventNode *headEventNode;
    EventNode *lastEventNode;

    EventNode *headErrorNode;
    EventNode *lastErrorNode;

    //node creation utils
    void addEvent(int id, QString timeStamp, QString eventString);
    void addError(int id, QString timeStamp, QString eventString, bool cleared);

    //free memory utils
    void freeError(int id);
    void freeLinkedLists();

    //navigation utils
    EventNode* getNextNodeToPrint(EventNode*& eventPtr, EventNode*& errorPtr, bool& printErr);

    //load from serial message utils
    bool loadErrorData(QString message);
    bool loadEventData(QString message);
    bool loadEventDump(QString message);
    bool loadErrorDump(QString message);
    bool clearError(int id);
    //searches through log file and replaces the active error indicator
    //with the cleared error indicator
    bool clearErrorInLogFile(QString logFileName, int errorId);

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

signals:
    void RAMCleared();
};

#endif // EVENTS_H
