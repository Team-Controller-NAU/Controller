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
        if (ddmCon->connected) ddmCon->transmit(QString::number(static_cast<int>(CLOSING_CONNECTION)) + '\n');
        delete ddmCon;

        //open new connection
        ddmCon = new Connection(ddmPortName);

        //set up signal and slot (when a message is sent to DDMs serial port, the readyRead signal is emitted and
        //readSerialData() is called)
        connect(&ddmCon->serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

        qDebug() << "GUI is now listening to port " << ddmCon->portName;
    }
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

//toggles handshake process on and off. Once connected, allow for disconnect (send disconnect message to controller)
void MainWindow::on_handshake_button_clicked()
{
    QPixmap redButton(":/resources/Images/redButton.png");

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
        ui->connectionStatus->setPixmap(redButton);
        ui->ddm_port_selection->setEnabled(true);

        //allow user to modify connection settings
        enableConnectionChanges();

        // check for not empty
        if(events->totalNodes != 0)
        {
            // new "session" ended, save to log file
            qint64 secsSinceEpoch = QDateTime::currentSecsSinceEpoch();
            QString logFileName = QString::number(secsSinceEpoch);

            // save logfile - autosave conditon
            events->outputToLogFile(logFileName.toStdString() + "-logfile-A.txt");
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
        std::unique_ptr<Connection> conn(new Connection(csimPortName));

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

//reset all tab buttons to default style
void MainWindow::resetPageButton()
{
    ui->SettingsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->EventsPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->StatusPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->ElectricalPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
    ui->DevPageButton->setStyleSheet("color: rgb(255, 255, 255);background-color: rgb(39, 39, 39);font: 16pt Segoe UI;");
}
