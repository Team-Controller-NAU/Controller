#include "mainwindow.h"

//this file contains only the implementation of the GUI slots. Declare
//other processing functions in mainwindow.cpp

void MainWindow::on_baud_rate_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized
        return;
    }

    switch (index)
    {
    case 0:
        ddmCon->serialPort.setBaudRate(QSerialPort::Baud1200);
        break;
    case 1:
        ddmCon->serialPort.setBaudRate(QSerialPort::Baud2400);
        break;
    case 2:
        ddmCon->serialPort.setBaudRate(QSerialPort::Baud4800);
        break;
    case 3:
        ddmCon->serialPort.setBaudRate(QSerialPort::Baud9600);
        break;
    case 4:
        ddmCon->serialPort.setBaudRate(QSerialPort::Baud19200);
        break;
    case 5:
        ddmCon->serialPort.setBaudRate(QSerialPort::Baud38400);
        break;
    case 6:
        ddmCon->serialPort.setBaudRate(QSerialPort::Baud57600);
        break;
    case 7:
        ddmCon->serialPort.setBaudRate(QSerialPort::Baud115200);
        break;
    default:
        // Handle default case
        break;
    }
}

// slot for find functionality within the events_output box when the user presses CTRL+F on their keyboard
void MainWindow::findText()
{
    // make sure the user is in the events_output box
    if (qApp->focusWidget() != ui->events_output) return;

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

void MainWindow::on_data_bits_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized
        return;
    }

    switch (index)
    {
    case 0:
        ddmCon->serialPort.setDataBits(QSerialPort::Data5);
        break;
    case 1:
        ddmCon->serialPort.setDataBits(QSerialPort::Data6);
        break;
    case 2:
        ddmCon->serialPort.setDataBits(QSerialPort::Data7);
        break;
    case 3:
        ddmCon->serialPort.setDataBits(QSerialPort::Data8);
        break;
    default:
        // Handle default case
        break;
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

    //create connection class
    createDDMCon();
}

//download button for events in CSV format
void MainWindow::on_download_button_clicked()
{
    QString logFile;

    //check if user has set a custom log file output directory
    if ( !userSettings.value("portName").toString().isEmpty() )
    {
        //initialize the logfile into this directory
        logFile = userSettings.value("logfileLocation").toString();
    }
    //otherwise use default directory
    else
    {
        //use the path of the exe and add a "Log Files" directory
        logFile = QCoreApplication::applicationDirPath() + "/" + INITIAL_LOGFILE_LOCATION;
    }

    qint64 secsSinceEpoch = QDateTime::currentSecsSinceEpoch();
    logFile += QString::number(secsSinceEpoch);

    // save logfile - mannually done
    events->outputToLogFile(logFile + "-logfile-M.txt");
}

//sends user to electrical page when clicked
void MainWindow::on_ElectricalPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(3);
    resetPageButton();
    ui->ElectricalPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;font: 16pt Segoe UI;");
}

//sends user to events page when clicked
void MainWindow::on_EventsPageButton_clicked()
{
    // TODO: first visit refresh page with dump of whole LL??
    ui->Flow_Label->setCurrentIndex(0);
    resetPageButton();
    ui->EventsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;font: 16pt Segoe UI;");
}

void MainWindow::on_FilterBox_currentIndexChanged(int index)
{
    // check for which filter the user selected
    switch(index)
    {
    case ALL:

        qDebug() << "All filter selected";

        // set filter
        eventFilter = ALL;

        break;

    case EVENTS:

        qDebug() << "All events filter selected";

        // set filter
        eventFilter = EVENTS;

        break;

    case ERRORS:

        qDebug() << "All errors filter selected";

        //set filter
        eventFilter = ERRORS;

        break;

    case CLEARED_ERRORS:

        qDebug() << "All cleared errors filter selected";

        // set filter
        eventFilter = CLEARED_ERRORS;

        break;

    case NON_CLEARED_ERRORS:

        qDebug() << "All non-cleared errors filter selected";

        // set filter
        eventFilter = NON_CLEARED_ERRORS;

        break;

    default:

        // do nothing
        qDebug() << "Error: Unrecognized filter index.";
    }

    refreshEventsOutput();
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
        ddmCon->serialPort.setFlowControl(QSerialPort::NoFlowControl);
        break;
    case 1:
        ddmCon->serialPort.setFlowControl(QSerialPort::HardwareControl);
        break;
    case 2:
        ddmCon->serialPort.setFlowControl(QSerialPort::SoftwareControl);
        break;
    default:
        // Handle default case
        break;
    }
}

