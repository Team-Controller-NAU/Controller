#include <connection.h>
#include "mainwindow.h"
#include "csim.h"
#include "constants.h"
#include "./ui_mainwindow.h"

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

//sends user to developer page when clicked
void MainWindow::on_DevPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(1);
    resetPageButton();
    ui->DevPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;font: 16pt Segoe UI;");
}

//download button for events in CSV format
void MainWindow::on_download_button_clicked()
{
    // get current date
    //QString logFileName = QDateTime::currentDateTime().date().toString("MM-dd-yyyy");

    qint64 secsSinceEpoch = QDateTime::currentSecsSinceEpoch();
    QString logFileName = QString::number(secsSinceEpoch);

    // save logfile - mannually done
    events->outputToLogFile(logFileName.toStdString() + "-logfile-M.txt");
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
    // initialize slot
    EventNode* wkgErrPtr = events->headErrorNode;
    EventNode* wkgEventPtr = events->headEventNode;
    QString dumpMessage;
    bool printErr;

    // check for which filter
    switch(index)
    {
    case 0:

        qDebug() << "All filter selected";

        // set filter
        eventFilter = ALL;

        // reset dump
        dumpMessage = "";

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

        // update ui
        ui->events_output->setText(dumpMessage);

        break;

    case 1:

        qDebug() << "All events filter selected";

        // set filter
        eventFilter = EVENTS;

        // reset dump
        dumpMessage = "";

        // loop through all events
        while (wkgEventPtr != nullptr)
        {
            // set dump message
            if (dumpMessage != "") dumpMessage += '\n';
            dumpMessage += QString::number(wkgEventPtr->id) + ',' + wkgEventPtr->timeStamp + ',' + wkgEventPtr->eventString + ',';
            dumpMessage += "\n";
            wkgEventPtr = wkgEventPtr->nextPtr;
        }

        // update ui
        ui->events_output->setText(dumpMessage);

        break;

    case 2:

        qDebug() << "All errors filter selected";

        //set filter
        eventFilter = ERRORS;

        // reset dump
        dumpMessage = "";

        // loop through all errors
        while (wkgErrPtr != nullptr)
        {
            // set dump message
            if (dumpMessage != "") dumpMessage += '\n';
            dumpMessage += QString::number(wkgErrPtr->id) + ',' + wkgErrPtr->timeStamp + ',' + wkgErrPtr->eventString + ',';
            dumpMessage += (wkgErrPtr->cleared ? "1," : "0,");
            dumpMessage += "\n";
            wkgErrPtr = wkgErrPtr->nextPtr;
        }

        // update ui
        ui->events_output->setText(dumpMessage);

        break;

    case 3:

        qDebug() << "All cleared errors filter selected";

        // set filter
        eventFilter = CLEARED_ERRORS;

        // reset dump
        dumpMessage = "";

        // loop through all errors
        while (wkgErrPtr != nullptr)
        {
            // check for cleared
            if (wkgErrPtr->cleared)
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

        break;

    case 4:

        qDebug() << "All non-cleared errors filter selected";

        // set filter
        eventFilter = NON_CLEARED_ERRORS;

        // reset dump
        dumpMessage = "";

        // loop through all errors
        while (wkgErrPtr != nullptr)
        {
            // check for non-cleared
            if (!wkgErrPtr->cleared)
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

        break;

    default:

        // do nothing
        qDebug() << "Error: Unrecognized filter index.";
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
void MainWindow::on_handshake_button_clicked()
{
   // QPixmap redButton(":/resources/Images/redButton.png");

    // Check if the timer is started or ddmCon is not connected
    if ( !handshakeTimer->isActive() && !ddmCon->connected )
    {
        qDebug() << "Beginning handshake with controller" << Qt::endl;

        // Start the timer to periodically check the handshake status
        handshakeTimer->start();
        if(!lastMessageTimer->isActive())
        {
            lastMessageTimer->start();
        }

        timeLastReceived = QDateTime::currentDateTime();

        //refreshes connection button/displays
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
        if(lastMessageTimer->isActive())
        {
            lastMessageTimer->stop();
        }

        // update time since last message so its not frozen
        ui->DDMTimer->setText("Time Since Last Message: 00:00:00");
        ui->DDMTimer->setAlignment(Qt::AlignRight);

        //refreshes connection button/displays
        ui->handshake_button->setText("Connect");
        ui->handshake_button->setStyleSheet("color: rgb(255, 255, 255);border-color: rgb(255, 255, 255);background-color: #14AE5C;font: 15pt Segoe UI;");
        ui->connectionStatus->setPixmap(RED_LIGHT);
        ui->ddm_port_selection->setEnabled(true);

        //allow user to modify connection settings
        enableConnectionChanges();

        // check for not empty
        if(events->totalNodes != 0)
        {
            // new "session" ended, save to log file
            // qint64 secsSinceEpoch = QDateTime::currentSecsSinceEpoch();
            // QString logFileName = QString::number(secsSinceEpoch);

            // save logfile - autosave conditon
            //events->outputToLogFile(logFileName.toStdString() + "-logfile-A.txt");
        }

        ddmCon->connected = false;
    }
}

void MainWindow::on_output_messages_sent_button_clicked()
{
    //send request for csim to output its session string
    emit outputMessagesSentRequest();
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

    displaySavedConnectionSettings();
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

//sends user to settings page when clicked
void MainWindow::on_SettingsPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(2);
    resetPageButton();
    ui->SettingsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;font: 16pt Segoe UI;");
}

//sends user to status page when clicked
void MainWindow::on_StatusPageButton_clicked()
{
    ui->Flow_Label->setCurrentIndex(4);
    resetPageButton();
    ui->StatusPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: #9747FF;font: 16pt Segoe UI;");
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
    ui->SettingsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->EventsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->StatusPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->ElectricalPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->DevPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
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
            // check if can make path successfully
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
    userSettings.setValue("logfileLocation", QFileDialog::getExistingDirectory(this, tr("Create or Select a logfolder directory")));

    // check the success of saving settings
    if(userSettings.status() != QSettings::NoError)
    {
        qDebug() << "Error while saving user settings: " << userSettings.status();
    }
    // check if user exited the dialog box
    else if(userSettings.value("logfileLocation").toString() == "")
    {
        // revert to previous user setting
        userSettings.setValue("logfileLocation", previousPath);
        qDebug() << "No logfile directory set. Reverting to previous path: " << previousPath;
    }
    // otherwise, assume successful logfile directory creation
    else
    {
        qDebug() << "Successfully set user settings";
    }

    //sync user settings
    userSettings.sync();
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
