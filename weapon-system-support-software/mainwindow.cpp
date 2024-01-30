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

    //setting determines if automatic handshake starts after csim disconnects
    reconnect(false),

    //this determines what will be shown on the events page
    eventFilter(ALL),

    //timer is used to repeatedly transmit handshake signals
    handshakeTimer( new QTimer(this) )

{
    //init vars
    QString portName;

    //set output settings for qDebug
    qSetMessagePattern("[%{time h:mm:ss}] %{message}");

    //init gui
    ui->setupUi(this);

    //scan available ports, add port names to port selection combo boxes
    setup_connection_settings();

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

    //connect custom transmission requests from ddm to csims execution slot
    connect(this, &MainWindow::transmissionRequest, csimHandle, &CSim::completeTransmissionRequest);

    //connect custom clear error requests from ddm to csims execution slot
    connect(this, &MainWindow::clearErrorRequest, csimHandle, &CSim::clearError);

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

        //enable csim port selection
        ui->csim_port_selection->setEnabled(true);
    }
    //csim is not running, start it
    else
    {
        //set button to display the option to stop csim
        ui->CSim_button->setText("Stop CSim");

        //start csim
        csimHandle->startCSim(csimPortName);

        //temporarily disable csim port selection
        ui->csim_port_selection->setEnabled(false);
    }
}

//sends custom user input message
void MainWindow::on_send_message_button_clicked()
{
    //get user input from input box
    QString userInput = ui->message_contents->text();

    //add new line for data parsing
    userInput += '\n';

    //clear the contents of input box
    ui->message_contents->clear();

    //check if csim has an active connection
    if (csimHandle->connPtr != nullptr)
    {
        //send signal for csim to transmit message
        emit transmissionRequest(userInput);
    }
    //no active connection from csim, make tmp connection
    else
    {
        //open new connection on com4 (smart pointer auto frees memory when function exits)
        std::unique_ptr<Connection> conn(new Connection(csimPortName));

        //send message through csim port
        conn->transmit(userInput);
    }
}

