#include "TemplatesDragWidget.h"
#include "EgGraphForm.h"

#include "EgTemplateWidget.h"

#include <QtWidgets>
#include <QSizePolicy>
#include <QtMath>

using namespace std;

TemplatesDragWidget::TemplatesDragWidget(QWidget *parent) : QFrame(parent)
    , itemData (new QByteArray) //, painter (new QPainter) //, pixmapTmp (new QPixmap)
{
    setMinimumSize(100, 68);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
}

TemplatesDragWidget::~TemplatesDragWidget()
{
    delete itemData;

    while ((dragWidget = findChild<EgTemplateWidget*>()))
        delete dragWidget;
}

void TemplatesDragWidget::showEvent(QShowEvent *event)
{
    dragWidget = new EgTemplateWidget(this);
    dragWidget-> labelText = "New node";
    dragWidget-> fillColor = QColor(200, 200, 255, 255);
    // dragWidget-> imagePath = "images/feather.png";
    dragWidget-> resize(newNodeWidth, newNodeHeight);
    dragWidget-> move(4, 4);
    dragWidget-> show();

    dragWidget = new EgTemplateWidget(this);
    dragWidget-> labelText = "New node";
    dragWidget-> fillColor = QColor(200, 255, 200, 255);
    // dragWidget-> imagePath = "images/clover.png";
    dragWidget-> resize(newNodeWidth, newNodeHeight);
    dragWidget-> move(newNodeWidth + 8, 4);
    dragWidget-> show();

    dragWidget = new EgTemplateWidget(this);
    dragWidget-> labelText = "New node";
    dragWidget-> fillColor = QColor(255, 255, 200, 255);
    // dragWidget-> imagePath = "images/book.png";
    dragWidget-> resize(newNodeWidth, newNodeHeight);
    dragWidget-> move(newNodeWidth * 2 + 12, 4);
    dragWidget-> show();

    QFrame::showEvent(event);
}

/*
backpack.png
belt.png
bomb.png
book.png
bronze_coin.png
clover.png
feather.png
'half timbered house AA tileset 1.bmp'
house1.png
house2.png
ring.png
tree1.png
TSbuildings.BMP
*/

void TemplatesDragWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget* checkWidget = childAt(event->pos());       // check if click on a widget
    // if (checkWidget)
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
