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
    electricalObject(new electrical()),

    //this determines what will be shown on the events page
    eventFilter(ALL),

    //timer is used to repeatedly transmit handshake signals
    handshakeTimer( new QTimer(this) ),

    // timer is used to update last message received time
    lastMessageTimer( new QTimer(this) ),

    // timer is used to update controller running time
    runningControllerTimer( new QTimer(this) ),

    //init user settings to our organization and project
    userSettings("Team Controller", "WSSS"),

    //Load graphical resources
    BLANK_LIGHT(":/resources/Images/blankButton.png"),

    RED_LIGHT(":/resources/Images/redButton.png"),

    GREEN_LIGHT(":/resources/Images/greenButton.png")
{

    //set output settings for qDebug
    qSetMessagePattern(QDEBUG_OUTPUT_FORMAT);

    //init gui
    ui->setupUi(this);

    //scan available ports, add port names to port selection combo boxes
    setupSettings();

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
    connect(lastMessageTimer, &QTimer::timeout, this, &MainWindow::updateTimeSinceLastMessage);

    // connect running controller timer
    runningControllerTimer->setInterval(1000);
    connect(runningControllerTimer, &QTimer::timeout, this, &MainWindow::updateElapsedTime);

    //if handshake timeout is enabled, setup signal to timeout
    if (HANDSHAKE_TIMEOUT)
    {
        QTimer::singleShot(TIMEOUT_DURATION, this, &MainWindow::on_handshake_button_clicked);
    }

    qDebug() << "GUI is now listening to port " << ddmCon->portName;

    //set feed pos to be measured in 360 degrees
    ui->feedPosition->setMaximum(360);

    //set feed pos to be non-editable by user
    ui->feedPosition->setEnabled(false);

    //init trigger to grey buttons until updated by serial status updates
    ui->trigger1->setPixmap(BLANK_LIGHT);
    ui->trigger2->setPixmap(BLANK_LIGHT);

    // create a shortcut for ctrl + f
    QShortcut *find = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_F), this);
    connect(find, &QShortcut::activated, this, &MainWindow::findText);

    // ensures that the application will open on the events page
    on_EventsPageButton_clicked();

    // hide electrical data boxes until the data is filled in
    for(int index = 1; index <= MAX_ELECTRICAL_COMPONENTS; index++)
    {
        // get the current box name
        QString widgetName = QString("box%1_widget").arg(index);

        // get the current box based off name
        QWidget *widget = findChild<QWidget *>(widgetName);

        // check if widget exists, and hide it
        if(widget) widget->hide();
    }
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
    delete electricalObject;
}

//sets connection status, updates gui and timers
void MainWindow::updateConnectionStatus(bool connectionStatus)
{
    //update connection status
    ddmCon->connected = connectionStatus;

    //check if we are connected
    if (ddmCon->connected)
    {
        //disable changes to connection related settings
        disableConnectionChanges();

        //stop handshake protocols
        handshakeTimer->stop();

        // check if controller timer is not running
        if(!runningControllerTimer->isActive())
        {
            // start it
            runningControllerTimer->start();

            // update elapsed time
            ui->elapsedTime->setText("Elapsed Time: " + status->elapsedControllerTime);
            ui->elapsedTime->setAlignment(Qt::AlignRight);
        }

        //start last message timer
        timeLastReceived = QDateTime::currentDateTime();
        ui->DDMTimer->setText("Time Since Last Message: 00:00:00");
        ui->DDMTimer->setAlignment(Qt::AlignRight);
        lastMessageTimer->start();

        // update ui
        ui->handshake_button->setText("Disconnect");
        ui->handshake_button->setStyleSheet("QPushButton { padding-bottom: 3px; color: rgb(255, 255, 255); background-color: #FE1C1C; border: 1px solid; border-color: #cb0101; font: 15pt 'Segoe UI'; } "
                                            "QPushButton::hover { background-color: #fe3434; } "
                                            "QPushButton::pressed { background-color: #fe8080;}");
        ui->connectionLabel->setText("Connected ");
        ui->connectionStatus->setPixmap(GREEN_LIGHT);

        //clear events ll and output box
        events->freeLinkedLists();
        ui->events_output->clear();

        //reset event counters
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
    }
    //otherwise we are disconnected
    else
    {
        //stop timers if they are running
        runningControllerTimer->stop();
        lastMessageTimer->stop();
        handshakeTimer->stop();
        ui->DDMTimer->clear();

        //enable changes to connection related settings
        enableConnectionChanges();

        //refreshes connection button/displays
        ui->handshake_button->setText("Connect");
        ui->handshake_button->setStyleSheet("QPushButton { padding-bottom: 3px; color: rgb(255, 255, 255); background-color: #14AE5C; border: 1px solid; border-color: #0d723c; font: 15pt 'Segoe UI'; } "
                                            "QPushButton::hover { background-color: #1be479; } "
                                            "QPushButton::pressed { background-color: #76efae;}");
        ui->connectionStatus->setPixmap(RED_LIGHT);
        ui->connectionLabel->setText("Disconnected ");
    }
}

