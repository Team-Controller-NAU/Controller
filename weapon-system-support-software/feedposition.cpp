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
 * @brief Initialization constructor for a feedposition object
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
 * @brief Paints the feed position graphic
 */
void feedposition::paintEvent(QPaintEvent*)
{
    //variables
    int startAngle;
    int index;
    int dialSize;

    if(QDial::height() > QDial::width())
    {
        dialSize = QDial::width();
    }
    else
    {
        dialSize = QDial::height();
    }
    //arc length of the dial background
    int spanAngle = (360/NUM_FEED_POSITION) * 16;

    //dial value
    int value = QDial::value();

    //painter object
    QPainter painter(this);
    QPen text = painter.pen();

    // Smooth out the circle
    painter.setRenderHint(QPainter::Antialiasing);

    // Creates a black border around painted objects
    painter.setPen(QPen(QColor(0, 0, 0))); // Black color represented by RGB integers

    //change brush color
    painter.setBrush(QBrush(QColor(255, 133, 51))); // #ff8533 color represented by RGB integers

    //area of the dial background
    QRectF dialBackground(0, 0, dialSize, dialSize);

    //loops to create background
    for(index = 0; index < NUM_FEED_POSITION; index++)
    {
        //paints color on feed position
        if(alternatingColors)
        {
            if(index % 2 == 0)
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
        else
        {
            /*
             * TODO: Adjust this parameter to choose feedposition colored indexes
             */
            if(index > 6 || index < 1 )
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

        //starting angle of the dial background section
        startAngle = (22.5 + ((360/NUM_FEED_POSITION) * index)) * 16;

        //draws the section of the dial background
        painter.drawPie(dialBackground, startAngle, spanAngle);

    }

    // No border around painted objects
    painter.setPen(QPen(Qt::NoPen));

    //change brush color
    painter.setBrush(QBrush(QColor(27,27,27))); // #272727 color represented by RGB integers

    //draws center circle to match program background
    painter.drawEllipse(QPointF(dialSize/2, dialSize/2), dialSize/3, dialSize/3);

    //area of the needle
    QRectF needle(dialSize/8, dialSize/8, dialSize/1.33, dialSize/1.33);

    //change brush color
    painter.setBrush(QBrush(QColor(255, 255, 255))); // #FFFFFF color represented by RGB integers

    //draws knob for the needle
    painter.drawEllipse(QPointF(dialSize/2, dialSize/2), dialSize/10, dialSize/10);

    //draws the needle
    painter.drawPie(needle, -(value - 90) * 16, 3 * 16);

    //setting text
    painter.setPen(text);

    //change brush color
    painter.setPen(QPen(QColor(0, 0, 0))); // Black color represented by RGB integers

    //Initilize default font
    QFont textfont = painter.font();

    //Set font size
    textfont.setPixelSize(dialSize/20);

    //set the font to painter
    painter.setFont(textfont);

    //Displays the degree in the center of knob
    if (value > 99)
    {
        painter.drawText(dialSize/2.15,dialSize/2.15,dialSize,dialSize,0,QString::number(value));
    }
    else if (value > 9)
    {
        painter.drawText((dialSize/2.15) + 2.5,dialSize/2.15,dialSize,dialSize,0,QString::number(value));
    }
    else
    {
        painter.drawText((dialSize/2.15) + 5,dialSize/2.15,dialSize,dialSize,0,QString::number(value));
    }

    //disable user edits on the needle
    setDisabled(true);
}

/**
 * @brief Event processor for feed position tool tip. Displays tool tip on the GUI
 * @param event Pointer to the QEvent ToolTip object
 * @return True if the event is handled successfully, false otherwise
 */
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

