#include "EgLinkWidget.h"
#include "EgGraphWidget.h"
#include "EgGraphForm.h"
#include "EgNodeWidget.h"


#include<QtMath>
#include<QPointF>
#include <QPainter>
#include <QMouseEvent>
#include <QMimeData>

#include <iostream>
using namespace std;

EgLinkWidget::EgLinkWidget(QWidget *parent):  QWidget(parent)
    , itemData (new QByteArray) , linkPainter (new QPainter())
{
    setMouseTracking(true);
    // cout << "EgLinkWidget() linkInfo: " << std::hex << (int64_t) linkInfo << std::dec << endl;
}

EgLinkWidget::~EgLinkWidget()
{
    delete linkPainter;
}

void EgLinkWidget::drawArrow(QPoint start, QPoint end)
{
    QLineF line(end, start);
    linkPainter-> drawLine(line);

    arrowLength = std::min<qreal> ((qreal)line.length()/2, dynamic_cast<EgGraphWidget*> (parent())-> scaledArrowLength);

    if (arrowLength < 5) return; // too short

    qreal arroHalfWidth = arrowLength / 5;

    linkPainter-> setBrush(linkPainter-> pen().color());

    qreal dx = end.rx() - start.rx();
    qreal dy = end.ry() - start.ry();

    qreal ux = dx / line.length(); // ux,uy is a unit vector parallel to the line
    qreal uy = dy / line.length();

    qreal vx = -uy; // vx,vy is a unit vector perpendicular to ux,uy
    qreal vy = ux;

    QPointF arrowP1 = QPointF (end.rx() - arrowLength*ux + arroHalfWidth*vx,
                               end.ry() - arrowLength*uy + arroHalfWidth*vy);
    QPointF arrowP2 = QPointF (end.rx() - arrowLength*ux - arroHalfWidth*vx,
                               end.ry() - arrowLength*uy - arroHalfWidth*vy);

    QPolygonF arrowHead;
    arrowHead << line.p1() << arrowP1 << arrowP2;
    linkPainter-> drawPolygon(arrowHead);
}

void EgLinkWidget::drawDirAngleLink(QPoint start, QPoint mid, QPoint end)
{
    linkPainter-> drawLine(start, mid);
    drawArrow(mid, end);
}

void EgLinkWidget::drawThreeFoldSide(QPoint start, QPoint end, int delta)
{
    int centerX = abs (start.x() - end.x()) /2; // center line x
    if (start.x() < end.x())
        centerX += start.x();
    else
        centerX += end.x();
    centerX += delta; // shift to reduce collisions

    linkPainter-> drawLine(start.x(), start.y(), centerX, start.y()); // first segment
    linkPainter-> drawLine(centerX, start.y(), centerX, end.y());     // center segment
    drawArrow(QPoint(centerX, end.y()), QPoint(end.x(), end.y()));    // last segment - arrow
}


void EgLinkWidget::drawThreeFoldTop(QPoint start, QPoint end, int delta)
{
    // center line x
    int centerY = abs (start.y() - end.y()) /2;
    if (start.y() < end.y())
        centerY += start.y();
    else
        centerY += end.y();
    centerY += delta; // shift to reduce collisions

    linkPainter-> drawLine(start.x(), start.y(), start.x(), centerY); // first segment
    linkPainter-> drawLine(start.x(), centerY, end.x(), centerY);     // center segment
    drawArrow(QPoint(end.x(), centerY), QPoint(end.x(), end.y()));    // last segment - arrow
}

inline void EgLinkWidget::adjustGlobPointsToWidget()
{
    QPoint cornerPoint(this->x(), this->y());
    startPoint = startPointScaled - cornerPoint;
    endPoint   = endPointScaled   - cornerPoint;
}

