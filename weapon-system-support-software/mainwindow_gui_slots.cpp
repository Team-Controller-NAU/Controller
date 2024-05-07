#include "mainwindow.h"

//this file contains only the implementation of the GUI slots. Declare
//other processing functions in mainwindow.cpp

//======================================================================================
// Navigation
//======================================================================================

/**
 * @brief sends user to electrical page when clicked
 */
void MainWindow::on_ElectricalPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(3);
    resetPageButton();
    ui->ElectricalPageButton->setStyleSheet(SELECTED_NAV_BUTTON_STYLE);
}

/**
 * @brief sends user to events page when clicked
 */
void MainWindow::on_EventsPageButton_clicked()
{
    // TODO: first visit refresh page with dump of whole LL??
    ui->Flow_Label->setCurrentIndex(0);
    resetPageButton();
    ui->EventsPageButton->setStyleSheet(SELECTED_NAV_BUTTON_STYLE);
}

/**
 * @brief sends user to connection page when clicked
 */
void MainWindow::on_ConnectionPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(2);
    resetPageButton();
    ui->ConnectionPageButton->setStyleSheet(SELECTED_NAV_BUTTON_STYLE);
}

/**
 * @brief sends user to status page when clicked
 */
void MainWindow::on_StatusPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(4);
    resetPageButton();
    ui->StatusPageButton->setStyleSheet(SELECTED_NAV_BUTTON_STYLE);
}

/**
 * @brief sends user to settings page when clicked
 */
void MainWindow::on_SettingsPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(5);
    resetPageButton();
    ui->SettingsPageButton->setStyleSheet(SELECTED_SETTINGS_ICON);
}

/**
 * @brief sends user to notification page when clicked
 */
void MainWindow::on_NotificationPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(6);
    resetPageButton();
    ui->NotificationPageButton->setStyleSheet(SELECTED_NOTIFICATIONS_ICON);
}

/**
 * @brief reset all tab buttons to default style
 */
void MainWindow::resetPageButton()
{
    ui->ConnectionPageButton->setStyleSheet(NAV_BUTTON_STYLE);
    ui->EventsPageButton->setStyleSheet(NAV_BUTTON_STYLE);
    ui->StatusPageButton->setStyleSheet(NAV_BUTTON_STYLE);
    ui->ElectricalPageButton->setStyleSheet(NAV_BUTTON_STYLE);
    ui->SettingsPageButton->setStyleSheet(SETTINGS_ICON);
    ui->NotificationPageButton->setStyleSheet(NOTIFICATIONS_ICON);

#if DEV_MODE
    ui->DevPageButton->setStyleSheet(NAV_BUTTON_STYLE);
#endif
}

//======================================================================================
// General GUI slots
//======================================================================================

/**
 * @brief runs when user changes ddm port. Close old connection, then make new one
 * @param index The int representing users selection
 */
void MainWindow::on_ddm_port_selection_currentIndexChanged(int index)
{
    //create connection on selected port if combo box is set up
    if (allowSettingChanges)
    {
        createDDMCon();
        ddmPortName = ui->ddm_port_selection->currentText();
    }
}

/**
 * @brief Manually download events to log file
 */
void MainWindow::on_download_button_clicked()
{
    //if total nodes is 0 prevent download
    if (events->totalNodes == 0)
    {
        notifyUser("Download prevented", "No data is available to download.", true);
        return;
    }

    QString logFile = userSettings.value("logfileLocation").toString() + QString::number(QDateTime::currentSecsSinceEpoch()) + "-logfile-M.txt";

    // save logfile - mannually done
    if (events->outputToLogFile(logFile, false))
    {
       notifyUser("Download complete", "Log file loction: " + logFile, false);
    }
    else
    {
       notifyUser("Download failed", "Could not open " + logFile + " for writing", true);
    }
}
/**
 * @brief Changes filter on events output
 * @param index The users selected filter
 */
