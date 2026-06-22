#ifndef TEMPLATESDRAGWIDGET_H
#define TEMPLATESDRAGWIDGET_H

// #include "metainfo/egDatabase.h"
#include "guisupport/egPalettes.h"
#include "guisupport/egOnePalette.h"

#include <QFrame>
#include <QPainter>

const int newNodeWidth    {75};
const int newNodeHeight   {60};

class EgGraphForm;
class EgTemplateWidget;

void loadPaletteWidgetFromDb (EgDataNode& dataNode);  // new widget ptr stored to dataNode.serialDataPtr
void storePaletteWidgetToDb  (EgDataNode& dataNode);

class TemplatesDragWidget : public QFrame
{
public:

    EgGraphForm* myForm  {nullptr};

    EgTemplateWidget* dragWidget  {nullptr};

    QByteArray* itemData {nullptr};

    EgDatabase   theDatabase;
    EgPalettes   demoPalettes;
    EgOnePalette currentPalette;

    // EgDatabase   graphDB;
    // EgDataNodesSet* paletteWidgets  {nullptr};

    explicit TemplatesDragWidget(QWidget *parent = nullptr);
    ~TemplatesDragWidget();

    void showPalette();

    void loadPalette();
    void storePalette();

protected:

    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
};


#endif // TEMPLATESDRAGWIDGET_H
