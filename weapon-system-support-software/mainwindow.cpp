#include "mainwindow.h"
#include <QTextDocument>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent)

    //initialize imbedded classes/vars
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    ddmCon(nullptr),
    status(new Status()),
    electricalData(new electrical()),
    events(nullptr),

    //this determines what will be shown on the events page
    eventFilter(ALL),

    //timer is used to repeatedly transmit handshake signals
    handshakeTimer( new QTimer(this) ),

    //timer is used to hide notifications some time after they are displayed
    notificationTimer(new QTimer(this)),

    // timer is used to update last message received time
    lastMessageTimer( new QTimer(this) ),

    // timer is used to update controller running time
    runningControllerTimer( new QTimer(this) ),

    //timer for preventing spam of handshake button
    handshakeCooldownTimer(new QTimer(this)),

    //init user settings to our organization and project
    userSettings("Team Controller", "WSSS"),

    //init to false until connection page is setup
    allowSettingChanges(false),

    //Load graphical resources
    BLANK_LIGHT(":/resources/Images/blankButton.png"),

    RED_LIGHT(":/resources/Images/redButton.png"),

    GREEN_LIGHT(":/resources/Images/greenButton.png"),

    ORANGE_LIGHT(":/resources/Images/orangeButton.png")
{
    //init gui
    ui->setupUi(this);

    //setup user settings and init settings related gui elements
    setupSettings();

    //make new events class for this session
    events = new Events(userSettings.value("RAMClearing").toBool(), userSettings.value("maxDataNodes").toInt());

    //setup signal and slot to notify user when ram is cleared from events
    //this signal connects to a lambda function so we can call more than 1 function
    //using 1 signal-slot connection
    connect(events, &Events::RAMCleared, this, [=]() {
        notifyUser("RAM Cleared",
                   "Events and errors were removed from RAM to improve performance. "
                   "They are still being tracked by counters and our log file. You can also load them"
                   "back into the GUI after this session ends.", false);

        //show truncated label on the events page to tell user not all nodes are displayed
        ui->truncated_label->setVisible(true);

        //get rid of outdated display
        refreshEventsOutput();
    });

    //will be disabled until RAM is cleared
    ui->truncated_label->setVisible(false);

    //if dev mode is active, init CSim
    #if DEV_MODE

        //set output settings for qDebug
        qSetMessagePattern(QDEBUG_OUTPUT_FORMAT);

        #if GENERAL_DEBUG
        qDebug() << "Dev mode active";
        #endif

        //get csimPortName from port selection
        csimPortName = ui->csim_port_selection->currentText();

        //init csim class, assign serial port
        csimHandle = new CSim(nullptr, csimPortName);

        //init generation interval
        csimHandle->generationInterval = CSIM_GENERATION_INTERVAL;
        ui->csim_generation_interval_selection->setValue(csimHandle->generationInterval);

        //CSIM control slots ==============================================================

        //connect custom transmission requests from ddm to csims execution slot
        connect(this, &MainWindow::transmissionRequest, csimHandle, &CSim::completeTransmissionRequest);

        //connect custom clear error requests from ddm to csims execution slot
        connect(this, &MainWindow::clearErrorRequest, csimHandle, &CSim::clearError);

        //connect output session string to ddm output session string slot
        connect(this, &MainWindow::outputMessagesSentRequest, csimHandle, &CSim::outputMessagesSent);
        //=================================================================================
    //dev mode is not active, hide dev page button
    #else
        ui->DevPageButton->setVisible(false);
    #endif

    //Timers ===============================================================================

    //set handshake timer interval
    handshakeTimer->setInterval(HANDSHAKE_INTERVAL);

    //connect handshake function to a timer. After each interval handshake will be called.
    //this is necessary to prevent the gui from freezing. signals stop when timer is stoped
    connect(handshakeTimer, &QTimer::timeout, this, &MainWindow::handshake);

    //prevents spamming of the handshake button which may cause crash
    handshakeCooldownTimer->setInterval(HANDSHAKE_COOLDOWN_TIME);
    handshakeCooldownTimer->setSingleShot(true);

    //connect clear notification process to the notification timer. If run, the process
    //will trigger after the notification timeout
    connect(notificationTimer, &QTimer::timeout, this, [this]() {
        ui->notificationPopUp->setStyleSheet(INVISIBLE);
        ui->notificationPopUp->clear();
    });

    // connect update elapsed time function to a timer
    lastMessageTimer->setInterval(ONE_SECOND);
    connect(lastMessageTimer, &QTimer::timeout, this, &MainWindow::updateTimeSinceLastMessage);

    //connect running controller timer to slot
    runningControllerTimer->setInterval(ONE_SECOND);
    connect(runningControllerTimer, &QTimer::timeout, this, &MainWindow::updateElapsedTime);
    //======================================================================================

    //init trigger to grey buttons until updated by serial status updates
    ui->trigger1->setPixmap(BLANK_LIGHT);
    ui->trigger2->setPixmap(BLANK_LIGHT);

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
        //if(widget) widget->hide();
    }
}

