#include "electrical.h"

electrical::electrical()
{
    //initialize variables
    numNodes = 0;
    headNode = nullptr;
    lastNode = nullptr;

}

electrical::~electrical()
{
    //call free ll
    qDebug() << "Deleting Electrical Data";
    freeLL();
}

void electrical::addNode(QString name, int voltage, int amps)
{
    //create new node
    electricalNode *wkgNode = new electricalNode;

    //assign values to the node
    wkgNode->name = name;
    wkgNode->voltage = voltage;
    wkgNode->amps = amps;
    wkgNode->nextNode = nullptr;

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
    //qDebug() << "Electrical node created";
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

bool electrical::loadElecData(QString message)
{
    // parse message
    QStringList values = message.split(DELIMETER);

    // check for real electrical dump
    if(values.length()-1 == NUM_ELECTRIC_ELEMENTS)
    {
        // get values
        QString name = values[0];
        int voltage = values[1].toInt();
        int amps = values[2].toInt();

        //using extracted data, add an error to the end of the error linked list
        addNode(name, voltage, amps);
        return true;
    }
    else
    {
        qDebug() << "Invalid input to load electrical data: " << message << "\n";
        return false;
    }
}


bool electrical::loadElecDump(QString message)
{
    // Split the dump messages into individual error sets
    QStringList electricalSet = message.split(",,", Qt::SkipEmptyParts);

    // Iterate through the electrical sets and call loadElecData for each
    for (const QString &elec : electricalSet)
    {
        // check for empty
        if(!electricalSet.isEmpty() && elec != "\n")
        {
            // Call loadErrorData for each individual error set, if any errors, return
            if (!loadElecData(elec))
            {
                return false;
            }
        }
    }
    return true;
}

//converts the current contents of electrical class to a string
QString electrical::toString()
{
    electricalNode *wkgNode = headNode;
    QString electricalString;

    while(wkgNode != nullptr)
    {
        electricalString += wkgNode->name + DELIMETER + " Voltage: " +
                            QString::number(wkgNode->voltage) + DELIMETER
                            + " Amps: " + QString::number(wkgNode->amps);
        wkgNode = wkgNode->nextNode;

        if (wkgNode != nullptr)
        {
            electricalString += DELIMETER + " ";
        }
    }

    return electricalString;
}

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================
#if DEV_MODE
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
#endif
