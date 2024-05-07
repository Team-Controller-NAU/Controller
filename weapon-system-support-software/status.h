#ifndef STATUS_H
#define STATUS_H

#include <QObject>
#include <QString>
#include "constants.h"
#if DEV_MODE
#include <QRandomGenerator>
#endif

/********************************************************************************
** status.h
**
** This file implements the logic of maintaining the status data
**
** @author Team Controller
********************************************************************************/

class Status : public QObject
{
    Q_OBJECT
public:
    // constructors
    explicit Status(QObject *parent = nullptr);
    virtual ~Status() = default;

    // status information
    bool armed;
    TriggerStatus trigger1;
    TriggerStatus trigger2;
    ControllerState controllerState;
    FiringMode firingMode;
    FeedPosition feedPosition;

    // total counters
    int totalErrors;
    int totalFiringEvents;
    int totalEvents; // count of total non error events
    int burstLength;
    double firingRate;

    // controller information
    QString version;
    QString crc;
    QTime elapsedControllerTime;

    //reads a status message from controller and updates class variables
    bool loadData(QString statusMessage);

    //reads a message containing controller version and crc and updates corresponding class variables
    bool loadVersionData(QString versionMessage);

    //outputs values of status class to string format
    QString toString();

    //======================================================================================
    //DEV_MODE exclusive methods
    //======================================================================================
    #if DEV_MODE
    //randomization utility, relies heavily on values defined in constants.h, accounts
    //for single or double trigger data generation
    void randomize(bool secondTrigger);

    //generates a serial message given the current values in the status class
    QString generateMessage();
    #endif
};

#endif // STATUS_H
