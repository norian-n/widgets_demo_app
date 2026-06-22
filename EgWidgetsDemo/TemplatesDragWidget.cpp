#include "TemplatesDragWidget.h"
#include "EgGraphForm.h"

#include "EgTemplateWidget.h"
#include "guisupport/egLiteralsGuisupport.h"

#include <QtWidgets>
#include <QSizePolicy>
#include <QtMath>

using namespace std;

void loadPaletteWidgetFromDb (EgDataNode& dataNode) //  pumped to dataNode.serialDataPtr
{
    EgTemplateWidget* dragWidget = new EgTemplateWidget();
    dataNode.serialDataPtr = (void*) dragWidget;
    // dragWidget-> dataNodeID = dataNode.dataNodeID;
    dataNode[paletteWidgetNameName]        >> dragWidget-> labelText;
    dataNode[paletteFieldIndexName]        >> dragWidget-> paletteIndex;
    int tmpColor;
    dataNode[paletteWidgetColorName]   >> tmpColor;
    if (tmpColor)
        dragWidget-> fillColor = QColor(tmpColor);
    else
        dragWidget-> fillColor = QColor(Qt::white);

    /* const std::string paletteFieldNodesName     ("paletteNodesSet");
    const std::string paletteFieldNodeIDName    ("paletteNodeID"); */
}

void storePaletteWidgetToDb  (EgDataNode& dataNode)
{
    EgTemplateWidget* dragWidget = static_cast<EgTemplateWidget*> (dataNode.serialDataPtr);
    dataNode[paletteWidgetNameName] << dragWidget-> labelText;
    int tmpColor = dragWidget-> fillColor.rgba();
    dataNode[paletteWidgetColorName]  << tmpColor;
    dataNode[paletteFieldIndexName]   << dragWidget-> paletteIndex;
}

TemplatesDragWidget::TemplatesDragWidget(QWidget *parent) : QFrame(parent)
    , itemData (new QByteArray) //, painter (new QPainter) //, pixmapTmp (new QPixmap)
{
    setMinimumSize(100, 68);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);

    // demoPalettes.ConnectPalettesSet("testPalettesSet", theDatabase);
    // currentPalette.ConnectPalette("testOnePalette", theDatabase);

    currentPalette.paletteStorage.serialLoadFunction  = loadPaletteWidgetFromDb;
    currentPalette.paletteStorage.serialStoreFunction = storePaletteWidgetToDb;
}

TemplatesDragWidget::~TemplatesDragWidget()
{
    delete itemData;

    while ((dragWidget = findChild<EgTemplateWidget*>()))
        delete dragWidget;
}

void TemplatesDragWidget::showPalette()
{
    for (auto nodesIter : currentPalette.paletteStorage.dataMap)
    {
        EgTemplateWidget* paletteWidget = static_cast<EgTemplateWidget*> (nodesIter.second-> serialDataPtr);
        paletteWidget-> setParent(this);
        // cout << "loadPalette() loaded palette widget name: " << paletteWidget-> labelText.toStdString() << endl;
        paletteWidget-> resize(newNodeWidth, newNodeHeight);
        paletteWidget-> move((newNodeWidth + 4) * (paletteWidget-> paletteIndex - 1), 4);
        paletteWidget-> show();
    }
}

void TemplatesDragWidget::loadPalette()
{
    currentPalette.ConnectPalette("demoOnePalette", theDatabase);
    currentPalette.LoadPalette();

    // int pos { 0 };


    /* if (! graphNodes) {
        cout << "ERROR LoadDataNodes(): null nodes ptr" << endl;
        return;
    }
    if (! graphNodes-> isConnected)
        graphNodes-> Connect(graphNodes-> nodesSetName, graphDB);
    // if (! graphNodes-> isDataLoaded)
    graphNodes-> LoadAllNodes();
    // graphNodes->nodesContainer-> PrintDataNodesContainer();

    for (auto nodesIter : graphNodes-> dataMap) // 17 [first, second], <11 = dataFieldsNames.begin(); fieldsIter != dataFieldsNames.end(); ++fieldsIter) {
    // PrintEgDataNodeFields(*(nodesIter.second));
    {
        EgNodeWidget* newNodeWidget = static_cast<EgNodeWidget*> (nodesIter.second-> serialDataPtr);  // created on load as dataNode.serialDataPtr
        // cout << "LoadDataNodes() loaded data node name: " << newNodeWidget-> labelText.toStdString() << endl;
        newNodeWidget-> setParent(this);
        newNodeWidget-> setWhatsThis(QString("node"));

        moveResizeNodeWidget(newNodeWidget);
        newNodeWidget->show();

        // calcCanvasW = std::max (calcCanvasW, newNodeWidget->nodeRect.corner.origX + newNodeWidget->nodeRect.size.origW + globalIndentOrig);
        // calcCanvasH = std::max (calcCanvasH, newNodeWidget->nodeRect.corner.origY + newNodeWidget->nodeRect.size.origH + globalIndentOrig);
    } */
}

void TemplatesDragWidget::storePalette()
{
    currentPalette.StorePalette();
    /* if (! graphNodes) {
        cout << "ERROR StoreDataNodes(): null nodes ptr" << endl;
        return;
    }
    if (graphNodes-> isConnected)
        graphNodes-> Store();
    else
        cout << "StoreDataNodes(): not connected to egDb" << endl; */
}


void TemplatesDragWidget::showEvent(QShowEvent *event)
{
    loadPalette();
    showPalette();

    QFrame::showEvent(event);
}

void TemplatesDragWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget* checkWidget = childAt(event->pos());       // check if click on a widget
    //    cout << "mousePressEvent() widget: " << checkWidget-> whatsThis().toStdString() << endl;
    if (checkWidget) // widget is node
    {
        dragWidget = static_cast<EgTemplateWidget*> (checkWidget);
        QPixmap pixmap = dragWidget-> grab(); // had to be local

        QDataStream dataStream(itemData, QIODevice::WriteOnly);
        dataStream << pixmap << QPoint(event->pos() - dragWidget->pos()) << dragWidget-> fillColor;

        QMimeData *mimeData = new QMimeData;
        mimeData->setData("application/x-dnditemdata", *itemData);

        QDrag *drag = new QDrag(this);

        drag->setMimeData(mimeData);
        drag->setPixmap(pixmap);
        drag->setHotSpot(event->pos() - dragWidget->pos());

        dragWidget-> hide();

        myForm-> dragDropAction = true;
        drag-> exec(Qt::CopyAction);
        myForm-> dragDropAction = false;

        itemData->clear();
        delete drag;

        dragWidget-> show();
    }

}