void MainWindow::on_FilterBox_currentIndexChanged(int index)
{
    // check for which filter the user selected
    switch(index)
    {
    case ALL:
        #if DEV_MODE && GUI_DEBUG
        qDebug() << "All filter selected";
        #endif
        eventFilter = ALL;
        break;

    case EVENTS:
        #if DEV_MODE && GUI_DEBUG
        qDebug() << "Events filter selected";
        #endif
        eventFilter = EVENTS;
        break;

    case ERRORS:
        #if DEV_MODE && GUI_DEBUG
        qDebug() << "Errors filter selected";
        #endif
        eventFilter = ERRORS;
        break;

    case CLEARED_ERRORS:
        #if DEV_MODE && GUI_DEBUG
        qDebug() << "Cleared errors filter selected";
        #endif
        eventFilter = CLEARED_ERRORS;
        break;

    case NON_CLEARED_ERRORS:
        #if DEV_MODE && GUI_DEBUG
        qDebug() << "Non-cleared errors filter selected";
        #endif
        eventFilter = NON_CLEARED_ERRORS;
        break;

    default:
        qDebug() << "Error: on_FilterBox_currentIndexChanged unrecognized filter index."<< Qt::endl ;
    }

    //refresh to display with filter
    refreshEventsOutput();
}

/**
 * @brief toggles handshake process on and off. Once connected, allow for disconnect (send disconnect message to controller)
 * this button is seen as connect/connecting/disconnect on connection page
 */
void MainWindow::on_handshake_button_clicked()
{
    //prevent spam
    if (handshakeCooldownTimer->isActive()) {qDebug()<<"handshake spam prevented"; return;}

    //if port isnt open, attempt to open it
    if (ddmCon == nullptr)
    {
        createDDMCon();

        //if unsuccessful, notify user of fail and return
        if (ddmCon == nullptr)
        {
            notifyUser("Failed to open port " + ui->ddm_port_selection->currentText(), true);
            return;
        }
    }

    //catch possible errors
    if (!ddmCon->serialPort.isOpen())
    {
        notifyUser("Failed to open port" + ui->ddm_port_selection->currentText(), true);
        return;
    }

    // check if handshake is not in progress and ddm is not connected
    if ( !handshakeTimer->isActive() && !ddmCon->connected )
    {
        #if DEV_MODE && SERIAL_COMM_DEBUG
        qDebug() << "Beginning handshake with controller" << Qt::endl;
        #endif

        // Start the timer to periodically check the handshake status
        handshakeTimer->start();

        //refreshes connection button/displays
        ui->handshake_button->setText("Connecting");
        ui->handshake_button->setStyleSheet(CONNECTING_STYLE);

        ui->connectionStatus->setPixmap(ORANGE_LIGHT);
        ui->connectionLabel->setText("Connecting ");
        //disable changes to connection settings
        disableConnectionChanges();
    }
    else
    {
        //disconnect from controller
        ddmCon->sendDisconnectMsg();

        //update connection status to disconnected and update related objects
        //we use timer to allow grace period for final messages (disregard clazy warning)
        QTimer::singleShot(DISCONNECT_GRACE_PERIOD, [&]() {
            updateConnectionStatus(false);
        });

        if (ddmCon->connected)
        {
            notifyUser("User disconnect", "Session end",  false);
        }
    }

    handshakeCooldownTimer->start();
}

/**
 * @brief saves connection settings into the qSettings class for cross session storage
 */
void MainWindow::on_save_Button_clicked()
{
    // Load all of the current connection settings into the settings class
    // Storing the enum values as strings
    userSettings.setValue("baudRate", ui->baud_rate_selection->currentText());
    userSettings.setValue("dataBits", ui->data_bits_selection->currentText());
    userSettings.setValue("parity", ui->parity_selection->currentText());
    userSettings.setValue("stopBits", ui->stop_bit_selection->currentText());
    userSettings.setValue("flowControl", ui->flow_control_selection->currentText());
    userSettings.setValue("portName", ui->ddm_port_selection->currentText());
    userSettings.setValue("csimPortName", ui->csim_port_selection->currentText());

    //write changes to the registry
    userSettings.sync();

    notifyUser("Default settings saved.", false);

    #if DEV_MODE && GUI_DEBUG
        //output new settings to qDebug()
        displaySavedSettings();
    #endif
}

/**
 * @brief restores connection settings to the values saved to user settings (uses vals in constants.h until changed by user)
 */
