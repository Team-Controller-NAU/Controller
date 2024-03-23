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
    void test_loadData();
    void test_loadVersionData();
};


/**
 * Test case for loadData() in status.cpp
 */
void tst_status::test_loadData()
{
    QString hello = "hello";
    QCOMPARE(hello.toUpper(), "HELLO");
}

/**
 * Test case for loadVersionData() in status.cpp
 */
void tst_status::test_loadVersionData()
{

}

/**
 * Test case for generateMessage() in status.cpp if we are in dev mode
 */
// TODO: How do get the scope of the DEV_MODE variable?

QTEST_MAIN(tst_status)
#include "tst_status.moc"
