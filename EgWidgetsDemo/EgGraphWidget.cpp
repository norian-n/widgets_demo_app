#include "EgGraphWidget.h"
#include "EgNodeWidget.h"
#include "EgLinkWidget.h"
#include "EgGraphForm.h"
#include "NodeForm.h"
#include "NodesTableForm.h"
#include "EgSettingsForm.h"
// #include "ui_EgGraphForm.h"

#include <QtWidgets>
#include <QSizePolicy>
#include <QMessageBox>
#include <QPushButton>

using namespace std;

EgGraphWidget::EgGraphWidget(QWidget *parent)
    : QFrame(parent), itemData (new QByteArray), globPainter (new QPainter), pixmapTmp (new QPixmap)
{
    setFocusPolicy(Qt::WheelFocus);

    arrowIcon = new EgLinkWidget(this);  // dynamic connect link arrow, delete in destructor
    arrowIcon-> setWhatsThis(QString("arrow"));
    arrowIcon-> setMouseTracking(false);
    arrowIcon-> lineType = directConnect;
    arrowIcon-> hide();
    // cout << "EgGraphWidget() called" << endl;
}

EgGraphWidget::~EgGraphWidget()
{
    clearLayer();
    for (auto iter: deleteWidgets) // node widgets delayed cleanup
        delete iter;

    delete itemData; // itemData (new QByteArray), globPainter (new QPainter), pixmapTmp (new QPixmap)
    delete globPainter;
    delete pixmapTmp;

    delete arrowIcon; // arrowIcon = new EgLinkWidget(this);
}

void EgGraphWidget::clearLayer()
{
    clearEditLink();

    if (actionMode == connectMode)
        arrowIcon-> hide();

    if (graphNodes && graphNodes-> isConnected) {
        graphNodes-> Store();
        for (auto nodesIter : graphNodes-> dataMap)
        {
            EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);
            // cout << "clearLayer(): clear widget " << nodeWidget-> dataNodeID << endl;
            nodeWidget-> hide();
            deleteWidgets.append(nodeWidget); // late widgets clearup, seems events queue issue causes crash
            // nodesIter.second-> serialDataPtr = nullptr;
        }
        graphNodes-> clear();
        graphNodes-> isConnected = false;
    }

    if (graphLinks && graphLinks-> linksDataStorage.isConnected) {
        graphLinks->StoreLinks();
        for (auto linksIter : graphLinks-> dataMap)
        {
            EgLinkWidget* linkWidget = static_cast<EgLinkWidget*> (linksIter.second-> serialDataPtr);
            delete linkWidget;
            // linksIter.second-> serialDataPtr = nullptr;
        }
        graphLinks-> linksDataStorage.clear();
        graphLinks-> linksDataStorage.isConnected = false;
    }

    zoomFactor = 0;

    layerCanvas.corner.origX = 0;
    layerCanvas.corner.origY = 0;
    layerCanvas.corner.scaledX = 0;
    layerCanvas.corner.scaledY = 0;
    layerCanvas.size.scaledW = layerCanvas.size.origW;
    layerCanvas.size.scaledH = layerCanvas.size.origH;

    scaledArrowLength = arrowLengthOrig;
    scaledGlobalIndent = globalIndentOrig;

    dataNodeWidget    = nullptr;
    targetNodeWidget  = nullptr;
    tmpNodeWidget     = nullptr;

    // repaint();
}

void EgGraphWidget::setActionMode(int actMode) {
    actionMode = actMode;
    for (auto nodesIter : graphNodes-> dataMap)
    {
        EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);
        nodeWidget-> actionMode = actMode;
    }
}

inline void EgGraphWidget::moveResizeNodeWidget(EgNodeWidget* theWidget)
{
    theWidget-> resize(theWidget-> nodeRect.size.scaledW, theWidget-> nodeRect.size.scaledH);
    theWidget-> move  (theWidget-> nodeRect.corner.scaledX, theWidget-> nodeRect.corner.scaledY);
}

void EgGraphWidget::moveResizeLinkWidget(EgLinkWidget* theWidget)
{
    theWidget-> resize(theWidget-> linkRect.size.scaledW, theWidget-> linkRect.size.scaledH);
    theWidget-> move  (theWidget-> linkRect.corner.scaledX,  theWidget-> linkRect.corner.scaledY);
}


inline void EgGraphWidget::updateLayerCanvas()
{
    int newW = std::max (layerCanvas.size.origW, dataNodeWidget->nodeRect.corner.origX + dataNodeWidget->nodeRect.size.origW + globalIndentOrig);
    int newH = std::max (layerCanvas.size.origH, dataNodeWidget->nodeRect.corner.origY + dataNodeWidget->nodeRect.size.origH + globalIndentOrig);

    if (newW > layerCanvas.size.origW || newH > layerCanvas.size.origH) {
        layerCanvas.size.origW = newW;
        layerCanvas.size.origH = newH;
        origToScaledLayer(layerCanvas, zoomFactor);
        graphLayers[layerID]->layerWidth = layerCanvas.size.origW;
        graphLayers[layerID]->layerHeight = layerCanvas.size.origH;
        graphLayers.updateWH(layerID, layerCanvas.size.origW, layerCanvas.size.origH);
        setMinimumSize(layerCanvas.size.origW, layerCanvas.size.origH);
        repaint();
    }
}

