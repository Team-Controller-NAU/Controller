#include <QCoreApplication>
#include <QTest>
//#include "../weapon-system-support-software/electrical.cpp"

// add necessary includes here

class tst_status : public QObject
{
    Q_OBJECT

public:

private:


private slots:
    void wawa();
};



void tst_status::wawa()
{
    QString hello = "hello";
    QCOMPARE(hello, hello.toUpper());
}

QTEST_MAIN(tst_status)
#include "tst_status.moc"
