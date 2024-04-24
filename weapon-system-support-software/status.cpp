#include "status.h"
/********************************************************************************
** status.cpp
**
** This file implements the logic of maintaining the status data
**
** @author Team Controller
********************************************************************************/

/**
 * Initialization constructor for the status class
 *
 * Sets totalFiringEvents to default values
 *
 * @param parent Object used for GUI display
 */
Status::Status(QObject *parent)
    : QObject{parent}
{
    totalFiringEvents = 0;
}

/**
 * Loads data into the status class given a status message
 *
 * Parses statusMessage and updates status class variabels with new data
 *
 * @param statusMessage Message containing data to be parsed
 */
bool Status::loadData(QString statusMessage)
{
    /* the statusMessage contains csv data in the following order
     *
        bool armed;
        TriggerStatus trigger1;
        TriggerStatus trigger2;
        ControllerState controllerState;
        FiringMode firingMode;
        FeedPosition feedPosition;
        //to find total events including errors, add total errors and events
        int totalFiringEvents;
        int burstLength;
        double firingRate;
     *
     */

    QStringList values = statusMessage.split(DELIMETER);

    // check if message contains too few or too many items
    if (values.length()-1 != NUM_STATUS_ELEMENTS)
    {
        qDebug() << "Error: Status::loadData invalid number of delimeters: " << statusMessage << Qt::endl;
        return false;
    }

    //verify all data is valid====================================================

    //get data into vars to prevent inefficiency caused by recasting over and over
    bool firRateConversionResult = false;
    int trig1 = values[1].toInt();
    int trig2 = values[2].toInt();
    int conState = values[3].toInt();
    int firMode = values[4].toInt();
    int feedPos = values[5].toInt();
    int totFirEvents = values[6].toInt();
    int burLen = values[7].toInt();
    double firRate = values[8].toDouble(&firRateConversionResult);

    //check each value to ensure it is of correct type. if the toInt() method fails
    //the return will be 0. So if we find an int =0 and its string != 0, its a fail.
    //we also check that the integers are in range of their respective enumerations
    //declared in constants. Firing mode and feed position are exceptions, each
    //val is manually checked because their enumerations represent degrees.

    //armed
    if (values[0] != "0" && values[0] != "1")
    {
        qDebug()<< "Error: Status::loadData invalid armed value: " << values[0]<< Qt::endl;
    }
    //trigger 1
    else if ((values[1] != "0" && trig1 == 0) || trig1 < 0
               || trig1 >= NUM_TRIGGER_STATUS )
    {
        qDebug() << "Error: Status::loadData invalid trigger 1 value: " << values[1] << Qt::endl;
    }
    //trigger 2
    else if ((values[2] != "0" && trig2 == 0) || trig2 < 0
             || trig2 >= NUM_TRIGGER_STATUS )
    {
        qDebug() << "Error: Status::loadData invalid trigger 2 value: " << values[2] << Qt::endl;
    }
    //controller state
    else if ((values[3] != "0" && conState == 0) || conState < 0
             || conState >= NUM_CONTROLLER_STATE )
    {
        qDebug() << "Error: Status::loadData invalid controller state value: " << values[3]<< Qt::endl;
    }
    //firing mode
    else if ( (values[4] != "0" && firMode == 0) || (firMode != SAFE &&
               firMode != SINGLE && firMode != BURST && firMode != FULL_AUTO))
    {
        qDebug() << "Error: Status::loadData invalid firing mode value: " << values[4]<< Qt::endl;
    }
    //feed pos
    else if ( (values[5] != "0" && feedPos == 0) || (feedPos != CHAMBERING
               && feedPos != LOCKING
               && feedPos != FIRING && feedPos != UNLOCKING
               && feedPos != EXTRACTING && feedPos != EJECTING
               && feedPos != COCKING && feedPos != FEEDING))
    {
        qDebug() << "Error: Status::loadData invalid feed position value: " << values[5] << Qt::endl;
    }
    //total firing events
    else if ( (values[6] != "0" && totFirEvents == 0) || totFirEvents < 0)
    {
        qDebug() << "Error: Status::loadData invalid total firing events value: " << values[6]<< Qt::endl;
    }
    //burst length
    else if ( (values[6] != "0" && burLen == 0) || burLen < 0)
    {
        qDebug() << "Error: Status::loadData invalid burst length value: " << values[7]<< Qt::endl;
    }
    //firing rate
    else if ( !firRateConversionResult || firRate < 0)
    {
        qDebug() << "Error: Status::loadData invalid firing rate value: " << values[8]<< Qt::endl;
    }
    //all data has been verified at this point==========================================
    else
    {
        //extract armed value from message
        armed = (values[0] == "1");

        //extract trigger1 status
        trigger1 = static_cast<TriggerStatus>(trig1);

        //extract trigger2
        trigger2 = static_cast<TriggerStatus>(trig2);

        //extract controller state
        controllerState = static_cast<ControllerState>(conState);

        //extract firing mode
        firingMode = static_cast<FiringMode>(firMode);

        //extract feed pos
        feedPosition = static_cast<FeedPosition>(feedPos);

        //extract
        totalFiringEvents = totFirEvents;

        //extract
        burstLength = burLen;

        //extract
        firingRate = firRate;

        return true;
    }
    //one of the tests failed, no data was loaded from this message
    return false;
}

/**
 * Updates crc and controller versions at the bottom on GUI
 *
 * Given a message containing the controller version and crc updates corresponding class variables
 *
 * @param versionMessage Message containing the controller and crc data
 */
