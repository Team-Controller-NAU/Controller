#ifndef FEEDPOSITION_H
#define FEEDPOSITION_H

#include <QDial>
#include <QMouseEvent>

class feedposition : public QDial
{
    Q_OBJECT

public:

    feedposition(QWidget * parent = nullptr);

private:

    virtual void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent *event) override;

};
#endif // FEEDPOSITION_H