//destructor
MainWindow::~MainWindow()
{
    //call destructors for classes declared in main window
    delete ui;
    delete ddmCon;
    delete status;
    delete events;
    delete handshakeTimer;
    delete notificationTimer;
    delete lastMessageTimer;
    delete runningControllerTimer;
    delete handshakeCooldownTimer;
    delete electricalData;
    #if DEV_MODE
        delete csimHandle;
    #endif
}

//sets connection status, updates gui and timers
void MainWindow::updateConnectionStatus(bool connectionStatus)
{
    if (ddmCon == nullptr)
    {
        return;
    }

    //update connection status
    ddmCon->connected = connectionStatus;

    //check if we are connected
    if (ddmCon->connected)
    {

        //disable changes to connection related settings
        disableConnectionChanges();

        //stop handshake protocols
        handshakeTimer->stop();

        ui->truncated_label->setVisible(false);

        // check if controller timer is not running
        if(!runningControllerTimer->isActive())
        {
            // start it
            runningControllerTimer->start();

            // update elapsed time
            ui->elapsed_time_label->setText("Elapsed Time: ");
            ui->elapsedTime->setText( status->elapsedControllerTime.toString(TIME_FORMAT));
        }

        //free old electrical data if any exists
        electricalData->freeLL();

        //start last message timer
        timeLastReceived = QDateTime::currentDateTime();
        ui->DDM_timer_label->setText("Time Since Last Message: ");
        ui->DDMTimer->setText("00:00:00");
        lastMessageTimer->start();

        // update ui
        ui->handshake_button->setText("Disconnect");
        ui->handshake_button->setStyleSheet(CONNECTED_STYLE);
        ui->connectionLabel->setText("Connected ");
        ui->connectionStatus->setPixmap(GREEN_LIGHT);

        //clear events and full clear the class output box
        ui->events_output->clear();

        events->freeLinkedLists(true);

        //reset event counters
        ui->TotalEventsOutput->setText("0");
        ui->statusEventOutput->setText("0");

        ui->TotalErrorsOutput->setText("0");
        ui->statusErrorOutput->setText("0");

        ui->ClearedErrorsOutput->setText("0");

        ui->ActiveErrorsOutput->setText("0");
    }
    //otherwise we are disconnected
    else
    {
        //stop timers if they are running
        runningControllerTimer->stop();
        lastMessageTimer->stop();
        handshakeTimer->stop();

        //clear time since last message
        ui->DDMTimer->clear();
        ui->DDM_timer_label->clear();

        //enable changes to connection related settings
        enableConnectionChanges();

        //refreshes connection button/displays
        ui->handshake_button->setText("Connect");
        ui->handshake_button->setStyleSheet(DISCONNECTED_STYLE);
        ui->connectionStatus->setPixmap(RED_LIGHT);
        ui->connectionLabel->setText("Disconnected ");

        //output session stats
        if (autosaveLogFile != "")
        {
            notifyUser("Session statistics ready", getSessionStatistics(), false);
        }

        //if advanced log file is enabled, add details to log file
        if (advancedLogFile)
        {
            logAdvancedDetails(ELECTRICAL);
            logAdvancedDetails(CLOSING_CONNECTION);
        }
    }
}

