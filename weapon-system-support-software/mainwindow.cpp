#include <connection.h>
#include "mainwindow.h"
#include "csim.h"
#include "constants.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)

    //initialize imbedded classes/vars
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    ddmCon(nullptr),
    status(new Status()),
    events(new Events()),
    handshakeTimer( new QTimer(this) )

{
    //init vars
    QString portName;

    //set output settings for qDebug
    qSetMessagePattern("[%{time h:mm:ss}] %{message}");

    //init gui
    ui->setupUi(this);

    //scan available ports, add port names to port selection combo boxes
    setup_ddm_port_selection(0);
    setup_csim_port_selection(0);

    // Iterate through items in the port selection combo boxes
    for (int i = 0; i < ui->ddm_port_selection->count(); ++i)
    {
        //get val at current index
        portName = ui->ddm_port_selection->itemText(i);

        //if COM4 found,init csim to com4
        if (portName == INITIAL_CSIM_PORT)
        {
            ui->csim_port_selection->setCurrentIndex(i);
        }
        //if com5 found init ddm to com5
        else if (portName == INITIAL_DDM_PORT)
        {
            ui->ddm_port_selection->setCurrentIndex(i);
        }
    }

    //update class port name values
    csimPortName = ui->csim_port_selection->currentText();
    ddmPortName = ui->ddm_port_selection->currentText();

    //init ddm connection
    ddmCon = new Connection(ddmPortName);

    //init csim class, assign serial port
    csimHandle = new CSim(nullptr, csimPortName);

    //set up signal and slot (when a message is sent to DDMs serial port, the readyRead signal is emitted and
    //readSerialData() is called)
    connect(&ddmCon->serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

    //set handshake timer interval
    handshakeTimer->setInterval(2000);

    //connect handshake function to a timer. After each interval handshake will be called.
    //this is necessary to prevent the gui from freezing. signals stop when timer is stoped
    connect(handshakeTimer, &QTimer::timeout, this, &MainWindow::handshake);

    //if handshake timeout is enabled, setup signal to timeout
    if (HANDSHAKE_TIMEOUT)
    {
        QTimer::singleShot(TIMEOUT_DURATION, this, &MainWindow::on_handshake_button_clicked);
    }

    qDebug() << "GUI is now listening to port " << ddmCon->portName;
}

//destructor
MainWindow::~MainWindow()
{
    //call destructors for classes declared in main window
    delete ui;
    delete ddmCon;
    delete csimHandle;
    delete status;
    delete events;
}

//initial synchronization between controller and ddm
void MainWindow::handshake()
{
    // Send handshake message
    ddmCon->transmit(QString::number(LISTENING) + '\n');

    //keep gui interactive
    QCoreApplication::processEvents();
}


//button toggles csim random generation on and off. first click will setup thread and run csim.
//second will terminate thread and close csim connection.
void MainWindow::on_CSim_button_clicked()
{
    //check if csim is currently running
    if (csimHandle->isRunning())
    {
        //csim is running, shut it down
        csimHandle->stop = true;
        ui->CSim_button->setText("Start CSim");
        csimHandle->quit();
        csimHandle->wait();

        //enable custom message inputs
        ui->send_message_button->setEnabled(true);
        ui->csim_port_selection->setEnabled(true);
    }
    //csim is not running, start it
    else
    {
        //set button to display the option to stop csim
        ui->CSim_button->setText("Stop CSim");

        //start csim
        csimHandle->startCSim(csimPortName);

        //temporarily disable custom message inputs
        ui->send_message_button->setEnabled(false);
        ui->csim_port_selection->setEnabled(false);
    }
}

//sends custom user input message
void MainWindow::on_send_message_button_clicked()
{
    //open new connection on com4 (smart pointer auto frees memory when function exits)
    std::unique_ptr<Connection> conn(new Connection(csimPortName));

    //get user input from input box
    QString userInput = ui->message_contents->text();

    //add new line for data parsing
    userInput += '\n';

    //clear the contents of input box
    ui->message_contents->clear();

    //send message through csim port
    conn->transmit(userInput);
}

//this function is called as a result of the readyRead signal being emmited by a connected serial port
//in other words, this function is called whenever ddm port receives a new message
void MainWindow::readSerialData()
{
    //ensure port is open to prevent possible errors
    if (ddmCon->serialPort.isOpen())
    {
        //read lines until all sent data is processed
        while (ddmCon->serialPort.bytesAvailable() > 0)
        {
            //get serialized string from port
            QByteArray serializedMessage = ddmCon->serialPort.readLine();

            //deserialize string
            QString message = QString::fromUtf8(serializedMessage);

            qDebug() << "message: " << message;

            //update gui with new message
            ui->stdout_label->setText(message);

            //extract message id
            SerialMessageIdentifier messageId = static_cast<SerialMessageIdentifier>(QString(message[0]).toInt());

            //remove message id from message
            message = message.mid(1 + DELIMETER.length());

            //determine what kind of message this is
            //SerialMessageIdentifier {EVENT_DUMP = 0, ERROR_DUMP = 1, ELECTRICAL = 2, EVENT =3, ERROR =4, STATUS =5};
            switch (messageId)
            {
                case STATUS:

                    //update status class with new data
                    status->loadData(message);

                    ui->status_output->setText(message);

                    qDebug() <<  "Message id: status update" << qPrintable("\n");

                    break;

                case EVENT:

                    qDebug() <<  "Message id: event update" << qPrintable("\n");

                    //add new event to event ll
                    events->loadEventData(message);

                    ui->events_output->append(message);

                    break;

                case ERROR:

                    //add new error to error ll
                    events->loadErrorData(message);

                    ui->events_output->append(message);

                    qDebug() <<  "Message id: error update" << qPrintable("\n");

                    break;

                case ELECTRICAL:

                    break;

                case EVENT_DUMP:

                    qDebug() <<  "Message id: event dump" << qPrintable("\n");

                    //load all events to event linked list
                    events->loadEventDump(message);

                    break;

                case ERROR_DUMP:

                    qDebug() <<  "Message id: error dump" << qPrintable("\n");

                    //load all errors to error linked list
                    events->loadErrorDump(message);

                    //update gui


                    break;

                case BEGIN:

                    //stop handshake protocols
                    handshakeTimer->stop();

                    //
                    qDebug() << "Begin signal received, handshake complete";

                    ui->handshake_button->setText("Disconnect");

                    ddmCon->connected = true;

                    break;

                case CLOSING_CONNECTION:

                    //log
                    qDebug() << "Controller disconnect message received";

                    //assign conn flag
                    ddmCon->connected = false;

                    //attempt handshake to reconnect, optional setting?
                    //on_handshake_button_clicked();

                    break;

                default:

                    //log
                    qDebug() << "Unrecognized serial message received : " << message;

                    break;

                }

                //qDebug() << "remaining buffer: " << ddmCon->serialPort.peek(ddmCon->serialPort.bytesAvailable());
        }
    }
    else
    {
        qDebug() << "Serial port is closed, unable to read data";
    }
}


//runs when the user selects an option out of csim port drop down menu
void MainWindow::on_csim_port_selection_currentIndexChanged(int index)
{
    if (ui->ddm_port_selection->currentText() == "")
    {
        return;
    }

    //update port
    csimPortName = ui->csim_port_selection->currentText();

    qDebug() << "CSIM port set to " << csimPortName;
}

//scan for serial ports, add to csim port selection box
void MainWindow::setup_csim_port_selection(int index)
{
    // Fetch available serial ports and add their names to the combo box
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->csim_port_selection->addItem(info.portName());
    }
}


