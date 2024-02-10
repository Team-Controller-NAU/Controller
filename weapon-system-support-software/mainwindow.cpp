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
    handshakeTimer( new QTimer(this) ),

    // timer is used to update the GUI
    lastMessageTimer( new QTimer(this) ),

    //init user settings to our organization and project
    userSettings("Team Controller", "WSSS")

{

    //set output settings for qDebug
    qSetMessagePattern(QDEBUG_OUTPUT_FORMAT);

    //init gui
    ui->setupUi(this);

    //scan available ports, add port names to port selection combo boxes
    setup_connection_settings();

    //update class port name values
    csimPortName = ui->csim_port_selection->currentText();
    ddmPortName = ui->ddm_port_selection->currentText();

    //init ddm connection using the current values set in connection settings
    ddmCon = new Connection(ui->ddm_port_selection->currentText(),
                            fromStringBaudRate(ui->baud_rate_selection->currentText()),
                            fromStringDataBits(ui->data_bits_selection->currentText()),
                            fromStringParity(ui->parity_selection->currentText()),
                            fromStringStopBits(ui->stop_bit_selection->currentText()),
                            fromStringFlowControl(ui->flow_control_selection->currentText()));

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

    //CSIM control slots ==============================================================

    //connect custom transmission requests from ddm to csims execution slot
    connect(this, &MainWindow::transmissionRequest, csimHandle, &CSim::completeTransmissionRequest);

    //connect custom clear error requests from ddm to csims execution slot
    connect(this, &MainWindow::clearErrorRequest, csimHandle, &CSim::clearError);

    //connect output session string to ddm output session string slot
    connect(this, &MainWindow::outputMessagesSentRequest, csimHandle, &CSim::outputMessagesSent);
    //=================================================================================

    // connect update elapsed time function to a timer
    lastMessageTimer->setInterval(1000);
    connect(lastMessageTimer, &QTimer::timeout, this, &MainWindow::updateTimer);

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

void MainWindow::createDDMCon()
{
    //check if ddmCon is allocated
    if (ddmCon != nullptr)
    {
        //close current connection
        delete ddmCon;

        //open new connection
        ddmCon = new Connection(ui->ddm_port_selection->currentText(),
                                fromStringBaudRate(ui->baud_rate_selection->currentText()),
                                fromStringDataBits(ui->data_bits_selection->currentText()),
                                fromStringParity(ui->parity_selection->currentText()),
                                fromStringStopBits(ui->stop_bit_selection->currentText()),
                                fromStringFlowControl(ui->flow_control_selection->currentText()));

        //set up signal and slot (when a message is sent to DDMs serial port, the readyRead signal is emitted and
        //readSerialData() is called)
        connect(&ddmCon->serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

        qDebug() << "GUI is now listening to port " << ddmCon->portName;
    }
}

//initial synchronization between controller and ddm
void MainWindow::handshake()
{
    // Send handshake message
    ddmCon->transmit(QString::number(LISTENING) + '\n');

    //keep gui interactive
    QCoreApplication::processEvents();
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
            QStringList messageSet;

            //gets the approx time of the start of session for the logfile
            qint64 secsSinceEpoch = QDateTime::currentSecsSinceEpoch();
            //QString logFileName = QString::number(secsSinceEpoch);
            QString runningLogFileName = "C:/WSSS/appendTest1.txt";
            int eventLogCounter = 0;
            int errorLogCounter = 0;


            //get images for buttons
            QPixmap greenButton(":/resources/Images/greenButton.png");
            QPixmap redButton(":/resources/Images/redButton.png");

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
                    if (eventFilter == ALL || eventFilter == EVENTS) ui->events_output->append(message);

                    // update total events gui
                    ui->TotalEventsOutput->setText(QString::number(events->totalEvents));
                    ui->TotalEventsOutput->setAlignment(Qt::AlignCenter);
                    ui->statusEventOutput->setText(QString::number(events->totalEvents));
                    ui->statusEventOutput->setAlignment(Qt::AlignCenter);

                    events->appendMessageToLogfile(runningLogFileName, "Event: " + message, eventLogCounter, errorLogCounter, true);
                    eventLogCounter += 1;

                    break;

                case ERROR:

                    // status
                    qDebug() <<  "Message id: error update" << qPrintable("\n");

                    //add new error to error ll
                    events->loadErrorData(message);

                    // check for any type of error filter, including all
                    if(eventFilter != EVENTS)
                    {
                        // check for cleared filter
                        if(eventFilter == CLEARED_ERRORS && events->lastErrorNode->cleared)
                        {
                            ui->events_output->append(message);
                        }
                        // check for non-cleared filter
                        else if (eventFilter == NON_CLEARED_ERRORS && !events->lastErrorNode->cleared)
                        {
                            ui->events_output->append(message);
                        }
                        // check for all or errors filter
                        else if (eventFilter == ALL || eventFilter == ERRORS)
                        {
                            ui->events_output->append(message);
                        }
                    }
                    // otherwise do nothing

                    //update the cleared error selection box in dev tools
                    update_non_cleared_error_selection();

                    // update total errors gui
                    ui->TotalErrorsOutput->setText(QString::number(events->totalErrors));
                    ui->TotalErrorsOutput->setAlignment(Qt::AlignCenter);
                    ui->statusErrorOutput->setText(QString::number(events->totalErrors));
                    ui->statusErrorOutput->setAlignment(Qt::AlignCenter);

                    // update cleared errors gui
                    ui->ClearedErrorsOutput->setText(QString::number(events->totalCleared));
                    ui->ClearedErrorsOutput->setAlignment(Qt::AlignCenter);

                    // update active errors gui
                    ui->ActiveErrorsOutput->setText(QString::number(events->totalErrors - events->totalCleared));
                    ui->ActiveErrorsOutput->setAlignment(Qt::AlignCenter);

                    events->appendMessageToLogfile(runningLogFileName, message, eventLogCounter, errorLogCounter, false);
                    errorLogCounter += 1;

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

                    // check for all filter
                    if(eventFilter == ALL)
                    {
                        // loop through all errors and events
                        while (wkgErrPtr != nullptr || wkgEventPtr != nullptr)
                        {
                            // get next to print by ID
                            EventNode* nextPrintPtr = events->getNextNodeToPrint(wkgEventPtr, wkgErrPtr, printErr);

                            // set dump message
                            if (dumpMessage != "") dumpMessage += '\n';
                            dumpMessage += QString::number(nextPrintPtr->id) + ',' + nextPrintPtr->timeStamp + ',' + nextPrintPtr->eventString + ',';
                            if (printErr) dumpMessage += (nextPrintPtr->cleared ? "1," : "0,");
                            dumpMessage += "\n";
                        }

                        // update gui
                        ui->events_output->setText(dumpMessage);
                    }
                    // check for events filter
                    else if(eventFilter == EVENTS)
                    {
                        // split the event dump messages up
                        messageSet = message.split(",,", Qt::SkipEmptyParts);

                        // iterate through the event set
                        for (const QString &event : messageSet)
                        {
                            // check for empty
                            if(!messageSet.isEmpty() && event != "\n")
                            {
                                // update gui
                                ui->events_output->append(event + ",\n");
                            }
                        }
                    }
                    // otherwise do nothing

                    // update total events gui
                    ui->TotalEventsOutput->setText(QString::number(events->totalEvents));
                    ui->TotalEventsOutput->setAlignment(Qt::AlignCenter);
                    ui->statusEventOutput->setText(QString::number(events->totalEvents));
                    ui->statusEventOutput->setAlignment(Qt::AlignCenter);

                    // write dump to log file
                    events->appendMessageToLogfile(runningLogFileName, dumpMessage, eventLogCounter, errorLogCounter, true);
                    break;

                case ERROR_DUMP:

                    qDebug() <<  "Message id: error dump" << qPrintable("\n");

                    // load all errors to error linked list
                    events->loadErrorDump(message);

                    // reset dump
                    dumpMessage = "";
                    wkgErrPtr = events->headErrorNode;
                    wkgEventPtr = events->headEventNode;

                    // check for all filter
                    if(eventFilter == ALL)
                    {
                        // loop through all errors and events
                        while (wkgErrPtr != nullptr || wkgEventPtr != nullptr)
                        {
                            // get next to print by ID
                            EventNode* nextPrintPtr = events->getNextNodeToPrint(wkgEventPtr, wkgErrPtr, printErr);

                            // set dump message
                            if (dumpMessage != "") dumpMessage += '\n';
                            dumpMessage += QString::number(nextPrintPtr->id) + ',' + nextPrintPtr->timeStamp + ',' + nextPrintPtr->eventString + ',';
                            if (printErr) dumpMessage += (nextPrintPtr->cleared ? "1," : "0,");
                            dumpMessage += "\n";
                        }

                        // update gui
                        ui->events_output->setText(dumpMessage);
                    }
                    // check for any errors filter
                    else if(eventFilter != EVENTS)
                    {
                        // check for cleared filter
                        if(eventFilter == CLEARED_ERRORS)
                        {
                            // loop through all errors
                            while(wkgErrPtr != nullptr)
                            {
                                // check for cleared
                                if(wkgErrPtr->cleared)
                                {
                                    // set dump message
                                    if (dumpMessage != "") dumpMessage += '\n';
                                    dumpMessage += QString::number(wkgErrPtr->id) + ',' + wkgErrPtr->timeStamp + ',' + wkgErrPtr->eventString + ',';
                                    dumpMessage += (wkgErrPtr->cleared ? "1," : "0,");
                                    dumpMessage += "\n";
                                }
                                wkgErrPtr = wkgErrPtr->nextPtr;
                            }
                            // update ui
                            ui->events_output->setText(dumpMessage);
                        }
                        // check for non-cleared filter
                        else if(eventFilter == NON_CLEARED_ERRORS)
                        {
                            // loop through all errors
                            while(wkgErrPtr != nullptr)
                            {
                                // check for non-cleared
                                if(!wkgErrPtr->cleared)
                                {
                                    // set dump message
                                    if (dumpMessage != "") dumpMessage += '\n';
                                    dumpMessage += QString::number(wkgErrPtr->id) + ',' + wkgErrPtr->timeStamp + ',' + wkgErrPtr->eventString + ',';
                                    dumpMessage += (wkgErrPtr->cleared ? "1," : "0,");
                                    dumpMessage += "\n";
                                }
                                wkgErrPtr = wkgErrPtr->nextPtr;
                            }
                            // update ui
                            ui->events_output->setText(dumpMessage);
                        }
                        // assume errors filter
                        else
                        {
                            // split the event dump messages up
                            messageSet = message.split(",,", Qt::SkipEmptyParts);

                            // iterate through the event set
                            for (const QString &error : messageSet)
                            {
                                // check for empty
                                if(!messageSet.isEmpty() && error != "\n")
                                {
                                    // update gui
                                    ui->events_output->append(error + ",\n");
                                }
                            }
                        }
                    }
                    // otherwise do nothing

                    // update total errors gui
                    ui->TotalErrorsOutput->setText(QString::number(events->totalErrors));
                    ui->TotalErrorsOutput->setAlignment(Qt::AlignCenter);
                    ui->statusErrorOutput->setText(QString::number(events->totalErrors));
                    ui->statusErrorOutput->setAlignment(Qt::AlignCenter);

                    // update cleared errors gui
                    ui->ClearedErrorsOutput->setText(QString::number(events->totalCleared));
                    ui->ClearedErrorsOutput->setAlignment(Qt::AlignCenter);

                    // update active errors gui
                    ui->ActiveErrorsOutput->setText(QString::number(events->totalErrors - events->totalCleared));
                    ui->ActiveErrorsOutput->setAlignment(Qt::AlignCenter);

                    events->appendMessageToLogfile(runningLogFileName, dumpMessage, eventLogCounter, errorLogCounter, false);

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

                    // start last message timer if not already active
                    if(!lastMessageTimer->isActive())
                    {
                        lastMessageTimer->start();
                    }

                    // debug
                    qDebug() << "Begin signal received, handshake complete";

                    // update ui
                    ui->handshake_button->setText("Disconnect");
                    ui->handshake_button->setStyleSheet("color: rgb(255, 255, 255);border-color: rgb(255, 255, 255);background-color: #FE1C1C;font: 15pt Segoe UI;");
                    ui->connectionStatus->setPixmap(greenButton);

                    events->freeLinkedLists();
                    ui->events_output->clear();

                    ui->TotalEventsOutput->setText("0");
                    ui->TotalEventsOutput->setAlignment(Qt::AlignCenter);
                    ui->statusEventOutput->setText("0");
                    ui->statusEventOutput->setAlignment(Qt::AlignCenter);

                    ui->TotalErrorsOutput->setText("0");
                    ui->TotalErrorsOutput->setAlignment(Qt::AlignCenter);
                    ui->statusErrorOutput->setText("0");
                    ui->statusErrorOutput->setAlignment(Qt::AlignCenter);

                    ui->ClearedErrorsOutput->setText("0");
                    ui->ClearedErrorsOutput->setAlignment(Qt::AlignCenter);

                    ui->ActiveErrorsOutput->setText("0");
                    ui->ActiveErrorsOutput->setAlignment(Qt::AlignCenter);

                    ddmCon->connected = true;

                    break;

                case CLOSING_CONNECTION:

                    //log
                    qDebug() << "Controller disconnect message received";

                    // check for not empty
                    if(events->totalNodes != 0)
                    {
                        // new "session" ended, save to log file
                        qint64 secsSinceEpoch = QDateTime::currentSecsSinceEpoch();
                        QString logFileName = QString::number(secsSinceEpoch);

                        // save logfile - autosave condition
                        events->outputToLogFile(logFileName.toStdString() + "-logfile-A.txt");
                    }

                    //assign conn flag
                    ddmCon->connected = false;

                    // update time since last message so its not frozen
                    ui->DDMTimer->setText("Time Since Last Message: 00:00:00");
                    ui->DDMTimer->setAlignment(Qt::AlignRight);

                    // stop last message timer if still active
                    if(lastMessageTimer->isActive())
                    {
                        lastMessageTimer->stop();
                    }

                    if (reconnect)
                    {
                        //attempt handshake to reconnect, (optional setting)
                        on_handshake_button_clicked();
                    }
                    else
                    {
                        //refreshes connection button/displays
                        ui->handshake_button->setText("Connect");
                        ui->handshake_button->setStyleSheet("color: rgb(255, 255, 255);border-color: rgb(255, 255, 255);background-color: #14AE5C;font: 15pt Segoe UI;");
                        ui->connectionStatus->setPixmap(redButton);
                    }

                    break;

                default:

                    //log
                    qDebug() << "Unrecognized serial message received : " << message;

                    break;

                }

                //qDebug() << "remaining buffer: " << ddmCon->serialPort.peek(ddmCon->serialPort.bytesAvailable());
        }
        // update the timestamp of last received message
        timeLastReceived = QDateTime::currentDateTime();
    }
    else
    {
        qDebug() << "Serial port is closed, unable to read data";
    }
}