//when ddm port is selected, create connection class to work with that port.
//apply serial settings from settings page
void MainWindow::createDDMCon()
{
    //close current connection
    if (ddmCon != nullptr)
    {
        //notify user of closed connection class
        notifyUser(ddmPortName + " closed",  false);
        delete ddmCon;
        ddmCon = nullptr;
    }

    //open new connection
    ddmCon = new Connection(ui->ddm_port_selection->currentText(),
                            fromStringBaudRate(ui->baud_rate_selection->currentText()),
                            fromStringDataBits(ui->data_bits_selection->currentText()),
                            fromStringParity(ui->parity_selection->currentText()),
                            fromStringStopBits(ui->stop_bit_selection->currentText()),
                            fromStringFlowControl(ui->flow_control_selection->currentText()));

    if (ddmCon == nullptr)
    {
        notifyUser("Failed to open " + ui->ddm_port_selection->currentText(), true);
    }
    //check for failure to open
    else if (!ddmCon->serialPort.isOpen())
    {
        delete ddmCon;
        ddmCon = nullptr;

        //generate notification
        notifyUser("Failed to open " + ui->ddm_port_selection->currentText(), true);
    }
    else
    {
        //set up signal and slot (when a message is sent to DDMs serial port, the readyRead signal is emitted and
        //readSerialData() is called)
        connect(&ddmCon->serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

        qDebug() << "GUI is now listening to port " << ddmCon->portName;

        notifyUser(ui->ddm_port_selection->currentText() + " opened", false);
    }
}

//initial synchronization between controller and ddm
void MainWindow::handshake()
{
    if (ddmCon == nullptr)
    {
        notifyUser("Handshake failed", "Connection class is not declared", true);
        return;
    }

    ddmCon->sendHandshakeMsg();
}

//this function is called as a result of the readyRead signal being emmited by a connected serial port
//in other words, this function is called whenever ddm port receives a new message
void MainWindow::readSerialData()
{
    if (ddmCon == nullptr)
    {
        notifyUser("Could not read serial data", "Connection class is not declared", true);
        return;
    }

    //read lines until all data in buffer is processed
    while (ddmCon->checkForValidMessage())
    {
        // declare variables
        SerialMessageIdentifier messageId;
        int errorId;
        int result;

        //get serialized string from port
        QByteArray serializedMessage = ddmCon->serialPort.readLine();

        //deserialize string
        QString message = QString::fromUtf8(serializedMessage);

        #if DEV_MODE && SERIAL_COMM_DEBUG
        qDebug() << "message: " << message;
        #endif
        #if DEV_MODE
        //update gui with new message
        ui->stdout_label->setText(message);
        #endif

        //check if message id is present and followed by a comma
        if (message[0].isDigit() && message[1] == DELIMETER)
        {
            //extract message id
            messageId = static_cast<SerialMessageIdentifier>(QString(message[0]).toInt());

            //remove message id from message (id has len=1 and delimeter has len=1 so 2 total)
            message = message.mid(2);

            //ensure we are in an active connection or attempting to connect
            if(!(handshakeTimer->isActive() || ddmCon->connected))
            {
                qDebug() << "Error: readSerialData unexpected communication from controller";
                notifyUser("Unexpected communication from controller", message, true);
                ddmCon->sendDisconnectMsg();
                return;
            }
            //ensure we are only getting begin message during handshake
            else if (handshakeTimer->isActive() && messageId != BEGIN)
            {
                notifyUser("Invalid handshake is occurring", message, true);
                ddmCon->sendDisconnectMsg();
                return;
            }

            //determine what kind of message this is
            switch ( messageId )
            {
            case STATUS:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() <<  "Message id: status update" << qPrintable("\n");
                #endif

                //update status class with new data
                if (!status->loadData(message))
                {
                    notifyUser("Invalid status message received", message, true);
                }

                //update gui
                updateStatusDisplay();

                //if advanced log file is enabled, log the status
                if (advancedLogFile) logAdvancedDetails(STATUS);

                break;

            case EVENT:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() <<  "Message id: event update" << qPrintable("\n");
                #endif

                //add new event to event ll, check for fail
                if (!events->loadEventData(message))
                {
                    notifyUser("Invalid event message received", message, true);
                }
                //otherwise success
                else
                {
                    // update log file
                    events->appendToLogfile(autosaveLogFile, events->lastEventNode);

                    // update GUI elements
                    updateEventsOutput(events->lastEventNode);
                }

                break;

            case ERROR:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() <<  "Message id: error update" << qPrintable("\n");
                #endif

                //add new error to error ll, check for fail
                if (!events->loadErrorData( message ))
                {
                    notifyUser("Invalid error message received", message, true);
                }
                //otherwise success
                else
                {
                    // update log file
                    events->appendToLogfile(autosaveLogFile, events->lastErrorNode);

                    //update gui elements
                    updateEventsOutput(events->lastErrorNode);

                    #if DEV_MODE
                        //update the cleared error selection box in dev tools
                        //(this can be removed when dev page is removed)
                        update_non_cleared_error_selection();
                    #endif
                }

                break;

            case ELECTRICAL:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() <<  "Message id: electrical" << qPrintable("\n");
                #endif

                //load new data into electrical ll, notify if fail
                if (!electricalData->loadElecDump(message))
                {
                    notifyUser("Invalid electrical dump received", message, true);
                }
                //otherwise success
                else
                {
                    //dynamically generate nodes on electrical page
                    renderElectricalPage();
                }

                break;

            case EVENT_DUMP:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() <<  "Message id: event dump" << qPrintable("\n");
                #endif

                // load all events to event linked list, notify if fail
                if (!events->loadEventDump(message))
                {
                    notifyUser("Invalid event dump received", message, true);
                }

                // create log file
                if (!events->outputToLogFile( autosaveLogFile, advancedLogFile ))
                {
                    notifyUser("Failed to open logfile","Manual download could save the data.", true);
                }

                //new auto save file created, enforce auto save limit
                enforceAutoSaveLimit();

                //refresh the events output with dumped event data
                refreshEventsOutput();

                break;

            case ERROR_DUMP:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() <<  "Message id: error dump" << qPrintable("\n");
                #endif

                // load all errors to error linked list, notify if fail
                if (!events->loadErrorDump(message))
                {
                    notifyUser("Invalid error dump received", message, true);
                }

                // create log file
                if (!events->outputToLogFile( autosaveLogFile, advancedLogFile ))
                {
                    notifyUser("Failed to open logfile","Manual download could save the data.", true);
                }

                //new auto save file created, enforce auto save limit
                enforceAutoSaveLimit();

                //refresh the events output with dumped error data
                refreshEventsOutput();

                break;

            case CLEAR_ERROR:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() << "Message id: clear error " << message << qPrintable("\n");
                #endif

                //extract error id from message
                errorId = message.left(message.indexOf(DELIMETER)).trimmed().toInt();

                //attempt clear
                result = events->clearError(errorId, autosaveLogFile );

                //check for fail (here failed to clear from ll indicates RAM dump)
                if (result != SUCCESS && result != FAILED_TO_CLEAR_FROM_LL)
                {
                    //notify user of fail type
                    if (result == FAILED_TO_CLEAR)
                    {
                        notifyUser("Failed to clear error", message, true);
                    }
                    else if (result == FAILED_TO_CLEAR_FROM_LOGFILE)
                    {
                        notifyUser("Error "+ QString::number(errorId) + " can't be cleared from logfile", true);
                    }
                }
                //otherwise success
                else
                {
                    //attempt to clear in events output
                    if (result == SUCCESS) clearErrorFromEventsOutput(errorId);

                    //update counters
                    ui->ClearedErrorsOutput->setText(QString::number(events->totalClearedErrors));
                    ui->statusClearedErrors->setText(QString::number(events->totalClearedErrors));

                    if (notifyOnErrorCleared) notifyUser("Error " + message.left(message.indexOf(DELIMETER)) + " Cleared", false);
                }

                #if DEV_MODE
                //update the cleared error selection box in dev tools (can be removed when dev page is removed)
                update_non_cleared_error_selection();
                #endif

                break;

            case BEGIN:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() << "Message id: begin " << message << qPrintable("\n");
                #endif

                //load controller crc and version, check for fail
                if (!status->loadVersionData(message))
                {
                    //report
                    notifyUser("Invalid 'begin' message received", message, true);

                    //end connection attempt
                    ddmCon->sendDisconnectMsg();
                }
                //otherwise success
                else
                {
                    notifyUser("Handshake complete", "Session start", false);

                    //set connection status to connected and update related objects
                    updateConnectionStatus(true);

                    // update controller version and crc on gui
                    ui->controllerLabel->setText("Controller Version: " + status->version);
                    ui->crcLabel->setText("CRC: " + status->crc);

                    //init logfile location (user setting)
                    setup_logfile_location();

                    #if DEV_MODE && SERIAL_COMM_DEBUG
                    qDebug() << "Begin signal received, handshake complete";
                    #endif
                }

                break;

            case CLOSING_CONNECTION:

                #if DEV_MODE && SERIAL_COMM_DEBUG
                qDebug() << "Disconnect message received from Controller";
                #endif

                notifyUser("Controller disconnected", "Session end", false);

                //set connection status false and update related objects
                updateConnectionStatus(false);

                break;

            default:
                qDebug() << "ERROR: readSerialData message from controller is not recognized";

                //report
                notifyUser("Unrecognized message received", message, true);

                break;
            }

            // update the timestamp of last received message
            timeLastReceived = QDateTime::currentDateTime();
        }
        //invalid message id detected
        else
        {
            qDebug() << "Error: readSerialData Unrecognized serial message received : " << message;
            notifyUser("Unrecognized serial message received", message, true);
        }
    }
}

//scans for available serial ports and adds them to ddm port selection box
void MainWindow::setup_ddm_port_selection(int index)
{
    // Check and set initial value for "portName"
    if (userSettings.value("portName").toString().isEmpty())
        userSettings.setValue("portName", INITIAL_DDM_PORT);

    //this is checked within the current index changed slot of ddm combo box
    //start false prevents an accidental selection of the first
    //index added.
    allowSettingChanges = false;

    //clear any current selections
    ui->ddm_port_selection->clear();

    // Fetch available serial ports and add their names to the combo box
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString portName = info.portName();
        ui->ddm_port_selection->addItem(portName);

        // Check if the current port name matches the one declared in settings
        if (portName == userSettings.value("portName").toString())
        {
            //allow us to select this port
            allowSettingChanges = true;

            // If a match is found, set the current index of the combo box
            ui->ddm_port_selection->setCurrentIndex(ui->ddm_port_selection->count() - 1);
        }
    }
    //initialization finished, allow port selection
    allowSettingChanges = true;
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
    ui->restore_Button->setDisabled(true);
    ui->refresh_serial_port_selections->setVisible(false);
    ui->setLogfileFolder->setDisabled(true);
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
    ui->restore_Button->setEnabled(true);
    ui->refresh_serial_port_selections->setVisible(true);
    ui->setLogfileFolder->setEnabled(true);
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
            qDebug() << "Error: setup_logfile_location Failed to create logfile folder on startup: " << autosaveLogFile;
            return;
        }
    }

    //if there are more auto saves than the current limit, delete the extras (oldest first)
    enforceAutoSaveLimit();

    // set unique logfile name for this session
    qint64 secsSinceEpoch = QDateTime::currentSecsSinceEpoch();
    autosaveLogFile += QString::number(secsSinceEpoch) + "-logfile-A.txt";

    notifyUser("Auto save log set", autosaveLogFile, false);
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
            qDebug() << "Error: enforceAutoSaveLimit failed to delete file: " << oldestFilePath;
            return;
        }
        else
        {
            #if DEV_MODE && GENERAL_DEBUG
            qDebug() << "An autosave file was deleted: " << oldestFilePath;
            #endif
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
    // Check if colored event setting does not exist
    if (!userSettings.contains("coloredEventOutput") || !userSettings.value("coloredEventOutput").isValid()) {
        // set the default value
        userSettings.setValue("coloredEventOutput", INITIAL_COLORED_EVENTS_OUTPUT);
    }

    //set session variable based on setting
    coloredEventOutput = userSettings.value("coloredEventOutput").toBool();

    //set gui display to match
    ui->colored_events_output->setChecked(coloredEventOutput);

    //==============================================================

    // Check if advanced log file setting does not exist
    if (!userSettings.contains("advancedLogFile") || !userSettings.value("advancedLogFile").isValid()) {
        // set the default value
        userSettings.setValue("advancedLogFile", INITIAL_ADVANCED_LOG_FILE);
    }

    //set session variable based on setting
    advancedLogFile = userSettings.value("advancedLogFile").toBool();

    //set gui display to match
    ui->advanced_log_file->setChecked(advancedLogFile);

    //==============================================================

    // Check if the cleared error notification setting does not exist
    if (!userSettings.contains("notifyOnErrorCleared") || !userSettings.value("notifyOnErrorCleared").isValid()) {
        // set the default value
        userSettings.setValue("notifyOnErrorCleared", INITIAL_NOTIFY_ON_ERROR_CLEARED);
    }

    //set session variable based on setting
    notifyOnErrorCleared = userSettings.value("notifyOnErrorCleared").toBool();

    //update gui to match
    ui->notify_error_cleared->setChecked(notifyOnErrorCleared);

    //==============================================================

    // Check if the auto save setting does not exist
    if (!userSettings.contains("autoSaveLimit") || !userSettings.value("autoSaveLimit").isValid()) {
        // set the default value
        userSettings.setValue("autoSaveLimit", INITIAL_AUTO_SAVE_LIMIT);
    }

    //set session variable based on setting
    autoSaveLimit = userSettings.value("autoSaveLimit").toInt();

    //update gui to match
    ui->auto_save_limit->setValue(autoSaveLimit);

    //==============================================================

    //check if the timeout setting does not exist
    if (!userSettings.contains("connectionTimeout") || !userSettings.value("connectionTimeout").isValid()) {
        // If it doesn't exist or is not valid, set the default value
        userSettings.setValue("connectionTimeout", INITIAL_CONNECTION_TIMEOUT);
    }

    //set session variable based on setting
    connectionTimeout = userSettings.value("connectionTimeout").toInt();

    //update gui to match
    ui->connection_timeout->setValue(connectionTimeout);

    //==============================================================

    // Check if ram clearing setting does not exist
    if (!userSettings.contains("RAMClearing") || !userSettings.value("RAMClearing").isValid()) {
        // set the default value
        userSettings.setValue("RAMClearing", INITIAL_RAM_CLEARING);
    }

    //set gui display to match
    ui->ram_clearing->setChecked(userSettings.value("RAMClearing").toBool());

    //check if the max nodes setting does not exist
    if (!userSettings.contains("maxDataNodes") || !userSettings.value("maxDataNodes").isValid()) {
        // If it doesn't exist or is not valid, set the default value
        userSettings.setValue("maxDataNodes", INITIAL_MAX_DATA_NODES);
    }

    //update gui to match
    ui->max_data_nodes->setValue(userSettings.value("maxDataNodes").toInt());

    //dont allow the user to go below this value for max data nodes
    ui->max_data_nodes->setMinimum(MIN_DATA_NODES_BEFORE_RAM_CLEAR);

    //set max node visibility based on ram clearing setting
    ui->max_data_nodes->setVisible(userSettings.value("RAMClearing").toBool());
    ui->max_data_nodes_label->setVisible(userSettings.value("RAMClearing").toBool());

    //==============================================================

    //sets up text options in connection settings drop down boxes
    setupConnectionPage();

    //setup port name selections on gui (scans for available ports)
    setup_ddm_port_selection(0);

    #if DEV_MODE
        //load port names for csim port selection
        setup_csim_port_selection(0);

        #if GENERAL_DEBUG
        // Display user settings
        displaySavedSettings();
        #endif
    #endif

    //in case settings were loaded from initial constants, sync settings to registry
    userSettings.sync();
}

