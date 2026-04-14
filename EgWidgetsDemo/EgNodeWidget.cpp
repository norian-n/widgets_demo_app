#include "EgNodeWidget.h"
#include "EgLinkWidget.h"
#include "EgGraphWidget.h"

using namespace std;

#include <QPainter>
#include <QMouseEvent>


EgNodeWidget::EgNodeWidget(QWidget *parent): QWidget(parent)
    , nodePainter (new QPainter), pixmap (new QPixmap)
{
    setMouseTracking(true);
    // cout << " EgNodeWidget() called " << endl;
}

EgNodeWidget::~EgNodeWidget()
{
    delete nodePainter; // nodePainter (new QPainter())
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
    if (nodeResizeMode) {
        nodeResizeMode = false;
        repaint();
    }
}

void EgNodeWidget::mouseMoveEvent(QMouseEvent *event)
{
    bool cornerChanged;
    if ((width() - event->pos().x() < 17) && (height() - event->pos().y() < 17)) {
        cornerChanged = !(nodeResizeMode);
        nodeResizeMode = true;
    } else {
        cornerChanged = nodeResizeMode;
        nodeResizeMode = false;
    }
    if (cornerChanged)
        repaint();
    // else
    event->ignore();
    // cout << "mouseMoveEvent() at: " << width() - event->pos().x() << " " << height() - event->pos().y() << " node ID: " << nodeInfo->dataNodeID << endl;
}

void EgNodeWidget::paintEvent(QPaintEvent *)
{
    nodePainter-> begin(this);
    nodePainter-> setPen(Qt::darkBlue);                            // QColor(0,20,172,255)); // frame
    if (! nodeDeleteMode)
        nodePainter-> setBrush(QBrush(fillColor));  // setBrush(QBrush(QColor(200, 200, 255, 127))); // 63, 63, 255, 64
    else
        nodePainter-> setBrush(QBrush(QColor(255, 120, 120, 255))); // 63, 63, 255, 64 // FIXME literals

    int fontSizeOrig   {11}; // FIXME literals
    int fontSizeScaled {11};
    int roundRadius    {5};

    zoomFactor = static_cast<EgGraphWidget*> (parent())-> zoomFactor;
    if (zoomFactor)
    {
        origToScaledScalar (fontSizeOrig, fontSizeScaled, zoomFactor/2);
        fontSizeScaled = max(fontSizeScaled, 6);
        roundRadius = max(6 - zoomFactor/2, 1);
    }
    nodePainter-> drawRoundedRect(0,0,width()-1, height()-1, roundRadius,roundRadius); // FIXME radius scale

    nodePainter-> setPen(Qt::darkBlue); // text
    nodePainter-> setFont(QFont("Arial", fontSizeScaled, QFont::Bold));   // FIXME font scale
    nodePainter-> drawText(4, fontSizeScaled+4, labelText);      // FIXME literals
    // cout << " moveMode: "  << dynamic_cast<EgGraphWidget*> (parent())-> moveMode << " connectMode: " << dynamic_cast<EgGraphWidget*> (parent())-> connectMode << endl;
        // small circle at corner
    if ( static_cast<EgGraphWidget*> (parent())-> actionMode == moveResizeMode && nodeResizeMode) // show corner spot
    // if (nodeResizeMode)
    {
        nodePainter-> setPen(Qt::NoPen);
        nodePainter-> setBrush(QBrush(QColor(255, 63, 63, 128))); // FIXME literal
        int resizeSpotArea = (resizeSpotAreaOrig * (resizeSpotAreaOrig - zoomFactor)/resizeSpotAreaOrig) + 2;
        // int resizeSpotArea = resizeSpotAreaOrig;
        nodePainter-> drawEllipse(width()-resizeSpotArea, height()-resizeSpotArea, resizeSpotArea, resizeSpotArea);  // resizeSpotArea
    }

    int scaledGlobalIndent {0};
    origToScaledScalar (globalIndentOrig, scaledGlobalIndent, zoomFactor);
    if (detailsLayerID) { // show details icon
        int theIndent = scaledGlobalIndent / 2;
        int cornX = width() - theIndent * 4 - 1;
        int cornY = theIndent;
        // nodePainter-> setPen(Qt::NoPen);
        // nodePainter-> setBrush(Qt::darkGray);
        nodePainter-> setPen(Qt::black);
        nodePainter-> setBrush(Qt::NoBrush);
        nodePainter-> drawRect(cornX, cornY, theIndent * 3, theIndent*3);
        // nodePainter-> setBrush(Qt::lightGray);
        nodePainter-> drawRect(cornX + theIndent, cornY + theIndent, theIndent, theIndent);
    }

    if (pixmap->width())
        nodePainter-> drawPixmap(4, 18, pixmap-> scaled(40,40)); // FIXME literal

    nodePainter-> end();
}