void MainWindow::setup_csim_port_selection(int index)
{
    // Fetch available serial ports and add their names to the combo box
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString portName = info.portName();
        ui->csim_port_selection->addItem(portName);

        // Check if the current port name matches the one declared in settings
        if (portName == userSettings.value("csimPortName").toString())
        {
            // If a match is found, set the current index of the combo box
            ui->csim_port_selection->setCurrentIndex(ui->csim_port_selection->count() - 1);
        }
    }
}

void MainWindow::setup_ddm_port_selection(int index)
{
    // Fetch available serial ports and add their names to the combo box
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString portName = info.portName();
        ui->ddm_port_selection->addItem(portName);

        // Check if the current port name matches the one declared in settings
        if (portName == userSettings.value("portName").toString())
        {
            // If a match is found, set the current index of the combo box
            ui->ddm_port_selection->setCurrentIndex(ui->ddm_port_selection->count() - 1);
        }
    }
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

//support function, outputs usersettings values to qdebug
void MainWindow::displaySavedConnectionSettings()
{
    logEmptyLine();
    qDebug() << "Connection Settings Saved Cross Session:";
    // Print the values of each setting
    qDebug() << "DDM Port: " << userSettings.value("portName").toString();
    qDebug() << "CSIM Port: " << userSettings.value("csimPortName").toString();
    qDebug() << "baudRate:" << userSettings.value("baudRate").toString();
    qDebug() << "dataBits:" << userSettings.value("dataBits").toString();
    qDebug() << "parity:" << userSettings.value("parity").toString();
    qDebug() << "stopBits:" << userSettings.value("stopBits").toString();
    qDebug() << "flowControl:" << userSettings.value("flowControl").toString() << Qt::endl;
}