bool Status::loadVersionData(QString versionMessage)
{
    //split along delimeter
    QStringList values = versionMessage.split(DELIMETER);

    //ensure message has correct format
    if(values.length()-1 != NUM_BEGIN_ELEMENTS)
    {
        return false;
    }

    //message is valid, load data

    // Split time string
    QStringList parts = values[0].split(':');
    if (parts.size() != 4) {
        qDebug() << "Error: loadVersionData: Invalid time string format in load version data: " << values[0]<< Qt::endl;
        return false;
    }

    // Calculate total milliseconds
    qint64 totalMilliseconds = parts[0].toInt() * 3600000LL + parts[1].toInt() * 60000LL + parts[2].toInt()
                               * 1000LL + parts[3].toInt();


    // Initialize elapsedControllerTime
    elapsedControllerTime = QTime::fromMSecsSinceStartOfDay(totalMilliseconds);

    version = values[1];
    crc = values[2];
    return true;
}

//converts the status class values to a string
QString Status::toString()
{
    //add each value and a delimeter to the end of the string, then return the string
    QString statusStr = "Armed: " + QString::number(armed) + DELIMETER;

    statusStr += " Trigger 1: " + QString::number(trigger1) + DELIMETER;

    statusStr += " Trigger 2: " + QString::number(trigger2) + DELIMETER;

    statusStr += " Controller State: " + QString::number(controllerState) + DELIMETER;

    statusStr += " Firing Mode: " + QString::number(firingMode) + DELIMETER;

    statusStr += " Feed Position: " + QString::number(feedPosition) + DELIMETER;

    statusStr += " Total Firing Events: " + QString::number(totalFiringEvents) + DELIMETER;

    statusStr += " Burst Length: " + QString::number(burstLength) + DELIMETER;

    statusStr += " Firing Rate: " + QString::number(firingRate) + DELIMETER;

    return statusStr;
}

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================

#if DEV_MODE
QString Status::generateMessage()
{
    /* will create a status message that contains csv data in the following order
     *
        bool armed;
        TriggerStatus trigger1;
        TriggerStatus trigger2;
        ControllerState controllerState;
        FiringMode firingMode;
        FeedPosition feedPosition;
        //to find total events including errors, add total errors and events
        int totalFiringEvents;
        int burstLength;
        double firingRate;
     *
     */

    //add each value and a delimeter to the end of the string, then return the string
    QString message = QString::number(armed) + DELIMETER;

    message += QString::number(trigger1) + DELIMETER;

    message += QString::number(trigger2) + DELIMETER;

    message += QString::number(controllerState) + DELIMETER;

    message += QString::number(firingMode) + DELIMETER;

    message += QString::number(feedPosition) + DELIMETER;

    message += QString::number(totalFiringEvents) + DELIMETER;

    message += QString::number(burstLength) + DELIMETER;

    message += QString::number(firingRate) + DELIMETER;

    return message + '\n';
}

//generate random values for status
void Status::randomize(bool secondTrigger)
{
    armed = QRandomGenerator::global()->bounded(0, 1 + 1);

    trigger1 = static_cast<TriggerStatus>(QRandomGenerator::global()->bounded(0, NUM_TRIGGER_STATUS -1));

    //check if second trigger is enabled
    if (secondTrigger)
    {
        //get random value
        trigger2 = static_cast<TriggerStatus>(QRandomGenerator::global()->bounded(0, NUM_TRIGGER_STATUS -1));
    }
    //disabled, set to NA
    else
    {
        trigger2 = NA;
    }

    controllerState = static_cast<ControllerState>(QRandomGenerator::global()->bounded(0, NUM_CONTROLLER_STATE));

    //firingMode = static_cast<FiringMode>(QRandomGenerator::global()->bounded(0, NUM_FIRING_MODE));

    switch(firingMode)
    {
        case SAFE:
            firingMode = SINGLE;
            break;
        case SINGLE:
            firingMode = BURST;
            break;
        case BURST:
            firingMode = FULL_AUTO;
            break;
        case FULL_AUTO:
            firingMode = SAFE;
            break;
        default:
            firingMode = SAFE;
            break;
    }

    //new randomization method to iterate feed position smoothly
    switch (feedPosition)
    {
    case FEEDING:
        feedPosition = CHAMBERING;
        break;
    case CHAMBERING:
        feedPosition = LOCKING;
        break;
    case LOCKING:
        feedPosition = FIRING;
        break;
    case FIRING:
        feedPosition = UNLOCKING;
        break;
    case UNLOCKING:
        feedPosition = EXTRACTING;
        break;
    case EXTRACTING:
        feedPosition = EJECTING;
        break;
    case EJECTING:
        feedPosition = COCKING;
        break;
    case COCKING:
        feedPosition = FEEDING; // Cycle back to FEEDING
        break;
    default:
        feedPosition = FEEDING;
        break;
    }

    //old randomization method to get random feed position
    //feedPosition = static_cast<FeedPosition>(QRandomGenerator::global()->bounded(0, NUM_FEED_POSITION)
    //                                         * FEED_POSITION_INCREMENT_VALUE);

    burstLength = QRandomGenerator::global()->bounded(2, 50 +1);

    firingRate = QRandomGenerator::global()->bounded(0, 2000 +1) + QRandomGenerator::global()->generateDouble();

    //1/8 odds of incrementing total firing events
    if (QRandomGenerator::global()->bounded(0,8 +1) == 4)
    {
        totalFiringEvents++;
    }
}
#endif
