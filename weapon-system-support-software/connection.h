#ifndef CLASSES_H
#define CLASSES_H

#include <QDebug>
#include <QtSerialPort>
#include <constants.h>

//use this class when you want to read data from weapon controller
class Connection : public QObject
{
    Q_OBJECT

public:
    //qt serial port class
    QSerialPort serialPort;

    //constructor, opens the port with given port name and initial settings
    Connection(QString portName, QSerialPort::BaudRate baudRate,
               QSerialPort::DataBits dataBits, QSerialPort::Parity parity,
               QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl);
    //overloaded constructor uses initial serail setting constants
    Connection(QString portName);

    //destructor, sends closing message to port then closes port
    ~Connection();

    //stores name of port given upon initialization
    QString portName;

    //true when proper handshake has occurred on connected port
    bool connected;

public slots:
    //sends message through port
    void transmit(QString message);

};

#endif // CLASSES_H