void MainWindow::setup_connection_settings()
{
    int i;

    // Check and set initial value for "portName"
    if (userSettings.value("portName").toString().isEmpty())
        userSettings.setValue("portName", INITIAL_DDM_PORT);

    // Check and set initial value for "csimPortName"
    if (userSettings.value("csimPortName").toString().isEmpty())
        userSettings.setValue("csimPortName", INITIAL_CSIM_PORT);

    // Check and set initial value for "baudRate"
    if (userSettings.value("baudRate").toString().isEmpty())
        userSettings.setValue("baudRate", toString(INITIAL_BAUD_RATE));

    // Check and set initial value for "dataBits"
    if (userSettings.value("dataBits").toString().isEmpty())
        userSettings.setValue("dataBits", toString(INITIAL_DATA_BITS));

    // Check and set initial value for "parity"
    if (userSettings.value("parity").toString().isEmpty())
        userSettings.setValue("parity", toString(INITIAL_PARITY));

    // Check and set initial value for "stopBits"
    if (userSettings.value("stopBits").toString().isEmpty())
        userSettings.setValue("stopBits", toString(INITIAL_STOP_BITS));

    // Check and set initial value for "flowControl"
    if (userSettings.value("flowControl").toString().isEmpty())
        userSettings.setValue("flowControl", toString(INITIAL_FLOW_CONTROL));


    // Display user settings
    displaySavedConnectionSettings();

    //setup port name selections on gui (scans for available ports)
    setup_ddm_port_selection(0);
    setup_csim_port_selection(0);

    // Add baud rate settings
    ui->baud_rate_selection->addItem(toString(QSerialPort::Baud1200));
    ui->baud_rate_selection->addItem(toString(QSerialPort::Baud2400));
    ui->baud_rate_selection->addItem(toString(QSerialPort::Baud4800));
    ui->baud_rate_selection->addItem(toString(QSerialPort::Baud9600));
    ui->baud_rate_selection->addItem(toString(QSerialPort::Baud19200));
    ui->baud_rate_selection->addItem(toString(QSerialPort::Baud38400));
    ui->baud_rate_selection->addItem(toString(QSerialPort::Baud57600));
    ui->baud_rate_selection->addItem(toString(QSerialPort::Baud115200));

    // Add data bits settings
    ui->data_bits_selection->addItem(toString(QSerialPort::Data5));
    ui->data_bits_selection->addItem(toString(QSerialPort::Data6));
    ui->data_bits_selection->addItem(toString(QSerialPort::Data7));
    ui->data_bits_selection->addItem(toString(QSerialPort::Data8));

    // Add parity settings
    ui->parity_selection->addItem(toString(QSerialPort::NoParity));
    ui->parity_selection->addItem(toString(QSerialPort::EvenParity));
    ui->parity_selection->addItem(toString(QSerialPort::OddParity));
    ui->parity_selection->addItem(toString(QSerialPort::SpaceParity));
    ui->parity_selection->addItem(toString(QSerialPort::MarkParity));

    // Add stop bits settings
    ui->stop_bit_selection->addItem(toString(QSerialPort::OneStop));
    ui->stop_bit_selection->addItem(toString(QSerialPort::OneAndHalfStop));
    ui->stop_bit_selection->addItem(toString(QSerialPort::TwoStop));

    // Add flow control settings
    ui->flow_control_selection->addItem(toString(QSerialPort::NoFlowControl));
    ui->flow_control_selection->addItem(toString(QSerialPort::HardwareControl));
    ui->flow_control_selection->addItem(toString(QSerialPort::SoftwareControl));

    // Set initial values for the q combo boxes
    ui->baud_rate_selection->setCurrentIndex(ui->baud_rate_selection->findText(userSettings.value("baudRate").toString()));
    ui->data_bits_selection->setCurrentIndex(ui->data_bits_selection->findText(userSettings.value("dataBits").toString()));
    ui->parity_selection->setCurrentIndex(ui->parity_selection->findText(userSettings.value("parity").toString()));
    ui->stop_bit_selection->setCurrentIndex(ui->stop_bit_selection->findText(userSettings.value("stopBits").toString()));
    ui->flow_control_selection->setCurrentIndex(ui->flow_control_selection->findText(userSettings.value("flowControl").toString()));


    // Set initial stop bits value
    switch (fromStringStopBits(userSettings.value("stopBits").toString())) {
    case QSerialPort::OneStop:
        ui->stop_bit_selection->setCurrentIndex(ui->stop_bit_selection->findText(toString(QSerialPort::OneStop)));
        break;
    case QSerialPort::OneAndHalfStop:
        ui->stop_bit_selection->setCurrentIndex(ui->stop_bit_selection->findText(toString(QSerialPort::OneAndHalfStop)));
        break;
    case QSerialPort::TwoStop:
        ui->stop_bit_selection->setCurrentIndex(ui->stop_bit_selection->findText(toString(QSerialPort::TwoStop)));
        break;
    }
}

