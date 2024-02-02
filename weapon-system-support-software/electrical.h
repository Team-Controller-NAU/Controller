#ifndef ELECTRICAL_H
#define ELECTRICAL_H

#include <QObject>
#include <QDateTime>

struct electricalNode {
    int id;
    QString name;
    int voltage;
    int amps;
    struct electricalNode *nextNode;
};

class electrical
{
public:
    //prototypes
    electrical();
    ~electrical();

    void addNode(int id, QString name, int voltage, int amps);
    void printNodes();
    void freeLL();
    QString generateElectricalMessage();
    void createElectricalLL();

    //variables
    QString name;
    int voltage;
    int amps;

    int numNodes;

    //ll node
    electricalNode *headNode;
    electricalNode *lastNode;

    QStringList names = {"Piston 1", "Servo Motor 1", "Servo Motor 2", "Cooling Motor", "Oil Sensor", "Hydraulic Actuator"};

};

#endif // ELECTRICAL_H