inline void EgGraphWidget::rescaleZoom()
{
    // cout << "rescaleZoom() zoomFactor: " << zoomFactor << endl;
    if (actionMode == connectMode)
        arrowIcon-> hide();

    scaledGlobalIndent = globalIndentOrig * (10 - zoomFactor)/10;
    scaledArrowLength  = arrowLengthOrig * (10 - zoomFactor)/10;

    origToScaledLayer(layerCanvas, zoomFactor);

    // cout << "rescaleZoom() scaledW: " << layerCanvas.size.scaledW << " scaledH:" << layerCanvas.size.scaledH << endl;
    // cout << "rescaleZoom() scaledX: " << layerCanvas.corner.scaledX << " scaledY:" << layerCanvas.corner.scaledY << endl;

    if (graphNodes-> isConnected)
    {
        for (auto nodesIter : graphNodes-> dataMap)
        {
            EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);
            // QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
            // nodeWidget-> calcOrigToScaled(zoomFactor, tmpPoint);
            // nodeWidget-> zoomFactor = zoomFactor;
            origToScaledRectCanvas   (nodeWidget->nodeRect, zoomFactor, layerCanvas.corner);
            moveResizeNodeWidget(nodeWidget);
            // cout << "rescaleZoom() nodeID: " << nodeWidget-> dataNodeID << endl;
        }
    }

    if (graphLinks->linksDataStorage.isConnected)
    {
        for (auto linksIter : graphLinks->dataMap)
        {
            EgLinkWidget* linkWidget = static_cast<EgLinkWidget*> (linksIter.second-> serialDataPtr);
            // QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
            origToScaledPointCanvas  (linkWidget-> linkPointStart, zoomFactor, layerCanvas.corner);
            origToScaledPointCanvas  (linkWidget-> linkPointEnd, zoomFactor, layerCanvas.corner);
            // linkWidget-> calcOrigToScaled(zoomFactor, layerCanvas);
            if (linkWidget != linkEditSelected)
                linkWidget-> calcLinkWidgetRect(zoomFactor, scaledGlobalIndent);
            else
                origToScaledRectCanvas   (linkWidget->linkRect, zoomFactor, layerCanvas.corner);
            moveResizeLinkWidget(linkWidget);
            linkWidget-> lower();
            // cout << "rescaleZoom() linkID: " << linkWidget->dataLinkID << endl;
        }
    }
    repaint();
}

inline EgNodeWidget* EgGraphWidget::getNodeWidget(EgDataNodeIDType nodeID) {
    return static_cast <EgNodeWidget*> (graphNodes->nodesContainer-> GetNodePtrByID(nodeID)-> serialDataPtr);
}

void loadNodeObjectFromDb (EgDataNode& dataNode) //  pumped to dataNode.serialDataPtr
{
    EgNodeWidget* nodeWidget = new EgNodeWidget(); // delete on layer cleanup clearLayer()
    dataNode.serialDataPtr = (void*) nodeWidget;

    nodeWidget-> dataNodeID = dataNode.dataNodeID;
    dataNode["detailsLayerID"] >> nodeWidget-> detailsLayerID;
    // cout << "loadNodeObjectFromDb() detailsLayerID: " << nodeWidget-> detailsLayerID << endl;

    dataNode["name"]        >> nodeWidget-> labelText;
    dataNode["description"] >> nodeWidget-> descText;

    dataNode["cornerX"] >> nodeWidget-> nodeRect.corner.origX; // scaledCornerX;
    dataNode["cornerY"] >> nodeWidget-> nodeRect.corner.origY; // scaledCornerY;
    dataNode["rectH"]   >> nodeWidget-> nodeRect.size.origH; // scaledRectH;
    dataNode["rectW"]   >> nodeWidget-> nodeRect.size.origW; // scaledRectW;

    int tmpColor;
    dataNode["fillColor"]   >> tmpColor;
    nodeWidget-> fillColor = QColor(tmpColor);

    if (dataNode["image"].dataSize) // && nodeWidget->pixmap
    {
        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        dataNode["image"] >> itemData;
        dataStream >> *(nodeWidget->pixmap);
    }

    resetToOrigRect(nodeWidget-> nodeRect);
    // cout << "loadNodeObjectFromDb() nodeName: " << newNodePtr-> labelText.toStdString() << endl;
}

void storeNodeObjectToDb  (EgDataNode& dataNode)
{
    EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> (dataNode.serialDataPtr);

    dataNode["detailsLayerID"] << nodeWidget-> detailsLayerID;
    // cout << "storeNodeObjectToDb() detailsLayerID: " << nodeWidgetPtr-> detailsLayerID << endl;

    dataNode["name"]        << nodeWidget-> labelText;     // PrintByteArray(dataNode["name"]);
    dataNode["description"] << nodeWidget-> descText;
    dataNode["cornerX"]     << nodeWidget-> nodeRect.corner.origX; // origCornerX;
    dataNode["cornerY"]     << nodeWidget-> nodeRect.corner.origY; //origCornerY;
    dataNode["rectH"]       << nodeWidget-> nodeRect.size.origH;
    dataNode["rectW"]       << nodeWidget-> nodeRect.size.origW; // origRectW;

    int tmpColor = nodeWidget-> fillColor.rgba();
    dataNode["fillColor"] << tmpColor;

    if (nodeWidget->pixmap && nodeWidget->pixmap-> width()) {
        QByteArray itemData;
        QDataStream dataStream(&itemData, QIODevice::WriteOnly);
        dataStream << nodeWidget->pixmap-> scaled(40,40);  // FIXME literal
        itemData >> dataNode["image"];
    } else {
        dataNode["image"].dataSize = 0;
    }

    resetToOrigRect(nodeWidget-> nodeRect);
}

void loadLinkObjectFromDb (EgDataNode& dataNode) //  pumped to dataNode.serialDataPtr
{
    EgLinkWidget* linkWidget = new EgLinkWidget(); // delete on layer cleanup clearLayer()
    dataNode.serialDataPtr = (void*) linkWidget;
    linkWidget-> dataLinkID = dataNode.dataNodeID;

    dataNode["fromID"]        >> linkWidget-> nodeIDFrom;
    dataNode["toID"]          >> linkWidget-> nodeIDTo;
    dataNode["portFrom"]      >> linkWidget-> portSideFrom;
    dataNode["portTo"]        >> linkWidget-> portSideTo;
    dataNode["sideCoordFrom"] >> linkWidget-> sideCoordFromOrig;
    dataNode["sideCoordTo"]   >> linkWidget-> sideCoordToOrig;
    dataNode["lineType"]      >> linkWidget-> lineType;

    linkWidget-> sideCoordFromScaled = linkWidget-> sideCoordFromOrig;
    linkWidget-> sideCoordToScaled   = linkWidget-> sideCoordToOrig;

    // cout << "loadLinkObjectFromDb() : " << std::dec << linkWidget-> dataLinkID << " sideCoordFromOrig: " << linkWidget-> sideCoordFromOrig
    //      << " sideCoordToOrig: " << linkWidget-> sideCoordToOrig << endl;

    // dataNode["startPointX"]   >> linkWidget-> linkPointStart.origX; // tmpInt;
    // dataNode["startPointY"]   >> linkWidget-> linkPointStart.origY; // tmpInt;
    resetToOrigPoint(linkWidget-> linkPointStart);

    // cout << "loadLinkObjectFromDb() linkPointStart: " << std::dec << newLinkPtr-> linkPointStart.scaledX << " : " << newLinkPtr-> linkPointStart.scaledY << endl;
    // dataNode["endPointX"] >> linkWidget-> linkPointEnd.origX; // tmpInt;
    // dataNode["endPointY"] >> linkWidget-> linkPointEnd.origY; // tmpInt;
    resetToOrigPoint(linkWidget-> linkPointEnd);
}