// method updates the elapsed time since last message received to DDM
void MainWindow::updateTimer()
{
    // initialize variables
    QTime elapsedTime;
    QString message;

    // calculate time elapsed since the last time DDM received a message
    QDateTime currentTime = QDateTime::currentDateTime();

    // the msecsto method returns the amount of ms from timeLastRecieved to currentTime
    qint64 elapsedMs = timeLastReceived.msecsTo(currentTime);

    // check for negative elapsed time
    if(elapsedMs < 0)
    {
        qDebug() << "Error: time since last DDM message received is negative.\n";
    }
    // check for invalid datetime
    else if(elapsedMs == 0)
    {
        qDebug() << "Error: either datetime is invalid.\n";
    }
    // assume positive elapsed time
    else
    {
        // convert back into QTime instead of qint64
        elapsedTime = QTime(0, 0, 0).addMSecs(elapsedMs);

        // update gui
        message = "Time Since Last Message: " + elapsedTime.toString("HH:mm:ss");
        ui->DDMTimer->setText(message);
        ui->DDMTimer->setAlignment(Qt::AlignRight);
    }
}

//======================================================================================
//To string methods for QSerialPortEnumeratedValues
//======================================================================================
// Convert QSerialPort::BaudRate to string
QString MainWindow::toString(QSerialPort::BaudRate baudRate) {
    switch (baudRate) {
    case QSerialPort::Baud1200: return "1200";
    case QSerialPort::Baud2400: return "2400";
    case QSerialPort::Baud4800: return "4800";
    case QSerialPort::Baud9600: return "9600";
    case QSerialPort::Baud19200: return "19200";
    case QSerialPort::Baud38400: return "38400";
    case QSerialPort::Baud57600: return "57600";
    case QSerialPort::Baud115200: return "115200";
    default:
        // Invalid input, throw an exception with the parameter value
        throw std::invalid_argument("Invalid baud rate enum value: " + QString::number(baudRate).toStdString());
    }
}