void MainWindow::on_restore_Button_clicked()
{
    // Retrieve the default connection settings from the settings class
    QString defaultBaudRate = userSettings.value("baudRate").toString();
    QString defaultDataBits = userSettings.value("dataBits").toString();
    QString defaultParity = userSettings.value("parity").toString();
    QString defaultStopBits = userSettings.value("stopBits").toString();
    QString defaultFlowControl = userSettings.value("flowControl").toString();

    // Set the default values to the GUI elements if they exist in the combo boxes
    if (ui->baud_rate_selection->findText(defaultBaudRate) != -1)
        ui->baud_rate_selection->setCurrentText(defaultBaudRate);

    if (ui->data_bits_selection->findText(defaultDataBits) != -1)
        ui->data_bits_selection->setCurrentText(defaultDataBits);

    if (ui->parity_selection->findText(defaultParity) != -1)
        ui->parity_selection->setCurrentText(defaultParity);

    if (ui->stop_bit_selection->findText(defaultStopBits) != -1)
        ui->stop_bit_selection->setCurrentText(defaultStopBits);

    if (ui->flow_control_selection->findText(defaultFlowControl) != -1)
        ui->flow_control_selection->setCurrentText(defaultFlowControl);
}

/**
 * @brief opens file explorer on users system to the log file folder
 */
void MainWindow::on_openLogfileFolder_clicked()
{
    #if DEV_MODE && GUI_DEBUG
    qDebug() << "on_openLogfileFolder_clicked: file explorer opening to : " << userSettings.value("logfileLocation").toString();
    #endif

    QUrl logFileUrl = QUrl::fromLocalFile(userSettings.value("logfileLocation").toString());

    // Ensure the directory exists and create it if not
    QDir logDir(userSettings.value("logfileLocation").toString());
    if (!logDir.exists())
    {
        if (!logDir.mkpath("."))
        {
            qDebug() << "Error: on_openLogfileFolder_clicked failed to create logfile directory: " << userSettings.value("logfileLocation").toString();
            notifyUser("Failed to open logfile directory", userSettings.value("logfileLocation").toString() +" does not exist", true );
            return;
        }
    }

    if (!QDesktopServices::openUrl(logFileUrl))
    {
        qDebug() << "Error: on_openLogfileFolder_clicked failed to open the directory:" << logFileUrl.toString();
    }
}

/**
 * @brief Opens file explorer for user to select new directory for log files to be stored
 */
void MainWindow::on_setLogfileFolder_clicked()
{
    // save prev user settings value
    QString previousPath = userSettings.value("logfileLocation").toString();

    // set logfile location with the user choice
    userSettings.setValue("logfileLocation", QFileDialog::getExistingDirectory(this, tr("Create or Select a logfolder directory")) + "/");

    // check the success of saving settings
    if(userSettings.status() != QSettings::NoError)
    {
        qDebug() << "Error: on_setLogfileFolder_clicked() failed to save logfile location: " << userSettings.status()<< Qt::endl;
    }
    // check if user exited the dialog box
    else if(userSettings.value("logfileLocation").toString() == "/")
    {
        // revert to previous user setting
        userSettings.setValue("logfileLocation", previousPath);
        qDebug() << "Error: on_setLogfileFolder_clicked() No logfile directory set. Reverting to previous path: " << previousPath << Qt::endl;
    }
    // otherwise, assume successful logfile directory creation
    else
    {
        #if DEV_MODE && GUI_DEBUG
        qDebug() << "New log file directory set: " << userSettings.value("logfileLocation").toString();
        #endif

        // Ensure the directory exists and create it if not
        QDir logDir(userSettings.value("logfileLocation").toString());
        if (!logDir.exists())
        {
            if (!logDir.mkpath("."))
            {
                qDebug() << "Error: setupSettings Failed to create logfile directory: " << userSettings.value("logfileLocation").toString();
                notifyUser("Failed to create logfile directory",userSettings.value("logfileLocation").toString(), true );
            }
        }
    }

    //sync user settings
    userSettings.sync();
}

/**
 * @brief opens file explorer to log file directory. User selects file then then
 * data is loaded into events class and rendered in events page.
 */
