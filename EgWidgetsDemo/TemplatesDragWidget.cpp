#include "TemplatesDragWidget.h"
#include "EgGraphForm.h"

#include <QtWidgets>
#include <QSizePolicy>

TemplatesDragWidget::TemplatesDragWidget(QWidget *parent)
    : QFrame(parent), itemData (new QByteArray), painter (new QPainter) //, pixmapTmp (new QPixmap)
{
    setMinimumSize(100, 88);
    resize(200, 88);

    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    // setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);  // Fixed

    // setAcceptDrops(true);

    QImage image(88,68, QImage::Format_ARGB32);
    image.fill(qRgba(0, 0, 0, 0));

    QPainter painter2(&image);

    painter2.setPen(QColor(0,20,172,255));
    painter2.drawRoundedRect(0,0,84,64, 7,7);

    painter2.setPen(Qt::blue);
    painter2.setFont(QFont("Arial", 10, QFont::Bold));
    painter2.drawText(2, 20, "New node");

    painter2.end();

    QLabel* imageIcon = new QLabel(this);
    imageIcon->setPixmap(QPixmap::fromImage(image));
    imageIcon->move(10, 10);
    imageIcon->show();
    imageIcon->setAttribute(Qt::WA_DeleteOnClose);

    imageIcon-> setProperty("egDbID", 101);  // FIXME STUB
}

void TemplatesDragWidget::mousePressEvent(QMouseEvent *event)
{
    QLabel *child = static_cast<QLabel*>(childAt(event->pos()));

    if (!child)
        return;

    QPixmap pixmap = child->pixmap();

    painter-> begin(&pixmap);
    painter-> fillRect(pixmap.rect(), QColor(63, 255, 63, 32)); //  QColor(127, 127, 127, 127));
    painter-> end();

    QDataStream dataStream(itemData, QIODevice::WriteOnly);
    dataStream << pixmap << QPoint(event->pos() - child->pos());

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dnditemdata", *itemData);

    QDrag *drag = new QDrag(this);

    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(event->pos() - child->pos());
/*
    QPixmap* tempPixmap = new QPixmap(pixmap);

    painter-> begin(tempPixmap);
    painter-> fillRect(pixmap.rect(), QColor(63, 255, 63, 32)); //  QColor(127, 127, 127, 127));
    painter-> end();

    child->setPixmap(*tempPixmap);
*/
    // qDebug() << "ID: " << child-> property("egDbID").toInt();

    myForm-> dragDropAction = true;
    drag-> exec(Qt::CopyAction);
    myForm-> dragDropAction = false;

        // restore and cleanup

    // child->setPixmap(pixmap);
    itemData->clear();
    if (drag) delete drag;
    // if (tempPixmap) delete tempPixmap;
}
