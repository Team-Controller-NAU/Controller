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
#include <QRegularExpression>


struct EventNode {
    int id;
    QString timeStamp;
    QString eventString;
    bool error;
    bool cleared;
    struct EventNode *nextPtr;
};

class Events
{
public:
    Events();
    ~Events();

    //class variables
    int totalEvents;
    int totalErrors;
    int totalNodes;
    int totalCleared;
    bool dataLoadedFromLogFile;

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
    int getErrorIdByPosition(int pos);

    //debug utils
    void displayErrorLL();

    //serial message generation utils
    QString generateNodeMessage(EventNode *event);
    QString generateDataDump(EventNode *headPtr);
    QString generateClearedMessage(EventNode *error);

    //load from serial message utils
    void loadErrorData(QString message);
    void loadEventData(QString message);
    void loadEventDump(QString message);
    void loadErrorDump(QString message);
    bool clearError(int id);

    //log file utils
    void outputToLogFile(QString logFileName);
    int loadDataFromLogFile(Events *&events, QString logFileName);
    void appendToLogfile(QString logfilePath, EventNode *event);
    QString nodeToString(EventNode *event);
    bool stringToNode(QString nodeString);
};

#endif // EVENTS_H
