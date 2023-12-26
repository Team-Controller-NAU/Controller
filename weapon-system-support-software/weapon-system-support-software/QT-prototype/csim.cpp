#include "csim.h"


CSim::CSim(QObject *parent)
    : QThread(parent), stop(false)
{
    // Additional initialization if needed...
}

CSim::~CSim()
{
    qDebug() << "Terminating controller simulator thread";

    // Stop the thread
    stop = true;
    wait(); // Waits for the thread to finish
}

//can be called to end the csim event loop
void CSim::stopSimulation()
{
    // Stop the simulation
    stop = true;
}

//starts csim in seperate thread, messages will be sent through given port
void CSim::startCSim(QString portNameInput)
{
    portName = portNameInput;

    if (!isRunning())
    {
        qDebug() << "Starting CSim in seperate thread";
        stop = false;
        start();
    }
}

//this function contains main event loop for simulating weapon controller.
//generates data and sends through serial port at specified time intervals
void CSim::run()
{
    //Use smart pointers for automatic memory management (resources auto free when run exits)
    std::unique_ptr<Connection> conn(new Connection(portName));
    std::unique_ptr<Status> status(new Status());

    QString message;

    //include error handling
    try
    {
        //loop until told to stop by owner of csim handle
        while (!stop)
        {
            //replace status values with randomized ones
            status->randomize();

            //generate message using given status values
            message = status->generateMessage();

            //send message through serial port
            conn->transmit(message);

            //wait for number of seconds specified in constants.h
            QThread::sleep(CSIM_GENERATION_INTERVAL);
        }
    }
    catch (const std::exception &ex)
    {
        qDebug() << "Exception in CSim::run(): " << ex.what();
    }
}
