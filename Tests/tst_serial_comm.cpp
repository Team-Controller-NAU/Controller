#include <QTest>
#include "../weapon-system-support-software/connection.h"

// add necessary includes here
class tst_serial_comm : public QObject
{
    friend class Connection;
    Q_OBJECT

private slots:
    void test_transmit();
};


void test_transmit()
{
    Connection *sender = new Connection("Com4", QSerialPort::Baud9600, QSerialPort::Data8,
               QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl);

    Connection *receiver = new Connection("Com5",QSerialPort::Baud9600, QSerialPort::Data8,
               QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl);

    QVERIFY(sender != nullptr);
    QVERIFY(receiver != nullptr);

    QString stringToTransmit = "Hello world!";

    sender->transmit(stringToTransmit);

    //allow time for transmit
    sender->serialPort.waitForReadyRead(1000);

    QString stringReceived = QString::fromUtf8(receiver->serialPort.readLine());

    QCOMPARE(stringToTransmit, stringReceived);

    delete sender;
    delete receiver;
}
QTEST_MAIN(tst_serial_comm)
#include "tst_serial_comm.moc"