void EgLinkWidget::paintEvent(QPaintEvent *)
{
    linkPainter-> begin(this);

    adjustGlobPointsToWidget();

    if (isEditSelected) {
        raise();
        QPen linkPen;
        linkPen.setWidth(2);
        linkPen.setColor(QColor(0,172,20,255));
        linkPainter-> setPen(linkPen);
        // linkPainter-> setPen(QColor(0,172,20,255));
    } else
        linkPainter-> setPen(QColor(255, 30, 30, 255)); // (0,20,172,255));    

    int delta = static_cast<EgGraphWidget*> (parent())-> scaledGlobalIndent / 2;

    if (lineType == directConnect)
        drawArrow(startPoint, endPoint);
    else if (lineType == sideConnect) {
        if (portSideFrom == portSideWest)
            drawThreeFoldSide(startPoint, endPoint, -delta);
        else
            drawThreeFoldSide(startPoint, endPoint, delta);
    } else if (lineType == topConnect) {
        if (portSideFrom == portSideSouth)
            drawThreeFoldTop(startPoint, endPoint, -delta);
        else
            drawThreeFoldTop(startPoint, endPoint, delta);
    }
    else if (lineType == dirAngle13Connect) {
        midPoint.setX(endPoint.x());
        midPoint.setY(startPoint.y());
        drawDirAngleLink(startPoint, midPoint, endPoint);
    } else if (lineType == dirAngle02Connect) {
        midPoint.setX(startPoint.x());
        midPoint.setY(endPoint.y());
        drawDirAngleLink(startPoint, midPoint, endPoint);
    }

    if (isEditSelected) {
        linkPainter-> setPen(Qt::NoPen);
        linkPainter-> setBrush(QBrush(QColor(31, 255, 31, 255)));
        if (editLinkIsOutLink)
            linkPainter-> drawRect(startPoint.x()-portRectSize/2, startPoint.y()-portRectSize/2, portRectSize, portRectSize);
        else
            linkPainter-> drawRect(endPoint.x()-portRectSize/2, endPoint.y()-portRectSize/2, portRectSize, portRectSize);
    }

    linkPainter-> end();

    return;
}

void EgLinkWidget::mousePressEvent(QMouseEvent *event)
{
    if (isEditSelected) {
        if (editLinkIsOutLink)
            useXforDrag = (portSideFrom == portSideNorth || portSideFrom == portSideSouth); // check what side to drag
        else
            useXforDrag = (portSideTo == portSideNorth || portSideTo == portSideSouth); // check what side to drag

        setAcceptDrops(true);

        QDrag* drag = new QDrag(this);
        dragStart   = event->position().toPoint();

        itemData-> clear();
        QDataStream dataStream(itemData, QIODevice::WriteOnly);
        dataStream << dragStart;

        QMimeData *mimeData = new QMimeData;
        mimeData->clear();
        mimeData->setData("application/x-dnditemdata", *itemData);
        drag-> setMimeData(mimeData);

        static_cast<EgGraphWidget*> (parent())-> graphLinks-> linksDataStorage.MarkUpdatedDataNode(dataLinkID);

        static_cast<EgGraphWidget*> (parent())-> myForm-> dragDropAction = true;
        drag-> exec(Qt::MoveAction);
        static_cast<EgGraphWidget*> (parent())-> myForm-> dragDropAction = false;
    }
}

void EgLinkWidget::dragEnterEvent(QDragEnterEvent *event)
{
    // cout << "link dragEnterEvent() drag constraints: " << dec << minCoordForDrag << " : " << maxCoordForDrag << endl;
    if (static_cast<EgGraphWidget*> (parent())->myForm-> dragDropAction) {
        if (event->source() == this && isEditSelected)
            event-> acceptProposedAction();
    }
}

void EgLinkWidget::dragMoveEvent(QDragMoveEvent *event)
{
    // cout << "link dragMoveEvent() pos+y(): " << event-> position().toPoint().y() + y() << endl;
    if (editLinkIsOutLink)
    {
        if (useXforDrag)
        {
            if (event-> position().toPoint().x() + x() <= maxCoordForDrag && event-> position().toPoint().x() + x() >= minCoordForDrag)
                startPointScaled.setX(event-> position().toPoint().x() + x());
            else
                if (event-> position().toPoint().x() + x() > maxCoordForDrag)
                    startPointScaled.setX(maxCoordForDrag);
                else
                    startPointScaled.setX(minCoordForDrag);
        } else {
            if (event-> position().toPoint().y() + y() <= maxCoordForDrag && event-> position().toPoint().y() + y() >= minCoordForDrag)
                startPointScaled.setY(event-> position().toPoint().y() + y());
            else
                if (event-> position().toPoint().y() + y() > maxCoordForDrag)
                    startPointScaled.setY(maxCoordForDrag);
                else
                    startPointScaled.setY(minCoordForDrag);
        }
    } else {
        if (useXforDrag)
        {
            if (event-> position().toPoint().x() + x() <= maxCoordForDrag && event-> position().toPoint().x() + x() >= minCoordForDrag)
                endPointScaled.setX(event-> position().toPoint().x() + x());
            else
                if (event-> position().toPoint().x() + x() > maxCoordForDrag)
                    endPointScaled.setX(maxCoordForDrag);
                else
                    endPointScaled.setX(minCoordForDrag);
        } else {
            if (event-> position().toPoint().y() + y() <= maxCoordForDrag && event-> position().toPoint().y() + y() >= minCoordForDrag)
                endPointScaled.setY(event-> position().toPoint().y() + y());
            else
                if (event-> position().toPoint().y() + y() > maxCoordForDrag)
                    endPointScaled.setY(maxCoordForDrag);
                else
                    endPointScaled.setY(minCoordForDrag);
        }
    }

     // ajust point to the grid
    if (editLinkIsOutLink)
    {
        if (portSideTo == portSideNorth || portSideTo == portSideNorth) {
            int tmpX =  endPointOrig.x();
            adjustToGridX (tmpX);
            endPointOrig.setX(tmpX);
        } else {
            int tmpY =  endPointOrig.y();
            adjustToGridY (tmpY);
            endPointOrig.setY(tmpY);
        }
    } else {
        if (portSideTo == portSideNorth || portSideTo == portSideNorth) {
            int tmpX =  endPointOrig.x();
            adjustToGridX (tmpX);
            endPointOrig.setX(tmpX);
        } else {
            int tmpY =  endPointOrig.y();
            adjustToGridY (tmpY);
            endPointOrig.setY(tmpY);
        }
    }

    calcLinkWidgetRect(static_cast<EgGraphWidget*> (parent())-> zoomFactor);
    static_cast<EgGraphWidget*> (parent())-> moveResizeLinkWidget(this);

    if (editLinkIsOutLink) {
        QPoint tmpPoint { static_cast<EgGraphWidget*> (parent())-> layerCanvas.corner.scaledX, static_cast<EgGraphWidget*> (parent())-> layerCanvas.corner.scaledY }; // FIXME STUB
        calcScaledToOrigStart(static_cast<EgGraphWidget*> (parent())-> zoomFactor, tmpPoint);
    } else {
        QPoint tmpPoint { static_cast<EgGraphWidget*> (parent())-> layerCanvas.corner.scaledX, static_cast<EgGraphWidget*> (parent())-> layerCanvas.corner.scaledY }; // FIXME STUB
        calcScaledToOrigEnd(static_cast<EgGraphWidget*> (parent())-> zoomFactor, tmpPoint);
    }
}

