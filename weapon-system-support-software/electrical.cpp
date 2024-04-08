#include "electrical.h"

/**
 * Initialization constructor for an electrical object.
 *
 * This initializes the default values for an electrical node
 */
electrical::electrical()
{
    //initialize variables
    name = "";
    voltage = 0;
    amps = 0;

    numNodes = 0;
    headNode = nullptr;
    lastNode = nullptr;

}

/**
 * Deconstructor for an electrical object.
 *
 * This deletes the electrical linked list.
 */
electrical::~electrical()
{
    //call free ll
    qDebug() << "Deleting Electrical Data";
    freeLL();
}

/**
 * Adds a node to the electrical linked list
 *
 * This takes in below parameters and adds a node with those values.
 * Either creates a single node, or adds to an existing linked list.
 *
 * @param name The name of the electrical component.
 * @param voltage The voltage amount of the electrical component.
 * @param amps The amp amount of the electrical component
 */
void electrical::addNode(QString name, int voltage, int amps)
{
    //create new node
    electricalNode *wkgNode = new electricalNode;

    //assign values to the node
    wkgNode->name = name;
    wkgNode->voltage = voltage;
    wkgNode->amps = amps;
    wkgNode->nextNode = nullptr;

    // add node to ll, check if ll is empty
    if (headNode == nullptr)
    {
        //set node to head
        headNode = wkgNode;

        //assign tail
        lastNode = wkgNode;
    }

    // otherwise, assume ll contains nodes
    else
    {
        lastNode->nextNode = wkgNode;

        lastNode = wkgNode;
    }
}

/**
 * Frees the memory associated with the linked list.
 *
 * This deletes all nodes in the electrical linked list.
 */
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

/**
 * Takes in a message and converts to a linked list
 *
 * Divides message into its delimeted parts (ex: name, (amps) 12, (volts) 14)
 * checks for a valid message and then creates a new node from those inputed values.
 *
 * @param message Electrical data message to be parsed into electrical linked list node.
 */
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

        //using extracted data, add an error to the end of the electrical linked list
        addNode(name, voltage, amps);
        return true;
    }
    else
    {
        qDebug() << "Invalid input to load electrical data: " << message << "\n";
        return false;
    }
}

/**
 * Takes in a dump message with multiple electrical messages and converts to electrical linked list.
 *
 * Divides message into its delimeted parts (ex: name, (amps) 12, (volts) 14,, name2,(amps) 12,(volts) 14),
 * checks for a valid message and then creates a new node from those inputed values.
 *
 * @param message Electrical data messages to be parsed to electrical linked list.
 */
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

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================
#if DEV_MODE
/**
 * Generates an electrical message (not used)
 *
 * Generates a message with randomized values.
 *
 * Returns the created, randomized message.
 */
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

/**
 * Diagnostic function: used to output the entire electrical linked list (not used)
 */
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