//toggles handshake process on and off. Once connected, allow for disconnect (send disconnect message to controller)
//this button is seen as connect/connecting/disconnect on connection page
void MainWindow::on_handshake_button_clicked()
{
    // Check if the timer is started or ddmCon is not connected
    if ( !handshakeTimer->isActive() && !ddmCon->connected )
    {
        qDebug() << "Beginning handshake with controller" << Qt::endl;

        // Start the timer to periodically check the handshake status
        handshakeTimer->start();

        //refreshes connection button/displays
        ui->handshake_button->setText("Connecting");
        ui->handshake_button->setStyleSheet("QPushButton { padding-bottom: 3px; color: rgb(255, 255, 255); background-color: #FF7518; border: 1px solid; border-color: #e65c00; font: 15pt 'Segoe UI'; } "
                                            "QPushButton::hover { background-color: #ff8533; } "
                                            "QPushButton::pressed { background-color: #ffa366;}");

        //disable changes to connection settings
        disableConnectionChanges();
    }
    else
    {
        qDebug() << "Sending disconnect message to controller" << Qt::endl;

        ddmCon->transmit(QString::number(CLOSING_CONNECTION) + '\n');

        //update connection status to disconnected and update related objects
        updateConnectionStatus(false);
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
        ddmCon->serialPort.setParity(QSerialPort::NoParity);
        break;
    case 1:
        ddmCon->serialPort.setParity(QSerialPort::EvenParity);
        break;
    case 2:
        ddmCon->serialPort.setParity(QSerialPort::OddParity);
        break;
    case 3:
        ddmCon->serialPort.setParity(QSerialPort::SpaceParity);
        break;
    case 4:
        ddmCon->serialPort.setParity(QSerialPort::MarkParity);
        break;
    default:
        // do nothing
        break;
    }
}

//saves user settings into the qSettings class for cross session storage
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

    //output new settings to qDebug()
    displaySavedSettings();
}

//sends user to settings page when clicked
void MainWindow::on_ConnectionPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(2);
    resetPageButton();
    ui->ConnectionPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;font: 16pt Segoe UI;");
}

//sends user to status page when clicked
void MainWindow::on_StatusPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(4);
    resetPageButton();
    ui->StatusPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;font: 16pt Segoe UI;");
}

void MainWindow::on_SettingsPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(5);
    resetPageButton();
    ui->SettingsPageButton->setStyleSheet("border-image: url(://resources/Images/purpleSettings.png);");
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
        ddmCon->serialPort.setStopBits(QSerialPort::OneStop);
        break;
    case 1:
        ddmCon->serialPort.setStopBits(QSerialPort::OneAndHalfStop);
        break;
    case 2:
        ddmCon->serialPort.setStopBits(QSerialPort::TwoStop);
        break;
    default:
        // Handle default case
        break;
    }
}

//reset all tab buttons to default style
void MainWindow::resetPageButton()
{
    ui->ConnectionPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->EventsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->StatusPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->ElectricalPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->SettingsPageButton->setStyleSheet("border-image: url(://resources/Images/whiteSettings.png)");

    #if DEV_MODE
        ui->DevPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    #endif
}

//restores connection settings to the values saved to the system as default
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

    //restart connection
    createDDMCon();
}

