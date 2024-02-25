#include "feedposition.h"

#include <QPainter>
#include <QColor>
#include <cmath>

feedposition::feedposition(QWidget* parent)
    : QDial(parent)
{
    // Default range
    QDial::setRange(0,100);
}

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
    painter.setPen(QPen(Qt::NoPen));

    //change brush color
    painter.setBrush(QBrush("#9747FF"));

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
        if(index % 2 == 0)
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

    //change brush color
    painter.setBrush(QBrush("#272727"));

    //draws center circle to match program background
    painter.drawEllipse(QPointF(QDial::height()/2,QDial::height()/2),QDial::height()/3,QDial::height()/3);

    //area of the pointer line
    QRectF positionLine(QDial::height()/8,  QDial::height()/8, QDial::height()/1.33, QDial::height()/1.33);

    //change brush color
    painter.setBrush(QBrush("#FFFFFF"));

    //draws knob for the pointer line
    painter.drawEllipse(QPointF(QDial::height()/2,QDial::height()/2),QDial::height()/20,QDial::height()/20);

    //draws the pointer line
    painter.drawPie(positionLine, value*16, 3*16);

    //setting text
    painter.setPen(text);
}
