#include "EgLinkWidget.h"
#include "EgGraphWidget.h"
#include "EgGraphForm.h"
#include "EgNodeWidget.h"


#include<QtMath>
#include<QPointF>
#include <QPainter>
#include <QMouseEvent>
#include <QMimeData>

using namespace std;

EgLinkWidget::EgLinkWidget(QWidget *parent):  QWidget(parent)
    ,linkPainter (new QPainter()), itemData (new QByteArray)
{
    setMouseTracking(true);
    // cout << "EgLinkWidget() linkInfo: " << std::hex << (int64_t) linkInfo << std::dec << endl;
}

EgLinkWidget::~EgLinkWidget()
{
    delete linkPainter;
    delete itemData;
}

void EgLinkWidget::drawArrow(QPoint start, QPoint end)
{
    linkPainter-> drawLine(end, start);

    const qreal minArrowLength {5.0};

    QLineF line(end, start); // draw arrow by float-point calc
    arrowLength = std::min<qreal> ((qreal)line.length()/2, dynamic_cast<EgGraphWidget*> (parent())-> scaledArrowLength);

    if (arrowLength < minArrowLength) return; // too short

    qreal arroHalfWidth = arrowLength / minArrowLength;

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

inline void EgLinkWidget::convertGlobPointsToWidget()
{
    QPoint cornerPoint(this->x(), this->y());
    startPoint = QPoint(linkPointStart.scaledX, linkPointStart.scaledY) - cornerPoint;
    endPoint   = QPoint(linkPointEnd.scaledX, linkPointEnd.scaledY) - cornerPoint;
}

void EgLinkWidget::paintEvent(QPaintEvent *)
{
    linkPainter-> begin(this);

    convertGlobPointsToWidget();

    if (isEditSelected) {

        linkPainter-> setPen(QColor(0,20,172,255)); // frame
        linkPainter-> drawRect(0,0,width()-1, height()-1);

        // QPen linkPen;
        // linkPen.setWidth(2);
        // linkPen.setColor(QColor(0,172,20,255));
        linkPainter-> setPen(Qt::red);
        // linkPainter-> setPen(QColor(0,172,20,255));
    } else
        // linkPainter-> setPen(QColor(255, 30, 30, 255)); // (0,20,172,255));
        linkPainter-> setPen(Qt::darkBlue); // (0,20,172,255));

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

    if (isEditSelected) { // draw edit knob rectangle
        linkPainter-> setPen(Qt::NoPen);
        // linkPainter-> setBrush(QBrush(QColor(31, 255, 31, 255)));
        linkPainter-> setBrush(QBrush(Qt::red));
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

        nodeWidgetFrom = static_cast<EgNodeWidget*> ((*(static_cast<EgGraphWidget*> (parent())-> graphNodes))[nodeIDFrom].serialDataPtr);
        nodeWidgetTo   = static_cast<EgNodeWidget*>    ((*(static_cast<EgGraphWidget*> (parent())-> graphNodes))[nodeIDTo].serialDataPtr);

        if (editLinkIsOutLink)
            useXforDrag = (portSideFrom == portSideNorth || portSideFrom == portSideSouth); // check what side to drag
        else
            useXforDrag = (portSideTo == portSideNorth || portSideTo == portSideSouth); // check what side to drag

        setAcceptDrops(true);

        QDrag* drag = new QDrag(this);
        // dragStart   = event->position().toPoint();

        /*itemData-> clear();
        QDataStream dataStream(itemData, QIODevice::WriteOnly);
        dataStream << dragStart;*/

        QMimeData *mimeData = new QMimeData;
        // mimeData->clear();
        mimeData->setData("application/x-dnditemdata", *itemData);
        drag-> setMimeData(mimeData);

        static_cast<EgGraphWidget*> (parent())-> myForm-> dragDropAction = true;
        drag-> exec(Qt::MoveAction);
        static_cast<EgGraphWidget*> (parent())-> myForm-> dragDropAction = false;

        static_cast<EgGraphWidget*> (parent())-> graphLinks-> linksDataStorage.MarkUpdatedDataNode(dataLinkID);

        // FIXME resize widget to move arrow

    } else
        event->ignore();
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
    int globalCoordScaled;
    int nodeCoordScaled;

    if (useXforDrag) {
        globalCoordScaled = event-> position().toPoint().x() + x();
        if (editLinkIsOutLink) {
            nodeCoordScaled = globalCoordScaled - nodeWidgetFrom-> x();
            if (nodeCoordScaled > nodeWidgetFrom-> width() - 1)
                nodeCoordScaled = nodeWidgetFrom-> width() - 1;
        } else {
            nodeCoordScaled = globalCoordScaled - nodeWidgetTo-> x();
            if (nodeCoordScaled > nodeWidgetTo-> width() - 1)
                nodeCoordScaled = nodeWidgetTo-> width() - 1;
        }
    } else {
        globalCoordScaled = event-> position().toPoint().y() + y();
        if (editLinkIsOutLink) {
            nodeCoordScaled = globalCoordScaled - nodeWidgetFrom-> y();
            if (nodeCoordScaled > nodeWidgetFrom-> height() - 1)
                nodeCoordScaled = nodeWidgetFrom-> height() - 1;
        } else {
            nodeCoordScaled = globalCoordScaled - nodeWidgetTo-> y();
            if (nodeCoordScaled > nodeWidgetTo-> height() - 1)
                nodeCoordScaled = nodeWidgetTo-> height() - 1;
        }
    }

    if (editLinkIsOutLink) {
        scaledToOrigScalar(nodeCoordScaled, sideCoordFromOrig, static_cast<EgGraphWidget*> (parent())-> zoomFactor);
        alignToGrid(sideCoordFromOrig, gridSizeOrig, 0);
        nodeWidgetFrom-> getLinkPointOrig(portSideFrom, sideCoordFromOrig, linkPointStart.origX, linkPointStart.origY);
        origToScaledPointCanvas(linkPointStart, static_cast<EgGraphWidget*> (parent())-> zoomFactor, static_cast<EgGraphWidget*> (parent())-> layerCanvas.corner);
    } else {
        scaledToOrigScalar(nodeCoordScaled, sideCoordToOrig, static_cast<EgGraphWidget*> (parent())-> zoomFactor);
        alignToGrid(sideCoordToOrig, gridSizeOrig, 0);
        nodeWidgetTo-> getLinkPointOrig(portSideTo, sideCoordToOrig, linkPointEnd.origX, linkPointEnd.origY);
        origToScaledPointCanvas(linkPointEnd, static_cast<EgGraphWidget*> (parent())-> zoomFactor, static_cast<EgGraphWidget*> (parent())-> layerCanvas.corner);
    }

    static_cast<EgGraphWidget*> (parent())-> graphLinks-> MarkUpdatedLink(dataLinkID);
    repaint();
    // cout << "link dragMoveEvent() dataLinkID: " << dec << dataLinkID << endl;
}

/*
void EgLinkWidget::alignToGrid(int& coord)
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
} */

void EgLinkWidget::calcLinkWidgetRect(int zoomFactor, int globalIndentScaled)
{
    linkRect.corner.scaledX = std::min<int> (linkPointStart.scaledX, linkPointEnd.scaledX) - globalIndentScaled;
    linkRect.corner.scaledY = std::min<int> (linkPointStart.scaledY, linkPointEnd.scaledY) - globalIndentScaled;
    int maxScaledX = std::max<int> (linkPointStart.scaledX, linkPointEnd.scaledX);
    int maxScaledY = std::max<int> (linkPointStart.scaledY, linkPointEnd.scaledY);

    linkRect.size.scaledW = maxScaledX - linkRect.corner.scaledX + globalIndentScaled * 2;
    linkRect.size.scaledH = maxScaledY - linkRect.corner.scaledY + globalIndentScaled * 2;
    // cout << "calcLinkWidgetRect() "  << cornerX << " : " << maxScaledX << " : " << cornerY << " : " << maxScaledY << endl;

    // scaledToOrigRectCanvas   (egRect& rect, int zoomFactor, egPoint& canvas); // FIXME check
}

void EgLinkWidget::updLinkAftMoveStart(QPoint& deltaStartPoint)
{
    linkPointStart.scaledX += deltaStartPoint.x();
    linkPointStart.scaledY += deltaStartPoint.y();
}

void EgLinkWidget::updLinkAftMoveEnd(QPoint& deltaEndPoint)
{
    linkPointEnd.scaledX += deltaEndPoint.x();
    linkPointEnd.scaledY += deltaEndPoint.y();
}

void EgLinkWidget::updLinkAftResizeStart(EgNodeWidget* nodeData, int oldW, int oldH)
{
    int oldOffset;
    int newOffset;
    switch (portSideFrom) {
    case portSideNorth:
        oldOffset = linkPointStart.origX - nodeData->nodeRect.corner.origX; // origCornerX;
        newOffset = oldOffset * nodeData->nodeRect.size.origW /*origRectW*/ / oldW;
        linkPointStart.origX = (nodeData->nodeRect.corner.origX /*origCornerX*/ + newOffset);
        break;
    case portSideEast:
        oldOffset = linkPointStart.origY - nodeData->nodeRect.corner.origY;
        newOffset = oldOffset * nodeData->nodeRect.size.origH / oldH;
        linkPointStart.origY = (nodeData->nodeRect.corner.origY + newOffset);
        linkPointStart.origX = (nodeData->nodeRect.corner.origX + nodeData->nodeRect.size.origW - 1);
        break;
    case portSideSouth:
        oldOffset = linkPointStart.origY - nodeData->nodeRect.corner.origX;
        newOffset = oldOffset * nodeData->nodeRect.size.origW / oldW;
        linkPointStart.origX = (nodeData->nodeRect.corner.origX + newOffset);
        linkPointStart.origY = (nodeData->nodeRect.corner.origY + nodeData->nodeRect.size.origH - 1);
        break;
    case portSideWest:
        oldOffset = linkPointStart.origY - nodeData->nodeRect.corner.origY;
        newOffset = oldOffset * nodeData->nodeRect.size.origH / oldH;
        linkPointStart.origY = (nodeData->nodeRect.corner.origY + newOffset);
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
        oldOffset = linkPointEnd.origX - nodeData->nodeRect.corner.origX;
        newOffset = oldOffset * nodeData->nodeRect.size.origW / oldW;
        linkPointEnd.origX = (nodeData->nodeRect.corner.origX + newOffset);
        break;
    case portSideEast:
        oldOffset = linkPointEnd.origY - nodeData->nodeRect.corner.origY;
        newOffset = oldOffset * nodeData->nodeRect.size.origH / oldH;
        linkPointEnd.origY = (nodeData->nodeRect.corner.origY + newOffset);
        linkPointEnd.origX = (nodeData->nodeRect.corner.origX + nodeData->nodeRect.size.origW - 1);
        break;
    case portSideSouth:
        oldOffset = linkPointEnd.origX - nodeData->nodeRect.corner.origX;
        newOffset = oldOffset * nodeData->nodeRect.size.origW / oldW;
        linkPointEnd.origX = (nodeData->nodeRect.corner.origX + newOffset);
        linkPointEnd.origY = (nodeData->nodeRect.corner.origY + nodeData->nodeRect.size.origH - 1);
        break;
    case portSideWest:
        oldOffset = linkPointEnd.origY - nodeData->nodeRect.corner.origY;
        newOffset = oldOffset * nodeData->nodeRect.size.origH / oldH;
        linkPointEnd.origY = (nodeData->nodeRect.corner.origY + newOffset);
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