void EgNodeWidget::calcPortPointScaled(int& portSide, QPoint& clickPoint, QPoint& portPoint, int& sideCoord)
{
    QPoint inNodeClickPoint = clickPoint - pos();

    int portX;
    int portY;
    int centricClickX = inNodeClickPoint.x()- nodeRect.size.scaledW / 2; // calc node center-based click coords
    int centricClickY = inNodeClickPoint.y()- nodeRect.size.scaledH / 2;

    if (std::abs(centricClickX) > std::abs(centricClickY) * nodeRect.size.scaledW / nodeRect.size.scaledH ) { // get click port area side (north, west, etc)
        portY = inNodeClickPoint.y();
        sideCoord = portY;
        if (centricClickX > 0) {
            portSide = portSideEast;
            portX = nodeRect.size.scaledW - 1;
        } else {
            portSide = portSideWest;
            portX = 0;
        }
    } else {
        portX = inNodeClickPoint.x();
        sideCoord = portX;
        if (centricClickY < 0) {
            portSide = portSideNorth;
            portY = 0;
        } else {
            portSide = portSideSouth;
            portY = nodeRect.size.scaledH - 1;
            // cout << "calcPortPointScaled():  south" << portX << " : " << portY << endl;
        }
    }
    // cout << "calcPortPointScaled() WH: "  << origRectW << " : " << origRectH << endl;
    portPoint.setX(portX);
    portPoint.setY(portY);
}

void EgNodeWidget::calcPortPointOrig(QPoint& clickPointScaled, int zoomFactor, int& portSide, int& sideCoordOrig, int& pointOrigX, int& pointOrigY)
{
    QPoint portPoinScaled;
    int sideCoordScaled;
    calcPortPointScaled(portSide, clickPointScaled, portPoinScaled, sideCoordScaled);
    scaledToOrigScalar (sideCoordScaled, sideCoordOrig, zoomFactor);
    alignToGrid(sideCoordOrig);
    getLinkPointOrig(portSide, sideCoordOrig, pointOrigX, pointOrigY);
}

void EgNodeWidget::getLinkPointOrig(int portSide, int sideCoordOrig, int& pointOrigX, int& pointOrigY)
{
    switch (portSide) {
    case portSideNorth: pointOrigX = nodeRect.corner.origX + sideCoordOrig;  pointOrigY = nodeRect.corner.origY; return;
    case portSideSouth: pointOrigX = nodeRect.corner.origX + sideCoordOrig;  pointOrigY = nodeRect.corner.origY + nodeRect.size.origH - 1; return;
    case portSideWest:  pointOrigX = nodeRect.corner.origX;  pointOrigY = nodeRect.corner.origY + sideCoordOrig; return;
    case portSideEast:  pointOrigX = nodeRect.corner.origX + nodeRect.size.origW - 1;  pointOrigY = nodeRect.corner.origY + sideCoordOrig; return;
    }
}

void EgNodeWidget::alignToGrid(int& coord)
{
    if (coord < globalIndentOrig)
        coord = globalIndentOrig;
    int modulo = coord % gridSizeOrig;
    if (! modulo)
        return;
    if (modulo < gridSizeOrig/2)
        coord -= modulo;
    else
        coord += gridSizeOrig-modulo;
    // cout << "alignToGrid() modulo: " << dec << modulo << " coord: " << coord << endl;
}