inline void EgLinkWidget::adjustToGridX(int& coordX)
{
    if (coordX < globalIndentOrig)
        coordX = globalIndentOrig;
    int modulo = coordX % gridSizeOrig;
    if (! modulo)
        return;
    if (modulo < gridSizeOrig/2)
        coordX -= modulo;
    else
        coordX += gridSizeOrig-modulo;
    cout << "adjustToGridX() modulo: " << dec << modulo << " coordX: " << coordX << endl;
}

inline void EgLinkWidget::adjustToGridY(int& coordY)
{
    if (coordY < globalIndentOrig)
        coordY = globalIndentOrig;
    int modulo = coordY % gridSizeOrig;
    if (! modulo)
        return;
    if (modulo < gridSizeOrig/2)
        coordY -= modulo;
    else
        coordY += gridSizeOrig-modulo;
    cout << "adjustToGridX() modulo: " << dec << modulo << " coordY: " << coordY << endl;
}

void EgLinkWidget::calcLinkWidgetRect(int zoomFactor)
{
    int globalIndent = globalIndentOrig * (10 - zoomFactor)/10;

    scaledCornerX = std::min<int> (startPointScaled.x(), endPointScaled.x()) - globalIndent;
    int maxScaledX = std::max<int> (startPointScaled.x(), endPointScaled.x()) + globalIndent;
    scaledCornerY = std::min<int> (startPointScaled.y(), endPointScaled.y()) - globalIndent;
    int maxScaledY = std::max<int> (startPointScaled.y(), endPointScaled.y()) + globalIndent;

    scaledRectW = maxScaledX - scaledCornerX;
    scaledRectH = maxScaledY - scaledCornerY;
    // cout << "calcLinkWidgetRect() "  << cornerX << " : " << maxScaledX << " : " << cornerY << " : " << maxScaledY << endl;
}

void EgLinkWidget::calcOrigToScaled(int zoomFactor, QPoint& globCanvasScaled)
{
    calcOrigToScaledStart(zoomFactor, globCanvasScaled);
    calcOrigToScaledEnd(zoomFactor, globCanvasScaled);
}

void EgLinkWidget::calcOrigToScaledStart(int zoomFactor, QPoint& globCanvasScaled)
{
    startPointScaled.setX(startPointOrig.x() * (10 - zoomFactor)/10 + globCanvasScaled.x());
    startPointScaled.setY(startPointOrig.y() * (10 - zoomFactor)/10 + globCanvasScaled.y());
}

void EgLinkWidget::calcOrigToScaledEnd(int zoomFactor, QPoint& globCanvasScaled)
{
    endPointScaled.setX  (endPointOrig.x() * (10 - zoomFactor)/10 + globCanvasScaled.x());
    endPointScaled.setY  (endPointOrig.y() * (10 - zoomFactor)/10 + globCanvasScaled.y());
}


void EgLinkWidget::calcScaledToOrigStart(int zoomFactor, QPoint& globCanvasScaled)
{
    startPointOrig.setX((startPointScaled.x() - globCanvasScaled.x()) * 10/(10 - zoomFactor));
    startPointOrig.setY((startPointScaled.y() - globCanvasScaled.y()) * 10/(10 - zoomFactor));
}

