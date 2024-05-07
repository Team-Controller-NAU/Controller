#include "firemode.h"
#include "constants.h"


/********************************************************************************
** firemode.cpp
**
** This class implements the logic to draw/re-draw the firemode graphic on the
** status page.
**
** @author Team Controller
********************************************************************************/


/**
 * @brief Initialization constructor for a firemode object
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
 * @brief Paints the firemode graphic
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

    //sets boarder color and thickness
    lines.setColor(QColor(255, 133, 51));
    lines.setWidth(3);

    //sets painter to lines
    painter.setPen(lines);

    //set brush color for background
    painter.setBrush(QBrush(QColor(27,27,27)));

    //sets the area for the outline and background
    QRectF outlineArea(-QDial::height()/14, QDial::height()/22, QDial::height()*1.55, QDial::height()*1.55);

    //draws the outline
    painter.drawPie(outlineArea, -(62.5-90)*16, 125*16);
    painter.drawLine(QDial::height()/1.44, QDial::height()/20,QDial::height()/1.44,QDial::height()/4.5);
    //painter.drawLine(QDial::height()/5.2, QDial::height()/4,QDial::height()/3,QDial::height()/2.7);


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

    //sets pen color for text
    painter.setPen(QPen(QColor(255, 255, 255))); // White color represented by RGB integers

    //Initilize text font
    QFont textfont = painter.font();

    //set font size
    textfont.setPixelSize(QDial::height()/10);

    //set painter font
    painter.setFont(textfont);

    //draws text
    painter.drawText(QDial::height()/13,QDial::height()/3,QDial::height(),QDial::height(),0,"Safe");

    painter.drawText(QDial::height()/3.5,QDial::height()/6.7,QDial::height(),QDial::height(),0,"Single");

    painter.drawText(QDial::height()/1.2,QDial::height()/6.7,QDial::height(),QDial::height(),0,"Burst");

    painter.drawText(QDial::height()*1.13,QDial::height()/3,QDial::height(),QDial::height(),0,"Auto");

    //set paint color for highlighted text
    painter.setPen(QPen(QColor(255, 133, 51))); // #ff8533 color represented by RGB integers

    //draws colored text
    if(value == SAFE)
    {
        painter.drawText(QDial::height()/13,QDial::height()/3,QDial::height(),QDial::height(),0,"Safe");
    }
    else if(value == SINGLE)
    {
        painter.drawText(QDial::height()/3.5,QDial::height()/6.7,QDial::height(),QDial::height(),0,"Single");
    }
    else if(value == BURST)
    {
        painter.drawText(QDial::height()/1.2,QDial::height()/6.7,QDial::height(),QDial::height(),0,"Burst");
    }
    else if(value == FULL_AUTO)
    {
        painter.drawText(QDial::height()*1.13,QDial::height()/3,QDial::height(),QDial::height(),0,"Auto");
    }

    //disable user edits on the needle
    setDisabled(true);
}

