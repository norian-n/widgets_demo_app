#ifndef EGTEMPLATEWIDGET_H
#define EGTEMPLATEWIDGET_H

#include "qtinterface/egQtInterface.h"

#include <QWidget>


const int fontSize       {10};
const int roundRadius    {5};
const int positionIndent {4};
const QString fontName   {"Arial"};


class EgTemplateWidget : public QWidget
{
    Q_OBJECT

public:
    QPainter* nodePainter  {nullptr};

    QString labelText      {"New node"};
    QColor  fillColor      {Qt::green};

    // QString imagePath;

    EgTemplateWidget(QWidget *parent = 0);
    ~EgTemplateWidget();

protected:
    void paintEvent(QPaintEvent *event) override;
};

#endif // EGTEMPLATEWIDGET_H
