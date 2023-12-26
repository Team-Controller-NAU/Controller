#include <connection.h>
#include "mainwindow.h"
#include "csim.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)

    //initialize imbedded classes
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    DDM_Con(new Connection("COM5"))

{
    ui->setupUi(this);

    //initialize classes needed for program here...

    //init csim class, set to send data to com4
    cSim_Handle = new CSim();
    cSim_Handle->portName = "COM4";

    //set up signal and slot (when a message is sent to DDMs serial port, the readyRead signal is emitted and
    //readSerialData() is called
    connect(&DDM_Con->serialPort, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

    qDebug() << "GUI is now listening to port " << DDM_Con->portName;

}


MainWindow::~MainWindow()
{
    //call destructors for classes declared in main window
    delete ui;
    delete DDM_Con;
    delete cSim_Handle;
}

//button toggles the csim utility on and off. first click will setup thread and run csim. second will terminate thread and close csim.
void MainWindow::on_CSim_button_clicked()
{
    //check if csim is currently running
    if (cSim_Handle->isRunning())
    {
        //csim is running, shut it down
        cSim_Handle->stop = true;
        ui->CSim_button->setText("Start CSim");
        cSim_Handle->quit();
        cSim_Handle->wait();

        //enable custom message inputs
        ui->send_message_button->setEnabled(true);
    }
    else
    {
        //csim is not running, start it

        //set button to display the option to stop csim
        ui->CSim_button->setText("Stop CSim");

        //start csim
        cSim_Handle->startCSim("COM4");

        //temporarily disable custom message inputs
        ui->send_message_button->setEnabled(false);
    }
}

//sends custom user input message
void MainWindow::on_send_message_button_clicked()
{
    //open new connection on com4
    std::unique_ptr<Connection> conn(new Connection("COM4"));

    //get user input from input box
    QString userInput = ui->message_contents->text();

    //add new line for data parsing
    userInput += '\n';

    //clear the contents of input box
    ui->message_contents->clear();

    //send message through com4
    conn->transmit(userInput);
}

//this function is called as a result of the readyRead signal being emmited by a connected serial port
//in other words, this function is called whenever com5 receives a new message
void MainWindow::readSerialData()
{
    //ensure port is open to prevent possible errors
    if (DDM_Con->serialPort.isOpen())
    {
        //get serialized string from port
        QByteArray message = DDM_Con->serialPort.readLine();

        //deserialize string
        QString messageString = QString::fromUtf8(message);

        //log
        qDebug() << "signal received from port : " << DDM_Con->portName << ", updating gui with new message: " <<  messageString << qPrintable("\n");

        //update gui with new message
        ui->stdoutLabel->setText(messageString);
    }
    else
    {
        qDebug() << "Serial port is closed, unable to read data";
    }
}

