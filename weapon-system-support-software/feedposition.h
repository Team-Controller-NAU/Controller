#ifndef FEEDPOSITION_H
#define FEEDPOSITION_H

#include <QDial>
#include "constants.h"
#include <QMouseEvent>
#include <QPainter>
#include <QColor>

class feedposition : public QDial
{
    Q_OBJECT

public:

    feedposition(QWidget * parent = nullptr);

private:

    virtual void paintEvent(QPaintEvent*) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

};
#endif // FEEDPOSITION_H
