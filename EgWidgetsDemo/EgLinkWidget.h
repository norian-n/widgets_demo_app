#ifndef EGLINKWIDGET_H
#define EGLINKWIDGET_H

#include <QWidget>
#include <QDrag>

// #include "egNodeToSpace.h"

const int directConnect     {0};
const int sideConnect       {1};
const int topConnect        {2};
const int dirAngle02Connect {3};
const int dirAngle13Connect {4};

const int portSideNorth     {1};
const int portSideEast      {2};
const int portSideSouth     {3};
const int portSideWest      {4};

class EgNodeWidget;

class EgLinkWidget : public QWidget
{
    Q_OBJECT

public:
    QPainter* linkPainter   {nullptr};

    int dataLinkID;
    int nodeIDFrom      {0};
    int nodeIDTo        {0};

    int portSideFrom    {0};
    QPoint startPointOrig;
    QPoint startPointScaled;

    int portSideTo      {0};
    QPoint endPointOrig;
    QPoint endPointScaled;

    int scaledCornerX {0};    // top right corner
    int scaledCornerY {0};

    int scaledRectW {100};    // rectangle width
    int scaledRectH {100};    // rectangle height

    int lineType  {directConnect};

    bool isEditSelected     {false};
    bool useXforDrag        {true};
    bool editLinkIsOutLink  {true};

    int minCoordForDrag     {0};
    int maxCoordForDrag     {1000};

    int tmpLinkQuadrant {0};
    QPoint startPoint;
    QPoint endPoint;
    QPoint midPoint;

    QPoint dragStart;
    QByteArray* itemData    {nullptr};

    qreal arrowLength   {20};

    EgLinkWidget(QWidget *parent = 0);
    ~EgLinkWidget();

    inline void adjustToGridX(int& coordX);
    inline void adjustToGridY(int& coordY);
    inline void adjustGlobPointsToWidget();  // convert to insde widget coords

    void drawArrow(QPoint start, QPoint end);
    void drawDirAngleLink(QPoint start, QPoint mid, QPoint end);
    void drawThreeFoldSide(QPoint start, QPoint end, int delta);
    void drawThreeFoldTop(QPoint start, QPoint end, int delta);

    void calcLinkWidgetRect(int zoomFactor);

    void calcOrigToScaled      (int zoomFactor, QPoint& globCanvasScaled);
    void calcOrigToScaledStart (int zoomFactor, QPoint& globCanvasScaled);
    void calcOrigToScaledEnd   (int zoomFactor, QPoint& globCanvasScaled);
    void calcScaledToOrigStart (int zoomFactor, QPoint& globCanvasScaled);
    void calcScaledToOrigEnd   (int zoomFactor, QPoint& globCanvasScaled);

    void updLinkAftMoveStart(QPoint& deltaStartPoint);
    void updLinkAftMoveEnd  (QPoint& deltaEndPoint);

    void updLinkAftResizeStart(EgNodeWidget* nodeData, int oldW, int oldH);
    void updLinkAftResizeEnd  (EgNodeWidget* nodeData, int oldW, int oldH);

    void calcLinkVisualType();

protected:
    void paintEvent(QPaintEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
};

#endif // EGLINKWIDGET_H
