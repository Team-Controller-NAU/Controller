#include "feedposition.h"

/********************************************************************************
** feeposition.cpp
**
** This file implements the functionality of the feed position graphic located on the
** status page
**
** @author Team Controller
********************************************************************************/

/**
 * Initialization constructor for a feeposition object
 *
 * Sets maximum position in degrees
 *
 * @param parent Object used for GUI display
 */
feedposition::feedposition(QWidget* parent)
    : QDial(parent)
{
    //set feed pos to be measured in 360 degrees
    QDial::setMaximum(360);
}

/**
 * Paints the feed position graphic
 */
void feedposition::paintEvent(QPaintEvent*)
{
    //variables
    int startAngle;
    int index;

    //arc length of the dial backgournd
    int spanAngle = 45 * 16;

    //dial value
    int value = QDial::value();

    //painter object
    QPainter painter(this);
    QPen text = painter.pen();

    // Smooth out the circle
    painter.setRenderHint(QPainter::Antialiasing);

    // No border
    painter.setPen(QPen("000000"));

    //change brush color
    painter.setBrush(QBrush("#ff8533"));

    //area of the dial background
    QRectF dialBackground(0, 0, QDial::height(), QDial::height());

    //loops to create background
    for(index = 0; index < 8; index++)
    {
        //starting angle of the dial background section
        startAngle = (22.5 + (45 * index)) * 16;

        //draws the section of the dial background
        painter.drawPie(dialBackground, startAngle, spanAngle);

        //alternates color
        if(index > 5)
        {
            //change brush color
            painter.setBrush(QBrush("#ff8533"));
        }
        else
        {
            //change brush color
            painter.setBrush(QBrush("#9747FF"));
        }
    }

    painter.setPen(QPen(Qt::NoPen));

    //change brush color
    painter.setBrush(QBrush("#272727"));

    //draws center circle to match program background
    painter.drawEllipse(QPointF(QDial::height()/2,QDial::height()/2),QDial::height()/3,QDial::height()/3);

    //area of the pointer line
    QRectF positionLine(QDial::height()/8,  QDial::height()/8, QDial::height()/1.33, QDial::height()/1.33);

    //change brush color
    painter.setBrush(QBrush("#FFFFFF"));

    //draws knob for the pointer line
    painter.drawEllipse(QPointF(QDial::height()/2,QDial::height()/2),QDial::height()/10,QDial::height()/10);

    //draws the pointer line
    painter.drawPie(positionLine, -(value-90)*16, 3*16);

    //setting text
    painter.setPen(text);

    painter.setPen(QPen("#000000"));

    QFont textfont = painter.font();

    textfont.setPixelSize(QDial::height()/20);

    painter.setFont(textfont);

    if (value > 99)
    {
        painter.drawText(QDial::height()/2.15,QDial::height()/2.15,QDial::height(),QDial::height(),0,QString::number(value));
    }
    else if (value > 9)
    {
        painter.drawText((QDial::height()/2.15) + 2.5,QDial::height()/2.15,QDial::height(),QDial::height(),0,QString::number(value));
    }
    else
    {
        painter.drawText((QDial::height()/2.15) + 5,QDial::height()/2.15,QDial::height(),QDial::height(),0,QString::number(value));
    }

    //disable user edits on feed pos
    setDisabled(true);
}
