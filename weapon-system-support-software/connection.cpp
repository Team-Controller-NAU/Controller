#include <connection.h>

/********************************************************************************
** connection.cpp
**
** This file implements the functionality of the Connection class, which manages
** the serial port connections within the entire program. The class encapsulates
** a QSerialPort object.
**
** @author Team Controller
********************************************************************************/

/**
 * Initialization constructor for a connection object.
 *
 * This initializes the parameters into a QSerialPort object, opens the port for reading
 * and writing, and displays the status.
 *
 * @param portName The name of the serial port (i.e. COMX, /dev/ttySX, etc).
 * @param baudRate The baud rate of the serial port (i.e. 9600).
 * @param dataBits The number of data bits used for communication.
 * @param parity The type of parity used for error checking.
 * @param stopBits The number of stop bits used for framing.
 * @param flowControl The type of flow control used for managing data flow.
 */
Connection::Connection(QString portName, QSerialPort::BaudRate baudRate,
                       QSerialPort::DataBits dataBits, QSerialPort::Parity parity,
                       QSerialPort::StopBits stopBits, QSerialPort::FlowControl flowControl)
                       : portName(portName), connected(false)
{
    // configure this connection to the given port name
    serialPort.setPortName(portName);

    // configure port settings
    serialPort.setBaudRate(baudRate);
    serialPort.setDataBits(dataBits);
    serialPort.setParity(parity);
    serialPort.setStopBits(stopBits);
    serialPort.setFlowControl(flowControl);

    // open the port
    serialPort.open(QIODevice::ReadWrite);

    // log result
    if (!serialPort.isOpen())
    {
        // notify
        qDebug() << "Failed to open " << portName;
    }
    else
    {
        // notify
        qDebug() << portName << " opened successfully";
        serialPort.clear();
        serialPort.readAll();
    }
}

/**
 * Initialization constructor overload for initializing a connection object using stored settings.
 *
 * This constructor delegates to the primary constructor, providing default values for baud rate, data bits,
 * parity, stop bits, and flow control settings using constants defined in constants.h.
 *
 * @param portName The name of the serial port.
 */
Connection::Connection(QString portName)
    : Connection(portName, INITIAL_BAUD_RATE, INITIAL_DATA_BITS,
                 INITIAL_PARITY, INITIAL_STOP_BITS, INITIAL_FLOW_CONTROL)
{
    // this constructor delegates to the first constructor
}

/**
 * Transmits a message through the serial port.
 *
 * The function waits for the full message to be sent before continuing. If the transmission fails,
 * an error message is logged.
 *
 * @param message The message to be transmitted through the serial port.
 */
void Connection::transmit(QString message)
{
    // confirm that the port is open
    if (!serialPort.isOpen())
    {
        // notify and return
        qDebug() << "Port " << portName << " is not open, transmission failed";
        return;
    }

    // serialize message string
    QByteArray data = message.toUtf8();

    // write to serial port
    qint64 bytesWritten = serialPort.write(data);

    // wait for full message to be sent before continuing
    serialPort.waitForBytesWritten(100);

    // check for failure
    if (bytesWritten == -1)
    {
        // notify
        qWarning() << "Failed to write to " << portName << " : " << serialPort.errorString();
    }
    else
    {
        // notify
        qDebug() << "Message sent through " << portName << " : " << message << qPrintable("\n");
        //qDebug() << bytesWritten << " bytes written to the serial port.";
    }
}

/**
 * Destructor for the Connection class.
 *
 * This destructor is responsible for closing the serial port connection associated with the Connection object.
 * If a handshake has occurred and the port is indeed connected to another port, it will transmit a closing
 * connection message through the serial port and closes it.
 */
Connection::~Connection()
{
    // send debug
    qDebug() << "Closing connection on port " << portName << qPrintable("\n");

    // confirm connection to another port
    if (connected)
    {
        // transmit closing message through port
        transmit(QString::number(static_cast<int>(CLOSING_CONNECTION)) + DELIMETER + '\n');
    }

    // close the port
    serialPort.close();
}


