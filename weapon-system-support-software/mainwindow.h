#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtConcurrent>
#include <QDesktopServices>
#include <connection.h>
#include <csim.h>
#include <events.h>
#include <electrical.h>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //class for ddms serial communication
    Connection *ddmCon;

    //csim simulator is contained within this class. It makes thread that can be managed through the handle
    CSim *csimHandle;

    //data classes
    Status *status;
    Events *events;
    electrical *electricalObject;
    QSettings userSettings;

    // vars
    QString csimPortName;
    QString ddmPortName;
    QTimer* handshakeTimer;
    QTimer* lastMessageTimer;
    QDateTime timeLastReceived;
    EventFilter eventFilter;
    QString logfileName;

    //set true to enable automatic handshake once CSim sends closing connection message
    bool reconnect;

    //creates a new connection using currently selected serial port settings
    void createDDMCon();

    //logs empty line to qDebug() output
    void logEmptyLine();

    // Declare toString methods
    static QString toString(QSerialPort::BaudRate baudRate);
    static QString toString(QSerialPort::DataBits dataBits);
    static QString toString(QSerialPort::Parity parity);
    static QString toString(QSerialPort::StopBits stopBits);
    static QString toString(QSerialPort::FlowControl flowControl);

    // Declare fromString methods
    static QSerialPort::BaudRate fromStringBaudRate(QString baudRateStr);
    static QSerialPort::DataBits fromStringDataBits(QString dataBitsStr);
    static QSerialPort::Parity fromStringParity(QString parityStr);
    static QSerialPort::StopBits fromStringStopBits(QString stopBitsStr);
    static QSerialPort::FlowControl fromStringFlowControl(QString flowControlStr);

signals:
    //signal to be connected to csim's completeTransmissionRequest() slot
    //will tell csim to send the message
    void transmissionRequest(QString message);

    //signal will be connected to csims clearError() slot.
    void clearErrorRequest(int clearedId);

    //signal will be connected to csims outputSessionString() slot.
    void outputMessagesSentRequest();

private slots:
    //non-gui triggered slots (should be declared in mainwindow.cpp)
    void updateTimer();
    void readSerialData();
    void displaySavedConnectionSettings();
    void updateStatusDisplay();
    void handshake();
    void resetPageButton();
    void disableConnectionChanges();
    void enableConnectionChanges();
    void resetFiringMode();

    //non-gui triggered slots relating exclusively to managing gui (should be declared in mainwindow.cpp)
    void setup_csim_port_selection(int index);
    void setup_ddm_port_selection(int index);
    void update_non_cleared_error_selection();
    void setup_connection_settings();

    //gui triggered slots (should be declared in mainwindow_gui_slots.cpp)
    void on_send_message_button_clicked();
    void on_csim_port_selection_currentIndexChanged(int index);
    void on_CSim_button_clicked();
    void on_ddm_port_selection_currentIndexChanged(int index);
    void on_handshake_button_clicked();
    void on_SettingsPageButton_clicked();
    void on_EventsPageButton_clicked();
    void on_StatusPageButton_clicked();
    void on_ElectricalPageButton_clicked();
    void on_DevPageButton_clicked();
    void on_download_button_clicked();
    void on_clear_error_button_clicked();
    void on_baud_rate_selection_currentIndexChanged(int index);
    void on_stop_bit_selection_currentIndexChanged(int index);
    void on_flow_control_selection_currentIndexChanged(int index);
    void on_parity_selection_currentIndexChanged(int index);
    void on_data_bits_selection_currentIndexChanged(int index);
    void on_FilterBox_currentIndexChanged(int index);
    void on_output_messages_sent_button_clicked();
    void on_save_Button_clicked();
    void on_restore_Button_clicked();
    void on_openLogfileFolder_clicked();
    void on_setLogfileFolder_clicked();

    void on_toggle_num_triggers_clicked();

private:
    Ui::MainWindow *ui;

    QPixmap GREEN_LIGHT;
    QPixmap RED_LIGHT;
    QPixmap BLANK_LIGHT;
};
#endif // MAINWINDOW_H