//displays the current values of the status class onto the gui status page
void MainWindow::updateStatusDisplay()
{
    //update feed position text and graphic
    ui->feedPosition->setValue(status->feedPosition);
    ui->feed_position_label->setText(FEED_POSITION_NAMES[status->feedPosition/FEED_POSITION_INCREMENT_VALUE]);

    //update fire mode graphic
    ui->fireMode->setValue(status->firingMode);

    //update text (fire rate, firing events, burst length, processor state)
    ui->fireRateOutput->setText(QString::number(status->firingRate));
    ui->firingEventsOutput->setText(QString::number(status->totalFiringEvents));
    ui->burstOutput->setText(QString::number(status->burstLength));
    ui->processorOutput->setText(CONTROLLER_STATE_NAMES[status->controllerState]);

    //update trigger 1 text
    ui->trigger1_label->setText(TRIGGER_STATUS_NAMES[status->trigger1]);

    //update trigger 1 graphic
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

    //update trigger 2 text
    ui->trigger2_label->setText(TRIGGER_STATUS_NAMES[status->trigger2]);

    //update trigger 2 graphic
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

    //update armed text
    ui->armed_label->setText(ARMED_NAMES[status->armed]);

    //update the armed graphic
    if(status->armed)
    {
        ui->armedOutput->setPixmap(GREEN_LIGHT);
    }
    else
    {
        ui->armedOutput->setPixmap(RED_LIGHT);
    }
}

