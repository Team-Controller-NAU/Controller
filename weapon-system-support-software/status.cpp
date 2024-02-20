#include "status.h"

Status::Status(QObject *parent)
    : QObject{parent}
{
    totalEvents = 0;
    totalErrors = 0;
    totalFiringEvents = 0;

}

//given a status message, update status class with new data
void Status::loadData(QString statusMessage)
{
    /* the statusMessage contains csv data in the following order
     *
        bool armed;
        TriggerStatus trigger1Status;
        TriggerStatus trigger2Status;
        ControllerState controllerState;
        FiringMode firingMode;
        FeedPosition feedPosition;
        //to find total events including errors, add total errors and events
        int totalErrors;
        int totalFiringEvents;
        int totalEvents; //count of total non error events
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
    trigger1Status = static_cast<TriggerStatus>(values[1].toInt());

    //extract trigger2
    trigger1Status = static_cast<TriggerStatus>(values[2].toInt());

    //extract controller state
    controllerState = static_cast<ControllerState>(values[3].toInt());

    //extract firing mode
    firingMode = static_cast<FiringMode>(values[4].toInt());

    //extract feed pos
    feedPosition = static_cast<FeedPosition>(values[5].toInt());

    //extract total errors
    totalErrors = values[6].toInt();

    //extract
    totalFiringEvents = values[7].toInt();

    //extract
    totalEvents = values[8].toInt();

    //extract
    burstLength = values[9].toInt();

    //extract
    firingRate = values[10].toDouble(&result);

    //emit signal, new status data is loaded
    emit newDataLoaded(this);
}



QString Status::generateMessage()
{
    /* will create a status message that contains csv data in the following order
     *
        bool armed;
        TriggerStatus trigger1Status;
        TriggerStatus trigger2Status;
        ControllerState controllerState;
        FiringMode firingMode;
        FeedPosition feedPosition;
        //to find total events including errors, add total errors and events
        int totalErrors;
        int totalFiringEvents;
        int totalEvents; //count of total non error events
        int burstLength;
        double firingRate;
     *
     */

    //add each value and a delimeter to the end of the string, then return the string
    QString message = QString::number(armed) + DELIMETER;

    message += QString::number(trigger1Status) + DELIMETER;

    message += QString::number(trigger2Status) + DELIMETER;

    message += QString::number(controllerState) + DELIMETER;

    message += QString::number(firingMode) + DELIMETER;

    message += QString::number(feedPosition) + DELIMETER;

    message += QString::number(totalErrors) + DELIMETER;

    message += QString::number(totalFiringEvents) + DELIMETER;

    message += QString::number(totalEvents) + DELIMETER;

    message += QString::number(burstLength) + DELIMETER;

    message += QString::number(firingRate) + DELIMETER;

    return message + '\n';
}

//generate random values for status
void Status::randomize()
{
    armed = QRandomGenerator::global()->bounded(0, 1 + 1);

    trigger1Status = static_cast<TriggerStatus>(QRandomGenerator::global()->bounded(0, NUM_TRIGGER_STATUS));

    trigger2Status = static_cast<TriggerStatus>(QRandomGenerator::global()->bounded(0, NUM_TRIGGER_STATUS));

    controllerState = static_cast<ControllerState>(QRandomGenerator::global()->bounded(0, NUM_CONTROLLER_STATE));

    firingMode = static_cast<FiringMode>(QRandomGenerator::global()->bounded(0, NUM_FIRING_MODE));

    feedPosition = static_cast<FeedPosition>(QRandomGenerator::global()->bounded(0, NUM_FEED_POSITION)
                                             * FEED_POSITION_INCREMENT_VALUE);

    burstLength = QRandomGenerator::global()->bounded(2, 50 +1);

    firingRate = QRandomGenerator::global()->bounded(0, 2000 +1) + QRandomGenerator::global()->generateDouble();
}
