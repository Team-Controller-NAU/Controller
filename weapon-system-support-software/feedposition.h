#ifndef FEEDPOSITION_H
#define FEEDPOSITION_H

#include <QDial>

class feedposition : public QDial
{
    Q_OBJECT

public:

    feedposition(QWidget * parent = nullptr);

private:

    virtual void paintEvent(QPaintEvent*) override;

};
#endif // FEEDPOSITION_H
