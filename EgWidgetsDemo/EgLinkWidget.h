#ifndef EGLINKWIDGET_H
#define EGLINKWIDGET_H

#include <QWidget>
#include <QDrag>

#include "qtinterface/egQtInterface.h"

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

    EgNodeWidget* nodeWidgetFrom {nullptr}; // for edit drag
    EgNodeWidget* nodeWidgetTo   {nullptr};

    int portSideFrom    {0};
    int portSideTo      {0};

    int sideCoordFromOrig   {0};
    int sideCoordFromScaled {0};
    int sideCoordToOrig     {0};
    int sideCoordToScaled   {0};

    egRect linkRect;
    egPoint linkPointStart;
    egPoint linkPointEnd;

    int lineType  {directConnect};

    bool isEditSelected     {false};
    bool useXforDrag        {true};
    bool editLinkIsOutLink  {true};

    QPoint dragStart;
    QByteArray* itemData    {nullptr};
    // int minCoordForDrag     {0};
    // int maxCoordForDrag     {1000}; // FIXME STUB

    // int tmpLinkQuadrant {0};
    QPoint startPoint;
    QPoint endPoint;
    QPoint midPoint;

    qreal arrowLength   {20};

    EgLinkWidget(QWidget *parent = 0);
    ~EgLinkWidget();

    // void alignToGrid(int& coord);

    inline void convertGlobPointsToWidget();  // convert to insde widget coords

    void drawArrow(QPoint start, QPoint end);
    void drawDirAngleLink(QPoint start, QPoint mid, QPoint end);
    void drawThreeFoldSide(QPoint start, QPoint end, int delta);
    void drawThreeFoldTop(QPoint start, QPoint end, int delta);

    void calcLinkWidgetRect(int zoomFactor, int globalIndentScaled);

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