//when ddm port is selected, create connection class to work with that port.
//apply serial settings from settings page
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
}

//this function is called as a result of the readyRead signal being emmited by a connected serial port
//in other words, this function is called whenever ddm port receives a new message
void MainWindow::readSerialData()
{
    //read lines until all data in buffer is processed
    while (ddmCon->checkForValidMessage())
    {
        // declare variables
        electricalNode* wkgElecPtr;
        SerialMessageIdentifier messageId;
        int boxIndex;

        //get serialized string from port
        QByteArray serializedMessage = ddmCon->serialPort.readLine();

        //deserialize string
        QString message = QString::fromUtf8(serializedMessage);

        qDebug() << "message: " << message;

        //update gui with new message
        ui->stdout_label->setText(message);

        //check if message id is present and followed by a comma
        if (message[0].isDigit() && message[1] == DELIMETER)
        {
            //extract message id
            messageId = static_cast<SerialMessageIdentifier>(QString(message[0]).toInt());

            //remove message id from message
            message = message.mid(1 + DELIMETER.length());

            //determine what kind of message this is
            switch ( messageId )
            {
            case STATUS:

                qDebug() <<  "Message id: status update" << qPrintable("\n");

                //update status class with new data
                status->loadData(message);

                //update gui
                updateStatusDisplay();

                break;

            case EVENT:

                qDebug() <<  "Message id: event update" << qPrintable("\n");

                //add new event to event ll
                events->loadEventData(message);

                // update log file
                events->appendToLogfile(autosaveLogFile, events->lastEventNode);

                // update GUI elements
                updateEventsOutput(events->lastEventNode);

                break;

            case ERROR:

                // status
                qDebug() <<  "Message id: error update" << qPrintable("\n");

                //add new error to error ll
                events->loadErrorData( message );

                // update log file
                events->appendToLogfile(autosaveLogFile, events->lastErrorNode);

                //update gui elements
                updateEventsOutput(events->lastErrorNode);

                //update the cleared error selection box in dev tools
                //(this can be removed when dev page is removed)
                update_non_cleared_error_selection();

                break;

            case ELECTRICAL:

                qDebug() <<  "Message id: electrical" << qPrintable("\n");

                // clear electrical ll
                electricalObject->freeLL();

                //load new data into electrical ll
                electricalObject->loadElecDump(message);

                //get head node into wkg ptr
                wkgElecPtr = electricalObject->headNode;

                // loop through each electrical data box
                for (boxIndex = 1; boxIndex <= MAX_ELECTRICAL_COMPONENTS; boxIndex++)
                {
                    // get the current box name
                    QString widgetName = "box" + QString::number(boxIndex) + "_widget";

                    // get the current box based off name
                    QWidget *widget = findChild<QWidget *>(widgetName);

                    // check if widget exists, and hide it
                    if(widget) widget->hide();
                }

                // loop through each electrical data box
                for (boxIndex = 1; boxIndex <= MAX_ELECTRICAL_COMPONENTS; boxIndex++)
                {
                    // get the current box name
                    QString widgetName = "box" + QString::number(boxIndex) + "_widget";

                    // get the names of the labels for this box
                    QString labelName = "box" + QString::number(boxIndex) + "_label";
                    QString statsName = "box" + QString::number(boxIndex) + "_stats";

                    // get the current box based off name
                    QWidget *widget = findChild<QWidget *>(widgetName);

                    // find the label objects with findChild
                    QLabel *boxLabel = findChild<QLabel *>(labelName);
                    QTextEdit *boxStats = findChild<QTextEdit *>(statsName);

                    // check if the current electrical node exists
                    if (wkgElecPtr != nullptr)
                    {
                        // update label with name if it exists
                        if (boxLabel) boxLabel->setText(" " + wkgElecPtr->name);

                        // update stats with voltage and amps if it exists
                        if (boxStats) boxStats->setPlainText("Voltage: " + QString::number(wkgElecPtr->voltage) +
                                              '\n' + "Amps: " + QString::number(wkgElecPtr->amps));

                        // check if the box exists, and show it
                        if(widget) widget->show();

                        // move to next electrical node
                        wkgElecPtr = wkgElecPtr->nextNode;
                    }
                    // else, there are no more electrical nodes
                    else
                    {
                        // break once we are done
                        break;
                    }
                }

                // break if we have reached the max number of electrical boxes to fill
                break;

            case EVENT_DUMP:

                qDebug() <<  "Message id: event dump" << qPrintable("\n");

                // load all events to event linked list
                events->loadEventDump(message);

                // create log file
                events->outputToLogFile( autosaveLogFile );

                //new auto save file created, enforce auto save limit
                enforceAutoSaveLimit();

                //refresh the events output with dumped event data
                refreshEventsOutput();

                break;

            case ERROR_DUMP:

                qDebug() <<  "Message id: error dump" << qPrintable("\n");

                // load all errors to error linked list
                events->loadErrorDump(message);

                // create log file
                events->outputToLogFile( autosaveLogFile );

                //new auto save file created, enforce auto save limit
                enforceAutoSaveLimit();

                //refresh the events output with dumped error data
                refreshEventsOutput();

                break;

            case CLEAR_ERROR:
                qDebug() << "Message id: clear error " << message << qPrintable("\n");

                //update cleared status of error with given id
                events->clearError(message.left(message.indexOf(DELIMETER)).toInt());

                //update the cleared error selection box in dev tools (can be removed when dev page is removed)
                update_non_cleared_error_selection();

                //refresh the events output with newly cleared error
                refreshEventsOutput();

                break;

            case BEGIN:

                //load controller crc and version
                status->loadVersionData(message);

                //set connection status to connected and update related objects
                updateConnectionStatus(true);

                // update controller version and crc on gui
                ui->controllerLabel->setText("Controller Version: " + status->version);
                ui->crcLabel->setText("CRC: " + status->crc);

                //check for empty logfile location and sets to default
                setup_logfile_location();

                qDebug() << "Begin signal received, handshake complete";

                break;

            case CLOSING_CONNECTION:

                //set connection status false and update related objects
                updateConnectionStatus(false);

                qDebug() << "Disconnect message received from Controller";

                break;

            default:
                qDebug() << "ERROR: message from controller is not recognized";

                break;
            }

            // update the timestamp of last received message
            timeLastReceived = QDateTime::currentDateTime();
        }
        //invalid message id detected
        else
        {
            qDebug() << "Unrecognized serial message received : " << message;
        }
    }
}

