#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//QT libraries
#include <QMainWindow>
#include <QDesktopServices>
#include <QShortcut>
#include <QTextCursor>
#include <QMessageBox>
#include <QInputDialog>
#include <QObject>
#include <QtCore>
#include <QTextDocument>

//Team Controller code
#include "constants.h"
#include "connection.h"
#include "events.h"
#include "status.h"
#include "electrical.h"
#include "./ui_mainwindow.h"


#if DEV_MODE
    #include "csim.h"
    #include <QtConcurrent/QtConcurrent>
#endif

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

    //data classes
    Status *status;
    Events *events;
    electrical *electricalData;
    QSettings userSettings;

    // vars
    QString ddmPortName;
    QTimer* handshakeTimer;
    QTimer* lastMessageTimer;
    QTimer* runningControllerTimer;
    QTimer* notificationTimer;
    QTimer *handshakeCooldownTimer;
    QDateTime timeLastReceived;
    EventFilter eventFilter;
    QString autosaveLogFile;
    bool allowSettingChanges;

    //user managed settings
    bool coloredEventOutput;
    int autoSaveLimit;
    int connectionTimeout;
    bool advancedLogFile;
    bool notifyOnErrorCleared;

    //creates a new connection using currently selected serial port settings
    void createDDMCon();

    #if DEV_MODE
        //csim simulator is contained within this class. It makes thread that can be managed through the handle
        CSim *csimHandle;
        QString csimPortName;
        //logs empty line to qDebug() output
        void logEmptyLine();
    #endif

signals:
    #if DEV_MODE
        //signal to be connected to csim's completeTransmissionRequest() slot
        //will tell csim to send the message
        void transmissionRequest(QString message);

        //signal will be connected to csims clearError() slot.
        void clearErrorRequest(int clearedId);

        //signal will be connected to csims outputSessionString() slot.
        void outputMessagesSentRequest();
    #endif

private slots:
    //==========================================================================
    //non-gui triggered slots (should be defined in mainwindow.cpp)
    //==========================================================================
    void updateTimeSinceLastMessage();
    void notifyUser(QString notificationText, bool error);
    void notifyUser(QString notificationText, QString logText, bool error);
    void updateElapsedTime();
    void readSerialData();
    void updateStatusDisplay();
    void handshake();
    void resetPageButton();
    void disableConnectionChanges();
    void enableConnectionChanges();
    void updateEventsOutput(EventNode *event);
    //clears current content of the events page text output and replaces
    //it with freshly generated data based on current contents of events class
    void refreshEventsOutput();
    //clears error in events output
    void clearErrorFromEventsOutput(int errorId);
    //checks if the current number of auto saved files is higher than the limit, deletes the
    //oldest one each iteration until the limit is enforced
    void enforceAutoSaveLimit();
    void updateConnectionStatus(bool connectionStatus);
    QString getSessionStatistics();
    void logAdvancedDetails(SerialMessageIdentifier id);
    void handleRAMClear();

    #if DEV_MODE
        void displaySavedSettings();
    #endif
    //==================================================================================


    //========================================================================================================
    //non-gui triggered slots relating exclusively to managing gui (should be defined in mainwindow.cpp)
    //=======================================================================================================
    #if DEV_MODE
        void setup_csim_port_selection(int index);
        void update_non_cleared_error_selection();
    #endif
    void setup_ddm_port_selection(int index);
    void setup_logfile_location();
    void setupSettings();
    void setupConnectionPage();
    void renderElectricalPage();
    void addElecBox(QWidget *horizontalWidget, QLayout *horizontalLayout, electricalNode *component);
    void freeElectricalPage();
    //========================================================================================================


    //================================================================================
    //gui triggered slots (should be defined in mainwindow_gui_slots.cpp)
    //====================================================================================
    //navigation
    void on_ConnectionPageButton_clicked();
    void on_EventsPageButton_clicked();
    void on_StatusPageButton_clicked();
    void on_ElectricalPageButton_clicked();
    void on_NotificationPageButton_clicked();
    void on_SettingsPageButton_clicked();

    //general
    void on_ddm_port_selection_currentIndexChanged(int index);
    void on_handshake_button_clicked();
    void on_download_button_clicked();
    void on_searchButton_clicked();
    void on_save_Button_clicked();
    void on_restore_Button_clicked();
    void on_openLogfileFolder_clicked();
    void on_setLogfileFolder_clicked();
    void on_load_events_from_logfile_clicked();
    void on_refresh_serial_port_selections_clicked();

    //settings
    void on_advanced_log_file_stateChanged(int arg1);
    void on_colored_events_output_stateChanged(int arg1);
    void on_auto_save_limit_valueChanged(int arg1);
    void on_notify_error_cleared_stateChanged(int arg1);
    void on_connection_timeout_valueChanged(int arg1);
    void on_ram_clearing_stateChanged(int arg1);
    void on_max_data_nodes_valueChanged(int arg1);

    #if (DEV_MODE)
        void on_send_message_button_clicked();
        void on_csim_port_selection_currentIndexChanged(int index);
        void on_CSim_button_clicked();
        void on_clear_error_button_clicked();
        void on_DevPageButton_clicked();
        void on_output_messages_sent_button_clicked();
        void on_toggle_num_triggers_clicked();
        void on_csim_generation_interval_selection_valueChanged(int arg1);
        void on_pause_csim_button_clicked();
    #endif
    //=========================================================================================================

    //================================================================================
    //connection settings (should be defined in mainwindow_connection_settings.cpp)
    //================================================================================

    void on_baud_rate_selection_currentIndexChanged(int index);
    void on_stop_bit_selection_currentIndexChanged(int index);
    void on_flow_control_selection_currentIndexChanged(int index);
    void on_parity_selection_currentIndexChanged(int index);
    void on_data_bits_selection_currentIndexChanged(int index);
    void on_FilterBox_currentIndexChanged(int index);

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
    //=========================================================================================================

private:
    Ui::MainWindow *ui;

    QPixmap GREEN_LIGHT;
    QPixmap RED_LIGHT;
    QPixmap BLANK_LIGHT;
    QPixmap ORANGE_LIGHT;
};
#endif // MAINWINDOW_H
