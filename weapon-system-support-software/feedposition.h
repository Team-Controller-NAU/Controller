#ifndef FEEDPOSITION_H
#define FEEDPOSITION_H

#include <QDial>
#include <QPainter>
#include <QColor>
#include <QToolTip>
#include <QEvent>
#include "constants.h"

/*
 * Adjust this value to make the feed postion alternate colors
 */
const bool alternatingColors = false;

class feedposition : public QDial
{
    Q_OBJECT

public:
    feedposition(QWidget * parent = nullptr);

private:
    virtual void paintEvent(QPaintEvent*) override;
    bool event(QEvent* event) override;

};
#endif // FEEDPOSITION_H
