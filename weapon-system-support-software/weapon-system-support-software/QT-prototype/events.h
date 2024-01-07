#ifndef EVENTS_H
#define EVENTS_H

#include <QObject>
#include <fstream>
#include <QDebug>
#include <iostream>
#include <QDateTime>

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
    void freeLinkedLists();
    void outputToLogFile(std::string logFileName);
    bool clearError(int id);

    QString generateNodeMessage(EventNode *event);
    QString generateDataDump(EventNode *headPtr);
    QString generateClearedMessage(EventNode *error);

    void loadErrorData(QString message);
    void loadEventData(QString message);
    void loadEventDump(QString message);
    void loadErrorDump(QString message);

    int totalEvents;
    int totalErrors;
    int totalNodes;

    EventNode *headEventNode;
    EventNode *lastEventNode;

    EventNode *headErrorNode;
    EventNode *lastErrorNode;
};

#endif // EVENTS_H
