#include "firemode.h"

firemode::firemode(QWidget* parent)
    : QDial(parent)
{
    //set feed pos to be measured in 360 degrees
    QDial::setMaximum(360);
}

void firemode::paintEvent(QPaintEvent*)
{
    //dial value
    int value = QDial::value();

    //painter object
    QPainter painter(this);
    QPen text = painter.pen();

    // Smooth out the circle
    painter.setRenderHint(QPainter::Antialiasing);

    // No border
    painter.setPen(QPen(QColor(0, 0, 0))); // Black color represented by RGB integers

    //change brush color
    painter.setBrush(QBrush(QColor(255, 133, 51))); // #ff8533 color represented by RGB integers

    painter.setPen(QPen(Qt::NoPen));

    //change brush color
    painter.setBrush(QBrush(QColor(39, 39, 39))); // #272727 color represented by RGB integers

    //area of the pointer line
    QRectF positionLine(QDial::height()/4.9,  QDial::height()/4.55, QDial::height(), QDial::height());

    //change brush color
    painter.setBrush(QBrush(QColor(255, 255, 255))); // #FFFFFF color represented by RGB integers

    //draws knob for the pointer line
    painter.drawEllipse(QPointF(QDial::height()/1.425,QDial::height()/1.4),QDial::height()/15,QDial::height()/15);

    //draws the pointer line
    painter.drawPie(positionLine, -(value-90)*16, 3*16);

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