//this function is called as a result of the readyRead signal being emmited by a connected serial port
//in other words, this function is called whenever ddm port receives a new message
void MainWindow::readSerialData()
{
    //ensure port is open to prevent possible errors
    if (ddmCon->serialPort.isOpen())
    {
        //read lines until all data in buffer is processed
        while (ddmCon->serialPort.bytesAvailable() > 0)
        {
            // declare variables
            EventNode* wkgErrPtr;
            EventNode* wkgEventPtr;
            bool printErr;
            QString dumpMessage;

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

                    qDebug() <<  "Message id: status update" << qPrintable("\n");

                    //update status class with new data
                    status->loadData(message);

                    ui->status_output->setText(message);

                    break;

                case EVENT:

                    // status
                    qDebug() <<  "Message id: event update" << qPrintable("\n");

                    //add new event to event ll
                    events->loadEventData(message);

                    // update GUI
                    ui->events_output->append(message);

                    // update total events gui
                    ui->TotalEventsOutput->setText(QString::number(events->totalEvents));
                    ui->TotalEventsOutput->setAlignment(Qt::AlignCenter);

                    break;

                case ERROR:

                    // status
                    qDebug() <<  "Message id: error update" << qPrintable("\n");

                    //add new error to error ll
                    events->loadErrorData(message);

                    // update GUI
                    ui->events_output->append(message);

                    //update the cleared error selection box in dev tools
                    update_non_cleared_error_selection();

                    // update total errors gui
                    ui->TotalErrorsOutput->setText(QString::number(events->totalErrors));
                    ui->TotalErrorsOutput->setAlignment(Qt::AlignCenter);

                    // update cleared errors gui
                    ui->ClearedErrorsOutput->setText(QString::number(events->totalCleared));
                    ui->ClearedErrorsOutput->setAlignment(Qt::AlignCenter);

                    // update active errors gui
                    ui->ActiveErrorsOutput->setText(QString::number(events->totalErrors - events->totalCleared));
                    ui->ActiveErrorsOutput->setAlignment(Qt::AlignCenter);

                    break;

                case ELECTRICAL:

                    qDebug() <<  "Message id: electrical" << qPrintable("\n");

                    break;

                case EVENT_DUMP:

                    qDebug() <<  "Message id: event dump" << qPrintable("\n");

                    // load all events to event linked list
                    events->loadEventDump(message);

                    // reset dump
                    dumpMessage = "";
                    wkgErrPtr = events->headErrorNode;
                    wkgEventPtr = events->headEventNode;

                    // loop through all errors and events
                    while (wkgErrPtr != nullptr || wkgEventPtr != nullptr)
                    {
                        // get next to print by ID
                        EventNode* nextPrintPtr = events->getNextNodeToPrint(wkgEventPtr, wkgErrPtr, printErr);

                        // set dump message
                        if (nextPrintPtr->id != 0) dumpMessage += '\n';
                        dumpMessage += QString::number(nextPrintPtr->id) + ',' + nextPrintPtr->timeStamp + ',' + nextPrintPtr->eventString + ',';
                        if (printErr) dumpMessage += (nextPrintPtr->cleared ? "1," : "0,");
                        dumpMessage += "\n";
                    }

                    // update total events gui
                    ui->events_output->setText(dumpMessage);
                    ui->TotalEventsOutput->setText(QString::number(events->totalEvents));
                    ui->TotalEventsOutput->setAlignment(Qt::AlignCenter);

                    break;

                case ERROR_DUMP:

                    qDebug() <<  "Message id: error dump" << qPrintable("\n");

                    // load all errors to error linked list
                    events->loadErrorDump(message);

                    // reset dump
                    dumpMessage = "";
                    wkgErrPtr = events->headErrorNode;
                    wkgEventPtr = events->headEventNode;

                    // loop through all errors and events
                    while (wkgErrPtr != nullptr || wkgEventPtr != nullptr)
                    {
                        // get next to print by ID
                        EventNode* nextPrintPtr = events->getNextNodeToPrint(wkgEventPtr, wkgErrPtr, printErr);

                        // set dump message
                        if (nextPrintPtr->id != 0) dumpMessage += '\n';
                        dumpMessage += QString::number(nextPrintPtr->id) + ',' + nextPrintPtr->timeStamp + ',' + nextPrintPtr->eventString + ',';
                        if (printErr) dumpMessage += (nextPrintPtr->cleared ? "1," : "0,");
                        dumpMessage += "\n";
                    }

                    // update total errors gui
                    ui->events_output->setText(dumpMessage);
                    ui->TotalErrorsOutput->setText(QString::number(events->totalErrors));
                    ui->TotalErrorsOutput->setAlignment(Qt::AlignCenter);

                    // update cleared errors gui
                    ui->ClearedErrorsOutput->setText(QString::number(events->totalCleared));
                    ui->ClearedErrorsOutput->setAlignment(Qt::AlignCenter);

                    // update active errors gui
                    ui->ActiveErrorsOutput->setText(QString::number(events->totalErrors - events->totalCleared));
                    ui->ActiveErrorsOutput->setAlignment(Qt::AlignCenter);

                    break;

                case CLEAR_ERROR:
                    qDebug() << "Message id: clear error " << message << qPrintable("\n");

                    //update cleared status of error with given id
                    events->clearError(message.left(message.indexOf(DELIMETER)).toInt());

                    //update the cleared error selection box in dev tools
                    update_non_cleared_error_selection();

                    // update cleared errors gui
                    ui->ClearedErrorsOutput->setText(QString::number(events->totalCleared));
                    ui->ClearedErrorsOutput->setAlignment(Qt::AlignCenter);

                    // update active errors gui
                    ui->ActiveErrorsOutput->setText(QString::number(events->totalErrors - events->totalCleared));
                    ui->ActiveErrorsOutput->setAlignment(Qt::AlignCenter);

                    break;

                case BEGIN:

                    //stop handshake protocols
                    handshakeTimer->stop();

                    //
                    qDebug() << "Begin signal received, handshake complete";

                    ui->handshake_button->setText("Disconnect");
                    ui->handshake_button->setStyleSheet("color: rgb(255, 255, 255);border-color: rgb(255, 255, 255);background-color: #FE1C1C;font: 15pt Segoe UI;");

                    ddmCon->connected = true;

                    break;

                case CLOSING_CONNECTION:

                    //log
                    qDebug() << "Controller disconnect message received";

                    //assign conn flag
                    ddmCon->connected = false;

                    if (reconnect)
                    {
                        //attempt handshake to reconnect, (optional setting)
                        on_handshake_button_clicked();
                    }
                    else
                    {
                        ui->handshake_button->setText("Connect");
                        ui->handshake_button->setStyleSheet("color: rgb(255, 255, 255);border-color: rgb(255, 255, 255);background-color: #14AE5C;font: 15pt Segoe UI;");
                    }

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

        ui->handshake_button->setText("Connecting");
        ui->handshake_button->setStyleSheet("color: #FFFFFF;border-color: rgb(255, 255, 255);background-color: #FF7518;font: 15pt Segoe UI;");
        ui->ddm_port_selection->setEnabled(false);

        //disable changes to connection settings
        disableConnectionChanges();
    }
    else
    {
        qDebug() << "Sending disconnect message to controller" << Qt::endl;

        ddmCon->transmit(QString::number(CLOSING_CONNECTION) + '\n');

        handshakeTimer->stop();

        ui->handshake_button->setText("Connect");
        ui->handshake_button->setStyleSheet("color: rgb(255, 255, 255);border-color: rgb(255, 255, 255);background-color: #14AE5C;font: 15pt Segoe UI;");
        ui->ddm_port_selection->setEnabled(true);

        //allow user to modify connection settings
        enableConnectionChanges();

        ddmCon->connected = false;
    }
}

//sends user to settings page when clicked
void MainWindow::on_SettingsPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(2);
    resetPageButton();
    ui->SettingsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;");
}

