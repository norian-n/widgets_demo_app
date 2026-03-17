#include "EgNodeWidget.h"
#include "EgLinkWidget.h"
#include "EgGraphWidget.h"

using namespace std;

#include <QPainter>
#include <QMouseEvent>

EgNodeWidget::EgNodeWidget(QWidget *parent): QWidget(parent)
    , nodePainter (new QPainter())
{
    setMouseTracking(true);
    // cout << " EgNodeWidget() called " << endl;
}

EgNodeWidget::~EgNodeWidget()
{
    delete nodePainter;
    // cout << " ~EgNodeWidget() called " << endl;
}

/*
void EgNodeWidget::enterEvent(QEvent *event)
{
    // cout << "enterEvent() node ID: " << nodeInfo->dataNodeID << endl;
}
*/
void EgNodeWidget::leaveEvent(QEvent *event)
{
    if (nodeShowCorner) {
        nodeShowCorner = false;
        this -> repaint();
    }
}

void EgNodeWidget::mouseMoveEvent(QMouseEvent *event)
{
    bool cornerChanged;
    if ((width() - event->pos().x() < 17) && (height() - event->pos().y() < 17)) {
        cornerChanged = !(nodeShowCorner);
        nodeShowCorner = true;
    } else {
        cornerChanged = nodeShowCorner;
        nodeShowCorner = false;
    }
    if (cornerChanged)
        this -> repaint();
    // cout << "mouseMoveEvent() at: " << width() - event->pos().x() << " " << height() - event->pos().y() << " node ID: " << nodeInfo->dataNodeID << endl;
}

void EgNodeWidget::paintEvent(QPaintEvent *)
{
    nodePainter-> begin(this);
    nodePainter-> setPen(QColor(0,20,172,255)); // frame
    if (! nodeDeleteMode)
        nodePainter-> setBrush(QBrush(QColor(180, 180, 255, 127))); // 63, 63, 255, 64
    else
        nodePainter-> setBrush(QBrush(QColor(255, 120, 120, 255))); // 63, 63, 255, 64

    int fontSize    {10};
    int roundRadius {5};
    if (static_cast<EgGraphWidget*> (parent())-> zoomFactor)
    {
        int zoomFactor = dynamic_cast<EgGraphWidget*> (parent())-> zoomFactor;
        // cout << "zoomFactor: " << zoomFactor << endl;
        fontSize = max(10 - zoomFactor/2, 6);
        roundRadius = max(6 - zoomFactor/2, 1);
    }
    nodePainter-> drawRoundedRect(0,0,width()-1, height()-1, roundRadius,roundRadius); // FIXME radius scale

    nodePainter-> setPen(Qt::blue); // text
    nodePainter-> setFont(QFont("Arial", fontSize, QFont::Bold));   // FIXME font scale
    nodePainter-> drawText(4, fontSize+4, labelText);      // FIXME consts
    // cout << " moveMode: "  << dynamic_cast<EgGraphWidget*> (parent())-> moveMode << " connectMode: " << dynamic_cast<EgGraphWidget*> (parent())-> connectMode << endl;
        // small circle at corner
    if (dynamic_cast<EgGraphWidget*> (parent())-> actionMode == moveMode)
        if (nodeShowCorner) // nodeInfo && (dynamic_cast<EgGraphWidget*> (parent())-> resizeMode &&
        {
            nodePainter-> setPen(Qt::NoPen);
            nodePainter-> setBrush(QBrush(QColor(255, 63, 63, 128)));
            int resizeSpotArea = (resizeSpotAreaOrig * (resizeSpotAreaOrig - dynamic_cast<EgGraphWidget*> (parent())-> zoomFactor)/resizeSpotAreaOrig) + 2;
            // int resizeSpotArea = resizeSpotAreaOrig;
            nodePainter-> drawEllipse(width()-resizeSpotArea, height()-resizeSpotArea, resizeSpotArea, resizeSpotArea);  // resizeSpotArea
        }

    nodePainter-> end();
}

void EgNodeWidget::calcOrigToScaled(int zoomFactor, QPoint& globCanvasScaled)
{
    scaledCornerX = origCornerX * (10 - zoomFactor)/10 + globCanvasScaled.x();
    scaledCornerY = origCornerY * (10 - zoomFactor)/10 + globCanvasScaled.y();
    scaledRectW   = origRectW   * (10 - zoomFactor)/10;
    scaledRectH   = origRectH   * (10 - zoomFactor)/10;
}

void EgNodeWidget::calcScaledToOrig(int zoomFactor, QPoint& globCanvasScaled)
{
    origCornerX  = (scaledCornerX - globCanvasScaled.x()) * 10/(10 - zoomFactor);
    origCornerY  = (scaledCornerY - globCanvasScaled.y()) * 10/(10 - zoomFactor);
    // cout << "calcScaledToOrig(): " << zoomFactor << " : " << origCornerX << " : " << origCornerY << endl;
}

void EgNodeWidget::calcPortPoint(int& portSide, QPoint& clickPoint, QPoint& portPoint)
{
    int portX;
    int portY;
    int centricClickX = clickPoint.x()-scaledRectW/2; // calc node center-based click coords
    int centricClickY = clickPoint.y()-scaledRectH/2;

    if (std::abs(centricClickX) > std::abs(centricClickY) * scaledRectW/scaledRectH) { // get click port area side (north, west, etc)
        portY = clickPoint.y();
        if (centricClickX > 0) {
            portSide = portSideEast;
            portX = scaledRectW - 1;
        } else {
            portSide = portSideWest;
            portX = 0;
        }
    } else {
        portX = clickPoint.x();
        if (centricClickY < 0) {
            portSide = portSideNorth;
            portY = 0;
        } else {
            portSide = portSideSouth;
            portY = scaledRectH - 1;
            // cout << "calcPortPoint():  south" << portX << " : " << portY << endl;
        }
    }
    // cout << "calcPortPoint() WH: "  << origRectW << " : " << origRectH << endl;
    portPoint.setX(portX);
    portPoint.setY(portY);
}