void EgLinkWidget::calcScaledToOrigEnd(int zoomFactor, QPoint& globCanvasScaled)
{
    endPointOrig.setX((endPointScaled.x() - globCanvasScaled.x()) * 10/(10 - zoomFactor));
    endPointOrig.setY((endPointScaled.y() - globCanvasScaled.y()) * 10/(10 - zoomFactor));
}

void EgLinkWidget::updLinkAftMoveStart(QPoint& deltaStartPoint)
{
    startPointScaled += deltaStartPoint;
}

void EgLinkWidget::updLinkAftMoveEnd(QPoint& deltaEndPoint)
{
    endPointScaled += deltaEndPoint;
}

void EgLinkWidget::updLinkAftResizeStart(EgNodeWidget* nodeData, int oldW, int oldH)
{
    int oldOffset;
    int newOffset;
    switch (portSideFrom) {
    case portSideNorth:
        oldOffset = startPointOrig.x() - nodeData->origCornerX;
        newOffset = oldOffset * nodeData->origRectW / oldW;
        startPointOrig.setX(nodeData->origCornerX + newOffset);
        break;
    case portSideEast:
        oldOffset = startPointOrig.y() - nodeData->origCornerY;
        newOffset = oldOffset * nodeData->origRectH / oldH;
        startPointOrig.setY(nodeData->origCornerY + newOffset);
        startPointOrig.setX(nodeData->origCornerX + nodeData->origRectW - 1);
        break;
    case portSideSouth:
        oldOffset = startPointOrig.x() - nodeData->origCornerX;
        newOffset = oldOffset * nodeData->origRectW / oldW;
        startPointOrig.setX(nodeData->origCornerX + newOffset);
        startPointOrig.setY(nodeData->origCornerY + nodeData->origRectH - 1);
        break;
    case portSideWest:
        oldOffset = startPointOrig.y() - nodeData->origCornerY;
        newOffset = oldOffset * nodeData->origRectH / oldH;
        startPointOrig.setY(nodeData->origCornerY + newOffset);
        break;
    }
    // cout << "updLinkAftResizeStart(): " << oldW << " : " << newW << " : " << oldH << " : " << newH << endl;
}

void EgLinkWidget::updLinkAftResizeEnd(EgNodeWidget* nodeData, int oldW, int oldH)
{
    int oldOffset;
    int newOffset;
    switch (portSideTo) {
    case portSideNorth:
        oldOffset = endPointOrig.x() - nodeData->origCornerX;
        newOffset = oldOffset * nodeData->origRectW / oldW;
        endPointOrig.setX(nodeData->origCornerX + newOffset);
        break;
    case portSideEast:
        oldOffset = endPointOrig.y() - nodeData->origCornerY;
        newOffset = oldOffset * nodeData->origRectH / oldH;
        endPointOrig.setY(nodeData->origCornerY + newOffset);
        endPointOrig.setX(nodeData->origCornerX + nodeData->origRectW - 1);
        break;
    case portSideSouth:
        oldOffset = endPointOrig.x() - nodeData->origCornerX;
        newOffset = oldOffset * nodeData->origRectW / oldW;
        endPointOrig.setX(nodeData->origCornerX + newOffset);
        endPointOrig.setY(nodeData->origCornerY + nodeData->origRectH - 1);
        break;
    case portSideWest:
        oldOffset = endPointOrig.y() - nodeData->origCornerY;
        newOffset = oldOffset * nodeData->origRectH / oldH;
        endPointOrig.setY(nodeData->origCornerY + newOffset);
        break;
    }
    // cout << "updLinkAftResizeStart(): " << oldW << " : " << newW << " : " << oldH << " : " << newH << endl;
}

void EgLinkWidget::calcLinkVisualType()
{
    if ((portSideFrom == portSideNorth || portSideFrom == portSideSouth) && (portSideTo == portSideEast || portSideTo == portSideWest)) // angle
        lineType = dirAngle02Connect;
    else if ((portSideFrom == portSideEast || portSideFrom == portSideWest) && (portSideTo == portSideNorth || portSideTo == portSideSouth)) // angle
        lineType = dirAngle13Connect;
    else if ((portSideFrom == portSideNorth && portSideTo == portSideSouth) || (portSideFrom == portSideSouth && portSideTo == portSideNorth)) // top
        lineType = topConnect;
    else if ((portSideFrom == portSideEast && portSideTo == portSideWest) || (portSideFrom == portSideWest && portSideTo == portSideEast)) // side
        lineType = sideConnect;
    else lineType = directConnect;
    // cout << "calcLinkVisualType(): " << lineType << " portSides: " << portSideFrom << " " << portSideTo << endl;
}
