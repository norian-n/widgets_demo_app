#ifndef TEMPLATESDRAGWIDGET_H
#define TEMPLATESDRAGWIDGET_H

#include <QFrame>
#include <QPainter>

class EgGraphForm;

class TemplatesDragWidget : public QFrame
{
public:

    EgGraphForm* myForm {nullptr};

    QByteArray* itemData {nullptr};
    QPainter* painter    {nullptr};
    // QPixmap* pixmapTmp   {nullptr};

    explicit TemplatesDragWidget(QWidget *parent = nullptr);
    ~TemplatesDragWidget() { if (painter) delete painter; }

protected:

    void mousePressEvent(QMouseEvent *event) override;
};


#endif // TEMPLATESDRAGWIDGET_H
