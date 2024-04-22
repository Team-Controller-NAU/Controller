#ifndef ELECTRICAL_H
#define ELECTRICAL_H

#include <QObject>
#include <QDateTime>
#include "constants.h"

struct electricalNode {
    int id;
    QString name;
    double voltage;
    double amps;
    struct electricalNode *nextNode;
};

class electrical
{
public:
    //prototypes
    electrical();
    ~electrical();

    int numNodes;

    //ll node
    electricalNode *headNode;
    electricalNode *lastNode;

    void addNode(QString name, double voltage, double amps);
    void freeLL();
    bool loadElecData(QString message);
    bool loadElecDump(QString message);
    QString toString();

    #if DEV_MODE
        QStringList names = {"Piston 1", "Servo Motor 1", "Servo Motor 2", "Cooling Motor", "Oil Sensor", "Hydraulic Actuator"};
        QString generateElectricalMessage();
        void printNodes();
    #endif
};

#endif // ELECTRICAL_H