//scans for available serial ports and adds them to csim port selection box
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

//scans for available serial ports and adds them to ddm port selection box
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

//updates the dev page non cleared error selection box
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
    ui->load_events_from_logfile->setDisabled(true);
}

//makes all settings in connection settings editable (call when ddm connection
//ends)
void MainWindow::enableConnectionChanges()
{
    ui->ddm_port_selection->setEnabled(true);
    ui->baud_rate_selection->setEnabled(true);
    ui->data_bits_selection->setEnabled(true);
    ui->parity_selection->setEnabled(true);
    ui->stop_bit_selection->setEnabled(true);
    ui->flow_control_selection->setEnabled(true);
    ui->load_events_from_logfile->setEnabled(true);
}

//support function, outputs usersettings values to qdebug
void MainWindow::displaySavedSettings()
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
    qDebug() << "flowControl:" << userSettings.value("flowControl").toString();
    qDebug() << "logfile location: " << userSettings.value("logfileLocation").toString();
    qDebug() << "Colored Event Output: " << userSettings.value("coloredEventOutput").toBool();
    qDebug() << "Auto Save Limit: " << userSettings.value("autoSaveLimit").toInt() << Qt::endl;
}

//checks if user has setup a custom log file directory, if not, the default directory is selected
//the auto save log file for this session will be stored in the directory chosen by this
//function
void MainWindow::setup_logfile_location()
{
    //check if user has set a custom log file output directory
    if ( !userSettings.value("logfileLocation").toString().isEmpty() )
    {
        //initialize the logfile into this directory
        autosaveLogFile = userSettings.value("logfileLocation").toString();
    }
    //otherwise use default directory
    else
    {
        //use the path of the exe and add a "Log Files" directory
        autosaveLogFile = QCoreApplication::applicationDirPath() + "/" + INITIAL_LOGFILE_LOCATION;
    }

    //initialize a directory object in selected location
    QDir dir(autosaveLogFile);

    //check if directory doesnt exist
    if(!dir.exists())
    {
        //attempt to create the directory
        if(!dir.mkpath(autosaveLogFile))
        {
            qDebug() << "Failed to create logfile folder on startup" << autosaveLogFile;
            return;
        }
    }

    //if there are more auto saves than the current limit, delete the extras (oldest first)
    enforceAutoSaveLimit();

    // set unique logfile name for this session
    qint64 secsSinceEpoch = QDateTime::currentSecsSinceEpoch();
    autosaveLogFile += QString::number(secsSinceEpoch) + "-logfile-A.txt";

    qDebug() << "Auto Save log file for this session: " << autosaveLogFile;
}

