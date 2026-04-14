#include "EgTemplateWidget.h"

#include <QPainter>

using namespace std;

EgTemplateWidget::EgTemplateWidget(QWidget *parent): QWidget(parent)
    , nodePainter (new QPainter())
{
    // cout << " EgTemplateWidget() called " << endl;
}

EgTemplateWidget::~EgTemplateWidget()
{
    delete nodePainter; // nodePainter (new QPainter())
}

void EgTemplateWidget::paintEvent(QPaintEvent *)
{
    // cout << " EgTemplateWidget paintEvent called " << endl;
    nodePainter-> begin(this);

    nodePainter-> setPen(Qt::darkBlue);
    nodePainter-> setBrush(QBrush(fillColor));

    nodePainter-> drawRoundedRect(0,0,width()-1, height()-1, roundRadius,roundRadius);

    nodePainter-> setFont(QFont(fontName, fontSize, QFont::Bold));
    nodePainter-> drawText(positionIndent, fontSize + positionIndent, labelText);

    // QPixmap pixmap(imagePath);
    // nodePainter-> drawPixmap(4, 18, pixmap.scaled(40,40));

    nodePainter-> end();
}

