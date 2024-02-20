#ifndef STATUS_H
#define STATUS_H

#include <QObject>
#include <constants.h>
#include <QString>
#include <QRandomGenerator>

class Status : public QObject
{
    Q_OBJECT
public:
    explicit Status(QObject *parent = nullptr);
    bool armed;
    TriggerStatus trigger1;
    TriggerStatus trigger2;
    ControllerState controllerState;
    FiringMode firingMode;
    FeedPosition feedPosition;
    //to find total events including errors, add total errors and events
    int totalErrors;
    int totalFiringEvents;
    int totalEvents; //count of total non error events
    int burstLength;
    double firingRate;
    QString version;
    QString crc;

    //serial communication utilities===================================================================

    //reads a status message from controller and updates class variables
    void loadData(QString statusMessage);

    //reads a message containing controller version and crc and updates corresponding class variables
    void loadVersionData(QString versionMessage);

    //generates a serial message given the current values in the status class
    QString generateMessage();

    //=================================================================================================

    //randomization utility, relies heavily on values defined in constants.h, accounts
    //for single or double trigger data generation
    void randomize(bool secondTrigger);

signals:
    //signal is emitted after new data is loaded into status from a status message
    void newDataLoaded(Status *status);
};

#endif // STATUS_H
