#ifndef EGNODEWIDGET_H
#define EGNODEWIDGET_H

#include <QWidget>

#include "qtinterface/egQtInterface.h"

const int portPointsCount {4};
const int portRectSize {10};

class EgNodeWidget : public QWidget
{
    Q_OBJECT

public:
    QPainter* nodePainter   {nullptr};
    QPixmap*  pixmap        {nullptr};

    QString labelText       {"New node"};
    QString descText        {""};
    QColor  fillColor       {Qt::lightGray};

    EgDataNodeIDType dataNodeID     {0};
    EgDataNodeIDType detailsLayerID {0};

    bool nodeFormOkFlag    {false};

    bool nodeResizeMode    {false};
    // bool nodeShowCorner    {false};
    bool nodeDeleteMode    {false};

    int  zoomFactor {0};
    int  actionMode {0};

    egRect nodeRect;

    EgNodeWidget(QWidget *parent = 0);
    ~EgNodeWidget();

    void alignToGrid(int& coord);

    void calcPortPointScaled(int& portSide, QPoint& clickPoint, QPoint& portPoint, int& sideCoord);
    void calcPortPointOrig(QPoint& clickPointScaled, int zoomFactor, int& portSide, int& sideCoordOrig, int& pointOrigX, int& pointOrigY);
    void getLinkPointOrig(int portSide, int sideCoordOrig, int& pointOrigX, int& pointOrigY);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    // void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
};

#endif // EGNODEWIDGET_H
