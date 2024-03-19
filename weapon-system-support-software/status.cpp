#include "status.h"

Status::Status(QObject *parent)
    : QObject{parent}
{
    totalFiringEvents = 0;
}

//given a status message, update status class with new data
void Status::loadData(QString statusMessage)
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
    //create bool ptr to store result of string->double conversion for fire rate
    bool result = true;

    QStringList values = statusMessage.split(DELIMETER);

    //extract armed value from message
    armed = (values[0] == "1");

    //extract trigger1 status
    trigger1 = static_cast<TriggerStatus>(values[1].toInt());

    //extract trigger2
    trigger2 = static_cast<TriggerStatus>(values[2].toInt());

    //extract controller state
    controllerState = static_cast<ControllerState>(values[3].toInt());

    //extract firing mode
    firingMode = static_cast<FiringMode>(values[4].toInt());

    //extract feed pos
    feedPosition = static_cast<FeedPosition>(values[5].toInt());

    //extract
    totalFiringEvents = values[6].toInt();

    //extract
    burstLength = values[7].toInt();

    //extract
    firingRate = values[8].toDouble(&result);
}

//given a message containing the controller version and crc updates corresponding class variables
void Status::loadVersionData(QString versionMessage)
{
    QStringList values = versionMessage.split(DELIMETER);

    elapsedControllerTime = values[0];
    version = values[1];
    crc = values[2];
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

    firingMode = static_cast<FiringMode>(QRandomGenerator::global()->bounded(0, NUM_FIRING_MODE));

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