//sends user to events page when clicked
void MainWindow::on_EventsPageButton_clicked()
{
    // TODO: first visit refresh page with dump of whole LL??
    ui->Flow_Label->setCurrentIndex(0);
    resetPageButton();
    ui->EventsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;");
}

//sends user to status page when clicked
void MainWindow::on_StatusPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(4);
    resetPageButton();
    ui->StatusPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;");
}

//sends user to electrical page when clicked
void MainWindow::on_ElectricalPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(3);
    resetPageButton();
    ui->ElectricalPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;");
}

//sends user to developer page when clicked
void MainWindow::on_DevPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(1);
    resetPageButton();
    ui->DevPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;");
}

void MainWindow::resetPageButton()
{
    ui->SettingsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);border-color: rgb(255, 255, 255);");
    ui->EventsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);border-color: rgb(255, 255, 255);");
    ui->StatusPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);border-color: rgb(255, 255, 255);");
    ui->ElectricalPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);border-color: rgb(255, 255, 255);");
    ui->DevPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);border-color: rgb(255, 255, 255);");
}

//download button for events in CSV format
void MainWindow::on_download_button_clicked()
{
    // get current date
    QString logFileName = QDateTime::currentDateTime().date().toString("MM-dd-yyyy");

    // save logfile
    events->outputToLogFile(logFileName.toStdString() + "-logfile.txt");
}

void MainWindow::on_clear_error_button_clicked()
{

    //get error from combo box and split the error on delimeter
    QStringList errorElements = ui->non_cleared_error_selection->currentText().split(DELIMETER);

    if (errorElements.isEmpty() || errorElements.first().isEmpty())
    {
        return;
    }

    //get the id of the error
    int errorId = errorElements[0].toInt();

    //make a request to csim to clear the error
    emit clearErrorRequest(errorId);
}

void MainWindow::update_non_cleared_error_selection()
{
    //clear combo box
    ui->non_cleared_error_selection->clear();

    //check for valid events ptr
    if (csimHandle->eventsPtr != nullptr)
    {
        //get head node
        EventNode *wkgPtr = csimHandle->eventsPtr->headErrorNode;

        //loop until list ends
        while (wkgPtr != nullptr)
        {
            //add the uncleared error to the combo box
            ui->non_cleared_error_selection->addItem(QString::number(wkgPtr->id) + DELIMETER + wkgPtr->eventString);

            //get next error
            wkgPtr = wkgPtr->nextPtr;
        }
    }
}

//makes all settings in connection settings uneditable (call when ddm connection
//is made)
void MainWindow::disableConnectionChanges()
{
    ui->ddm_port_selection->setDisabled(true);
    ui->baud_rate_selection->setDisabled(true);
    ui->data_bits_selection->setDisabled(true);
    ui->parity_selection->setDisabled(true);
    ui->stop_bit_selection->setDisabled(true);
    ui->flow_control_selection->setDisabled(true);
}


void MainWindow::enableConnectionChanges()
{
    ui->ddm_port_selection->setEnabled(true);
    ui->baud_rate_selection->setEnabled(true);
    ui->data_bits_selection->setEnabled(true);
    ui->parity_selection->setEnabled(true);
    ui->stop_bit_selection->setEnabled(true);
    ui->flow_control_selection->setEnabled(true);
}


