#ifndef EGNODEWIDGET_H
#define EGNODEWIDGET_H

#include <QWidget>

#include "nodes/egDataNodesSet.h"

const int portPointsCount {4};
const int portRectSize {10};

class EgNodeWidget : public QWidget
{
    Q_OBJECT

public:
    QPainter* nodePainter  {nullptr};
    EgDataNodeIDType dataNodeID     {0};
    EgDataNodeIDType detailsLayerID {0};
    QString labelText      {"New node"};
    bool nodeFormOkFlag    {false};

    bool nodeResizeMode    {false};
    bool nodeShowCorner    {false};
    bool nodeDeleteMode    {false};

    int origCornerX        {0};
    int origCornerY        {0};
    int origRectW          {100};
    int origRectH          {100};

    int scaledCornerX      {0};    // top right corner
    int scaledCornerY      {0};
    int scaledRectW        {100};    // rectangle width
    int scaledRectH        {100};    // rectangle height

    EgNodeWidget(QWidget *parent = 0);
    ~EgNodeWidget();

    void calcPortPoint(int& portSide, QPoint& clickPoint, QPoint& portPoint);

    void calcOrigToScaled(int zoomFactor, QPoint& globCanvasScaled);
    void calcScaledToOrig(int zoomFactor, QPoint& globCanvasScaled);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    // void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif // EGNODEWIDGET_H
