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

    //serial communication utilities
    void loadData(QString statusMessage);
    QString generateMessage();

    //randomization utility, relies heavily on values defined in constants.h
    void randomize();

signals:
    //signal is emitted after new data is loaded into status from a status message
    void newDataLoaded(Status *status);
};

#endif // STATUS_H
