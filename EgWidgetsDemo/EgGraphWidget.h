#ifndef EGGRAPHWIDGET_H
#define EGGRAPHWIDGET_H

#include "metainfo/egDatabase.h"
#include "links/egLinks.h"
#include "metainfo/egLayers.h"
#include "metainfo/egOneLayer.h"
#include "qtinterface/egQtInterface.h"

#include <QDrag>
#include <QFrame>
#include <QPainter>
#include <QMap>

// #include <unordered_set>

class EgGraphForm;
class NodeForm;
class NodesTableForm;
class EgNodeWidget;
class EgLinkWidget;

const int maxZoomFactor      {8};

const int globalIndentOrig   {8};
const int gridSizeOrig       {16};
const int resizeSpotAreaOrig {16};
const int minLinkWidgetSize  {0};

const int defNewNodeWidth    {75};
const int defNewNodeHeight   {60};

const qreal arrowLengthOrig  {20};
    // operation modes
const int moveResizeMode           {1};
const int connectMode        {2};
const int linkEditMode       {3};
const int nodeDeleteMode     {4};
const int detailsLayerMode   {5};

// typedef void (*serialLoadStoreFunctionType) (EgDataNode& dataNode);
void loadNodeObjectFromDb (EgDataNode& dataNode);  // new widget ptr stored to dataNode.serialDataPtr
void storeNodeObjectToDb  (EgDataNode& dataNode);
void loadLinkObjectFromDb (EgDataNode& dataNode);  // new widget ptr stored to dataLink.serialDataPtr
void storeLinkObjectToDb  (EgDataNode& dataNode);

class EgGraphWidget : public QFrame
{
public:
    EgGraphForm*    myForm      {nullptr};
    NodeForm*       nodeForm    {nullptr};
    NodesTableForm* tableForm   {nullptr};

    EgDatabase   graphDB;
    EgLayers     graphLayers;

    EgDataNodesSet* graphNodes  {nullptr};
    EgLinksSet*     graphLinks  {nullptr};

    int zoomFactor                  { 0 };
    EgDataNodeIDType layerID        { 1 };
    EgDataNodeIDType parentLayerID  { 0 };

    egRect layerCanvas; // corner, size, egPoint/Size, origX/W, origY/H, scaledX/W, scaledY/H

    qreal scaledArrowLength   {arrowLengthOrig};
    int scaledGlobalIndent    {globalIndentOrig};

    int actionMode            {moveResizeMode};
    bool resizeMode           {false}; // submode of move mode

    QColor newFillColor       {Qt::lightGray};
    QPoint dragStart;
    int oldNodeWidthOrig  {0};
    int oldNodeHeightOrig {0};

    QList<EgNodeWidget*> deleteWidgets;

    EgNodeWidget* dataNodeWidget    {nullptr};
    EgNodeWidget* targetNodeWidget  {nullptr};
    EgNodeWidget* tmpNodeWidget     {nullptr};

    EgLinkWidget* arrowIcon         {nullptr};
    EgLinkWidget* linkEditSelected  {nullptr};
    // bool          isOutLinkselected {true};

    QByteArray*   itemData          {nullptr};
    QPainter*     globPainter       {nullptr};
    QPixmap*      pixmapTmp         {nullptr};

    explicit EgGraphWidget(QWidget* parent = nullptr);
    ~EgGraphWidget();

    void clearLayer();

    void setActionMode(int actMode);
    void OpenTableForm();

    void LoadDataNodes();
    void StoreDataNodes();

    void LoadLayersInfo();
    void LoadLayer();
    void LayerUp();

    void LoadDataLinks();
    inline void ShowDataLink(EgLinkWidget* newLinkWidget);
    void StoreDataLinks();

    // void PrintNodeLinks(EgDataNode& dataNode);
    inline void UpdateOneDataLink(EgLinkWidget* updLinkInfo);
    void UpdateLinksAftMove(EgDataNode& dataNode, QPoint& deltaPoint);
    void UpdateLinksAftResize(EgDataNode& dataNode, int oldW, int oldH);
    void DeleteLinksOfNode(EgDataNode& dataNode);

    inline void rescaleZoom();

    inline EgNodeWidget* getNodeWidget(EgDataNodeIDType nodeID); // FIXME check usage

    inline void moveResizeNodeWidget(EgNodeWidget* theWidget);
    void moveResizeLinkWidget(EgLinkWidget* theWidget);

    inline void updateLayerCanvas();

    inline void connectPressAction(QPoint clickPoint, QDrag* dragPtr);
    inline void movePressAction(QPoint clickPoint, QDrag* dragPtr);
    inline void linkEditPressAction();
    inline void nodeDeletePressAction();
    inline void detailsLayerPressAction();

    void clearEditLink(); // from peer form button
    inline void markEditedLink(EgLinkWidget* nextLinkDataPtr, bool editLinkIsOutLink);
    inline void getNextLinkOfNode();

    inline void connectDragAction(QPoint dragPoint);
    inline void resizeDragAction(QPoint dragPoint);

    inline void connectDropAction(QPoint dropPoint);
    inline void moveDropAction(QPoint dropPoint, bool newNode);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

    void paintEvent(QPaintEvent *event) override;
    // void resizeEvent(QResizeEvent *event) override;

    void showEvent(QShowEvent *event) override;
};

#endif // EGGRAPHWIDGET_H
