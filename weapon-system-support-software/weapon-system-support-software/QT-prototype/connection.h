#ifndef CLASSES_H
#define CLASSES_H

#include <QDebug>
#include <QtSerialPort>

//use this class when you want to read data from weapon controller
class Connection : public QObject
{
    Q_OBJECT

public:
    //qt serial port class
    QSerialPort serialPort;

    //stores name of port given upon initialization
    QString portName;

    //constructor, opens the port, applies RS422 serial settings
    Connection(QString portName);

    //destructor, closes the port
    ~Connection();


public slots:
    void transmit(QString message);
};

#endif // CLASSES_H
