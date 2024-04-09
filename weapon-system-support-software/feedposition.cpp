#include "feedposition.h"

feedposition::feedposition(QWidget* parent)
    : QDial(parent)
{
    //set feed pos to be measured in 360 degrees
    QDial::setMaximum(360);
}

void feedposition::paintEvent(QPaintEvent*)
{
    //variables
    int startAngle;
    int index;

    //arc length of the dial background
    int spanAngle = 45 * 16;

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
            painter.setBrush(QBrush(QColor(255, 133, 51))); // #ff8533 color represented by RGB integers
        }
        else
        {
            //change brush color
            painter.setBrush(QBrush(QColor(151, 71, 255))); // #9747FF color represented by RGB integers
        }
    }

    painter.setPen(QPen(Qt::NoPen));

    //change brush color
    painter.setBrush(QBrush(QColor(27,27,27))); // #272727 color represented by RGB integers

    //draws center circle to match program background
    painter.drawEllipse(QPointF(QDial::height()/2, QDial::height()/2), QDial::height()/3, QDial::height()/3);

    //area of the pointer line
    QRectF positionLine(QDial::height()/8, QDial::height()/8, QDial::height()/1.33, QDial::height()/1.33);

    //change brush color
    painter.setBrush(QBrush(QColor(255, 255, 255))); // #FFFFFF color represented by RGB integers

    //draws knob for the pointer line
    painter.drawEllipse(QPointF(QDial::height()/2, QDial::height()/2), QDial::height()/10, QDial::height()/10);

    //draws the pointer line
    painter.drawPie(positionLine, -(value - 90) * 16, 3 * 16);

    //setting text
    painter.setPen(text);

    painter.setPen(QPen(QColor(0, 0, 0))); // Black color represented by RGB integers

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

//misc event processor for feed pos
bool feedposition::event(QEvent* event)
{
    //process tool tip event
    if (event->type() == QEvent::ToolTip)
    {
        // Show the static tooltip message (we could make it dynamic depending on feed pos value as well)
        QToolTip::showText(mapToGlobal(rect().center()), "Feed position display");
        return true;
    }
    return QDial::event(event);
}

