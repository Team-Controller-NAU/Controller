#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <connection.h>
#include <QtConcurrent>
#include <csim.h>

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
    //listener, messages sent to this connections port will be displayed
    Connection *DDM_Con;

    //csim simulator is contained within this class. It makes thread that can be managed through the handle
    CSim *cSim_Handle;

private slots:
    //all gui slots go here. Any time an event happens, you may connect to a slot here
    //and the mainwindow class can execute functions based on events.
    void on_CSim_button_clicked();
    void readSerialData();
    void on_send_message_button_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