void MainWindow::on_load_events_from_logfile_clicked()
{
    //declare file browser class
    QFileDialog dialog(this);

    //get user logfile setting as starting dir
    dialog.setDirectory(userSettings.value("logfileLocation").toString());

    // Open a file dialog for the user to select a logfile
    QString selectedFile = dialog.getOpenFileName(this, tr("Select Log File"), QString(), tr("Log Files (*.txt);;All Files (*)"));

    // Check if the user canceled the dialog
    if (selectedFile.isEmpty())
    {
        return;
    }

    #if DEV_MODE && GUI_DEBUG
        logEmptyLine();
        qDebug() << "Loading data from: " << selectedFile;
    #endif

    // Pass the selected file name to the loadDataFromLogFile function
    int result = events->loadDataFromLogFile(events, selectedFile);

    // Handle the result if needed
    if (result == INCORRECT_FORMAT)
    {
        qDebug() << "Error: on_load_events_from_logfile_clicked() Log file was of incorrect format." << Qt::endl;
        notifyUser("Load failed on corrupt logfile.", selectedFile, true);
    }
    else if (result == DATA_NOT_FOUND)
    {
        qDebug() << "Error: on_load_events_from_logfile_clicked() Log file could not be found" << Qt::endl;
        notifyUser("Load failed on missing logfile.", true);
    }
    else
    {
        notifyUser("Logfile loaded.", selectedFile, false);
        ui->truncated_label->setVisible(false);
        connect(events, &Events::RAMCleared, this, &MainWindow::handleRAMClear);
    }

    //refresh the events output
    refreshEventsOutput();
}

/**
 * @brief scans for serial ports on users system
 *
 * useful in case an adaptor is plugged in after
 * program launch (also updates dev page selection if dev mode is active)
 */
void MainWindow::on_refresh_serial_port_selections_clicked()
{
    notifyUser("Scanning for serial ports", false);
    setup_ddm_port_selection(0);
    #if DEV_MODE
        setup_csim_port_selection(0);
    #endif
}
/**
 * @brief Opens search dialog for searching keywords in events output
 */
void MainWindow::on_searchButton_clicked()
{
    // get the text to find from the user by opening an inputdialog box
    QString searchText;
    QInputDialog inputDialog = QInputDialog(this);
    inputDialog.setInputMode(QInputDialog::TextInput);
    inputDialog.setLabelText("Search for some text:");
    inputDialog.setOkButtonText("Find Next");
    inputDialog.setWindowTitle("Search");
    inputDialog.setStyleSheet("color:white;");
    QSize minSize = inputDialog.minimumSizeHint();
    inputDialog.setFixedSize(minSize);

    // use findChild to get the QLineEdit instance of the QInputDialog so we can set placeholder text
    QLineEdit *lineEdit = inputDialog.findChild<QLineEdit *>();
    if (lineEdit) lineEdit->setPlaceholderText("Enter text to find...");

    bool windowOpen = true;

    // start loop for continuous search until the user presses cancel or closes the window
    while (windowOpen)
    {
        // open the dialog box and get the user input
        windowOpen = inputDialog.exec();

        // get text from user input
        searchText = inputDialog.textValue();
        int index = 0;

        // check if the window is still open and the search text is not empty
        if (windowOpen && !searchText.isEmpty())
        {
            // get all the text content from events_output
            QString eventsText = ui->events_output->toPlainText();

            // set up cursor object
            QTextCursor cursor = ui->events_output->textCursor();

            // check if we have done a search in this loop
            if (cursor.hasSelection())
            {
                // set index to current position
                index = cursor.position() + 1;
            }

            // find the next occurrence of the text in the events_output, ignoring caps
            int position = eventsText.indexOf(searchText, index, Qt::CaseInsensitive);

            // check if we have actually found the text
            if (position != -1) {
                // if the text is found, move the cursor to the found position and highlight it
                cursor.setPosition(position);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, searchText.length());
                ui->events_output->setTextCursor(cursor);
            }
            // otherwise, assume we have not found the text
            else
            {
                // reset cursor selection
                cursor.clearSelection();
                cursor.setPosition(0);
                ui->events_output->setTextCursor(cursor);

                // show error message
                QMessageBox informationBox = QMessageBox(this);
                informationBox.setWindowTitle("Search");
                informationBox.setText("Next occurrence not found.");
                informationBox.setIcon(QMessageBox::Information);
                informationBox.setStyleSheet("color: white;");
                informationBox.exec();
            }
        }
        else
        {
            // break the loop if the window is closed or the search text is empty
            QTextCursor cursor = ui->events_output->textCursor();
            cursor.clearSelection();
            cursor.setPosition(0);
            ui->events_output->setTextCursor(cursor);
            break;
        }
    }
}