void MainWindow::on_openLogfileFolder_clicked()
{
    // if the user hasnt set log file location user settings, temp folder is opened
    if (userSettings.value("logfileLocation").toString().isEmpty())
    {
        // create a QDir object for the path
        QDir path(INITIAL_LOGFILE_LOCATION);

        // check if the path does not lead to anything
        if(!path.exists())
        {
            // check if we can make path successfully
            if(path.mkpath("."))
            {
                qDebug() << "Logfile directory created at: " << INITIAL_LOGFILE_LOCATION;
            }

            else
            {
                qDebug() << "The log file directory failed to create";
            }
        }
        QDesktopServices::openUrl(QUrl::fromLocalFile(INITIAL_LOGFILE_LOCATION));
    }

    else
    {
        //open from the user settings
        QDesktopServices::openUrl(QUrl::fromLocalFile(userSettings.value("logfileLocation").toString()));
        qDebug() << "log file location opening: " << userSettings.value("logfileLocation").toString();
    }
    // save user settings
    userSettings.sync();
}

void MainWindow::on_setLogfileFolder_clicked()
{
    // save prev user settings value
    QString previousPath = userSettings.value("logfileLocation").toString();

    // set logfile location with the user choice
    userSettings.setValue("logfileLocation", QFileDialog::getExistingDirectory(this, tr("Create or Select a logfolder directory")) + "/");

    // check the success of saving settings
    if(userSettings.status() != QSettings::NoError)
    {
        qDebug() << "Error while saving user settings: " << userSettings.status();
    }
    // check if user exited the dialog box
    else if(userSettings.value("logfileLocation").toString() == "/")
    {
        // revert to previous user setting
        userSettings.setValue("logfileLocation", previousPath);
        qDebug() << "No logfile directory set. Reverting to previous path: " << previousPath;
    }
    // otherwise, assume successful logfile directory creation
    else
    {
        qDebug() << "New log file directory set: " << userSettings.value("logfileLocation").toString();
    }

    //sync user settings
    userSettings.sync();
}

//opens log file directory to prompt user to select log file. data from log file is then
//loaded into events class and rendered in events page.
void MainWindow::on_load_events_from_logfile_clicked()
{
    //declare file browser class
    QFileDialog dialog(this);

    //set initial directory to log file directory set by user
    dialog.setDirectory(userSettings.value("logfileLocation").toString());

    // Open a file dialog for the user to select a logfile
    QString selectedFile = dialog.getOpenFileName(this, tr("Select Log File"), QString(), tr("Log Files (*.txt);;All Files (*)"));

    // Check if the user canceled the dialog
    if (selectedFile.isEmpty())
    {
        return;
    }

    logEmptyLine();
    qDebug() << "Loading data from: " << selectedFile;

    // Pass the selected file name to the loadDataFromLogFile function
    int result = events->loadDataFromLogFile(events, selectedFile);

    // Handle the result if needed
    if (result == INCORRECT_FORMAT)
    {
        // Handle error
        qDebug() << "Log file was of incorrect format.";
    }
    else if (result == DATA_NOT_FOUND)
    {
        qDebug() << "Log file could not be found";
    }

    //refresh the events output
    refreshEventsOutput();
}

//toggle colored events output (from settings page)
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
    refreshEventsOutput();
}

//choose the number of auto save log files before overwrites occur (from settings page)
void MainWindow::on_auto_save_limit_valueChanged(int arg1)
{
    autoSaveLimit = arg1;
    userSettings.setValue("autoSaveLimit", autoSaveLimit);
}

//======================================================================================
//DEV_MODE exclusive methods
//======================================================================================

#if DEV_MODE
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

        // stop ddm timer
        lastMessageTimer->stop();

        // update ui
        ui->CSim_button->setText("Start CSim");

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

//sends user to developer page when clicked
void MainWindow::on_DevPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(1);
    resetPageButton();
    ui->DevPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;font: 16pt Segoe UI;");
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
    }
}
#endif
