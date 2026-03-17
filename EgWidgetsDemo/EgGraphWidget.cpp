#include "EgGraphWidget.h"
#include "EgNodeWidget.h"
#include "EgLinkWidget.h"
#include "EgGraphForm.h"
#include "NodeForm.h"
#include "ui_EgGraphForm.h"

#include <QtWidgets>
#include <QSizePolicy>
#include <QMessageBox>
#include <QPushButton>

using namespace std;

EgGraphWidget::EgGraphWidget(QWidget *parent)
    : QFrame(parent), itemData (new QByteArray), globPainter (new QPainter), pixmapTmp (new QPixmap)
{
    // myForm = (EgGraphForm*) parent;
    setFocusPolicy(Qt::WheelFocus); // setFocus()

    arrowIcon = new EgLinkWidget(this);  // dynamic connect link arrow
    arrowIcon-> setWhatsThis(QString("arrow"));
    arrowIcon-> hide();
    arrowIcon-> setAttribute(Qt::WA_DeleteOnClose);

    // cout << "EgGraphWidget() called" << endl;
}

EgGraphWidget::~EgGraphWidget()
{
    while ( QWidget* w = findChild<QWidget*>() )
        delete w;

    if (graphNodes && graphNodes-> isConnected)
        graphNodes-> clear();

    if (graphLinks && graphLinks-> linksDataStorage.isConnected)
        graphLinks-> linksDataStorage.clear();

    delete graphNodes;
    delete graphLinks;
    delete itemData;
    delete globPainter;
    delete pixmapTmp;
}


void EgGraphWidget::clear()
{
    clearEditLink();

    if (actionMode == connectMode)
        arrowIcon-> hide();

    /*for (auto nodesIter : dataNodesMap) { // clear all nodes types of layert
        graphNodes = nodesIter.second; */
    if (! graphNodes) {
        cout << "clear(): null nodes ptr" << endl;
    } else if (graphNodes-> isConnected)
    {
        for (auto nodesIter : graphNodes-> dataMap)
        {
            EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);
            delete nodeWidget;
        }
        graphNodes-> clear();
    }
    // }

    /* for (auto linksIter : dataLinksMap) { // clear all link types of layert
        graphLinks = linksIter.second; */
    if (! graphLinks) {
        cout << "ERROR clear(): null links ptr" << endl;
    } else if (graphLinks-> linksDataStorage.isConnected)
    {
        for (auto linksIter : graphLinks-> dataMap)
        {
            EgLinkWidget* linkWidget = static_cast<EgLinkWidget*> (linksIter.second-> serialDataPtr);
            delete linkWidget;
        }
        graphLinks-> linksDataStorage.clear();
    }
    // }

    zoomFactor = 0;

    /*globCanvasScaledCorner.setX(0);
    globCanvasScaledCorner.setY(0);
    globCanvasScaledWidth  = globCanvasOrig.x(); // FIXME set to window size
    globCanvasScaledHeight = globCanvasOrig.y(); */

    layerCanvas.corner.origX = 0;
    layerCanvas.corner.origY = 0;
    layerCanvas.corner.scaledX = 0;
    layerCanvas.corner.scaledY = 0;
    layerCanvas.size.scaledW = layerCanvas.size.origW;
    layerCanvas.size.scaledH = layerCanvas.size.origH;

    scaledArrowLength = arrowLengthOrig;

    this-> repaint();
}

void EgGraphWidget::clearLayer()
{
    clearEditLink();

    if (actionMode == connectMode)
        arrowIcon-> hide();
    // graphNodes = currentLayer -> getNextNodesType(); while (graphNodes) { ... graphNodes = currentLayer -> getNextNodesType(); }
    /* for (auto nodesIter : dataNodesNames) {
        auto findIter = dataNodesMap.find(nodesIter); // // try to find loaded nodes type
        if (findIter != dataNodesMap.end()) {
            graphNodes = findIter-> second; */
    if (! graphNodes) {
        cout << "clearLayer(): null nodes ptr" << endl;
    } else if (graphNodes-> isConnected) {
        for (auto nodesIter : graphNodes-> dataMap)
        {
            EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);
            delete nodeWidget;
            nodesIter.second-> serialDataPtr = nullptr;
        }
        graphNodes-> clear();
        graphNodes-> isConnected = false;
    }
    // }
    // }
    /*for (auto linksIter : dataLinksNames) {
        auto findIterLinks = dataLinksMap.find(linksIter); // // try to find loaded nodes type
        if (findIterLinks != dataLinksMap.end()) {
            graphLinks = findIterLinks-> second; */
    if (! graphLinks) {
        cout << "clearLayer(): null links ptr" << endl;
    } else if (graphLinks-> linksDataStorage.isConnected) {
        for (auto linksIter : graphLinks-> dataMap)
        {
            EgLinkWidget* linkWidget = static_cast<EgLinkWidget*> (linksIter.second-> serialDataPtr);
            delete linkWidget;
            linksIter.second-> serialDataPtr = nullptr;
        }
        graphLinks-> linksDataStorage.clear();
        graphLinks-> linksDataStorage.isConnected = false;
    }
    // }
    // }

    zoomFactor = 0;

    /*globCanvasScaledCorner.setX(0);
    globCanvasScaledCorner.setY(0);
    globCanvasScaledWidth  = globCanvasOrig.x(); // FIXME set to window size
    globCanvasScaledHeight = globCanvasOrig.y(); */

    layerCanvas.corner.origX = 0;
    layerCanvas.corner.origY = 0;
    layerCanvas.corner.scaledX = 0;
    layerCanvas.corner.scaledY = 0;
    layerCanvas.size.scaledW = layerCanvas.size.origW;
    layerCanvas.size.scaledH = layerCanvas.size.origH;

    scaledArrowLength = arrowLengthOrig;

    this-> repaint();
}

inline void EgGraphWidget::moveResizeNodeWidget(EgNodeWidget* theWidget)
{
    theWidget-> resize(theWidget->scaledRectW, theWidget->scaledRectH);
    theWidget-> move(theWidget->scaledCornerX, theWidget->scaledCornerY);
}

void EgGraphWidget::moveResizeLinkWidget(EgLinkWidget* theWidget)
{
    theWidget-> resize(theWidget->scaledRectW, theWidget->scaledRectH);
    theWidget-> move(theWidget->scaledCornerX,  theWidget->scaledCornerY);
}

inline void EgGraphWidget::updateGlobCanvas(EgNodeWidget* nodeWidget)
{
    layerCanvas.size.origW = std::max(layerCanvas.size.origW, nodeWidget->origCornerX + nodeWidget->origRectW + globalIndentOrig); // FIXME top left corner check
    layerCanvas.size.origH = std::max(layerCanvas.size.origH, nodeWidget->origCornerY + nodeWidget->origRectH + globalIndentOrig);

    origToScaledRect(layerCanvas, zoomFactor);

    setMinimumSize(layerCanvas.size.origW, layerCanvas.size.origH);
    // myForm-> scrollArea1->setMinimumSize(layerCanvas.size.origW, layerCanvas.size.origH);
}

