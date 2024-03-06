#include <QCoreApplication>
#include <QTest>

// add necessary includes here

class foo : public QObject
{
    Q_OBJECT

public:
    foo();
    ~foo();

private slots:
    void test_case1();
};

foo::foo() {}

foo::~foo() {}

void foo::test_case1()
{
    QString str = "Hello";
    QCOMPARE(str.toUpper(), QString("HELLO"));
}

QTEST_MAIN(foo)
#include "tst_foo.moc"