void storeLinkObjectToDb  (EgDataNode& dataNode)
{
    EgLinkWidget* linkWidget = static_cast<EgLinkWidget*> (dataNode.serialDataPtr);

    dataNode["fromID"]        << linkWidget-> nodeIDFrom;
    dataNode["toID"]          << linkWidget-> nodeIDTo;
    dataNode["portFrom"]      << linkWidget-> portSideFrom;
    dataNode["portTo"]        << linkWidget-> portSideTo;
    dataNode["sideCoordFrom"] << linkWidget-> sideCoordFromOrig;
    dataNode["sideCoordTo"]   << linkWidget-> sideCoordToOrig;

    // cout << "storeLinkObjectToDb() : " << std::dec << linkWidget-> dataLinkID << " sideCoordFromOrig: " << linkWidget-> sideCoordFromOrig
    //      << " sideCoordToOrig: " << linkWidget-> sideCoordToOrig << endl;

    // cout << "storeLinkObjectToDb() linkPointStart: " << std::dec << EgLinkWidgetPtr-> linkPointStart.scaledX << " : " << EgLinkWidgetPtr-> linkPointStart.scaledY << endl;
    // dataNode["startPointX"]   << linkWidget-> linkPointStart.origX;
    // dataNode["startPointY"]   << linkWidget-> linkPointStart.origY;
    // dataNode["endPointX"]     << linkWidget-> linkPointEnd.origX;
    // dataNode["endPointY"]     << linkWidget-> linkPointEnd.origY;
    dataNode["lineType"]      << linkWidget-> lineType;
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
    EgNodeWidget* nodeWidgetTmp     {nullptr};
    EgDataNode* prevLinkDataNodePtr {nullptr};
    // EgDataNode& selectedDataNode    {(*graphNodes)[dataNodeWidget->dataNodeID]}; // shortcut ref
        // out links
    EgLinkWidget* nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextOutLinkSerialPtr(graphLinks-> linkBlueprintID, prevLinkDataNodePtr));
    while (nextLinkDataPtr) // out links loop
    {
        nodeWidgetTmp = getNodeWidget(nextLinkDataPtr-> nodeIDFrom);
        nodeWidgetTmp-> getLinkPointOrig(nextLinkDataPtr-> portSideFrom, nextLinkDataPtr-> sideCoordFromOrig, nextLinkDataPtr-> linkPointStart.origX, nextLinkDataPtr-> linkPointStart.origY);
        origToScaledPointCanvas  (nextLinkDataPtr-> linkPointStart, zoomFactor, layerCanvas.corner);

        UpdateOneDataLink(nextLinkDataPtr);
        prevLinkDataNodePtr = &(graphLinks-> linksDataStorage[nextLinkDataPtr-> dataLinkID]);
        nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }
        // in links
    prevLinkDataNodePtr = nullptr;
    nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    while (nextLinkDataPtr) // in links loop
    {
        nodeWidgetTmp = getNodeWidget(nextLinkDataPtr-> nodeIDTo);
        nodeWidgetTmp-> getLinkPointOrig(nextLinkDataPtr-> portSideTo, nextLinkDataPtr-> sideCoordToOrig, nextLinkDataPtr-> linkPointEnd.origX, nextLinkDataPtr-> linkPointEnd.origY);
        origToScaledPointCanvas  (nextLinkDataPtr-> linkPointEnd, zoomFactor, layerCanvas.corner);

        UpdateOneDataLink(nextLinkDataPtr);
        prevLinkDataNodePtr = &(graphLinks->linksDataStorage[nextLinkDataPtr-> dataLinkID]); // ++
        nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }
}

