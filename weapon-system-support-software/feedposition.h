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

};
#endif // FEEDPOSITION_H
