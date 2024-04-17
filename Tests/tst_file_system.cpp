#include <QCoreApplication>
#include <QTest>
#include "../weapon-system-support-software/events.cpp"

class tst_file_system : public QObject
{
    Q_OBJECT

private slots:
    void tst_outputToLogFile();
};

void tst_file_system::tst_outputToLogFile()
{
    Events *eventObj = new Events();


    delete eventObj;
}

QTEST_MAIN(tst_file_system)
#include "tst_file_system.moc"
