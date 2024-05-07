#include "electrical.h"

/********************************************************************************
** electrical.cpp
**
** This file implements the logic of the weapon's simulated electrical data.
**
** @author Team Controller
********************************************************************************/


/**
 * @brief Initialization constructor for an electrical object.
 *
 * This initializes the default values for an electrical node
 */
electrical::electrical()
{
    //initialize variables
    numNodes = 0;
    headNode = nullptr;
    lastNode = nullptr;
}

/**
 * @brief Deconstructor for an electrical object.
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
 * @brief Adds a node to the electrical linked list
 *
 * This takes in below parameters and adds a node with those values.
 * Either creates a single node, or adds to an existing linked list.
 *
 * @param name The name of the electrical component.
 * @param voltage The voltage amount of the electrical component.
 * @param amps The amp amount of the electrical component
 */
void electrical::addNode(QString name, double voltage, double amps)
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
 * @brief Frees the memory associated with the linked list.
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
 * @brief Takes in a message and converts to a linked list
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
        // get name value from message
        QString name = values[0];
        if(name == "")
        {
            qDebug() << "Error: loadElecData empty electrical component name"<< Qt::endl;
            return false;
        }

        // get voltage value from message
        double voltage = values[1].toDouble();
        if(voltage <= -1 || values[1] != "0" && voltage == 0.0)
        {
            qDebug() << "Error: loadElecData invalid voltage: " << values[1]<< Qt::endl;
            return false;
        }

        // get amp value from message
        double amps = values[2].toDouble();
        if(amps <= -1 || values[2] != "0" && amps == 0.0)
        {
            qDebug() << "Error: loadElecData invalid amps: " << values[2]<< Qt::endl;
            return false;
        }

        // using extracted data, add a node to the end of the electrical linked list
        addNode(name, voltage, amps);
        return true;
    }
    else
    {
        // failed to get real dump
        qDebug() << "Invalid input to load electrical data: " << message << "\n"<< Qt::endl;
        return false;
    }
}

/**
 * @brief Takes in a dump message with multiple electrical messages and converts to electrical linked list.
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
                // return fail
                return false;
            }
        }
    }
    // return success
    return true;
}

/**
 * @brief Converts the current contents of eletrical class to a string
 * @return electricalString The string containing the entire contents of the eletrical data
 */
QString electrical::toString()
{
    // initialize method
    electricalNode *wkgNode = headNode;
    QString electricalString;

    // loop through whole list
    while(wkgNode != nullptr)
    {
        // set up string
        electricalString += wkgNode->name + DELIMETER + " Voltage: " +
                            QString::number(wkgNode->voltage) + DELIMETER
                            + " Amps: " + QString::number(wkgNode->amps);

        // get next node
        wkgNode = wkgNode->nextNode;

        // check for final node
        if (wkgNode != nullptr)
        {
            // add space
            electricalString += DELIMETER + " ";
        }
    }

    // return converted data string
    return electricalString;
}

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================
#if DEV_MODE
/**
 * @brief Generates an electrical message (not used)
 *
 * Generates a message with randomized values.
 *
 * @return message The created, randomized message.
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
 * @brief Diagnostic function: used to output the entire electrical linked list (not used)
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
