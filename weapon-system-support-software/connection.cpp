#include "connection.h"

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
        qDebug() << "Failed to open " << portName;
    }
    else
    {
        qDebug() << portName << " opened successfully";
        serialPort.clear();
        serialPort.readAll();
    }
}

//returns true if a valid message is in the serial port, false otherwise

/**
 * Verifies that the incomming serialized message is valid
 *
 * The function first checks the serial port is open, then captures the
 * the message to check for expected end of message character (\n).
 *
 * Returns true if the message is valid, false otherwise
 */
int Connection::checkForValidMessage()
{
    //ensure port is open to prevent possible errors
    if (serialPort.isOpen())
    {
        //copy data from serial port buffer without altering buffer
        QByteArray serializedMessage = serialPort.peek(serialPort.bytesAvailable());

        QString message = QString::fromUtf8(serializedMessage);

        //check for complete message
        if ( message.contains("\n") || message.contains('\n') )
        {
            return VALID_MESSAGE;
        }
        else if (message=="")
        {
            return EMPTY_BUFFER;
        }
        else
        {
            return UNTERMINATED_MESSAGE;
        }
        /*#if DEV_MODE && SERIAL_COMM_DEBUG
        else
        {
            qDebug() << "checkForValidMessage returned false for: " + message;
            // Display the contents of the QByteArray in hexadecimal format
            qDebug() << "Data received (hex): ";
            for (int i = 0; i < serializedMessage.size(); ++i)
            {
                qDebug().noquote() << QString("%1 ").arg((quint8)serializedMessage[i], 2, 16, QLatin1Char('0')).toUpper();
            }
        }
        #endif*/
    }
    else
    {
        qDebug() << "Error: serial port is closed, message cannot be read" << Qt::endl;
    }

    //invalid message
    return false;
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
    serialPort.waitForBytesWritten(500);

    // check for failure
    if (bytesWritten != data.size())
    {
        // notify
        qWarning() << "Failed to write to " << portName << " : " << serialPort.errorString();
    }
    else
    {
        #if DEV_MODE && SERIAL_COMM_DEBUG
        qDebug() << "Message sent through " << portName << " : " << message << qPrintable("\n");
        qDebug() << bytesWritten << " bytes written to the serial port.";
        #endif
    }

    // Display the contents of the QByteArray in hexadecimal format
   /* #if DEV_MODE && SERIAL_COMM_DEBUG
    qDebug() << "Data sent (hex): ";
    for (int i = 0; i < data.size(); ++i)
    {
        qDebug().noquote() << QString("%1 ").arg((quint8)data[i], 2, 16, QLatin1Char('0')).toUpper();
    }
    #endif*/
}

//send disconnect to communicating party
void Connection::sendDisconnectMsg()
{
    #if DEV_MODE && SERIAL_COMM_DEBUG
    qDebug() << "Sending disconnect message to controller" << Qt::endl;
    #endif
    transmit(QString::number(CLOSING_CONNECTION) + DELIMETER + "\n");
}

//send handshake attempt to communicating party
void Connection::sendHandshakeMsg()
{
    // Send handshake message
    transmit(QString::number(LISTENING) + '\n');
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
    qDebug() << "Closing connection on port " << portName << qPrintable("\n");

    // confirm connection to another port
    if ( connected )
    {
        //transmit(QString::number(static_cast<int>(CLOSING_CONNECTION)) + DELIMETER + "\n");
        sendDisconnectMsg();

        //avoid prematurely closing serial port before closing message is sent
        serialPort.waitForReadyRead(1000);
    }

    // close the port
    serialPort.close();
}

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================
#if DEV_MODE
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
#endif