void MainWindow::setup_connection_settings()
{
    int i;

    //setup port name selections
    setup_ddm_port_selection(0);
    setup_csim_port_selection(0);

    //add settings for baud rate
    for (i=1200; i <= 38400; i*=2 )
    {
        ui->baud_rate_selection->addItem(QString::number(i));
    }
    ui->baud_rate_selection->addItem(QString::number(57600));
    ui->baud_rate_selection->addItem(QString::number(115200));

    //add settings for data bits
    for (i=5; i <= 8; i++)
    {
        ui->data_bits_selection->addItem(QString::number(i));
    }

    //add settings for parity
    ui->parity_selection->addItem("No Parity");
    ui->parity_selection->addItem("Even Parity");
    ui->parity_selection->addItem("Odd Parity");
    ui->parity_selection->addItem("Space Parity");
    ui->parity_selection->addItem("Mark Parity");

    //add settings for stop bits
    ui->stop_bit_selection->addItem("One Stop");
    ui->stop_bit_selection->addItem("One and Half Stop");
    ui->stop_bit_selection->addItem("Two Stop");

    //add flow control settings
    ui->flow_control_selection->addItem("No Flow Control");
    ui->flow_control_selection->addItem("Hardware Control");
    ui->flow_control_selection->addItem("Software Control");

    //set starting values
    ui->baud_rate_selection->setCurrentIndex(ui->baud_rate_selection->findText(QString::number(INITIAL_BAUD_RATE)));
    ui->data_bits_selection->setCurrentIndex(ui->data_bits_selection->findText(QString::number(INITIAL_DATA_BITS)));
    ui->parity_selection->setCurrentIndex(static_cast<int>(INITIAL_PARITY));
    ui->flow_control_selection->setCurrentIndex(static_cast<int>(INITIAL_FLOW_CONTROL));

    switch (INITIAL_STOP_BITS)
    {
    case QSerialPort::OneStop:
        ui->stop_bit_selection->setCurrentIndex(0); // Assuming One Stop is the first item
        break;
    case QSerialPort::OneAndHalfStop:
        ui->stop_bit_selection->setCurrentIndex(1); // Assuming One and a half Stop is the second item
        break;
    case QSerialPort::TwoStop:
        ui->stop_bit_selection->setCurrentIndex(2); // Assuming Two Stop is the third item
        break;
    default:
        // Handle default case
        break;
    }
}


void MainWindow::on_baud_rate_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized
        return;
    }

    switch (index)
    {
    case 0:
        ddmCon->baudRate = QSerialPort::Baud1200;
        break;
    case 1:
        ddmCon->baudRate = QSerialPort::Baud2400;
        break;
    case 2:
        ddmCon->baudRate = QSerialPort::Baud4800;
        break;
    case 3:
        ddmCon->baudRate = QSerialPort::Baud9600;
        break;
    case 4:
        ddmCon->baudRate = QSerialPort::Baud19200;
        break;
    case 5:
        ddmCon->baudRate = QSerialPort::Baud38400;
        break;
    case 6:
        ddmCon->baudRate = QSerialPort::Baud57600;
        break;
    case 7:
        ddmCon->baudRate = QSerialPort::Baud115200;
        break;
    default:
        // Handle default case
        break;
    }
}


void MainWindow::on_stop_bit_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized
        return;
    }

    switch (index)
    {
    case 0:
        ddmCon->stopBits = QSerialPort::OneStop;
        break;
    case 1:
        ddmCon->stopBits = QSerialPort::OneAndHalfStop;
        break;
    case 2:
        ddmCon->stopBits = QSerialPort::TwoStop;
        break;
    default:
        // Handle default case
        break;
    }
}

void MainWindow::on_flow_control_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized
        return;
    }

    switch (index)
    {
    case 0:
        ddmCon->flowControl = QSerialPort::NoFlowControl;
        break;
    case 1:
        ddmCon->flowControl = QSerialPort::HardwareControl;
        break;
    case 2:
        ddmCon->flowControl = QSerialPort::SoftwareControl;
        break;
    default:
        // Handle default case
        break;
    }
}

void MainWindow::on_parity_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized
        return;
    }

    switch (index)
    {
    case 0:
        ddmCon->parity = QSerialPort::NoParity;
        break;
    case 1:
        ddmCon->parity = QSerialPort::EvenParity;
        break;
    case 2:
        ddmCon->parity = QSerialPort::OddParity;
        break;
    case 3:
        ddmCon->parity = QSerialPort::SpaceParity;
        break;
    case 4:
        ddmCon->parity = QSerialPort::MarkParity;
        break;
    default:
        // Handle default case
        break;
    }
}

void MainWindow::on_data_bits_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized
        return;
    }

    switch (index)
    {
    case 0:
        ddmCon->dataBits = QSerialPort::Data5;
        break;
    case 1:
        ddmCon->dataBits = QSerialPort::Data6;
        break;
    case 2:
        ddmCon->dataBits = QSerialPort::Data7;
        break;
    case 3:
        ddmCon->dataBits = QSerialPort::Data8;
        break;
    default:
        // Handle default case
        break;
    }
}