//checks if the number of auto saved log files is greater than the user
//set max value. Deletes the oldest until auto save limit is enforced
void MainWindow::enforceAutoSaveLimit()
{
    QString path;

    //check if user has set a custom log file output directory
    if ( !userSettings.value("logfileLocation").toString().isEmpty() )
    {
        //initialize the logfile into this directory
        path = userSettings.value("logfileLocation").toString();
    }
    //otherwise use default directory
    else
    {
        //use the path of the exe and add a "Log Files" directory
        path = QCoreApplication::applicationDirPath() + "/" + INITIAL_LOGFILE_LOCATION;
    }

    // Set a filter to display only files
    QDir dir(path);
    dir.setFilter(QDir::Files);

    // Get list of files in directory
    QStringList fileList = dir.entryList();

    // Create a new list to add auto save file names to
    QStringList autoSaveFileList;

    // Loop through names of files in directory
    for (const QString &fileName : fileList)
    {
        // Check if this is an auto save file
        if (fileName.endsWith("logfile-A.txt"))
        {
            // Append to the new list
            autoSaveFileList.append(fileName);
        }
    }

    // Delete the oldest file if the number of auto saved files is above the limit
    while (autoSaveFileList.size() > autoSaveLimit)
    {
        QString oldestFilePath;
        QDateTime oldestCreationTime;

        // Find the oldest file in the list
        for (const QString &autoSaveFile : autoSaveFileList)
        {
            // Get the path of the file
            QString filePath = dir.filePath(autoSaveFile);
            // Get the creation time of the file
            QFileInfo fileInfo(filePath);
            QDateTime creationTime = fileInfo.lastModified();

            // Check if this is the oldest file so far
            if (oldestCreationTime.isNull() || creationTime < oldestCreationTime)
            {
                oldestCreationTime = creationTime;
                oldestFilePath = filePath;
            }
        }

        // Remove the oldest file
        if (!QFile::remove(oldestFilePath))
        {
            qDebug() << "Failed to delete file: " << oldestFilePath;
            return;
        }
        else
        {
            qDebug() << "An autosave file was deleted";
        }

        // Remove the oldest file name from the list
        autoSaveFileList.removeOne(QFileInfo(oldestFilePath).fileName());
    }
}

