#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <connection.h>
#include <QtConcurrent>
#include <csim.h>
#include <events.h>

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

    // vars
    QString csimPortName;
    QString ddmPortName;
    QTimer* handshakeTimer;
    //set true to enable automatic handshake once CSim sends closing connection message
    bool reconnect;


private slots:
    //all gui slots go here. Any time an event happens, you may connect to a slot here
    //and the mainwindow class can execute functions based on events.
    void on_CSim_button_clicked();
    void readSerialData();
    void on_send_message_button_clicked();

    void on_csim_port_selection_currentIndexChanged(int index);

    void setup_csim_port_selection(int index);

    void setup_ddm_port_selection(int index);

    void on_ddm_port_selection_currentIndexChanged(int index);

    void on_handshake_button_clicked();
    void handshake();

    void on_SettingsPageButton_clicked();

    void on_EventsPageButton_clicked();

    void on_StatusPageButton_clicked();

    void on_ElectricalPageButton_clicked();

    void on_DevPageButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