//======================================================================================
// User settings
//======================================================================================

/**
 * @brief toggle colored events output (from settings page)
 * @param arg1 users on-off selection
 */
void MainWindow::on_colored_events_output_stateChanged(int arg1)
{
    //arg1 represents the state of the checkbox
    switch(arg1)
    {
    //unchecked
    case 0:
        coloredEventOutput = false;
        break;

        //checked
    default:
        coloredEventOutput = true;
    }

    userSettings.setValue("coloredEventOutput", coloredEventOutput);

    if (!allowSettingChanges) return;
    refreshEventsOutput();

    //write changes to the registry
    userSettings.sync();
}

/**
 * @brief user sets the number of auto save log files before overwrites occur (from settings page)
 * @param arg1 The users selected value
 */
void MainWindow::on_auto_save_limit_valueChanged(int arg1)
{
    autoSaveLimit = arg1;
    userSettings.setValue("autoSaveLimit", autoSaveLimit);

    //write changes to the registry
    userSettings.sync();
}

/**
 * @brief changes timeout duration based on user selection
 * @param arg1 The users selected value
 */
void MainWindow::on_connection_timeout_valueChanged(int arg1)
{
    connectionTimeout = arg1;
    userSettings.setValue("connectionTimeout", connectionTimeout);

    //write changes to the registry
    userSettings.sync();
}

/**
 * @brief toggles notifications on error cleared, based on user input
 * @param arg1 The users on-off selection
 */
void MainWindow::on_notify_error_cleared_stateChanged(int arg1)
{
    //arg1 represents the state of the checkbox
    switch(arg1)
    {
    //unchecked
    case 0:
        notifyOnErrorCleared = false;

        break;

        //checked
    default:
        notifyOnErrorCleared = true;
    }

    userSettings.setValue("notifyOnErrorCleared", notifyOnErrorCleared);

    userSettings.sync();
}

/**
 * @brief Toggles advanced log file outputs (electrical details, status updates, session statistics)
 * @param arg1 The users on-off selection
 */
void MainWindow::on_advanced_log_file_stateChanged(int arg1)
{
    //arg1 represents the state of the checkbox
    switch(arg1)
    {
    //unchecked
    case 0:
        advancedLogFile = false;

        break;

        //checked
    default:
        advancedLogFile = true;
    }

    userSettings.setValue("advancedLogFile", advancedLogFile);

    //check if ddmCon does not exist
    if (ddmCon == NULL)
    {
        return;
    }

    //if connected, add updated advanced log file setting notification to logfile
    if (ddmCon->connected)
    {
        QFile file(autosaveLogFile);

        //attempt to open in append mode
        if (!file.open(QIODevice::Append | QIODevice::Text))
        {
            qDebug() << "Error: on_advanced_log_file_stateChanged could not open log file for appending: " << autosaveLogFile << Qt::endl;
        }
        else
        {
            QTextStream out(&file);

            if (advancedLogFile)
            {
                out << ADVANCED_LOG_FILE_INDICATOR + "ADVANCED LOG FILE ENABLED" << "\n";
            }
            else
            {
                out << ADVANCED_LOG_FILE_INDICATOR + "ADVANCED LOG FILE DISABLED" << "\n";
            }
            file.close();
        }
    }

    //write changes to the registry
    userSettings.sync();
}

/**
 * @brief Toggle RAM clearing based on user input
 * @param arg1 The users on-off selection
 */
void MainWindow::on_ram_clearing_stateChanged(int arg1)
{
    //arg1 represents the state of the checkbox
    switch(arg1)
    {
    //unchecked
    case 0:
        userSettings.setValue("RAMClearing", false);

        break;

        //checked
    default:
        userSettings.setValue("RAMClearing", true);
    }

    //update value in events class
    if (events != nullptr) events->RAMClearing = userSettings.value("RAMClearing").toBool();

    //set visibility of max nodes based on ram clearing setting
    ui->max_data_nodes->setVisible(userSettings.value("RAMClearing").toBool());
    ui->max_data_nodes_label->setVisible(userSettings.value("RAMClearing").toBool());

    userSettings.sync();
}