//Checks if registry values exist for all settings in userSettings. If so settings
//are loaded into local variables. If not, initial settings are taken from constants.h
//and loaded into registry and local variables. Selection boxes for connection settings
//are loaded with Qt serial options.
void MainWindow::setupSettings()
{
    int i;

    //Connection Setttings

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


    //Misc. Settings


    // Check if the setting exists and is valid
    if (!userSettings.contains("coloredEventOutput") || !userSettings.value("coloredEventOutput").isValid()) {
        // If it doesn't exist or is not valid, set the default value
        userSettings.setValue("coloredEventOutput", INITIAL_COLORED_EVENTS_OUTPUT);
    }

    //set session variable based on setting
    coloredEventOutput = userSettings.value("coloredEventOutput").toBool();

    //set gui display to match
    ui->colored_events_output->setChecked(coloredEventOutput);

    // Check if the setting exists and is valid
    if (!userSettings.contains("autoSaveLimit") || !userSettings.value("autoSaveLimit").isValid()) {
        // If it doesn't exist or is not valid, set the default value
        userSettings.setValue("autoSaveLimit", INITIAL_AUTO_SAVE_LIMIT);
    }

    //set session variable based on setting
    autoSaveLimit = userSettings.value("autoSaveLimit").toInt();

    //update gui to match
    ui->auto_save_limit->setValue(autoSaveLimit);

    // Display user settings
    displaySavedSettings();

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

//displays the current values of the status class onto the gui status page
void MainWindow::updateStatusDisplay()
{
    resetFiringMode();

    //update font color of the active firing mode
    if(status->firingMode == SAFE)
    {
        ui->safeLabel->setStyleSheet("color: #FF7518;font: 20pt Segoe UI;");
    }
    else if(status->firingMode == SINGLE)
    {
        ui->singleLabel->setStyleSheet("color: #FF7518;font: 20pt Segoe UI;");
    }
    else if(status->firingMode == BURST)
    {
        ui->burstLabel->setStyleSheet("color: #FF7518;font: 20pt Segoe UI;");
    }
    else
    {
        ui->automaticLabel->setStyleSheet("color: #FF7518;font: 20pt Segoe UI;");
    }

    //update feed position
    ui->feedPosition->setValue(status->feedPosition);

    //update trigger 1 light
    switch (status->trigger1)
    {
    case ENGAGED:
        ui->trigger1->setPixmap(GREEN_LIGHT);

        break;

    case DISENGAGED:
        ui->trigger1->setPixmap(RED_LIGHT);

        break;

    default:
        ui->trigger1->setPixmap(BLANK_LIGHT);
    }


    //update trigger 2 light
    switch (status->trigger2)
    {
        case ENGAGED:
            ui->trigger2->setPixmap(GREEN_LIGHT);

            break;

        case DISENGAGED:
            ui->trigger2->setPixmap(RED_LIGHT);

            break;

        default:
            ui->trigger2->setPixmap(BLANK_LIGHT);
    }
}

// method updates the running elapsed controller time
void MainWindow::updateElapsedTime()
{
    // get the current timestamp
    QString timestamp = ui->elapsedTime->toPlainText();

    // extract the time, remove "ELapsed Time: "
    timestamp = timestamp.mid(14); // assuming this will always be in position 14, it should never change

    // split up the day and time components
    // this is required because a QTime object does not support days (more than 24hrs)
    QStringList timeParts = timestamp.split(":");
    int days = timeParts.takeFirst().toInt(); // removes the days from the list as well

    // convert to QTime and add one second
    QTime currentTime = QTime::fromString(timeParts.join(":"), "HH:mm:ss");
    currentTime = currentTime.addSecs(1);

    // check if the time exceeds 23:59:59
    // when it does, the entire timestamp will be reset since QTime can not exceed 24 hours
    if (currentTime.hour() == 0 && currentTime.minute() == 0 && currentTime.second() == 0)
    {
        // increment number of days
        days++;
    }

    // update the GUI
    status->elapsedControllerTime = QString::number(days) + ":" + currentTime.toString("HH:mm:ss");
    ui->elapsedTime->setText("Elapsed Time: " + QString::number(days) + ":" + currentTime.toString("HH:mm:ss"));
    ui->elapsedTime->setAlignment(Qt::AlignRight);
}

// method updates the elapsed time since last message received to DDM
void MainWindow::updateTimeSinceLastMessage()
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
// To string methods for QSerialPortEnumeratedValues
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

//writes empty line to qdebug
void MainWindow::logEmptyLine()
{
    //revert to standard output format
    qSetMessagePattern("%{message}");

    //log empty line
    qDebug();

    //enable custom message format
    qSetMessagePattern(QDEBUG_OUTPUT_FORMAT);
}

void MainWindow::resetFiringMode()
{
    ui->automaticLabel->setStyleSheet("color: rgb(255, 255, 255);font: 20pt Segoe UI;");
    ui->burstLabel->setStyleSheet("color: rgb(255, 255, 255);font: 20pt Segoe UI;");
    ui->safeLabel->setStyleSheet("color: rgb(255, 255, 255);font: 20pt Segoe UI;");
    ui->singleLabel->setStyleSheet("color: rgb(255, 255, 255);font: 20pt Segoe UI;");
}

//overloaded function to add simplicity when possible
void MainWindow::updateEventsOutput(EventNode *event)
{
    updateEventsOutput(events->nodeToString(event), event->error, event->cleared);
}


//updates gui with given message, dynamically colors output based on the type of outString
//accounts for filtering settings and only renders the outString if it is being filtered for
// by the user
void MainWindow::updateEventsOutput(QString outString, bool error, bool cleared)
{
    QTextDocument document;
    QString richText;

    //check if we have an event as input and check if filter allows printing events
    if (!error )
    {
        if (eventFilter == EVENTS || eventFilter == ALL)
        {
           //change output text color to white
           richText = "<p style='color: #FFFFFF; font-size: 16px'>"+ outString + "</p>";

           //activate html for the output
           document.setHtml(richText);

           //append styled string to the events output
           ui->events_output->append(document.toHtml());
        }
    }
    //otherwise check for cleared error and if filter allows printing cleared errors
    else if (cleared)
    {
        if (eventFilter == ALL || eventFilter == ERRORS || eventFilter == CLEARED_ERRORS)
        {
            if (coloredEventOutput)
            {
                //change output text color to green
                richText = "<p style='color: #14AE5C; font-size: 16px'>"+ outString + "</p>";
            }
            else
            {
                //change output text color to white
                richText = "<p style='color: #FFFFFF; font-size: 16px'>"+ outString + "</p>";
            }

            //activate html for the output
            document.setHtml(richText);

            //append styled string to the events output
            ui->events_output->append(document.toHtml());
        }
    }
    //otherwise this is a non-cleared error check if filtering allows printing non-cleared errors
    else if (eventFilter == ALL || eventFilter == NON_CLEARED_ERRORS || eventFilter == ERRORS)
    {
        if (coloredEventOutput)
        {
            //change output text color to red
            richText = "<p style='color: #FE1C1C; font-size: 16px'>"+ outString + "</p>";
        }
        else
        {
            //change output text color to white
            richText = "<p style='color: #FFFFFF; font-size: 16px'>"+ outString + "</p>";
        }

       //activate html for the output
       document.setHtml(richText);

       //append styled string to the events output
       ui->events_output->append(document.toHtml());
    }

    // update total events gui
    ui->TotalEventsOutput->setText(QString::number(events->totalEvents));
    ui->TotalEventsOutput->setAlignment(Qt::AlignCenter);
    ui->statusEventOutput->setText(QString::number(events->totalEvents));
    ui->statusEventOutput->setAlignment(Qt::AlignCenter);

    if (!error) return;

    // update total errors gui
    ui->TotalErrorsOutput->setText(QString::number(events->totalErrors));
    ui->TotalErrorsOutput->setAlignment(Qt::AlignCenter);
    ui->statusErrorOutput->setText(QString::number(events->totalErrors));
    ui->statusErrorOutput->setAlignment(Qt::AlignCenter);

    if ( cleared )
    {
        // update cleared errors gui
        ui->ClearedErrorsOutput->setText(QString::number(events->totalCleared));
        ui->ClearedErrorsOutput->setAlignment(Qt::AlignCenter);
    }
    else
    {
        // update active errors gui
        ui->ActiveErrorsOutput->setText(QString::number(events->totalErrors - events->totalCleared));
        ui->ActiveErrorsOutput->setAlignment(Qt::AlignCenter);
    }
}

//clears events tab gui element, then repopulates it with current event data
void MainWindow::refreshEventsOutput()
{
    // reset gui element
    ui->events_output->clear();

    //init vars
    EventNode *wkgErrPtr = events->headErrorNode;
    EventNode *wkgEventPtr = events->headEventNode;
    EventNode *nextPrintPtr;
    bool printErr;

    //loop through all events and errors
    while(wkgErrPtr != nullptr || wkgEventPtr != nullptr)
    {
        // get next to print
        nextPrintPtr = events->getNextNodeToPrint(wkgEventPtr, wkgErrPtr, printErr);

        //update events output if filter allows
        updateEventsOutput(nextPrintPtr);
    }
}