QString MainWindow::toString(QSerialPort::DataBits dataBits) {
    switch (dataBits) {
    case QSerialPort::Data5: return "5";
    case QSerialPort::Data6: return "6";
    case QSerialPort::Data7: return "7";
    case QSerialPort::Data8: return "8";
    default:
        // Invalid input, throw an exception with the parameter value
        throw std::invalid_argument("Invalid data bits enum value: " + QString::number(dataBits).toStdString());
    }
}

QString MainWindow::toString(QSerialPort::Parity parity) {
    switch (parity) {
    case QSerialPort::NoParity: return "No Parity";
    case QSerialPort::EvenParity: return "Even Parity";
    case QSerialPort::OddParity: return "Odd Parity";
    case QSerialPort::SpaceParity: return "Space Parity";
    case QSerialPort::MarkParity: return "Mark Parity";
    default:
        // Invalid input, throw an exception with the parameter value
        throw std::invalid_argument("Invalid parity enum value: " + QString::number(parity).toStdString());
    }
}

QString MainWindow::toString(QSerialPort::StopBits stopBits) {
    switch (stopBits) {
    case QSerialPort::OneStop: return "1";
    case QSerialPort::OneAndHalfStop: return "1.5";
    case QSerialPort::TwoStop: return "2";
    default:
        // Invalid input, throw an exception with the parameter value
        throw std::invalid_argument("Invalid stop bits enum value: " + QString::number(stopBits).toStdString());
    }
}

