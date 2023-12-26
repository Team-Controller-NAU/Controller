#ifndef CSIM_H
#define CSIM_H

#include <QThread>
#include <status.h>
#include <connection.h>
#include <QDebug>

// This class simulates the weapon controller, must be called in seperate thread so it doesnt
// block mainwindow functions from running
class CSim : public QThread
{
    Q_OBJECT

public:
    // Constructor for CSim
    CSim(QObject *parent = nullptr);

    // Destructor for CSim
    ~CSim();

    //initializes thread. Calls simulateWeaponController in new thread
    void startCSim(const QString portNameInput);

    //loops until signaled to stop by bool value or threading signals. Generates new status at a time interval
    //specified by value in constants.h
    //void simulateWeaponController();
    bool stop;

    //stores port name for initializing connection class
    QString portName;

public slots:
    void stopSimulation();

private:
    //overloaded threading function.
    void run() override;
};

#endif // CSIM_H