void EgGraphWidget::UpdateLinksAftResize(EgDataNode& dataNode, int oldW, int oldH)
{
    EgNodeWidget* nodeWidgetTmp     {nullptr};
    EgDataNode* prevLinkDataNodePtr {nullptr};
        // out/from links update
    EgLinkWidget* nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    while (nextLinkDataPtr)
    {
        nodeWidgetTmp = getNodeWidget(nextLinkDataPtr-> nodeIDFrom);
        if (nextLinkDataPtr-> portSideFrom == portSideNorth || nextLinkDataPtr-> portSideFrom == portSideSouth)
            nextLinkDataPtr-> sideCoordFromOrig = nextLinkDataPtr-> sideCoordFromOrig * nodeWidgetTmp->nodeRect.size.origW / oldW;
        else
            nextLinkDataPtr-> sideCoordFromOrig = nextLinkDataPtr-> sideCoordFromOrig * nodeWidgetTmp->nodeRect.size.origH / oldH;
        // nextLinkDataPtr-> alignToGrid(nextLinkDataPtr-> sideCoordFromOrig); // FIXME check
        alignToGrid(nextLinkDataPtr-> sideCoordFromOrig, gridSizeOrig, globalIndentOrig);

        nodeWidgetTmp-> getLinkPointOrig(nextLinkDataPtr-> portSideFrom, nextLinkDataPtr-> sideCoordFromOrig, nextLinkDataPtr-> linkPointStart.origX, nextLinkDataPtr-> linkPointStart.origY);
        origToScaledPointCanvas  (nextLinkDataPtr-> linkPointStart, zoomFactor, layerCanvas.corner);

        UpdateOneDataLink(nextLinkDataPtr);
        prevLinkDataNodePtr = &(graphLinks->linksDataStorage[nextLinkDataPtr-> dataLinkID]);
        nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    }
        // in/to links update
    prevLinkDataNodePtr = nullptr;
    nextLinkDataPtr = static_cast<EgLinkWidget*> (dataNode.getNextInLinkSerialPtr(graphLinks->linkBlueprintID, prevLinkDataNodePtr));
    while (nextLinkDataPtr)
    {
        nodeWidgetTmp = getNodeWidget(nextLinkDataPtr-> nodeIDTo);
        if (nextLinkDataPtr-> portSideTo == portSideNorth || nextLinkDataPtr-> portSideTo == portSideSouth)
            nextLinkDataPtr-> sideCoordToOrig = nextLinkDataPtr-> sideCoordToOrig * nodeWidgetTmp->nodeRect.size.origW / oldW;
        else
            nextLinkDataPtr-> sideCoordToOrig = nextLinkDataPtr-> sideCoordToOrig * nodeWidgetTmp->nodeRect.size.origH / oldH;
        // nextLinkDataPtr-> alignToGrid(nextLinkDataPtr-> sideCoordToOrig); // FIXME check ajust
        alignToGrid(nextLinkDataPtr-> sideCoordFromOrig, gridSizeOrig, globalIndentOrig);

        nodeWidgetTmp-> getLinkPointOrig(nextLinkDataPtr-> portSideTo, nextLinkDataPtr-> sideCoordToOrig, nextLinkDataPtr-> linkPointEnd.origX, nextLinkDataPtr-> linkPointEnd.origY);
        origToScaledPointCanvas  (nextLinkDataPtr-> linkPointEnd, zoomFactor, layerCanvas.corner);

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
    // if (! graphNodes-> isDataLoaded)
    graphNodes-> LoadAllNodes();
    // graphNodes->nodesContainer-> PrintDataNodesContainer();
    int calcCanvasW = defaultCanvasW;
    int calcCanvasH = defaultCanvasH;

    for (auto nodesIter : graphNodes-> dataMap) // 17 [first, second], <11 = dataFieldsNames.begin(); fieldsIter != dataFieldsNames.end(); ++fieldsIter) {
    // PrintEgDataNodeFields(*(nodesIter.second));
    {
        EgNodeWidget* newNodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);  // created on load as dataNode.serialDataPtr
        // cout << "LoadDataNodes() loaded data node name: " << newNodeWidget-> labelText.toStdString() << endl;
        newNodeWidget-> setParent(this);
        newNodeWidget-> setWhatsThis(QString("node"));
        newNodeWidget-> actionMode = actionMode;

        moveResizeNodeWidget(newNodeWidget);
        newNodeWidget->show();

        calcCanvasW = std::max (calcCanvasW, newNodeWidget->nodeRect.corner.origX + newNodeWidget->nodeRect.size.origW + globalIndentOrig);
        calcCanvasH = std::max (calcCanvasH, newNodeWidget->nodeRect.corner.origY + newNodeWidget->nodeRect.size.origH + globalIndentOrig);
    }

    if (calcCanvasW > defaultCanvasW || calcCanvasH > defaultCanvasH) {
        layerCanvas.size.origW = calcCanvasW;
        layerCanvas.size.origH = calcCanvasH;
        origToScaledLayer(layerCanvas, zoomFactor);
        graphLayers[layerID]->layerWidth = layerCanvas.size.origW;
        graphLayers[layerID]->layerHeight = layerCanvas.size.origH;
        graphLayers.updateWH(layerID, layerCanvas.size.origW, layerCanvas.size.origH);
        setMinimumSize(layerCanvas.size.origW, layerCanvas.size.origH);
        repaint();
    }
}

void EgGraphWidget::LoadLayersInfo()
{
    if ( ! graphLayers.layersStorage.isConnected) {
        graphLayers.ConnectLayers("demoAppLayers", graphDB); // FIXME literal
        graphLayers.LoadLayers();
    }
}

void EgGraphWidget::LoadLayer()
{
    // StoreDataNodes();
    // StoreDataLinks();

    QString nodeLabel;
    if (dataNodeWidget)
        nodeLabel = dataNodeWidget->labelText;

    clearLayer();
    if(! graphLayers[layerID]) {
        cout << "LoadLayer() layer not found in graphLayers, ID: " << layerID << endl;
        return;
    }

    parentLayerID = graphLayers[layerID]-> parentLayerID;

    layerCanvas.size.origW = graphLayers[layerID]-> layerWidth;
    layerCanvas.size.origH = graphLayers[layerID]-> layerHeight;

    origToScaledLayer(layerCanvas, zoomFactor);
    setMinimumSize(layerCanvas.size.origW, layerCanvas.size.origH);

    // cout << "LoadLayer() W: " << dec << layerCanvas.size.origW << " H: " << layerCanvas.size.origH << endl;
    graphLayers[layerID]-> getLayerNodes(graphNodes, loadNodeObjectFromDb, storeNodeObjectToDb);
    // cout << "LoadLayer() graphNodes: " << graphNodes-> nodesSetName << endl;
    LoadDataNodes();
    graphLayers[layerID]-> getLayerLinks(graphLinks, loadLinkObjectFromDb, storeLinkObjectToDb);
    LoadDataLinks();

    // QString numStr = QString::number(layerID);
    // myForm->ui->layerNumLabel-> setText(numStr);

    if (parentLayerID)
        myForm->setWindowTitle("Details of " + nodeLabel);
    else
        myForm->setWindowTitle("Top layer");

    repaint();
}

void EgGraphWidget::LayerUp()
{
    if (layerID > 1) {
        StoreDataNodes();
        StoreDataLinks();
        layerID = parentLayerID;
        LoadLayer();
        // QString numStr = QString::number(layerID);
        // myForm->ui->layerNumLabel-> setText(numStr);
        for (auto iter: deleteWidgets) // delayed node widgets cleanup
            delete iter;
        deleteWidgets.clear();
    }
    // cout << "LayerUp() : " << layerNum << endl;
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
    if (! graphLinks-> linksDataStorage.isConnected)
        graphLinks->ConnectLinks(graphLinks->linkNameShort, graphDB);
    // if (! graphLinks-> linksDataStorage.isDataLoaded) {
        // cout << "LoadDataLinks() LoadAllNodes" << endl;
        // graphLinks-> linksDataStorage.nodesContainer-> PrintNodesChain();
    graphLinks-> clear();
    graphLinks-> linksDataStorage.LoadAllNodes();
    graphLinks-> ResolveNodesIDsToPtrs(*graphNodes, *graphNodes);
    // }
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
    EgNodeWidget* nodeWidgetTmp = getNodeWidget(newLinkWidget-> nodeIDFrom);
    nodeWidgetTmp-> getLinkPointOrig(newLinkWidget-> portSideFrom, newLinkWidget-> sideCoordFromOrig, newLinkWidget-> linkPointStart.origX, newLinkWidget-> linkPointStart.origY);
    // cout << "ShowDataLink() pointFrom stored: " << dec << newLinkWidget-> linkPointStart.origX << " : "  << newLinkWidget-> linkPointStart.origY << endl;
    // cout << "ShowDataLink() pointFrom calc:   " << pointOrigX << " : "  << pointOrigY << endl;
    resetToOrigPoint(newLinkWidget-> linkPointStart);

    nodeWidgetTmp = getNodeWidget (newLinkWidget-> nodeIDTo);
    nodeWidgetTmp-> getLinkPointOrig(newLinkWidget-> portSideTo, newLinkWidget-> sideCoordToOrig, newLinkWidget-> linkPointEnd.origX, newLinkWidget-> linkPointEnd.origY);
    // cout << "ShowDataLink() pointTo stored: " << newLinkWidget-> linkPointEnd.origX << " : "  << newLinkWidget-> linkPointEnd.origY << endl;
    // cout << "ShowDataLink() pointTo calc:   " << pointOrigX << " : "  << pointOrigY << endl;
    resetToOrigPoint(newLinkWidget-> linkPointEnd);

    newLinkWidget-> calcLinkWidgetRect(zoomFactor, scaledGlobalIndent);
    moveResizeLinkWidget(newLinkWidget);

    newLinkWidget-> lower();
    newLinkWidget-> show();
}

