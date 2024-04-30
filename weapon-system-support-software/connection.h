#ifndef CONNECTION_H
#define CONNECTION_H

#include <QDebug>
#include <QtSerialPort/QtSerialPort>
#include "constants.h"

/********************************************************************************
** connection.h
**
** The Connection class provides functionality for establishing and
** managing a serial port connection, including transmitting a message.
**
** @author Team Controller
********************************************************************************/

class Connection : public QObject
{
    Q_OBJECT

public:
    // qt serial port class object
    QSerialPort serialPort;

    // constructor, opens the port with given port name and initial settings
    Connection(QString portName, QSerialPort::BaudRate baudRate,
               QSerialPort::DataBits dataBits, QSerialPort::Parity parity,
               QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl);

    //overloaded constructor uses initial serail setting constants
    Connection(QString portName);

    // destructor, sends closing message to port then closes port
    ~Connection();

    //checks if messages in the serial port are complete by checking if they contain \n
    // return codes are VALID_MESSAGE, EMPTY_BUFFER, or UNTERMINATED_MESSAGE
    int  checkForValidMessage();

    //stores name of port given upon initialization
    QString portName;

    // true when proper handshake has occurred on connected port
    bool connected;

public slots:
    // sends message through port
    void transmit(QString message);

    // sends disconnect message through port
    void sendDisconnectMsg();

    // sends disconnect message through port
    void sendHandshakeMsg();
};

#endif // CONNECTION_H
