#if DEV_MODE
#include <QTest>
#include "../weapon-system-support-software/connection.cpp"

// add necessary includes here
class tst_serial_comm : public QObject
{
    //friend class Connection;
    Q_OBJECT

public:
    QSettings userSettings;
    tst_serial_comm();

private:

private slots:
    void test_transmit();

    void test_checkForValidMessage();
};

tst_serial_comm::tst_serial_comm()
    : userSettings("Team Controller", "WSSS")
{}

void tst_serial_comm::test_transmit()
{
    Connection *sender = new Connection(userSettings.value("portName").toString(), QSerialPort::Baud9600, QSerialPort::Data8,
               QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl);

    Connection *receiver = new Connection(userSettings.value("csimPortName").toString(),QSerialPort::Baud9600, QSerialPort::Data8,
               QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl);

    QVERIFY(sender != nullptr);
    QVERIFY(receiver != nullptr);

    QString stringToTransmit = "Hello world!\n";

    sender->transmit(stringToTransmit);

    //allow time for transmit
    sender->serialPort.waitForReadyRead(100);
    receiver->serialPort.waitForReadyRead(100);

    QString stringReceived = QString::fromUtf8(receiver->serialPort.readLine());

    QCOMPARE(stringToTransmit, stringReceived);

    delete sender;
    delete receiver;
}

void tst_serial_comm::test_checkForValidMessage()
{
    Connection *sender = new Connection(userSettings.value("portName").toString(), QSerialPort::Baud9600, QSerialPort::Data8,
                                        QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl);

    Connection *receiver = new Connection(userSettings.value("csimPortName").toString(),QSerialPort::Baud9600, QSerialPort::Data8,
                                          QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::NoFlowControl);

    QCOMPARE(EMPTY_BUFFER, receiver->checkForValidMessage());

    QString validMessage = "Hello world\n";
    QString invalidMessage = "Hello world";

    sender->transmit(invalidMessage);

    //allow time for transmit
    sender->serialPort.waitForReadyRead(100);
    receiver->serialPort.waitForReadyRead(100);

    QCOMPARE(UNTERMINATED_MESSAGE, receiver->checkForValidMessage());

    sender->transmit(validMessage);

    //allow time for transmit
    sender->serialPort.waitForReadyRead(100);
    receiver->serialPort.waitForReadyRead(100);

    QCOMPARE(VALID_MESSAGE, receiver->checkForValidMessage());

    delete sender;
    delete receiver;
}



QTEST_MAIN(tst_serial_comm)
#include "tst_serial_comm.moc"
#endif
