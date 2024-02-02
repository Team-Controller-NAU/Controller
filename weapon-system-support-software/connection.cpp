#include <connection.h>

//connection class constructor
Connection::Connection(QString portName)

    : portName(portName), connected(false), baudRate(INITIAL_BAUD_RATE),
    dataBits(INITIAL_DATA_BITS), parity(INITIAL_PARITY), stopBits(INITIAL_STOP_BITS),
    flowControl(INITIAL_FLOW_CONTROL)
{
    //configure this connection to the given port name
    serialPort.setPortName(portName);

    //configure port settings
    serialPort.setBaudRate(INITIAL_BAUD_RATE);
    serialPort.setDataBits(INITIAL_DATA_BITS);
    serialPort.setParity(INITIAL_PARITY);
    serialPort.setStopBits(INITIAL_STOP_BITS);
    serialPort.setFlowControl(INITIAL_FLOW_CONTROL);

    //open the port
    serialPort.open(QIODevice::ReadWrite);

    //log result
    if (!serialPort.isOpen())
    {
        qDebug() << "Failed to open " << portName;
    }
    else
    {
        qDebug() << portName << " opened successfully";

        serialPort.clear();
        serialPort.readAll();
    }
}

//sends message through serial port
void Connection::transmit(QString message)
{
    if (!serialPort.isOpen())
    {
        qDebug() << "Port " << portName << " is not open, transmission failed";
        return;
    }

    //serialize message string
    QByteArray data = message.toUtf8();

    //write to serial port
    qint64 bytesWritten =serialPort.write(data);

    //wait for full message to be sent before continuing
    serialPort.waitForBytesWritten(100);

    if (bytesWritten == -1)
    {
        qWarning() << "Failed to write to " << portName << " : " << serialPort.errorString();
    }
    else
    {
        qDebug() << "Message sent through " << portName << " : " << message << qPrintable("\n");
        //qDebug() << bytesWritten << " bytes written to the serial port.";
    }
}

//destructor for Connection class
Connection::~Connection()
{
    qDebug() << "Closing connection on port " << portName << qPrintable("\n");

    serialPort.close();
}