inline void EgGraphWidget::UpdateOneDataLink(EgLinkWidget* updLinkInfo)
{
    updLinkInfo-> calcLinkWidgetRect(zoomFactor, scaledGlobalIndent);
    moveResizeLinkWidget(updLinkInfo); // , newcornerX, newcornerY, newrectW, newrectH);
    graphLinks-> MarkUpdatedLink(updLinkInfo-> dataLinkID);
    // cout << "UpdateOneDataLink() dataLinkID: " << updLinkInfo-> dataLinkID << endl;
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
    // cout << "paintEvent() layerCanvas: " << layerCanvas.corner.scaledX << " : " << layerCanvas.corner.scaledY << endl;
    // if (zoomFactor)
    {
        globPainter-> begin(this);
        globPainter-> setPen(Qt::darkGray);
        globPainter-> setPen(Qt::DotLine);
        globPainter-> drawRect(layerCanvas.corner.scaledX, layerCanvas.corner.scaledY, layerCanvas.size.scaledW, layerCanvas.size.scaledH);
        globPainter-> end();
    }
}

inline void EgGraphWidget::connectPressAction(QPoint clickPoint, QDrag* dragPtr)
{
    arrowIcon-> hide();
    dataNodeWidget-> calcPortPointOrig(clickPoint, zoomFactor, arrowIcon->portSideFrom, arrowIcon->sideCoordFromOrig, arrowIcon-> linkPointStart.origX, arrowIcon-> linkPointStart.origY);
    origToScaledPointCanvas  (arrowIcon-> linkPointStart, zoomFactor, layerCanvas.corner);

    QMimeData *mimeData = new QMimeData; // system dragdrop squats
    mimeData->setData("application/x-dnditemdata", *itemData);
    dragPtr-> setMimeData(mimeData);
    myForm-> dragDropAction = true;
    dragPtr-> exec(Qt::MoveAction);
    myForm-> dragDropAction = false;
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
        oldNodeWidthOrig  = dataNodeWidget-> nodeRect.size.origW;
        oldNodeHeightOrig = dataNodeWidget-> nodeRect.size.origH;
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
        linkEditSelected-> setAcceptDrops(false);
        linkEditSelected-> isEditSelected = false;
        linkEditSelected-> lower();
        // linkEditSelected-> repaint();
        linkEditSelected = nullptr;
    }
}

inline void EgGraphWidget::linkEditPressAction()
{
    clearEditLink();
    EgLinkWidget* firstLinkDataPtr =
        static_cast<EgLinkWidget*> ((*graphNodes)[dataNodeWidget->dataNodeID].getNextOutLinkSerialPtr(graphLinks->linkBlueprintID, nullptr));

    if (firstLinkDataPtr)
        cout << "linkEditPressAction() first link: " << firstLinkDataPtr-> dataLinkID << endl;

    if (firstLinkDataPtr) // out link found
    {
        markEditedLink(firstLinkDataPtr, true);
        return;
    }

    firstLinkDataPtr =
        static_cast<EgLinkWidget*> ((*graphNodes)[dataNodeWidget->dataNodeID].getNextInLinkSerialPtr(graphLinks->linkBlueprintID, nullptr));
    if (firstLinkDataPtr)
    {
        markEditedLink(firstLinkDataPtr, false);
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
    /* if (dataNodeWidget)
        cout << " detailsLayerPressAction() node ID: " << dataNodeWidget->dataNodeID << " details layer ID: " << dataNodeWidget-> detailsLayerID
             << " layerID: " << layerID << endl; */
    if (dataNodeWidget && ! dataNodeWidget-> detailsLayerID) { // check if details layer exists
        QMessageBox msgBox;
        msgBox.setText("Create new details of the node layer?");
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Cancel); // Set Cancel as the default focused button
        if (msgBox.exec() != QMessageBox::Ok)
            return;

        // create new details layer
        graphLayers.createDetailsLayer(dataNodeWidget->dataNodeID, dataNodeWidget-> detailsLayerID, layerID, defaultCanvasW, defaultCanvasH, // FIXME literals
                                       graphNodes->nodesSetName, "layerNodesBlueprint", "layerLinksBlueprint");
        cout << " detailsLayerPressAction() node ID: " << dataNodeWidget->dataNodeID << " new layer ID: " << dataNodeWidget-> detailsLayerID
             << " parent layer ID: " << layerID << endl;
        // store detailsLayerID and parent ID
        graphNodes-> MarkUpdatedDataNode(dataNodeWidget->dataNodeID);
        graphNodes-> Store();
    }
    // if (dataNodeWidget)
    StoreDataNodes();
    StoreDataLinks();
    layerID = dataNodeWidget-> detailsLayerID;
    LoadLayer();

}