// method updates the running elapsed controller time
void MainWindow::updateElapsedTime()
{
    // add 1 second to timer
    status->elapsedControllerTime = status->elapsedControllerTime.addSecs(1);

    // update the GUI
    ui->elapsedTime->setText(status->elapsedControllerTime.toString(TIME_FORMAT));
}

// method updates the elapsed time since last message received to DDM
void MainWindow::updateTimeSinceLastMessage()
{
    // initialize variables
    QTime elapsedTime;

    // calculate time elapsed since the last time DDM received a message
    QDateTime currentTime = QDateTime::currentDateTime();

    // the msecsto method returns the amount of ms from timeLastRecieved to currentTime
    qint64 elapsedMs = timeLastReceived.msecsTo(currentTime);

    // check for negative elapsed time
    if(elapsedMs < 0)
    {
        qDebug() << "Error: updateTimeSinceLastMessage time since last DDM message received is negative.\n";
    }
    //check if timeout was reached
    else if (elapsedMs >= connectionTimeout)
    {
        //run disconnect method
        on_handshake_button_clicked();
    }
    // assume positive elapsed time
    else
    {
        // convert back into QTime instead of qint64
        elapsedTime = QTime(0, 0, 0).addMSecs(elapsedMs);

        // update gui
        ui->DDMTimer->setText(elapsedTime.toString(TIME_FORMAT));
    }
}

