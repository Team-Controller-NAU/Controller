#include "mainwindow.h"

//This file contains gui slots and functions specific to the connection settings
//portion of the application. These settings are dependant on QT and care should be taken
//when altering this file. The settings found here are what QT offers in its serial port
//class, if the setting you want is not found here, its most likely not supported by QTs libraries.
//Note that the toString() and fromString() methods will cause intentional program termination if
//unsupported serial settings are encountered.

//======================================================================================
// GUI Drop boxes
//======================================================================================

//initial setup
void MainWindow::setupConnectionPage()
{
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

void MainWindow::on_baud_rate_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized after setup is done
        if (allowSettingChanges)
        {
            notifyUser("Connection setting can't be applied", "Connection class is not declared", true);
        }
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

void MainWindow::on_data_bits_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized after setup is done
        if (allowSettingChanges)
        {
            notifyUser("Connection setting can't be applied", "Connection class is not declared", true);
        }
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

void MainWindow::on_flow_control_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized after setup is done
        if (allowSettingChanges)
        {
            notifyUser("Connection setting can't be applied", "Connection class is not declared", true);
        }
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

void MainWindow::on_stop_bit_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized after setup is done
        if (allowSettingChanges)
        {
            notifyUser("Connection setting can't be applied", "Connection class is not declared", true);
        }
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

void MainWindow::on_parity_selection_currentIndexChanged(int index)
{
    if (ddmCon == nullptr) {
        // Handle case where ddmCon pointer is not initialized after setup is done
        if (allowSettingChanges)
        {
            notifyUser("Connection setting can't be applied", "Connection class is not declared", true);
        }
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
