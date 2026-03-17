#ifndef EGGRAPHWIDGET_H
#define EGGRAPHWIDGET_H

#include "../../../src/metainfo/egDatabase.h"
#include "../../../src/links/egLinks.h"
#include "../../../src/metainfo/egLayers.h"
#include "../../../src/metainfo/egOneLayer.h"
#include "../../../src/qtinterface/egQtInterface.h"

#include <QDrag>
#include <QFrame>
#include <QPainter>
#include <QMap>

#include <unordered_set>

class EgGraphForm;
class NodeForm;
class EgNodeWidget;
class EgLinkWidget;

const int maxZoomFactor      {8};

const int globalIndentOrig   {8};
const int gridSizeOrig       {8};
const int resizeSpotAreaOrig {16};
const int minLinkWidgetSize  {0};

const int defNewNodeWidth    {85};
const int defNewNodeHeight   {65};

const qreal arrowLengthOrig  {20};
    // operation modes
const int moveMode           {1};
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
    EgGraphForm*  myForm        {nullptr};
    NodeForm*     nodeForm      {nullptr};

    EgDatabase   graphDB;
    EgLayers     graphLayers;

    EgDataNodesSet* graphNodes {nullptr};
    EgLinksSet*     graphLinks {nullptr};

    std::string dataNodesName   {"nodes"};
    std::string dataLinksName   {"links"};

    /* std::unordered_set<std::string> dataNodesNames;
    std::unordered_set<std::string> dataLinksNames;

    std::unordered_map<std::string, EgDataNodesType*> dataNodesMap;
    std::unordered_map<std::string, EgLinksType*>     dataLinksMap; */

    int zoomFactor  {0};

    int layerID    {1};
    int maxLayerNum {2};

    egRect layerCanvas; // corner, size, egPoint/Size, origX/W, origY/H, scaledX/W, scaledY/H
    // bool showCanvasRectangle {false}; //  ==> zoomFactor

    /*struct egPoint {
        int origX; // x or width
        int origY; // y or height
        int scaledX;
        int scaledY;
    };

    struct egSize {
        int origW; // x or width
        int origH; // y or height
        int scaledW;
        int scaledH;
    };

    struct egRect {
        egPoint corner;
        egSize  size;
    };

    origToScaledRect   (egRect& rect, int zoomFactor);
    scaledToOrigRect   (egRect& rect, int zoomFactor);
*/

    /* QPoint globCanvasOrig;
    QPoint globCanvasMinArea  {0,0}; // area to fit all nodes

    QPoint globCanvasScaledCorner;
    int    globCanvasScaledWidth;
    int    globCanvasScaledHeight; */

    qreal scaledArrowLength   {arrowLengthOrig};
    int scaledGlobalIndent    {globalIndentOrig};

    int actionMode            {moveMode};
    bool resizeMode           {false}; // submode of move mode

    QPoint dragStart;
    int oldWidgetWidth  {0};
    int oldWidhetHeight {0};

    EgNodeWidget* dataNodeWidget    {nullptr};
    EgNodeWidget* targetNodeWidget  {nullptr};
    EgNodeWidget* tmpNodeWidget     {nullptr};

    EgLinkWidget* arrowIcon         {nullptr};
    EgLinkWidget* linkEditSelected  {nullptr};
    bool          isOutLinkselected {true};

    QByteArray*   itemData          {nullptr};
    QPainter*     globPainter       {nullptr};
    QPixmap*      pixmapTmp         {nullptr};

    explicit EgGraphWidget(QWidget* parent = nullptr);
    ~EgGraphWidget();

    void clear();
    void clearLayer();

    void LoadDataNodes();
    void StoreDataNodes();

    void LoadLayersInfo();
    void LoadLayer();

    void LayerUp();
    void LayerDown();

    void LoadDataLinks();
    inline void ShowDataLink(EgLinkWidget* newLinkWidget);
    void StoreDataLinks();

    // void PrintNodeLinks(EgDataNode& dataNode);
    inline void UpdateOneDataLink(EgLinkWidget* updLinkInfo);
    void UpdateLinksAftMove(EgDataNode& dataNode, QPoint& deltaPoint);
    void UpdateLinksAftResize(EgDataNode& dataNode, int oldW, int oldH);
    void DeleteLinksOfNode(EgDataNode& dataNode);

    int  calcLinkQuadrant(QPoint startPoint, QPoint currPoint);

    inline void rescaleZoom();

    // inline void calcOuterCorner(); // TODO update canvas area after move/resize
    inline void adjustPointToCanvas(QPoint& thePoint);
    inline void adjustPointToGrid(QPoint& thePoint);
    inline void adjustToGridX(int& coordX);
    inline void adjustToGridY(int& coordY);

    inline void moveResizeNodeWidget(EgNodeWidget* theWidget);
    void moveResizeLinkWidget(EgLinkWidget* theWidget);
    inline void updateGlobCanvas(EgNodeWidget* nodeWidget);

    inline void connectPressAction(QPoint clickPoint, QDrag* dragPtr);
    inline void movePressAction(QPoint clickPoint, QDrag* dragPtr);
    inline void linkEditPressAction(QPoint clickPoint);
    inline void nodeDeletePressAction();
    inline void detailsLayerPressAction();

    void clearEditLink(); // from peer form button
    inline void markEditedLink(EgLinkWidget* nextLinkDataPtr, bool editLinkIsOutLink);
    inline void getNextLinkOfNode();

    inline void connectDragAction(QPoint dragPoint);
    inline void moveDragAction(QPoint dragPoint);

    inline void connectDropAction(QPoint dropPoint);
    inline void moveDropAction(QPoint dropPoint, bool newNode);

    // void adjustNodeWidgetPosition(); // TODO move from canvas indents area

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
    void resizeEvent(QResizeEvent *event) override;

    void showEvent(QShowEvent *event) override;
};

#endif // EGGRAPHWIDGET_H