//updates gui with given message, dynamically colors output based on the type of outString
//accounts for filtering settings and only renders the outString if it is being filtered for
// by the user
void MainWindow::updateEventsOutput(EventNode *event)
{
    QTextDocument document;
    QString richText;
    QString outString = events->nodeToString(event);

    //check if we have an event as input and check if filter allows printing events
    if (!event->isError() )
    {
        if (eventFilter == EVENTS || eventFilter == ALL)
        {
            //change output text color to white
            richText = "<p style='color: "+EVENT_COLOR+"; font-size: "+EVENT_OUTPUT_SIZE+"px'>"+ outString + "</p>";

            //activate html for the output
            document.setHtml(richText);

            //append styled string to the events output
            ui->events_output->append(document.toHtml());
        }
    }
    //otherwise check for cleared error and if filter allows printing cleared errors
    else if (static_cast<ErrorNode *>(event)->cleared)
    {
        if (eventFilter == ALL || eventFilter == ERRORS || eventFilter == CLEARED_ERRORS)
        {
            if (coloredEventOutput)
            {
                //change output text color to green
                richText = "<p style='color: "+CLEARED_ERROR_COLOR+"; font-size: "+EVENT_OUTPUT_SIZE+"px'>"+ outString + "</p>";
            }
            else
            {
                //change output text color to white
                richText = "<p style='color: "+EVENT_COLOR+"; font-size: "+EVENT_OUTPUT_SIZE+"px'>"+ outString + "</p>";
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
            richText = "<p style='color: "+ACTIVE_ERROR_COLOR+"; font-size: "+EVENT_OUTPUT_SIZE+"px'>"+ outString + "</p>";
        }
        else
        {
            //change output text color to white
            richText = "<p style='color: "+EVENT_COLOR+"; font-size: "+EVENT_OUTPUT_SIZE+"px'>"+ outString + "</p>";
        }

        //activate html for the output
        document.setHtml(richText);

        //append styled string to the events output
        ui->events_output->append(document.toHtml());
    }

    // update total events gui
    ui->TotalEventsOutput->setText(QString::number(events->totalEvents));
    ui->statusEventOutput->setText(QString::number(events->totalEvents));

    if (!event->isError()) return;

    // update total errors gui
    ui->TotalErrorsOutput->setText(QString::number(events->totalErrors));
    ui->statusErrorOutput->setText(QString::number(events->totalErrors));

    if ( static_cast<ErrorNode *>(event)->cleared )
    {
        // update cleared errors gui
        ui->ClearedErrorsOutput->setText(QString::number(events->totalClearedErrors));
        ui->statusClearedErrors->setText(QString::number(events->totalClearedErrors));
    }
    else
    {
        // update active errors gui
        ui->ActiveErrorsOutput->setText(QString::number(events->totalErrors - events->totalClearedErrors));
    }
}

//clears events tab gui element, then repopulates it with current event data
void MainWindow::refreshEventsOutput()
{
    // reset gui element
    ui->events_output->clear();

    //init vars
    ErrorNode *wkgErrPtr = events->headErrorNode;
    EventNode *wkgEventPtr = events->headEventNode;
    EventNode *nextPrintPtr;

    //loop through all events and errors
    while(wkgErrPtr != nullptr || wkgEventPtr != nullptr)
    {
        // get next to print
        nextPrintPtr = events->getNextNode(wkgEventPtr, wkgErrPtr);

        //update events output if filter allows
        updateEventsOutput(nextPrintPtr);
    }

    #if DEV_MODE && GUI_DEBUG
    qDebug() << "Events output refreshed";
    #endif
}

//clears error in event output by replacing activeIndicator with clearedIndicator
//also changes color of text if coloredEventOutput is on
void MainWindow::clearErrorFromEventsOutput(int errorId)
{
    //if user is on specific error filter, refresh to update their screen
    if (eventFilter == NON_CLEARED_ERRORS || eventFilter == CLEARED_ERRORS)
    {
        refreshEventsOutput();
        return;
    }

    QTextCharFormat newColor;
    if (coloredEventOutput)
    {
        newColor.setForeground(QColor(20, 174, 92));
    }

    // Get the QTextDocument of the QTextEdit
    QTextDocument *document = ui->events_output->document();

    // Create a QTextCursor to manipulate the text
    QTextCursor cursor(document);

    // Move to the beginning of the document
    cursor.movePosition(QTextCursor::Start);

    cursor = document->find("ID: " + QString::number(errorId) + ",", cursor, QTextDocument::FindWholeWords);
    if (!cursor.isNull())
    {
        if (coloredEventOutput)
        {
            //store our starting place
            QTextCursor tmpCursor = cursor;

            //highlight the entire line
            cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

            // Apply new color to the highlighted text
            cursor.mergeCharFormat(newColor);

            //move cursor to prev position
            cursor = tmpCursor;
        }

        //find active error indicator
        cursor = document->find(events->activeIndicator, cursor, QTextDocument::FindWholeWords);
        // Replace active indicator with cleared indicator
        cursor.insertText(events->clearedIndicator);

        #if DEV_MODE && (EVENTS_DEBUG || GUI_DEBUG)
        qDebug() << "Cleared error " << errorId << " on events output";
        #endif
    }
    else
    {
        qDebug() << "Error: clearErrorFromEventsOutput failed to find error" << errorId;
    }
}

//overloaded function for convenience
void MainWindow::notifyUser(QString notificationText, bool error)
{
    notifyUser(notificationText, "", error);
}

//renders a notification for the user that lasts 3 seconds. Updates the
//notification page button to indicate unread messages. bool urgent is used to
//toggle the notification outline from orange to red
void MainWindow::notifyUser(QString notificationText, QString logText, bool error)
{
    // Get the current timestamp
    QString timeStamp = QDateTime::currentDateTime().toString("[" + TIME_FORMAT +"] ");

    QString notificationRichText = "<p style='" + NOTIFICATION_TIMESTAMP_STYLE + "'>" + timeStamp +
                                   " " + "<span style='color: ";

    QString popUpStyle = "border: 3px solid ";

    // get new lines as literals
    notificationText.replace("\n", "\\n");

    if (error)
    {
        //replace new line literals with \n symbols
        logText.replace("\n", "\\n");
        notificationRichText += ERROR_COLOR;
        popUpStyle += ERROR_COLOR;
    }
    else
    {
        notificationRichText += STANDARD_COLOR;
        popUpStyle += STANDARD_COLOR;
    }
    notificationRichText += "; font-size: "+NOTIFICATION_SIZE+"px'>" + notificationText;
    popUpStyle += "; " + POP_UP_STYLE;

    if (logText != "")
    {
        notificationRichText += " : " + logText;
    }
    notificationRichText+= + "</span></p>";

    //set pop up style
    ui->notificationPopUp->setStyleSheet(popUpStyle);

    //add full notification to notification page output
    ui->notificationOutput->append(notificationRichText);

    //display notification on "navbar"
    ui->notificationPopUp->setText(notificationText);

    //check if user is on notification page, and if this is an error
    if (ui->Flow_Label->currentIndex() != 6 && error)
    {
        //update the notification icon to get user attention
        ui->NotificationPageButton->setStyleSheet(URGENT_NOTIFICATION_ICON);
    }

    //stop old timer if running
    notificationTimer->stop();

    //give fresh timeout to avoid premature notification clearing
    notificationTimer->start(NOTIFICATION_DURATION);
}

QString MainWindow::getSessionStatistics()
{
    return "Duration: " + status->elapsedControllerTime.toString(TIME_FORMAT) + ", Total Events: " +
           QString::number(events->totalEvents) + ", Total Errors: " + QString::number(events->totalErrors)
           + ", Non-cleared errors: " + QString::number(events->totalClearedErrors)
           + ", Total Firing events: " + QString::number(status->totalFiringEvents);
}

//called when advanced log file setting is active, meant to log status updates and electrical data
void MainWindow::logAdvancedDetails(SerialMessageIdentifier id)
{
    if (autosaveLogFile == "")
    {
        return;
    }

    //retreive the given file
    QFile file(autosaveLogFile);
    QString outString;

    //get proper msg id
    switch(id)
    {
        case ELECTRICAL:
            outString = ADVANCED_LOG_FILE_INDICATOR + "Electrical Data: " + electricalData->toString();

            break;

        case STATUS:
            outString = ADVANCED_LOG_FILE_INDICATOR + "Status Update: " + status->toString();

            break;

        case CLOSING_CONNECTION:
            outString = ADVANCED_LOG_FILE_INDICATOR + "Session Statistics: " + getSessionStatistics();

            break;

        default:
            break;
    }

    //attempt to open in append mode
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        qDebug() <<  "Error: logAdvancedDetails Could not open log file for appending: " << autosaveLogFile;
        notifyUser("Failed to open logfile", "log text \"" + outString + "\" discarded", true);
    }
    else
    {
        //append the text to the log file
        QTextStream out(&file);
        out << outString << Qt::endl;
        file.close();
    }
}