QString MainWindow::toString(QSerialPort::FlowControl flowControl) {
    switch (flowControl) {
    case QSerialPort::NoFlowControl: return "No Flow Control";
    case QSerialPort::HardwareControl: return "Hardware Control";
    case QSerialPort::SoftwareControl: return "Software Control";
    default:
        // Invalid input, throw an exception with the parameter value
        throw std::invalid_argument("Invalid flow control enum value: " + QString::number(flowControl).toStdString());
    }
}



//======================================================================================
//From string methods for QSerialPortEnumeratedValues
//======================================================================================
QSerialPort::BaudRate MainWindow::fromStringBaudRate(QString baudRateStr) {
    if (baudRateStr == "1200") {
        return QSerialPort::Baud1200;
    } else if (baudRateStr == "2400") {
        return QSerialPort::Baud2400;
    } else if (baudRateStr == "4800") {
        return QSerialPort::Baud4800;
    } else if (baudRateStr == "9600") {
        return QSerialPort::Baud9600;
    } else if (baudRateStr == "19200") {
        return QSerialPort::Baud19200;
    } else if (baudRateStr == "38400") {
        return QSerialPort::Baud38400;
    } else if (baudRateStr == "57600") {
        return QSerialPort::Baud57600;
    } else if (baudRateStr == "115200") {
        return QSerialPort::Baud115200;
    } else {
        throw std::invalid_argument("Invalid baud rate string: " + baudRateStr.toStdString());
    }
}

