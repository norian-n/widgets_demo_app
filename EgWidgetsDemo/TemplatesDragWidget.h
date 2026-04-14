#ifndef TEMPLATESDRAGWIDGET_H
#define TEMPLATESDRAGWIDGET_H

#include <QFrame>
#include <QPainter>

const int newNodeWidth    {75};
const int newNodeHeight   {60};

class EgGraphForm;
class EgTemplateWidget;

class TemplatesDragWidget : public QFrame
{
public:

    EgGraphForm* myForm  {nullptr};

    EgTemplateWidget* dragWidget  {nullptr};

    QByteArray* itemData {nullptr};
    // QPainter*   painter  {nullptr};

    // QPixmap* pixmapTmp   {nullptr};

    explicit TemplatesDragWidget(QWidget *parent = nullptr);
    ~TemplatesDragWidget();

protected:

    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
};


#endif // TEMPLATESDRAGWIDGET_H