inline void EgGraphWidget::rescaleZoom()
{
    cout << "rescaleZoom() zoomFactor: " << zoomFactor << endl;
    if (actionMode == connectMode)
        arrowIcon-> hide();

    scaledGlobalIndent = globalIndentOrig * (10 - zoomFactor)/10;
    scaledArrowLength  = arrowLengthOrig * (10 - zoomFactor)/10;

    origToScaledLayer(layerCanvas, zoomFactor);

    cout << "rescaleZoom() scaledW: " << layerCanvas.size.scaledW << " scaledH:" << layerCanvas.size.scaledH << endl;
    cout << "rescaleZoom() scaledX: " << layerCanvas.corner.scaledX << " scaledY:" << layerCanvas.corner.scaledY << endl;

    if (graphNodes-> isConnected)
    {
        for (auto nodesIter : graphNodes-> dataMap)
        {
            EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);
            QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
            nodeWidget-> calcOrigToScaled(zoomFactor, tmpPoint);
            moveResizeNodeWidget(nodeWidget);
            // cout << "rescaleZoom() nodeID: " << nodeWidget-> dataNodeID << endl;
        }
    }

    if (graphLinks->linksDataStorage.isConnected)
    {
        for (auto linksIter : graphLinks->dataMap)
        {
            EgLinkWidget* linkWidget = static_cast<EgLinkWidget*> (linksIter.second-> serialDataPtr);
            QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
            linkWidget-> calcOrigToScaled(zoomFactor, tmpPoint);
            linkWidget-> calcLinkWidgetRect(zoomFactor);
            moveResizeLinkWidget(linkWidget);
            linkWidget-> lower();
            // cout << "rescaleZoom() linkID: " << linkWidget->dataLinkID << endl;
        }
    }
    this-> repaint();
}


/*
inline void EgGraphWidget::calcOuterCorner()
{
    int maxOrigCanvX {0};
    int maxOrigCanvY {0};

    if (graphNodes-> isConnected)
    {
        for (auto nodesIter : graphNodes-> dataMap)
        {
            EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);
            maxOrigCanvX = std::max (maxOrigCanvX, nodeWidget-> origCornerX + nodeWidget->origRectW);
            maxOrigCanvY = std::max (maxOrigCanvY, nodeWidget-> origCornerY + nodeWidget->origRectH);
        }
    }
    cout << "calcOuterCorner(): " << dec << maxOrigCanvX << " : " << maxOrigCanvY << endl;
    cout << "globCanvasMinArea : " << globCanvasMinArea.x() << " : " <<  globCanvasMinArea.y() << endl;

    globCanvasMinArea.setX(maxOrigCanvX);
    globCanvasMinArea.setY(maxOrigCanvY);

    globCanvasOrig.setX(maxOrigCanvX);
    globCanvasOrig.setY(maxOrigCanvY);
} */

