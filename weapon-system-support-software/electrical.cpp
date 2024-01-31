#include "electrical.h"
#include "constants.h"

electrical::electrical()
{
    //initialize variables
    name = "";
    voltage = 0;
    amps = 0;

    numNodes = 0;

}

electrical::~electrical()
{
    //call free ll
    qDebug() << "Deleting Electrical Data";
    freeLL();
}

void electrical::addNode(int id, QString name, int voltage, int amps)
{
    //create new node
    electricalNode *wkgNode = new electricalNode;

    //assign values to the node
    wkgNode->id = id;
    wkgNode->name = name;
    wkgNode->voltage = voltage;
    wkgNode->amps = amps;

    //add to counter
    numNodes++;

    qDebug() << "Electrical node created";

    //add node to ll, check if ll is empty
    if (headNode == nullptr)
    {
        //set node to head
        headNode = wkgNode;

        //assign tail
        lastNode = wkgNode;
    }
    //otherwise, assume ll contains nodes
    else
    {
        lastNode->nextNode = wkgNode;

        lastNode = wkgNode;
    }
}


void electrical::printNodes()
{
    electricalNode *wkgNode = headNode;
    electricalNode *traversalNode;

    qDebug() << "=== electrical nodes ===";
    while(wkgNode != nullptr)
    {
        qDebug() << "Electrical: " << wkgNode->name << " " << wkgNode->amps <<
            ", " << wkgNode->voltage;

        //move wkgnode
        wkgNode = wkgNode->nextNode;
    }
    qDebug() << "=== end of electrical nodes ===";
}

void electrical::freeLL()
{
    electricalNode *wkgNode = headNode;
    electricalNode *travNode;

    while(wkgNode != nullptr)
    {
        travNode = wkgNode->nextNode;

        delete wkgNode;

        wkgNode = travNode;
    }

    //once out of loop, set head and last to null
    headNode = lastNode = nullptr;
}

QString electrical::generateElectricalMessage()
{
    // electrical components have:
    /**
     * name
     * amps
     * volts
     * */

    //add values to message
    QString message = QString::number(ELECTRICAL) + DELIMETER;
    message += QString::number(7) + DELIMETER;
    message += names[rand()%6] + DELIMETER;
    message += QString::number(rand()%100) + DELIMETER;
    message += QString::number(rand()%30) + DELIMETER;

    return message + '\n';
}

void electrical::createElectricalLL()
{
    for(int index = 0; index < 6; index ++)
    {
        //create a new ll node
        // uses the index for id, a names list in .h
        // voltage is a random value from 0 to 99
        // amp is a random value from 0 to 49
        addNode(index, names[index], rand() % 100, rand() % 50);
    }
}