//scan for serial ports, add to ddm port selection box
void MainWindow::setup_ddm_port_selection(int index)
{
    // Fetch available serial ports and add their names to the combo box
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->ddm_port_selection->addItem(info.portName());
    }
}

//runs when user changes ddm port. Close old connection, make new one and connect to ready
//read signal to listen for controller.
void MainWindow::on_ddm_port_selection_currentIndexChanged(int index)
{
    if (ui->ddm_port_selection->currentText() == "")
    {
        return;
    }

    //update port name
    ddmPortName = ui->ddm_port_selection->currentText();

    //check if ddmCon is allocated
    if (ddmCon != nullptr)
    {
        //close current connection
        delete ddmCon;

        //open new connection
        ddmCon = new Connection(ddmPortName);

        //set up signal and slot (when a message is sent to DDMs serial port, the readyRead signal is emitted and
        //readSerialData() is called)
        connect(&ddmCon->serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

        qDebug() << "GUI is now listening to port " << ddmCon->portName;
    }
}

//toggles handshake process on and off. Once connected, allow for disconnect (send disconnect message to controller)
void MainWindow::on_handshake_button_clicked()
{
    // Check if the timer is started or ddmCon is not connected
    if ( !handshakeTimer->isActive() && !ddmCon->connected )
    {
        qDebug() << "Beginning handshake with controller" << Qt::endl;

        // Start the timer to periodically check the handshake status
        handshakeTimer->start();

        ui->handshake_button->setText("Stop handshake");
        ui->ddm_port_selection->setEnabled(false);
    }
    else
    {
        qDebug() << "Sending disconnect message to controller" << Qt::endl;

        ddmCon->transmit(QString::number(CLOSING_CONNECTION) + '\n');

        handshakeTimer->stop();

        ui->handshake_button->setText("Handshake");
        ui->ddm_port_selection->setEnabled(true);

        ddmCon->connected = false;
    }
}