inline void EgGraphWidget::adjustToGridX(int& coordX)
{
    if (coordX > layerCanvas.size.origW)
        coordX = layerCanvas.size.origW - globalIndentOrig;
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

inline void EgGraphWidget::adjustToGridY(int& coordY)
{
    if (coordY > layerCanvas.size.origH)
        coordY = layerCanvas.size.origH - globalIndentOrig;
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

inline void EgGraphWidget::adjustPointToGrid(QPoint& thePoint)
{
    int coordX {thePoint.x()};
    int coordY {thePoint.y()};
    adjustToGridX(coordX);
    adjustToGridY(coordY);
    thePoint.setX(coordX);
    thePoint.setY(coordY);
}

void loadNodeObjectFromDb (EgDataNode& dataNode) //  pumped to dataNode.serialDataPtr
{
    EgNodeWidget* newNodePtr = new EgNodeWidget();
    dataNode.serialDataPtr = (void*) newNodePtr;

    newNodePtr-> dataNodeID = dataNode.dataNodeID;

    dataNode["name"]    >> newNodePtr-> labelText;
    dataNode["cornerX"] >> newNodePtr-> scaledCornerX;
    dataNode["cornerY"] >> newNodePtr-> scaledCornerY;
    dataNode["rectH"]   >> newNodePtr-> scaledRectH;
    dataNode["rectW"]   >> newNodePtr-> scaledRectW;

    newNodePtr-> origCornerX = newNodePtr-> scaledCornerX;
    newNodePtr-> origCornerY = newNodePtr-> scaledCornerY;
    newNodePtr-> origRectW   = newNodePtr-> scaledRectW;
    newNodePtr-> origRectH   = newNodePtr-> scaledRectH;
    // cout << "loadNodeObjectFromDb() nodeName: " << newNodePtr-> labelText.toStdString() << endl;
}

void storeNodeObjectToDb  (EgDataNode& dataNode)
{
    EgNodeWidget* nodeShowtimePtr = static_cast<EgNodeWidget*> (dataNode.serialDataPtr);

    dataNode["name"]    << nodeShowtimePtr-> labelText;
    // PrintByteArray(dataNode["name"]);
    dataNode["cornerX"] << nodeShowtimePtr-> origCornerX;
    dataNode["cornerY"] << nodeShowtimePtr-> origCornerY;
    dataNode["rectH"]   << nodeShowtimePtr-> origRectH;
    dataNode["rectW"]   << nodeShowtimePtr-> origRectW;

    nodeShowtimePtr-> scaledCornerX = nodeShowtimePtr-> origCornerX; // FIXME STUB
    nodeShowtimePtr-> scaledCornerY = nodeShowtimePtr-> origCornerY;
    nodeShowtimePtr-> scaledRectW = nodeShowtimePtr-> origRectW;
    nodeShowtimePtr-> scaledRectH = nodeShowtimePtr-> origRectH;
}

void loadLinkObjectFromDb (EgDataNode& dataNode) //  pumped to dataNode.serialDataPtr
{
    EgLinkWidget* newLinkPtr = new EgLinkWidget();
    dataNode.serialDataPtr = (void*) newLinkPtr;
    newLinkPtr-> dataLinkID = dataNode.dataNodeID;

    dataNode["toID"]        >> newLinkPtr-> nodeIDTo;
    dataNode["fromID"]      >> newLinkPtr-> nodeIDFrom;
    dataNode["portFrom"]    >> newLinkPtr-> portSideFrom;
    dataNode["portTo"]      >> newLinkPtr-> portSideTo;
    dataNode["lineType"]    >> newLinkPtr-> lineType;

    int tmpInt;
    dataNode["startPointX"] >> tmpInt;
    newLinkPtr-> startPointOrig.setX(tmpInt);
    dataNode["startPointY"] >> tmpInt;
    newLinkPtr-> startPointOrig.setY(tmpInt);

    dataNode["endPointX"]   >> tmpInt;
    newLinkPtr-> endPointOrig.setX(tmpInt);
    dataNode["endPointY"]   >> tmpInt;
    newLinkPtr-> endPointOrig.setY(tmpInt);
    // cout << "loadLinkObjectFromDb() dataLinkID: " << std::dec << newLinkPtr-> dataLinkID << endl;
}

void storeLinkObjectToDb  (EgDataNode& dataNode)
{
    EgLinkWidget* EgLinkWidgetPtr = static_cast<EgLinkWidget*> (dataNode.serialDataPtr);

    dataNode["toID"]        << EgLinkWidgetPtr-> nodeIDTo;
    dataNode["fromID"]      << EgLinkWidgetPtr-> nodeIDFrom;
    dataNode["startPointX"] << EgLinkWidgetPtr-> startPointOrig.x();
    dataNode["startPointY"] << EgLinkWidgetPtr-> startPointOrig.y();
    dataNode["endPointX"]   << EgLinkWidgetPtr-> endPointOrig.x();
    dataNode["endPointY"]   << EgLinkWidgetPtr-> endPointOrig.y();
    dataNode["portFrom"]    << EgLinkWidgetPtr-> portSideFrom;
    dataNode["portTo"]      << EgLinkWidgetPtr-> portSideTo;
    dataNode["lineType"]    << EgLinkWidgetPtr-> lineType;
    // PrintEgDataNodeFields(dataNode);
}
/*
void EgGraphWidget::PrintNodeLinks(EgDataNode& dataNode)
{
    // cout << " PrintNodeLinks() 'from' linkID: " << std::dec << nodePtrIterFrom.first;
    for (auto linksIterFrom : dataNode.inLinks)
    {
        for (auto nodePtrIterFrom : linksIterFrom.second)
        {
            cout << " PrintNodeLinks() 'from' linkID: " << std::dec << nodePtrIterFrom.first;
            if (nodePtrIterFrom.second)
                cout << " node IDs: " << std::dec << dataNode.dataNodeID << " <- " << nodePtrIterFrom.second->dataNodeID << endl;
        }
    }
    for (auto linksIterTo : dataNode.outLinks)
    {
        for (auto nodePtrIterTo : linksIterTo.second)
        {
            cout << " PrintNodeLinks() 'to' linkID: " << std::dec << nodePtrIterTo.first;
            if (nodePtrIterTo.second)
                cout << " node IDs: " << std::dec << dataNode.dataNodeID << " -> " << nodePtrIterTo.second->dataNodeID << endl;
        }
    }
}*/

void EgGraphWidget::UpdateLinksAftMove(EgDataNode& dataNode, QPoint& deltaPoint)
{
    EgDataNode* prevLinkDataNodePtr {nullptr};
    EgDataNode& selectedDataNode    {(*graphNodes)[dataNodeWidget->dataNodeID]}; // shortcut ref
        // out links
    EgLinkWidget* nextLinkDataPtr = static_cast<EgLinkWidget*> (selectedDataNode.getNextOutLinkSerialPtr(graphLinks-> linkBlueprintID, prevLinkDataNodePtr));
    while (nextLinkDataPtr) // out links loop
    {
        nextLinkDataPtr-> startPointScaled += deltaPoint;
        QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
        nextLinkDataPtr-> calcScaledToOrigStart(zoomFactor, tmpPoint);
        UpdateOneDataLink(nextLinkDataPtr);

        prevLinkDataNodePtr = &(graphLinks-> linksDataStorage[nextLinkDataPtr-> dataLinkID]);
        EgDataNode* linkedNode = selectedDataNode.getOutLinkedNode(graphLinks-> linkBlueprintID, prevLinkDataNodePtr);
        if (linkedNode)
            cout << "UpdateLinksAftMove() OUT linked node ID: " << std::dec << linkedNode-> dataNodeID << endl;

        nextLinkDataPtr = static_cast<EgLinkWidget*> (selectedDataNode.getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }
        // in links
    prevLinkDataNodePtr = nullptr;
    nextLinkDataPtr = static_cast<EgLinkWidget*> (selectedDataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    while (nextLinkDataPtr) // in links loop
    {
        nextLinkDataPtr-> endPointScaled += deltaPoint;
        QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
        nextLinkDataPtr-> calcScaledToOrigEnd(zoomFactor, tmpPoint);
        UpdateOneDataLink(nextLinkDataPtr);
        prevLinkDataNodePtr = &(graphLinks->linksDataStorage[nextLinkDataPtr-> dataLinkID]); // ++
        nextLinkDataPtr = static_cast<EgLinkWidget*> (selectedDataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }
}

void EgGraphWidget::UpdateLinksAftResize(EgDataNode& dataNode, int oldW, int oldH)
{
    EgDataNode* prevLinkDataNodePtr {nullptr};
    EgLinkWidget* nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    while (nextLinkDataPtr) // out links loop
    {
        nextLinkDataPtr-> updLinkAftResizeStart(static_cast<EgNodeWidget*>(dataNode.serialDataPtr), oldW, oldH);
        QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
        nextLinkDataPtr-> calcOrigToScaled(zoomFactor, tmpPoint);
        UpdateOneDataLink(nextLinkDataPtr);

        prevLinkDataNodePtr = &(graphLinks->linksDataStorage[nextLinkDataPtr-> dataLinkID]);
        nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }
        // in links
    prevLinkDataNodePtr = nullptr;
    nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    while (nextLinkDataPtr) // in links loop
    {
        nextLinkDataPtr-> updLinkAftResizeEnd(static_cast<EgNodeWidget*>(dataNode.serialDataPtr), oldW, oldH);
        QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
        nextLinkDataPtr-> calcOrigToScaled(zoomFactor, tmpPoint);
        UpdateOneDataLink(nextLinkDataPtr);

        prevLinkDataNodePtr = &(graphLinks->linksDataStorage[nextLinkDataPtr-> dataLinkID]);
        nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }
}

void EgGraphWidget::DeleteLinksOfNode(EgDataNode& dataNode)
{
    EgDataNode* prevLinkDataNodePtr {nullptr}; // del all out links
    EgLinkWidget*   delLinkWidget = static_cast<EgLinkWidget*> (dataNode.getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    while (delLinkWidget) // out links loop
    {
        prevLinkDataNodePtr = &(graphLinks->linksDataStorage[delLinkWidget-> dataLinkID]);
        graphLinks->DeleteOutLink(delLinkWidget-> dataLinkID);
        delete delLinkWidget;
        delLinkWidget = static_cast<EgLinkWidget*> (dataNode.getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }

    prevLinkDataNodePtr = nullptr; // del all in links
    delLinkWidget = static_cast<EgLinkWidget*> (dataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    while (delLinkWidget) // in links loop
    {
        prevLinkDataNodePtr = &(graphLinks->linksDataStorage[delLinkWidget-> dataLinkID]);
        graphLinks->DeleteInLink(delLinkWidget-> dataLinkID);
        delete delLinkWidget;
        delLinkWidget = static_cast<EgLinkWidget*> (dataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }
}

void EgGraphWidget::LoadDataNodes()
{
    if (! graphNodes) {
        cout << "ERROR LoadDataNodes(): null nodes ptr" << endl;
        return;
    }
    if (! graphNodes-> isConnected)
        graphNodes-> Connect(graphNodes-> nodesSetName, graphDB);
    if (! graphNodes-> isDataLoaded)
        graphNodes-> LoadAllNodes();
    // PrintDataNodesContainer(*(graphNodes.nodesContainer));
    for (auto nodesIter : graphNodes-> dataMap) // 17 [first, second], <11 = dataFieldsNames.begin(); fieldsIter != dataFieldsNames.end(); ++fieldsIter) {
    // PrintEgDataNodeFields(*(nodesIter.second));
    {
        EgNodeWidget* newNodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);
        // cout << "loaded data node name: " << newNodePtr-> labelText.toStdString() << endl;
        newNodeWidget-> setParent(this);
        newNodeWidget-> setWhatsThis(QString("node"));

        moveResizeNodeWidget(newNodeWidget);
        newNodeWidget->show();
        updateGlobCanvas(newNodeWidget);
    }
}

void EgGraphWidget::LoadLayersInfo()
{
    if ( ! graphLayers.layersStorage.isConnected) {
        graphLayers.ConnectLayers("demoAppLayers", graphDB); // FIXME literal
        graphLayers.LoadLayers();
    }

    /* EgDataNodeIDType layerID;
    graphLayers.createDetailsLayer(777, layerID, 1000, 600, "layerNodesBlueprint", "layerLinksBlueprint");
    cout << "LoadLayersInfo() createDetailsLayer ID: " << layerID << endl; */
}

void EgGraphWidget::   LoadLayer()
{
    clearLayer();
    if(! graphLayers[layerID]) {
        cout << "LoadLayer() layer not found in graphLayers, ID: " << layerID << endl;
        return;
    }

    layerCanvas.size.origW = graphLayers[layerID]-> layerWidth;
    layerCanvas.size.origH = graphLayers[layerID]-> layerHeight;

    setMinimumSize(layerCanvas.size.origW, layerCanvas.size.origH);

    cout << "LoadLayer() W: " << layerCanvas.size.origW << " H: " << layerCanvas.size.origH << endl;
    graphLayers[layerID]-> getLayerNodes(graphNodes, loadNodeObjectFromDb, storeNodeObjectToDb);
    cout << "LoadLayer() graphNodes: " << graphNodes-> nodesSetName << endl;
    LoadDataNodes();
    graphLayers[layerID]-> getLayerLinks(graphLinks, loadLinkObjectFromDb, storeLinkObjectToDb);
    LoadDataLinks();

    /* graphLayers.getLayerNodesAndLinks(dataNodesNames, dataLinksNames, layerID);
    for (auto nodesIter : dataNodesNames) {
        auto findIter = dataNodesMap.find(nodesIter); // // try to find loaded nodes type
        if (findIter == dataNodesMap.end()) {
            graphNodes = new EgDataNodesType;
            graphNodes-> Connect(nodesIter, graphDB);
            graphNodes-> serialLoadFunction  = &loadNodeObjectFromDb;
            graphNodes-> serialStoreFunction = &storeNodeObjectToDb;
            dataNodesMap.insert(std::make_pair(nodesIter, graphNodes));
            // cout << "LoadLayer() dataNodesMap insert: " << nodesIter << endl;
        } else {
            graphNodes = findIter-> second;
            // cout << "LoadLayer() dataNodesMap found type: " << nodesIter << endl;
        }
        LoadDataNodes();
    }
    for (auto linksIter : dataLinksNames) {
        // cout << "dataLinksNames: " << linksIter << endl;
        auto findIterLinks = dataLinksMap.find(linksIter); // // try to find loaded nodes type
        if (findIterLinks == dataLinksMap.end()) {
            graphLinks = new EgLinksType;
            graphLinks-> ConnectLinks(linksIter, graphDB);
            graphLinks-> linksDataStorage.serialLoadFunction  = &loadLinkObjectFromDb;
            graphLinks-> linksDataStorage.serialStoreFunction = &storeLinkObjectToDb;

            dataLinksMap.insert(std::make_pair(linksIter, graphLinks));
            // cout << "LoadLayer() dataLinksMap insert: " << linksIter << endl;
        } else {
            graphLinks = findIterLinks-> second;
            // cout << "LoadLayer() dataLinksMap found type: " << linksIter << endl;
        }
        LoadDataLinks();
    } */
}

void EgGraphWidget::LayerUp()
{
    if (layerID > 1) {
        StoreDataNodes();
        StoreDataLinks();
        layerID--;
        LoadLayer();
        QString numStr = QString::number(layerID);
        myForm->ui->layerNumLabel-> setText(numStr);
    }
    // cout << "LayerUp() : " << layerNum << endl;
}

void EgGraphWidget::LayerDown()
{
    if (layerID < maxLayerNum) {
        StoreDataNodes();
        StoreDataLinks();
        layerID++;
        LoadLayer();
        QString numStr = QString::number(layerID);
        myForm->ui->layerNumLabel-> setText(numStr);
    }
    // cout << "LayerDown() : " << layerNum << endl;
}

void EgGraphWidget::StoreDataNodes()
{
    if (! graphNodes) {
        cout << "ERROR StoreDataNodes(): null nodes ptr" << endl;
        return;
    }
    if (graphNodes-> isConnected)
        graphNodes-> Store();
    else
        cout << "StoreDataNodes(): not connected to egDb" << endl;
}

void EgGraphWidget::LoadDataLinks()
{
    // cout << "LoadDataLinks() isDataLoaded: " << graphLinks-> linksDataStorage.isDataLoaded << endl;
    if (! graphLinks) {
        cout << "ERROR LoadDataLinks(): null links ptr" << endl;
        return;
    }
    if (! graphLinks-> linksDataStorage.isDataLoaded) {
        // cout << "LoadDataLinks() LoadAllNodes" << endl;
        // graphLinks-> linksDataStorage.nodesContainer-> PrintNodesChain();
        graphLinks-> linksDataStorage.LoadAllNodes();
        graphLinks-> ResolveNodesIDsToPtrs(*graphNodes, *graphNodes);
    }
    for (auto linksIter : graphLinks-> dataMap)
    {
        EgLinkWidget* newLinkWidget = static_cast<EgLinkWidget*> (linksIter.second-> serialDataPtr);
        newLinkWidget-> setParent(this);
        newLinkWidget-> setWhatsThis(QString("link"));
        // cout << "loaded data link ID: " << newLinkWidget-> dataLinkID << endl;
        ShowDataLink(newLinkWidget);
    }
}

inline void EgGraphWidget::ShowDataLink(EgLinkWidget* newLinkWidget)
{
    newLinkWidget-> startPointScaled = newLinkWidget-> startPointOrig;
    newLinkWidget-> endPointScaled   = newLinkWidget-> endPointOrig;

    newLinkWidget->calcLinkWidgetRect(zoomFactor);
    moveResizeLinkWidget(newLinkWidget); // , newcornerX, newcornerY, newrectW, newrectH);

    newLinkWidget-> lower();
    newLinkWidget-> show();
    // cout << "ShowDataLink() dataLinkID: " << newIcon->dataLinkID << endl;
}

inline void EgGraphWidget::UpdateOneDataLink(EgLinkWidget* updLinkInfo)
{
    updLinkInfo-> calcLinkWidgetRect(zoomFactor);
    moveResizeLinkWidget(updLinkInfo); // , newcornerX, newcornerY, newrectW, newrectH);
    graphLinks-> MarkUpdatedLink(updLinkInfo-> dataLinkID);
    cout << "UpdateOneDataLink() dataLinkID: " << updLinkInfo-> dataLinkID << endl;
}

void EgGraphWidget::StoreDataLinks()
{    if (! graphLinks) {
        cout << "ERROR StoreDataLinks(): null links ptr" << endl;
        return;
    }
    graphLinks-> StoreLinks();
}

void EgGraphWidget::paintEvent(QPaintEvent* event)
{
    // cout << "paintEvent() globCanvasScaledCorner: " << globCanvasScaledCorner.x() << " : " << globCanvasScaledCorner.y()
    //      << " : " << globCanvasScaledWidth << " : " << globCanvasScaledHeight << " showCanvasRectangle: " << showCanvasRectangle << endl;
    // if (zoomFactor)
    {
        QPainter globPainter(this); // glob canvas frame
        globPainter.setPen(Qt::darkGray);
        globPainter.drawRect(layerCanvas.corner.scaledX, layerCanvas.corner.scaledY, layerCanvas.size.scaledW, layerCanvas.size.scaledH);
        globPainter.end();
    }
}

inline void EgGraphWidget::connectPressAction(QPoint clickPoint, QDrag* dragPtr)
{
    arrowIcon-> hide();
    QPoint portPoint;
    QPoint inNodeClickPoint = clickPoint - dataNodeWidget->pos();
    dragPtr-> setHotSpot(inNodeClickPoint);

    dataNodeWidget->calcPortPoint(arrowIcon->portSideFrom, inNodeClickPoint, portPoint);
    dragStart = dataNodeWidget->pos() + portPoint;

    arrowIcon->startPointScaled = dragStart;

    QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
    arrowIcon->calcScaledToOrigStart(zoomFactor, tmpPoint);
    arrowIcon->lineType = directConnect;

    QDataStream dataStream(itemData, QIODevice::WriteOnly);
    dataStream << inNodeClickPoint;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", *itemData);

    dragPtr-> setMimeData(mimeData);

    myForm-> dragDropAction = true;
    dragPtr-> exec(Qt::MoveAction);
    myForm-> dragDropAction = false;
//    cout << "connectPressAction() exit" << endl;
}

inline void EgGraphWidget::movePressAction(QPoint clickPoint, QDrag* dragPtr)
{
    QPoint nodeOffset = clickPoint - dataNodeWidget->pos();
    dragPtr-> setHotSpot(nodeOffset);
    // cout << "movePressAction() nodeOffset "  << nodeOffset.x() << " : " << nodeOffset.y() << endl;
    int resizeSpotAreaScaled = resizeSpotAreaOrig; // check resize corner
    resizeMode = (((dataNodeWidget-> width() - nodeOffset.x()) < resizeSpotAreaScaled ) && ((dataNodeWidget-> height() - nodeOffset.y()) < resizeSpotAreaScaled));

    if (! resizeMode) // move
    {
        QPixmap pixmap = dataNodeWidget-> grab(); // had to be local

        QDataStream dataStream(itemData, QIODevice::WriteOnly);
        dataStream << pixmap << nodeOffset;
        QMimeData *mimeData = new QMimeData;
        mimeData->setData("application/x-dnditemdata", *itemData);

        dragPtr-> setMimeData(mimeData);
        dragPtr-> setPixmap(pixmap);
        dataNodeWidget-> hide();
    }
    else // resize
    {
        oldWidgetWidth  = dataNodeWidget-> width();
        oldWidhetHeight = dataNodeWidget-> height();
        // cout << "movePressAction() oldWidgetWidth: "  << oldWidgetWidth << " oldWidhetHeight: " << oldWidhetHeight << endl;
        QDataStream dataStream(itemData, QIODevice::WriteOnly);
        dataStream << nodeOffset;
        QMimeData *mimeData = new QMimeData;
        mimeData->setData("application/x-dnditemdata", *itemData);

        dragPtr-> setMimeData(mimeData);
        dataNodeWidget->repaint();
    }
        // drag action
    myForm-> dragDropAction = true;
    dragPtr-> exec(Qt::MoveAction);
    myForm-> dragDropAction = false;

    if (! resizeMode)
        dataNodeWidget->show();
    else
    {
        dataNodeWidget-> nodeResizeMode = false;
        dataNodeWidget-> repaint(); // clear resize circle knob
    }
}

void EgGraphWidget::clearEditLink()
{
    if (linkEditSelected) {
        linkEditSelected-> lower();
        linkEditSelected-> setAcceptDrops(false);
        linkEditSelected-> isEditSelected = false;
        linkEditSelected-> repaint();
        linkEditSelected = nullptr;
    }
}

inline void EgGraphWidget::linkEditPressAction(QPoint clickPoint)
{
    // cout << "linkEditPressAction() (start) node ID: "  << dataNodeWidget->dataNodeID << endl;
    // PrintResolvedLinks(graphNodes[dataNodeWidget->dataNodeID]);
    clearEditLink();
    EgLinkWidget* firstLinkDataPtr =
        static_cast<EgLinkWidget*> ((*graphNodes)[dataNodeWidget->dataNodeID].getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, nullptr));

    if (firstLinkDataPtr)
    {
        markEditedLink(firstLinkDataPtr, true);
        // cout << "linkEditPressAction() first OUT link data ptr found, ID: "  << firstLinkDataPtr-> dataLinkID << endl;
        return;
    }

    firstLinkDataPtr =
        static_cast<EgLinkWidget*> ((*graphNodes)[dataNodeWidget->dataNodeID].getNextInLinkSerialPtr(graphLinks->linkBlueprintID, nullptr));
    if (firstLinkDataPtr)
    {
        markEditedLink(firstLinkDataPtr, false);
        // cout << "linkEditPressAction() first IN link data ptr found, ID: "  << firstLinkDataPtr-> dataLinkID << endl;
        return;
    }
}

void EgGraphWidget::nodeDeletePressAction()
{
    // cout << " Double click at node " << dataNode->labelText.toStdString() << endl;
    QMessageBox msgBox;
    msgBox.setText("Delete this data node?");
    // msgBox.setInformativeText("This action cannot be undone.");
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel); // Set Cancel as the default focused button

    dataNodeWidget-> nodeDeleteMode = true;
    dataNodeWidget-> repaint(); // clear resize circle

    int ret = msgBox.exec(); // Show the dialog and wait for user input
    if (ret == QMessageBox::Ok)
    {
        // cout << "Node delete simulation: " << dataNode->labelText.toStdString() << endl;
        DeleteLinksOfNode((*graphNodes)[dataNodeWidget->dataNodeID]);
        graphNodes->DeleteDataNode(dataNodeWidget->dataNodeID);
        delete dataNodeWidget;  // widget
    }
    else
    {
        dataNodeWidget-> nodeDeleteMode = false;
        dataNodeWidget-> repaint();
    }
}


void EgGraphWidget::detailsLayerPressAction()
{
    // check if details layer exists

    // create new details layer
    EgDataNodeIDType newLayerID;
    graphLayers.createDetailsLayer(dataNodeWidget->dataNodeID, newLayerID, 1000, 800, "layerNodesBlueprint", "layerLinksBlueprint");
    cout << " detailsLayerPressAction() node ID: " << dataNodeWidget->dataNodeID << " new layer ID: " << newLayerID << endl;
    // store parent ID

    // open layer
    layerID = newLayerID; // FIXME STUB
    LoadLayer();
}

void EgGraphWidget::mousePressEvent(QMouseEvent *event)
{
    resizeMode = false;
    dataNodeWidget = static_cast<EgNodeWidget*>(childAt(event->pos()));       // check if click on a widget
    if (!dataNodeWidget || (dataNodeWidget-> whatsThis() != QString("node"))) // widget is not node
    {
        dataNodeWidget = nullptr;
        return;
    }

    if (actionMode == nodeDeleteMode) {
        nodeDeletePressAction();
        return;
    }

    if (actionMode == linkEditMode) {
        linkEditPressAction(event->pos());
        return;
    }

    if (actionMode == detailsLayerMode) {

        detailsLayerPressAction();
        return;
    }

    QDrag* drag = new QDrag(this);
    dragStart   = event->pos();

    if (actionMode == connectMode) // mode select
        connectPressAction(event->pos(), drag);
    else if (actionMode == moveMode)
        movePressAction(event->pos(), drag);

    itemData-> clear(); // cleanup
    if (drag) delete drag;
    dataNodeWidget = nullptr;
}

void EgGraphWidget::dragEnterEvent(QDragEnterEvent *event)
{
    // cout << "EgGraphWidget dragEnterEvent()" << endl;
    if (myForm-> dragDropAction) {
            event-> acceptProposedAction();
    }
}

void EgGraphWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    if ((actionMode == connectMode) && myForm-> dragDropAction) {
        arrowIcon-> hide();
    }
}


int EgGraphWidget::calcLinkQuadrant(QPoint startPoint, QPoint currPoint)
{
    if (currPoint.x() >= startPoint.x() && currPoint.y() <= startPoint.y())
        return 0;
    if (currPoint.x() >= startPoint.x() && currPoint.y() >= startPoint.y())
        return 1;
    if (currPoint.x() <= startPoint.x() && currPoint.y() >= startPoint.y())
        return 2;
    return 3;
}

inline void EgGraphWidget::connectDragAction(QPoint dragPoint)
{
    QPoint itemSize = dragPoint - dragStart;
    int itemSizeW = max(std::abs(itemSize.x()), minLinkWidgetSize) + scaledGlobalIndent*2;
    int itemSizeH = max(std::abs(itemSize.y()), minLinkWidgetSize) + scaledGlobalIndent*2;

    arrowIcon-> resize ( itemSizeW,  itemSizeH);
    arrowIcon->endPointOrig = dragPoint;
    arrowIcon->endPointScaled = dragPoint;

    arrowIcon-> tmpLinkQuadrant = calcLinkQuadrant(dragStart, dragPoint);
    // cout << "tmpLinkQuadrant: " << arrowIcon-> tmpLinkQuadrant << endl;

    switch (arrowIcon-> tmpLinkQuadrant) {
    case 0:
        arrowIcon-> move (dragStart.x() - scaledGlobalIndent, dragStart.y()-itemSizeH + scaledGlobalIndent);
        break;
    case 1:
        arrowIcon-> move (dragStart.x() - scaledGlobalIndent, dragStart.y() - scaledGlobalIndent);
        break;
    case 2:
        arrowIcon-> move (dragStart.x()-itemSizeW + scaledGlobalIndent, dragStart.y() - scaledGlobalIndent);
        break;
    default:
        arrowIcon-> move (dragStart.x()-itemSizeW + scaledGlobalIndent, dragStart.y()-itemSizeH + scaledGlobalIndent); // + offset);  // event->position().toPoint()            break;
    }

    arrowIcon->scaledCornerX = arrowIcon->x();
    arrowIcon->scaledCornerY = arrowIcon->y();
    arrowIcon-> show();
}

inline void EgGraphWidget::moveDragAction(QPoint dragPoint)
{   // resize func only
    QPoint offset = dragPoint - dragStart;

    offset.setX(offset.x() + oldWidgetWidth);
    offset.setY(offset.y() + oldWidhetHeight);

    int defNewNodeWidthScaled   = defNewNodeWidth * (10 - zoomFactor)/10;
    int defNewNodeHeightScaled  = defNewNodeHeight * (10 - zoomFactor)/10;

    if (offset.x() < defNewNodeWidthScaled)
        offset.setX(defNewNodeWidthScaled);
    if (offset.y() < defNewNodeHeightScaled)
        offset.setY(defNewNodeHeightScaled);

    if (dataNodeWidget-> scaledRectW != offset.x() && dataNodeWidget->scaledRectH != offset.y())
    {
        dataNodeWidget-> scaledRectW   = offset.x();
        dataNodeWidget-> scaledRectH   = offset.y();

        dataNodeWidget-> origRectW = dataNodeWidget->scaledRectW * 10/(10 - zoomFactor);
        dataNodeWidget-> origRectH = dataNodeWidget->scaledRectH * 10/(10 - zoomFactor);

        graphNodes-> MarkUpdatedDataNode(dataNodeWidget->dataNodeID);

        dataNodeWidget-> resize(dataNodeWidget->scaledRectW,  dataNodeWidget->scaledRectH);  // FIXME update global canvas
    }
}

void EgGraphWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if ((actionMode == connectMode) && myForm-> dragDropAction) // connect nodes drag
        connectDragAction(event->position().toPoint());
    else if (resizeMode && myForm-> dragDropAction && dataNodeWidget) // node resize drag // ((event->source() == this)))
        moveDragAction(event->position().toPoint());
}

inline void EgGraphWidget::connectDropAction(QPoint dropPoint)
{
    arrowIcon-> hide();
    arrowIcon-> lower();

    QWidget* targetWidget = childAt(dropPoint);
    if (targetWidget)
    {
        // cout << "targetWidget whatsThis(): " << targetWidget->whatsThis().toStdString() << endl;
        targetNodeWidget = static_cast<EgNodeWidget*>(targetWidget);

        if (targetNodeWidget && dataNodeWidget && (targetNodeWidget-> whatsThis() == QString("node")) && (targetNodeWidget != dataNodeWidget)) // check for valid node and port to connect
        {

            targetNodeWidget-> repaint(); // show port

            EgLinkWidget* newLinkWidget = new EgLinkWidget(this);
            newLinkWidget-> setWhatsThis(QString("link"));
            newLinkWidget-> setAttribute(Qt::WA_DeleteOnClose);

            newLinkWidget->nodeIDFrom = dataNodeWidget->dataNodeID;
            newLinkWidget->nodeIDTo = targetNodeWidget->dataNodeID;

            newLinkWidget->startPointOrig   = arrowIcon->startPointOrig;
            newLinkWidget->startPointScaled = arrowIcon->startPointScaled;

            QPoint endPortPoint;
            QPoint inNodeClickPoint = dropPoint - targetNodeWidget->pos();
            targetNodeWidget->calcPortPoint(newLinkWidget->portSideTo, inNodeClickPoint, endPortPoint);

            newLinkWidget->portSideFrom = arrowIcon->portSideFrom;
            newLinkWidget->calcLinkVisualType();

            newLinkWidget->endPointScaled = targetNodeWidget->pos() + endPortPoint;
            QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
            newLinkWidget->calcScaledToOrigEnd(zoomFactor, tmpPoint);
            // cout << "connectDropAction() targetNodeWidget->pos(): " << targetNodeWidget->pos().x() << " : " << targetNodeWidget->pos().y()  << endl;
            if (newLinkWidget->portSideTo == portSideNorth || newLinkWidget->portSideTo == portSideNorth) {
                int tmpX = newLinkWidget-> endPointOrig.x();
                adjustToGridX (tmpX);
                newLinkWidget-> endPointOrig.setX(tmpX);
            } else {
                int tmpY = newLinkWidget-> endPointOrig.y();
                adjustToGridY (tmpY);
                newLinkWidget-> endPointOrig.setY(tmpY);
            }

            *graphLinks << new EgDataNode(graphLinks->dataNodeBlueprint, (void*)newLinkWidget); // newNode
            newLinkWidget->dataLinkID = graphLinks->getAddedNodeID();

            graphLinks-> AddLinkPtrsToNodes(graphLinks-> linksDataStorage[newLinkWidget->dataLinkID],
                                            (*graphNodes)[dataNodeWidget->dataNodeID], (*graphNodes)[targetNodeWidget->dataNodeID]);

            newLinkWidget->calcLinkWidgetRect(zoomFactor);
            newLinkWidget-> lower();
            moveResizeLinkWidget(newLinkWidget); //, newIcon->cornerX, newIcon->cornerY, newIcon->rectW, newIcon->rectH);
            newLinkWidget-> show();
            // cout << "connectDropAction() dataLinkID: " << newIcon->dataLinkID << endl;
            return;

        }
    }
}

inline void EgGraphWidget::moveDropAction(QPoint dropPoint, bool newNode)
{
    // cout << "resizeMode: " << resizeMode << " newNode: " << newNode << endl;
    if (! resizeMode)
    {
        dropPoint.setX(std::max (dropPoint.x(), layerCanvas.corner.scaledX + scaledGlobalIndent));
        dropPoint.setY(std::max (dropPoint.y(), layerCanvas.corner.scaledY + scaledGlobalIndent));

        if (newNode) // new node dragged-dropped
        {
            EgNodeWidget* newNodeWidget = new EgNodeWidget(this);
            // newNodeWidget->setAttribute(Qt::WA_DeleteOnClose);
            newNodeWidget-> setWhatsThis(QString("node"));

            newNodeWidget->origRectW   = defNewNodeWidth;
            newNodeWidget->origRectH   = defNewNodeHeight;

            newNodeWidget->scaledCornerX = dropPoint.x();
            newNodeWidget->scaledCornerY = dropPoint.y();

            if (zoomFactor)
            {
                QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
                QPoint deltaP = dropPoint - tmpPoint;
                newNodeWidget->origCornerX = deltaP.x() * 10/(10 - zoomFactor);
                newNodeWidget->origCornerY = deltaP.y() * 10/(10 - zoomFactor);

                newNodeWidget->scaledRectW   = newNodeWidget->origRectW * (10 - zoomFactor)/10;
                newNodeWidget->scaledRectH   = newNodeWidget->origRectH * (10 - zoomFactor)/10;
            } else {
                adjustPointToGrid(dropPoint);
                newNodeWidget->origCornerX = dropPoint.x();
                newNodeWidget->origCornerY = dropPoint.y();
                newNodeWidget->scaledRectW = defNewNodeWidth;
                newNodeWidget->scaledRectH = defNewNodeHeight;
            }

            *graphNodes << new EgDataNode(graphNodes-> dataNodeBlueprint, (void*)newNodeWidget); // newNode
            newNodeWidget-> dataNodeID = graphNodes->getAddedNodeID();
            cout << "moveDropAction() New node added at " ;
            cout << "cornerX: " << std::dec << newNodeWidget->scaledCornerX << " cornerY: " << newNodeWidget->scaledCornerY << " dataNodeID: " << newNodeWidget->dataNodeID << endl;
            moveResizeNodeWidget(newNodeWidget);
            newNodeWidget->show();
            dataNodeWidget = newNodeWidget; // for canvas recalc
        } else { // old node moved
            if (dataNodeWidget)
            {
                dataNodeWidget-> nodeShowCorner = false;
                QPoint oldCorner(dataNodeWidget->scaledCornerX, dataNodeWidget->scaledCornerY);
                QPoint deltaPoint = dropPoint - oldCorner;
                // cout << "deltaPoint: " << std::dec << deltaPoint.x() << " : " << deltaPoint.y() << endl;
                dataNodeWidget->scaledCornerX = dropPoint.x();
                dataNodeWidget->scaledCornerY = dropPoint.y();
                QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
                dataNodeWidget->calcScaledToOrig(zoomFactor, tmpPoint);
                adjustToGridX (dataNodeWidget-> origCornerX);
                adjustToGridY (dataNodeWidget-> origCornerY);
                dataNodeWidget->calcOrigToScaled(zoomFactor, tmpPoint);

                UpdateLinksAftMove((*graphNodes)[dataNodeWidget->dataNodeID], deltaPoint);

                dataNodeWidget-> move(QPoint(dataNodeWidget->scaledCornerX, dataNodeWidget->scaledCornerY)); // FIXME check: update global canvas
                graphNodes-> MarkUpdatedDataNode(dataNodeWidget->dataNodeID);
            }
        }
    } else { // resize
        if (dataNodeWidget)
        {
            // cout << " moveDropAction() resize: " << endl;
            int oldOrigW = oldWidgetWidth * 10/(10 - zoomFactor);
            int oldOrigH = oldWidhetHeight * 10/(10 - zoomFactor);
            UpdateLinksAftResize((*graphNodes)[dataNodeWidget->dataNodeID], oldOrigW, oldOrigH);
            resizeMode = false;
        }
    }
    // calcOuterCorner(); // update canvas size
}

void EgGraphWidget::dropEvent(QDropEvent *event)
{
    // cout << " dropEvent()" << endl;
    if (myForm-> dragDropAction) // && (event->source() != this))
    {
        if ((event->source() != this) && (actionMode ==linkEditMode) && linkEditSelected)
        {
            cout << " dropEvent() linkEditMode event->source() not this " << endl;
            event->acceptProposedAction();
            return;
        }
        // cout << " dropEvent() action" << endl;
        QByteArray itemData = event->mimeData()-> data("application/x-dnditemdata");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        QPoint dragStartPoint (0,0);
        if ((event->source() == this) && (actionMode ==connectMode)) // dont add new nodes in connect mode
        {
            connectDropAction(event->position().toPoint());
            event->ignore();
            return;
        }
        else if ((event->source() != this) && (actionMode ==connectMode))
        {
            arrowIcon-> hide();
            event->ignore();
            return;
        }
        if (! resizeMode) // move node as picture
            dataStream >> *pixmapTmp >> dragStartPoint;
        else
            dataStream >> dragStartPoint;

        QPoint dropPoint = event->position().toPoint() - dragStartPoint;
        if (dropPoint.x() < 0)
            dropPoint.setX(scaledGlobalIndent); // FIXME

        if (dropPoint.y() < 0)
            dropPoint.setY(scaledGlobalIndent);

        bool newNode = (event->source() != this);
        moveDropAction(dropPoint, newNode);

        if (dataNodeWidget)
            updateGlobCanvas(dataNodeWidget);

        event->acceptProposedAction();
    }
    else
        event->ignore(); // foreign drop
}

void EgGraphWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    // if (! (actionMode == moveMode))
    //    return;
    dataNodeWidget = static_cast<EgNodeWidget*>(childAt(event->position().toPoint())); // check if click to a node widget
    if (!dataNodeWidget)
        return;
    // cout << " Double click at node " << dataNode->labelText.toStdString() << endl;
    if (! nodeForm)
        nodeForm = new NodeForm(this);

    dataNodeWidget-> nodeFormOkFlag = false; // reset
    nodeForm-> dataNodeWidget = dataNodeWidget;
    nodeForm-> openNode();
    nodeForm-> show();

    if(dataNodeWidget-> nodeFormOkFlag)
    {
        graphNodes-> MarkUpdatedDataNode(dataNodeWidget-> dataNodeID);
        dataNodeWidget->repaint();
    }
}

void EgGraphWidget::wheelEvent(QWheelEvent *event)
{   
    if (event->angleDelta().y() != 0) {
        int oldZoomFactor = zoomFactor;
        if (event->angleDelta().y() > 0) {
            if (zoomFactor > 0)
                zoomFactor--;
        } else {
            if (zoomFactor < maxZoomFactor)
                zoomFactor++;
        }
        // cout << "zoomFactor: " << zoomFactor << endl;
        if (oldZoomFactor != zoomFactor)
            rescaleZoom();
        event->accept(); // Indicate that the event has been handled
    }
    if (!event->isAccepted()) {
        QWidget::wheelEvent(event);
    }
}

inline void EgGraphWidget::markEditedLink(EgLinkWidget* nextLinkDataPtr, bool editLinkIsOutLink)
{
    clearEditLink();
    linkEditSelected = nextLinkDataPtr;
    linkEditSelected-> isEditSelected = true;
    linkEditSelected-> editLinkIsOutLink = editLinkIsOutLink;
    int actPortSide; // port side of edited link
    if (editLinkIsOutLink)
        actPortSide = linkEditSelected->portSideFrom;
    else
        actPortSide = linkEditSelected->portSideTo;
    if (actPortSide == portSideNorth || actPortSide == portSideSouth)
    {
        linkEditSelected-> minCoordForDrag = dataNodeWidget-> x();
        linkEditSelected-> maxCoordForDrag = dataNodeWidget-> x() + dataNodeWidget-> width() - 1;
    } else {
        linkEditSelected-> minCoordForDrag = dataNodeWidget-> y();
        linkEditSelected-> maxCoordForDrag = dataNodeWidget-> y() + dataNodeWidget-> height() - 1;
    }
    linkEditSelected-> repaint();
}

inline void EgGraphWidget::getNextLinkOfNode()
{
    // cout << "getNextLinkOfNode() node ID: "  << dataNodeWidget->dataNodeID << endl;
    EgLinkWidget* nextLinkDataPtr {nullptr};
    EgDataNode* prevLinkDataPtr {nullptr};

    prevLinkDataPtr = &(graphLinks-> linksDataStorage[linkEditSelected-> dataLinkID]);
    EgDataNode& selectedDataNode {(*graphNodes)[dataNodeWidget->dataNodeID]};

    isOutLinkselected = linkEditSelected-> editLinkIsOutLink;

    if (isOutLinkselected) // out links loop
    {
        nextLinkDataPtr = static_cast<EgLinkWidget*> (selectedDataNode.getNextOutLinkSerialPtr(graphLinks-> linkBlueprintID, prevLinkDataPtr));
        if (nextLinkDataPtr)
        {
            markEditedLink(nextLinkDataPtr, true);
            return;
        }
        isOutLinkselected = false; // switch to in links
        prevLinkDataPtr = nullptr;
    }

    if (! isOutLinkselected) // in links loop
    {
        nextLinkDataPtr = static_cast<EgLinkWidget*> (selectedDataNode.getNextInLinkSerialPtr(graphLinks-> linkBlueprintID, prevLinkDataPtr));
        if (nextLinkDataPtr)
        {
            markEditedLink(nextLinkDataPtr, false);
            return;
        }
        clearEditLink();
        prevLinkDataPtr = nullptr;
        isOutLinkselected = true;
    }

    if (isOutLinkselected) // try out link again if no more in links
    {
        nextLinkDataPtr = static_cast<EgLinkWidget*> (selectedDataNode.getNextOutLinkSerialPtr(graphLinks-> linkBlueprintID, prevLinkDataPtr));
        if (nextLinkDataPtr)
        {
            markEditedLink(nextLinkDataPtr, true);
            return;
        }
        clearEditLink();
        prevLinkDataPtr = nullptr;
        isOutLinkselected = false; // switch to in links again
    }
}

void EgGraphWidget::keyPressEvent(QKeyEvent *event)
{
        // "Press Q to cycle links, D to delete"
    if (event->key() == Qt::Key_Q)
    {
        if (linkEditSelected) // at least one link
            getNextLinkOfNode();
    } else if (event->key() == Qt::Key_Delete) { // delete link
        if (linkEditSelected)
        {
                    // show confirm dialog
            QMessageBox msgBox;
            msgBox.setText("Delete this link?");
            msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel); // Set Cancel as the default focused button
            int ret = msgBox.exec(); // Show the dialog and wait for user input
            if (ret == QMessageBox::Ok)
            {
                EgLinkWidget* linkDeletePtr {linkEditSelected}; // copy active link
                EgDataNode& delLinkData {graphLinks->linksDataStorage[linkDeletePtr-> dataLinkID]};
                if (linkDeletePtr-> editLinkIsOutLink) {
                    (*graphNodes)[dataNodeWidget->dataNodeID].deleteOutLink(graphLinks->linkBlueprintID, &delLinkData); // del resolved pointer
                    (*graphNodes)[linkDeletePtr-> nodeIDTo].deleteInLink (graphLinks->linkBlueprintID, &delLinkData); // find node to
                } else {
                    (*graphNodes)[dataNodeWidget->dataNodeID].deleteInLink (graphLinks->linkBlueprintID, &delLinkData);
                    (*graphNodes)[linkDeletePtr-> nodeIDFrom].deleteOutLink(graphLinks->linkBlueprintID, &delLinkData); // find node from
                }
                getNextLinkOfNode(); // changes linkEditSelected
                graphLinks->linksDataStorage.DeleteDataNode(linkDeletePtr-> dataLinkID); // del link data
                // dataLinksMap.remove(linkDeletePtr-> dataLinkID); // del support map item
                delete linkDeletePtr; // del link widget
            }
        }
    } else if (event->key() == Qt::Key_A) { // move scroll bar
        if (myForm-> scrollArea1-> horizontalScrollBar()) {
            cout << "keyPressEvent() horizontalScrollBar: " << myForm-> scrollArea1->horizontalScrollBar()-> value() << endl;
            myForm-> scrollArea1->horizontalScrollBar()->setValue(myForm-> scrollArea1->horizontalScrollBar()-> value() - 20);
        }
    } else if (event->key() == Qt::Key_D) { // move scroll bar

        if (myForm-> scrollArea1-> horizontalScrollBar()) {
            cout << "keyPressEvent() horizontalScrollBar: " << myForm-> scrollArea1->horizontalScrollBar()-> value() << endl;
            myForm-> scrollArea1->horizontalScrollBar()->setValue(myForm-> scrollArea1->horizontalScrollBar()-> value() + 20);
        }
    } else {
        QWidget::keyPressEvent(event);
    }
}

void EgGraphWidget::resizeEvent(QResizeEvent *event)
{
    // showCanvasRectangle =  globCanvasOrig.x() < event-> size().width() || globCanvasOrig.y() < event-> size().height();
    // cout << "resizeEvent() globCanvasOrig: " << globCanvasOrig.x() << " : " << globCanvasOrig.y() << " showCanvasRectangle: " << showCanvasRectangle << endl;
    QFrame::resizeEvent(event);
}

void EgGraphWidget::showEvent(QShowEvent *event)
{
    LoadLayersInfo();
    LoadLayer();
    QFrame::showEvent(event);
}
