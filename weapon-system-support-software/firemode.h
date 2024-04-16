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

    firemode(QWidget * parent = nullptr);

private:

    virtual void paintEvent(QPaintEvent*) override;

};
#endif // FIREMODE_H