/**
 * @brief updates value of max data nodes for events class based on user selection
 * @param arg1 The user selected number of max nodes
 */
void MainWindow::on_max_data_nodes_valueChanged(int arg1)
{
    userSettings.setValue("maxDataNodes", arg1);

    //write changes to the registry
    userSettings.sync();

    if (events != nullptr) events->maxNodes = arg1;
}


//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================

#if DEV_MODE

//sends user to developer page when clicked
void MainWindow::on_DevPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(1);
    resetPageButton();
    ui->DevPageButton->setStyleSheet(SELECTED_NAV_BUTTON_STYLE);
}

//manually clear errors from dev page
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

//button toggles csim random generation on and off. first click will setup thread and run csim.
//second will terminate thread and close csim connection.
void MainWindow::on_CSim_button_clicked()
{
    //check if csim is currently running
    if (csimHandle->isRunning())
    {
        // csim is running, shut it down
        csimHandle->stopSimulation();

        csimHandle->pause=false;

        // update ui
        ui->CSim_button->setText("Start CSim");

        //enable csim port selection
        ui->csim_port_selection->setEnabled(true);

        ui->non_cleared_error_selection->clear();
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

        QTimer::singleShot(1000, this, [this]() {
            if (csimHandle->connPtr == nullptr)
                {
                // csim is running, shut it down
                csimHandle->stopSimulation();

                // update ui
                ui->CSim_button->setText("Start CSim");

                //enable csim port selection
                ui->csim_port_selection->setEnabled(true);

                notifyUser("Could not open " + ui->csim_port_selection->currentText() + " for CSIM", true);
                }
        });
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

void MainWindow::on_output_messages_sent_button_clicked()
{
    //send request for csim to output its session string
    emit outputMessagesSentRequest();
}

//toggles csim between accounting for 1 and 2 triggers
void MainWindow::on_toggle_num_triggers_clicked()
{
    //check if secondTrigger is currently enabled
    if ( csimHandle->secondTrigger )
    {
        //disable it
        csimHandle->secondTrigger = false;

        //update button text
        ui->toggle_num_triggers->setText("Set to two triggers");

        //empty the trigger status current value
        status->trigger2 = NA;
    }
    //secondTrigger is currently disabled
    else
    {
        //enable it
        csimHandle->secondTrigger = true;
        //update button text
        ui->toggle_num_triggers->setText("Set to single trigger");
    }
}

//sends custom user input message
void MainWindow::on_send_message_button_clicked()
{
    // Get user input from input box
    QString userInput = ui->message_contents->toPlainText();

    // Replace literal "\n" characters with actual newline characters
    userInput.replace("\\n", "\n");

    // Add newline character if userInput does not end with newline
    if (!userInput.endsWith('\n'))
        userInput += '\n';

    // Clear the contents of input box
    ui->message_contents->clear();

    // Check if csim has an active connection
    if (csimHandle->connPtr != nullptr)
    {
        // Send signal for csim to transmit message
        emit transmissionRequest(userInput);
    }
    // No active connection from csim, make temporary connection
    else
    {
        // Open new connection on com4 (smart pointer auto frees memory when function exits)
        std::unique_ptr<Connection> conn(new Connection(ui->csim_port_selection->currentText(),
                                                        fromStringBaudRate(ui->baud_rate_selection->currentText()),
                                                        fromStringDataBits(ui->data_bits_selection->currentText()),
                                                        fromStringParity(ui->parity_selection->currentText()),
                                                        fromStringStopBits(ui->stop_bit_selection->currentText()),
                                                        fromStringFlowControl(ui->flow_control_selection->currentText())));

        // Send message through csim port
        conn->transmit(userInput);

        conn->serialPort.waitForReadyRead(1000);
    }
}

void MainWindow::on_csim_generation_interval_selection_valueChanged(int arg1)
{
    csimHandle->generationInterval = arg1;
}


void MainWindow::on_pause_csim_button_clicked()
{
    if (csimHandle->pause)
    {
        csimHandle->pause=false;
        ui->pause_csim_button->setText("pause");
    }
    else
    {
       csimHandle->pause=true;
        ui->pause_csim_button->setText("unpause");
    }
}
#endif
