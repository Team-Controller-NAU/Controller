#include <connection.h>

//connection class constructor
Connection::Connection(QString portName, QSerialPort::BaudRate baudRate,
                       QSerialPort::DataBits dataBits, QSerialPort::Parity parity,
                       QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl)
    : portName(portName),
    connected(false)
{
    //configure this connection to the given port name
    serialPort.setPortName(portName);

    //configure port settings
    serialPort.setBaudRate(baudRate);
    serialPort.setDataBits(dataBits);
    serialPort.setParity(parity);
    serialPort.setStopBits(stopBits);
    serialPort.setFlowControl(flowControl);

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

// Constructor overload (uses stored settings)
Connection::Connection(QString portName)
    : Connection(portName, INITIAL_BAUD_RATE, INITIAL_DATA_BITS,
                 INITIAL_PARITY, INITIAL_STOP_BITS, INITIAL_FLOW_CONTROL)
{
    // This constructor delegates to the first constructor
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

    if (connected)
    {
        transmit(QString::number(static_cast<int>(CLOSING_CONNECTION)) + '\n');
    }

    serialPort.close();
}


