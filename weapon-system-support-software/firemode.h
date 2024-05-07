#ifndef FIREMODE_H
#define FIREMODE_H

#include <QDial>
#include <QMouseEvent>
#include <QPainter>
#include <QColor>

class firemode : public QDial
{
    Q_OBJECT

public:
    // constructor
    firemode(QWidget * parent = nullptr);

private:
    // firemode graphic painter
    virtual void paintEvent(QPaintEvent*) override;

};
#endif // FIREMODE_H