//uses data in electrical class to render electrical page
void MainWindow::renderElectricalPage()
{
    int boxIndex;
    //get head node into wkg ptr
    electricalNode* wkgElecPtr = electricalData->headNode;

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
}

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================

#if DEV_MODE
//scans for available serial ports and adds them to csim port selection box
void MainWindow::setup_csim_port_selection(int index)
{
    // Check and set initial value for "csimPortName"
    if (userSettings.value("csimPortName").toString().isEmpty())
        userSettings.setValue("csimPortName", INITIAL_CSIM_PORT);

    ui->csim_port_selection->clear();

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

//updates the dev page non cleared error selection box
void MainWindow::update_non_cleared_error_selection()
{
    //clear combo box
    ui->non_cleared_error_selection->clear();

    //check for valid events ptr
    if (csimHandle->eventsPtr != nullptr)
    {
        //get head node
        ErrorNode *wkgPtr = csimHandle->eventsPtr->headErrorNode;

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
    qDebug() << "Auto Save Limit: " << userSettings.value("autoSaveLimit").toInt();
    qDebug() << "Notify on error cleared: " << userSettings.value("notifyOnErrorCleared").toBool();
    qDebug() << "Advanced log file: " << userSettings.value("advancedLogFile").toBool();
    qDebug() << "Connection Timeout duration: " << userSettings.value("connectionTimeout").toInt();
    qDebug() << "RAM Clearing: " << userSettings.value("RAMClearing").toBool();
    qDebug() << "Max Data Nodes: " << userSettings.value("maxDataNodes").toInt() << Qt::endl;
}
#endif

