#ifndef CSIM_H
#define CSIM_H

#include <QThread>
#include <status.h>
#include <connection.h>
#include <events.h>
#include <electrical.h>
#include <QDebug>

// This class simulates the weapon controller, must be called in seperate thread so it doesnt
// block mainwindow functions from running
class CSim : public QThread
{
    Q_OBJECT

public:
    // Constructor for CSim
    CSim(QObject *parent = nullptr, QString portName="");

    // Destructor for CSim
    ~CSim();

    //initializes thread. Calls simulateWeaponController in new thread
    void startCSim(const QString portNameInput);

    //loops until signaled to stop by bool value or threading signals. Generates new status at a time interval
    //specified by value in constants.h, generates events and errors randomly, clears non-cleared errors randomly
    //void simulateWeaponController();
    bool stop;

    //stores port name for initializing connection class
    QString portName;

    //written to when ddm is not connected. when connection occurs, data dump is performed
    QString eventDumpMessage;
    QString errorDumpMessage;

    //used for the electrical data dump at beginning of run
    QString electricalDumpMessage;

    //stores the location of the conn class declared in the run function
    Connection *connPtr;
    //stores locatin of events class declared in run function
    Events *eventsPtr;

    electrical *electricalPtr;

public slots:
    void stopSimulation();

    //reads messages from ddm, updates connected flag
    void checkConnection(Connection *conn);

    //sends custom message from csim
    void completeTransmissionRequest(const QString &message);

    //clears error with given id, sends clear error message through
    //serial port
    void clearError(int clearedId);

private:
    //overloaded threading function.
    void run() override;
};

#endif // CSIM_H
