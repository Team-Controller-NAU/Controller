#include "firemode.h"
/********************************************************************************
** firemode.cpp
**
** This class implements the logic to draw/re-draw the firemode graphic on the
** status page.
**
** @author Team Controller
********************************************************************************/


/**
 * Initialization constructor for a firemode object
 *
 * Sets maximum position in degrees
 *
 * @param parent Object used for GUI display
 */
firemode::firemode(QWidget* parent)
    : QDial(parent)
{
    //set feed pos to be measured in 360 degrees
    QDial::setMaximum(360);
}

/**
 * Paints the firemode graphic
 */
void firemode::paintEvent(QPaintEvent*)
{
    //dial value
    int value = QDial::value();

    //painter object
    QPainter painter(this);
    QPen text = painter.pen();
    QPen lines = painter.pen();

    // Smooth out the circle
    painter.setRenderHint(QPainter::Antialiasing);

    lines.setColor(QColor(255, 133, 51));
    lines.setWidth(3);

    painter.setPen(lines);

    painter.setBrush(QBrush(QColor(27,27,27)));

    QRectF outlineArea(-QDial::height()/14, QDial::height()/22, QDial::height()*1.55, QDial::height()*1.55);

    //draws the outline
    painter.drawPie(outlineArea, -(62.5-90)*16, 125*16);

    painter.drawLine(QDial::height()/1.44, QDial::height()/20,QDial::height()/1.44,QDial::height()/5);

    // No border
    painter.setPen(QPen(Qt::NoPen));

    //area of the needle
    QRectF needleArea(QDial::height()/4.31,  QDial::height()/3.2, QDial::height()/1.05, QDial::height()/1.05);

    //change brush color
    painter.setBrush(QBrush(QColor(255, 255, 255))); // #FFFFFF color represented by RGB integers

    //draws knob for the needle
    painter.drawEllipse(QPointF(QDial::height()/1.414,QDial::height()/1.28),QDial::height()/15,QDial::height()/15);

    //draws the needle
    painter.drawPie(needleArea, -(value-90)*16, 3*16);

    //setting text
    painter.setPen(text);

    painter.setPen(QPen(QColor(255, 255, 255))); // White color represented by RGB integers

    QFont textfont = painter.font();

    textfont.setPixelSize(QDial::height()/10);

    painter.setFont(textfont);

    painter.drawText(QDial::height()/13,QDial::height()/3,QDial::height(),QDial::height(),0,"Safe");

    painter.drawText(QDial::height()/3.5,QDial::height()/6.7,QDial::height(),QDial::height(),0,"Single");

    painter.drawText(QDial::height()/1.2,QDial::height()/6.7,QDial::height(),QDial::height(),0,"Burst");

    painter.drawText(QDial::height()*1.13,QDial::height()/3,QDial::height(),QDial::height(),0,"Auto");

    painter.setPen(QPen(QColor(255, 133, 51))); // #ff8533 color represented by RGB integers

    if(value == 305)
    {
        painter.drawText(QDial::height()/13,QDial::height()/3,QDial::height(),QDial::height(),0,"Safe");
    }
    else if(value == 330)
    {
        painter.drawText(QDial::height()/3.5,QDial::height()/6.7,QDial::height(),QDial::height(),0,"Single");
    }
    else if(value == 30)
    {
        painter.drawText(QDial::height()/1.2,QDial::height()/6.7,QDial::height(),QDial::height(),0,"Burst");
    }
    else if(value == 55)
    {
        painter.drawText(QDial::height()*1.13,QDial::height()/3,QDial::height(),QDial::height(),0,"Auto");
    }

    //disable user edits on feed pos
    setDisabled(true);
}

