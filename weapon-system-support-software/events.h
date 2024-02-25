#ifndef EVENTS_H
#define EVENTS_H

#include <QObject>
#include <fstream>
#include <QDebug>
#include <iostream>
#include <QDateTime>
#include <QFileDialog>
#include <QFileInfo>
#include <QSettings>


struct EventNode {
    int id;
    QString timeStamp;
    QString eventString;
    bool cleared;
    struct EventNode *nextPtr;
};

class Events
{
public:
    Events();
    ~Events();

    void addEvent(int id, QString timeStamp, QString eventString);
    void addError(int id, QString timeStamp, QString eventString, bool cleared);
    void freeError(int id);
    void freeLinkedLists();
    EventNode* getNextNodeToPrint(EventNode*& eventPtr, EventNode*& errorPtr, bool& printErr);
    void outputToLogFile(std::string logFileName);
    bool clearError(int id);

    void displayErrorLL();

    QString generateNodeMessage(EventNode *event);
    QString generateDataDump(EventNode *headPtr);
    QString generateClearedMessage(EventNode *error);

    int getErrorIdByPosition(int pos);

    void loadErrorData(QString message);
    void loadEventData(QString message);

    void loadEventDump(QString message);
    void loadErrorDump(QString message);

    void appendToLogfile(QString logfilePath, QString message, bool dump);

    QString nodeToString(EventNode *event, bool error);

    int totalEvents;
    int totalErrors;
    int totalNodes;
    int totalCleared;

    EventNode *headEventNode;
    EventNode *lastEventNode;

    EventNode *headErrorNode;
    EventNode *lastErrorNode;
};

#endif // EVENTS_H