void EgGraphWidget::mousePressEvent(QMouseEvent *event)
{
    resizeMode =     false;
    dataNodeWidget = nullptr;
    QDrag* drag    { nullptr };
    QWidget* checkWidget = childAt(event->pos());       // check if click on a widget
    // if (checkWidget)
    //    cout << "mousePressEvent() widget: " << checkWidget-> whatsThis().toStdString() << endl;
    if (checkWidget && (checkWidget-> whatsThis() == QString("node"))) // widget is node
    {
        dataNodeWidget = static_cast<EgNodeWidget*> (checkWidget);
        dataNodeWidget-> actionMode = actionMode;
        if (event->button() == Qt::LeftButton) {
            switch (actionMode) {
            case nodeDeleteMode:   nodeDeletePressAction(); return;
            case linkEditMode:     linkEditPressAction(); return;
            case connectMode: {
                drag = new QDrag(this);
                dragStart   = event->pos();
                connectPressAction(event->pos(), drag); break;
            }
            case moveResizeMode: {
                drag = new QDrag(this);
                dragStart   = event->pos();
                movePressAction(event->pos(), drag); break;
            }
            }
            itemData-> clear(); // cleanup
            delete drag;
            dataNodeWidget = nullptr;
            return;
        } else if (event->button() == Qt::RightButton) {
            // actionMode = detailsLayerMode;
            detailsLayerPressAction();
            return;
        }
    } else if (event->button() == Qt::RightButton) {
        // actionMode = detailsLayerMode;
        if (! checkWidget) // flaky bug with link click, seems events queue issue
            LayerUp();
    } else
        event->ignore();
    // QWidget::mousePressEvent(event);
}

void EgGraphWidget::dragEnterEvent(QDragEnterEvent *event)
{       
    // cout << "dragEnterEvent() source is this" << endl;
    if (myForm-> dragDropAction /* && actionMode == moveResizeMode*/) {
        if (event->source() == this || actionMode == moveResizeMode)
            event-> acceptProposedAction();
    }
}

void EgGraphWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    if ((actionMode == connectMode) && myForm-> dragDropAction) {
        arrowIcon-> hide();
    }
}

inline void EgGraphWidget::connectDragAction(QPoint dragPoint)
{
    arrowIcon-> hide();

    arrowIcon-> linkPointEnd.scaledX = dragPoint.x();
    arrowIcon-> linkPointEnd.scaledY = dragPoint.y();

    arrowIcon-> calcLinkWidgetRect(zoomFactor, scaledGlobalIndent);
    moveResizeLinkWidget(arrowIcon);

    arrowIcon-> show();
}

inline void EgGraphWidget::resizeDragAction(QPoint dragPoint)
{   // resize only, move use sys drag
    QPoint offset = dragPoint - dragStart;

    int scaledW {0};
    int scaledH {0};

    origToScaledScalar (oldNodeWidthOrig, scaledW, zoomFactor);
    origToScaledScalar (oldNodeHeightOrig, scaledH, zoomFactor);

    offset.setX(offset.x() + scaledW);
    offset.setY(offset.y() + scaledH);

    int defNewNodeWidthScaled   {0};
    int defNewNodeHeightScaled  {0};

    origToScaledScalar (defNewNodeWidth, defNewNodeWidthScaled, zoomFactor);
    origToScaledScalar (defNewNodeHeight, defNewNodeHeightScaled, zoomFactor);

    if (offset.x() < defNewNodeWidthScaled)
        offset.setX(defNewNodeWidthScaled);
    if (offset.y() < defNewNodeHeightScaled)
        offset.setY(defNewNodeHeightScaled);

    if (dataNodeWidget-> nodeRect.size.scaledW != offset.x() && dataNodeWidget->nodeRect.size.scaledH != offset.y())
    {
        dataNodeWidget-> nodeRect.size.scaledW  = offset.x();
        dataNodeWidget-> nodeRect.size.scaledH  = offset.y();

        scaledToOrigSize   (dataNodeWidget-> nodeRect.size, zoomFactor);

        graphNodes-> MarkUpdatedDataNode(dataNodeWidget->dataNodeID);
        dataNodeWidget-> resize(dataNodeWidget->nodeRect.size.scaledW,  dataNodeWidget->nodeRect.size.scaledH);
    }
}

void EgGraphWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if ((actionMode == connectMode) && myForm-> dragDropAction) // connect nodes drag
        connectDragAction(event->position().toPoint());
    else if (resizeMode && myForm-> dragDropAction && dataNodeWidget) // node resize drag // ((event->source() == this)))
        resizeDragAction(event->position().toPoint());
}

inline void EgGraphWidget::connectDropAction(QPoint dropPoint)
{
    arrowIcon-> hide();
    arrowIcon-> lower();

    QWidget* checkWidget = childAt(dropPoint);       // check if click on a widget
    if (! checkWidget || (checkWidget-> whatsThis() != QString("node"))) // widget is not node
        return;
    targetNodeWidget = static_cast<EgNodeWidget*> (checkWidget);
    if (targetNodeWidget && dataNodeWidget && targetNodeWidget != dataNodeWidget)
    {
        EgLinkWidget* newLinkWidget = new EgLinkWidget(this);
        newLinkWidget-> setWhatsThis(QString("link"));
        newLinkWidget-> setAttribute(Qt::WA_DeleteOnClose);

        newLinkWidget->nodeIDFrom = dataNodeWidget->dataNodeID;
        newLinkWidget->nodeIDTo = targetNodeWidget->dataNodeID;

        newLinkWidget-> linkPointStart = arrowIcon-> linkPointStart; // copy from dynamic link
        newLinkWidget-> sideCoordFromOrig   = arrowIcon-> sideCoordFromOrig;
        // newLinkWidget-> sideCoordFromScaled = arrowIcon-> sideCoordFromScaled;

        targetNodeWidget-> calcPortPointOrig(dropPoint, zoomFactor, newLinkWidget->portSideTo, newLinkWidget->sideCoordToOrig,
                                            newLinkWidget-> linkPointEnd.origX, newLinkWidget-> linkPointEnd.origY);
        origToScaledPointCanvas  (newLinkWidget-> linkPointEnd, zoomFactor, layerCanvas.corner);

        newLinkWidget-> portSideFrom = arrowIcon->portSideFrom;
        newLinkWidget-> calcLinkVisualType();

        *graphLinks << new EgDataNode(graphLinks->dataNodeBlueprint, (void*)newLinkWidget); // newNode
        newLinkWidget->dataLinkID = graphLinks->getAddedNodeID();

        graphLinks-> AddLinkPtrsToNodes(graphLinks-> linksDataStorage[newLinkWidget->dataLinkID],
                                       (*graphNodes)[dataNodeWidget->dataNodeID], (*graphNodes)[targetNodeWidget->dataNodeID]);

        newLinkWidget->calcLinkWidgetRect(zoomFactor, scaledGlobalIndent);
        moveResizeLinkWidget(newLinkWidget);
        newLinkWidget-> lower();
        newLinkWidget-> show();
        // cout << "connectDropAction() dataLinkID: " << newIcon->dataLinkID << endl;
    }
}

