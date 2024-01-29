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

    //variables
    QString name;
    int voltage;
    int amps;

    int numNodes;

    //ll node
    electricalNode *headNode;
    electricalNode *lastNode;

};

#endif // ELECTRICAL_H
