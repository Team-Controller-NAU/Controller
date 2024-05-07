#ifndef ELECTRICAL_H
#define ELECTRICAL_H

#include <QObject>
#include <QDateTime>
#include "constants.h"

/********************************************************************************
** electrical.h
**
** The Electrical class provides functionality for loading and managing
** electrical messages from the controller.
**
** @author Team Controller
********************************************************************************/

struct electricalNode {
    // member variables for node
    int id;
    QString name;
    double voltage;
    double amps;
    struct electricalNode *nextNode;
};

class electrical
{
public:
    // constructor/destructor
    electrical();
    ~electrical();

    // number of nodes in list
    int numNodes;

    // head/bottom of linked list
    electricalNode *headNode;
    electricalNode *lastNode;

    // adds a node to the linked list
    void addNode(QString name, double voltage, double amps);

    // frees the memory
    void freeLL();

    // loads electrical message(s) into linked lists
    bool loadElecData(QString message);
    bool loadElecDump(QString message);

    // convert to string
    QString toString();

    #if DEV_MODE
        QStringList names = {"Piston 1", "Servo Motor 1", "Servo Motor 2", "Cooling Motor", "Oil Sensor", "Hydraulic Actuator"};
        QString generateElectricalMessage();
        void printNodes();
    #endif
};

#endif // ELECTRICAL_H