QSerialPort::DataBits MainWindow::fromStringDataBits(QString dataBitsStr) {
    if (dataBitsStr == "5") {
        return QSerialPort::Data5;
    } else if (dataBitsStr == "6") {
        return QSerialPort::Data6;
    } else if (dataBitsStr == "7") {
        return QSerialPort::Data7;
    } else if (dataBitsStr == "8") {
        return QSerialPort::Data8;
    } else {
        throw std::invalid_argument("Invalid data bits string: " + dataBitsStr.toStdString());
    }
}

QSerialPort::Parity MainWindow::fromStringParity(QString parityStr) {
    if (parityStr == "No Parity") {
        return QSerialPort::NoParity;
    } else if (parityStr == "Even Parity") {
        return QSerialPort::EvenParity;
    } else if (parityStr == "Odd Parity") {
        return QSerialPort::OddParity;
    } else if (parityStr == "Space Parity") {
        return QSerialPort::SpaceParity;
    } else if (parityStr == "Mark Parity") {
        return QSerialPort::MarkParity;
    } else {
        throw std::invalid_argument("Invalid parity string: " + parityStr.toStdString());
    }
}

QSerialPort::StopBits MainWindow::fromStringStopBits(QString stopBitsStr) {
    if (stopBitsStr == "1") {
        return QSerialPort::OneStop;
    } else if (stopBitsStr == "1.5") {
        return QSerialPort::OneAndHalfStop;
    } else if (stopBitsStr == "2") {
        return QSerialPort::TwoStop;
    } else {
        throw std::invalid_argument("Invalid stop bits string: " + stopBitsStr.toStdString());
    }
}

QSerialPort::FlowControl MainWindow::fromStringFlowControl(QString flowControlStr) {
    if (flowControlStr == "No Flow Control") {
        return QSerialPort::NoFlowControl;
    } else if (flowControlStr == "Hardware Control") {
        return QSerialPort::HardwareControl;
    } else if (flowControlStr == "Software Control") {
        return QSerialPort::SoftwareControl;
    } else {
        throw std::invalid_argument("Invalid flow control string: " + flowControlStr.toStdString());
    }
}

//logs empty line to qdebug
void MainWindow::logEmptyLine()
{
    //revert to standard output format
    qSetMessagePattern("%{message}");

    //log empty line
    qDebug();

    //enable custom message format
    qSetMessagePattern(QDEBUG_OUTPUT_FORMAT);
}