inline void EgGraphWidget::moveDropAction(QPoint dropPoint, bool newNode)
{
    // cout << "resizeMode: " << resizeMode << " newNode: " << newNode << endl;
    if (! resizeMode)
    {
        dropPoint.setX(std::max (dropPoint.x(), layerCanvas.corner.scaledX + scaledGlobalIndent)); // FIXME expand canvas to left
        dropPoint.setY(std::max (dropPoint.y(), layerCanvas.corner.scaledY + scaledGlobalIndent));

        if (newNode) // new node dragged-dropped
        {
            dataNodeWidget = new EgNodeWidget(this); // new node widget by common link
            dataNodeWidget-> setWhatsThis(QString("node"));
            dataNodeWidget-> fillColor = newFillColor;

            dataNodeWidget->nodeRect.size.origW   = defNewNodeWidth;
            dataNodeWidget->nodeRect.size.origH   = defNewNodeHeight;

            if (zoomFactor)
            {
                // QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
                // QPoint deltaP = dropPoint - tmpPoint;
                dataNodeWidget->nodeRect.corner.scaledX = dropPoint.x();
                dataNodeWidget->nodeRect.corner.scaledY = dropPoint.y();
                scaledToOrigPointCanvas  (dataNodeWidget->nodeRect.corner, zoomFactor, layerCanvas.corner);
                alignToGrid(dataNodeWidget-> nodeRect.corner.origX, gridSizeOrig, globalIndentOrig);
                alignToGrid(dataNodeWidget-> nodeRect.corner.origY, gridSizeOrig, globalIndentOrig);
                origToScaledPointCanvas  (dataNodeWidget->nodeRect.corner, zoomFactor, layerCanvas.corner);
                origToScaledSize   (dataNodeWidget->nodeRect.size, zoomFactor);
            } else {
                dataNodeWidget->nodeRect.corner.origX = dropPoint.x();
                dataNodeWidget->nodeRect.corner.origY = dropPoint.y();
                alignToGrid(dataNodeWidget-> nodeRect.corner.origX, gridSizeOrig, globalIndentOrig);
                alignToGrid(dataNodeWidget-> nodeRect.corner.origY, gridSizeOrig, globalIndentOrig);
                resetToOrigRect(dataNodeWidget->nodeRect);
            }

            *graphNodes << new EgDataNode(graphNodes-> dataNodeBlueprint, (void*)dataNodeWidget); // new db Node
            dataNodeWidget-> dataNodeID = graphNodes->getAddedNodeID();
            dataNodeWidget-> labelText = "Node " + QString::number(dataNodeWidget-> dataNodeID);
            (*graphNodes)[dataNodeWidget->dataNodeID]["name"] << dataNodeWidget-> labelText;      // set name for table form
            // cout << "moveDropAction() New node added at " ;
            // cout << "cornerX: " << std::dec << dataNodeWidget->scaledCornerX << " cornerY: " << dataNodeWidget->scaledCornerY << " dataNodeID: " << dataNodeWidget->dataNodeID << endl;
            moveResizeNodeWidget(dataNodeWidget);
            dataNodeWidget->show();
        } else { // old node moved
            if (dataNodeWidget)
            {
                dataNodeWidget-> nodeResizeMode = false;
                QPoint oldCorner(dataNodeWidget->nodeRect.corner.scaledX, dataNodeWidget->nodeRect.corner.scaledY);
                QPoint deltaPoint = dropPoint - oldCorner;
                // cout << "deltaPoint: " << std::dec << deltaPoint.x() << " : " << deltaPoint.y() << endl;
                dataNodeWidget->nodeRect.corner.scaledX = dropPoint.x();
                dataNodeWidget->nodeRect.corner.scaledY = dropPoint.y();
                scaledToOrigPointCanvas  (dataNodeWidget->nodeRect.corner, zoomFactor, layerCanvas.corner); // FIXME grid adj
                // QPoint tmpPoint { layerCanvas.corner.scaledX, layerCanvas.corner.scaledY }; // FIXME STUB
                // dataNodeWidget->calcScaledToOrig(zoomFactor, tmpPoint);
                alignToGrid(dataNodeWidget-> nodeRect.corner.origX, gridSizeOrig, globalIndentOrig);
                alignToGrid(dataNodeWidget-> nodeRect.corner.origY, gridSizeOrig, globalIndentOrig);
                origToScaledPointCanvas  (dataNodeWidget->nodeRect.corner, zoomFactor, layerCanvas.corner);
                // dataNodeWidget->calcOrigToScaled(zoomFactor, tmpPoint);

                dataNodeWidget-> move(QPoint(dataNodeWidget->nodeRect.corner.scaledX, dataNodeWidget->nodeRect.corner.scaledY)); // FIXME check: update global canvas
                graphNodes-> MarkUpdatedDataNode(dataNodeWidget->dataNodeID);
                UpdateLinksAftMove((*graphNodes)[dataNodeWidget->dataNodeID], deltaPoint);
            }
        }
    } else { // resize
        if (dataNodeWidget)
        {
            // cout << " moveDropAction() resize: " << endl;
            // int oldOrigW = oldNodeWidthOrig * 10/(10 - zoomFactor);
            // int oldOrigH = oldNodeHeightOrig * 10/(10 - zoomFactor);
            UpdateLinksAftResize((*graphNodes)[dataNodeWidget->dataNodeID], oldNodeWidthOrig, oldNodeHeightOrig);
            resizeMode = false;
        }
    }
    updateLayerCanvas(); // update canvas size
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

        bool newNode = (event->source() != this); // drag from new nodes panel

        if (! resizeMode) // move node as picture
            if (newNode)
                dataStream >> *pixmapTmp >> dragStartPoint >> newFillColor;
            else
                dataStream >> *pixmapTmp >> dragStartPoint;
        else
            dataStream >> dragStartPoint;

        QPoint dropPoint = event->position().toPoint() - dragStartPoint;
        if (dropPoint.x() < 0)
            dropPoint.setX(scaledGlobalIndent);

        if (dropPoint.y() < 0)
            dropPoint.setY(scaledGlobalIndent);

        moveDropAction(dropPoint, newNode);

        event->acceptProposedAction();
    }
    else
        event->ignore(); // foreign drop
}

void EgGraphWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    QWidget* checkWidget = childAt(event->pos());       // check if click on a widget
    if (! checkWidget || (checkWidget-> whatsThis() != QString("node"))) // widget is not node
        return;
    dataNodeWidget = static_cast<EgNodeWidget*> (checkWidget);
    // cout << " Double click at node " << dataNodeWidget->labelText.toStdString() << endl;
    if (! nodeForm)
        nodeForm = new NodeForm(myForm);

    dataNodeWidget-> nodeFormOkFlag = false; // reset
    nodeForm-> GraphWidget = this;
    nodeForm-> dataNodeWidget = dataNodeWidget;

    nodeForm-> openNode();
    nodeForm-> setWindowModality(Qt::WindowModal);
    nodeForm-> show();

    if(dataNodeWidget-> nodeFormOkFlag)
    {
        graphNodes-> MarkUpdatedDataNode(dataNodeWidget-> dataNodeID);
        dataNodeWidget->repaint();
    }
}


void EgGraphWidget::OpenTableForm()
{
    if (! tableForm) {
        tableForm = new NodesTableForm(myForm);
        tableForm-> GraphWidget = this;
    }
    // tableForm-> graphNodes = graphNodes;
    tableForm-> setWindowModality(Qt::WindowModal);
    tableForm-> show();
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

    int minCoordForDrag (0);
    int maxCoordForDrag {0};

    int editMaxCoord {0}; // set anti-freeze temporary link widget size
    int editMinCoord {0};

    if (actPortSide == portSideNorth || actPortSide == portSideSouth)
    {
        minCoordForDrag = dataNodeWidget-> x();
        maxCoordForDrag = dataNodeWidget-> x() + dataNodeWidget-> width() - 1;
        editMaxCoord = std::max<int> (linkEditSelected-> linkRect.corner.scaledX + linkEditSelected-> linkRect.size.scaledW, maxCoordForDrag + scaledGlobalIndent);
        editMinCoord = std::min<int> (linkEditSelected-> linkRect.corner.scaledX, minCoordForDrag - scaledGlobalIndent);
        linkEditSelected-> linkRect.corner.scaledX = editMinCoord; // - scaledGlobalIndent;
        linkEditSelected-> linkRect.size.scaledW = editMaxCoord - editMinCoord;
    } else {
        minCoordForDrag = dataNodeWidget-> y();
        maxCoordForDrag = dataNodeWidget-> y() + dataNodeWidget-> height() - 1;
        editMaxCoord = std::max<int> (linkEditSelected-> linkRect.corner.scaledY + linkEditSelected-> linkRect.size.scaledH, maxCoordForDrag + scaledGlobalIndent);
        editMinCoord = std::min<int> (linkEditSelected-> linkRect.corner.scaledY, minCoordForDrag - scaledGlobalIndent);
        linkEditSelected-> linkRect.corner.scaledY = editMinCoord; // - scaledGlobalIndent;
        linkEditSelected-> linkRect.size.scaledH = editMaxCoord - editMinCoord; // + scaledGlobalIndent;
    }
    scaledToOrigRectCanvas(linkEditSelected->linkRect, zoomFactor, layerCanvas.corner);
    linkEditSelected-> raise();
    moveResizeLinkWidget(linkEditSelected);
}

inline void EgGraphWidget::getNextLinkOfNode()
{
    if (! linkEditSelected) {
        cout << "ERROR: getNextLinkOfNode() linkEditSelected is null " << endl;
        return;
    }
    EgLinkWidget* nextLinkWidget {nullptr};
    EgDataNode* prevLinkDataPtr  {nullptr};

    prevLinkDataPtr = &(graphLinks-> linksDataStorage[linkEditSelected-> dataLinkID]);
    EgDataNode& selectedDataNode {(*graphNodes)[dataNodeWidget->dataNodeID]};

    if (linkEditSelected-> editLinkIsOutLink) // out links loop
    {
        nextLinkWidget = static_cast<EgLinkWidget*> (selectedDataNode.getNextOutLinkSerialPtr(graphLinks-> linkBlueprintID, prevLinkDataPtr));
        if (nextLinkWidget)
        {
            markEditedLink(nextLinkWidget, true);
            return;
        }
    } else { // in links loop
        nextLinkWidget = static_cast<EgLinkWidget*> (selectedDataNode.getNextInLinkSerialPtr(graphLinks-> linkBlueprintID, prevLinkDataPtr));
        if (nextLinkWidget)
        {
            markEditedLink(nextLinkWidget, false);
            return;
        }
    }

    if (linkEditSelected-> editLinkIsOutLink) { // get first link of other type or to loop start
        nextLinkWidget = static_cast<EgLinkWidget*> (selectedDataNode.getNextInLinkSerialPtr(graphLinks-> linkBlueprintID, nullptr));
        if (nextLinkWidget)
        {
            markEditedLink(nextLinkWidget, false);
            return;
        }
        nextLinkWidget = static_cast<EgLinkWidget*> (selectedDataNode.getNextOutLinkSerialPtr(graphLinks-> linkBlueprintID, nullptr));
        if (nextLinkWidget)
        {
            markEditedLink(nextLinkWidget, true);
            return;
        }
    } else {
        nextLinkWidget = static_cast<EgLinkWidget*> (selectedDataNode.getNextOutLinkSerialPtr(graphLinks-> linkBlueprintID, nullptr));
        if (nextLinkWidget)
        {
            markEditedLink(nextLinkWidget, true);
            return;
        }
        nextLinkWidget = static_cast<EgLinkWidget*> (selectedDataNode.getNextInLinkSerialPtr(graphLinks-> linkBlueprintID, nullptr));
        if (nextLinkWidget)
        {
            markEditedLink(nextLinkWidget, false);
            return;
        }
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

/*
void EgGraphWidget::resizeEvent(QResizeEvent *event)
{
    // showCanvasRectangle =  globCanvasOrig.x() < event-> size().width() || globCanvasOrig.y() < event-> size().height();
    // cout << "resizeEvent() globCanvasOrig: " << globCanvasOrig.x() << " : " << globCanvasOrig.y() << " showCanvasRectangle: " << showCanvasRectangle << endl;
    QFrame::resizeEvent(event);
} */

void EgGraphWidget::showEvent(QShowEvent *event)
{
    // cout << "showEvent() " << endl;
    if ( ! graphLayers.layersStorage.isConnected) {
        LoadLayersInfo();
        LoadLayer();
    }
    QFrame::showEvent(event);
}
