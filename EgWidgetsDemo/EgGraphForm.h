#ifndef EGGRAPHFORM_H
#define EGGRAPHFORM_H

#include <QMainWindow>

#include <QWidget>
#include <QScrollArea>
#include <QVBoxLayout>

// #include "NodeForm.h"
// #include <mainwindow.h>
#include "TemplatesDragWidget.h"
#include "EgGraphWidget.h"
#include "EgSettingsForm.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class EgGraphForm;
}
QT_END_NAMESPACE

class EgGraphForm : public QMainWindow
{
    Q_OBJECT

public:
    Ui::EgGraphForm * ui;
    EgSettingsForm* settingsForm {nullptr};

    bool dragDropAction {false};

    QScrollArea* scrollArea1;
    /*
    EgDatabaseType graphDB;

    EgDataNodesType* nodes; // FIXME
    EgDataNodesType* images; // FIXME

    EgLinksType* linktype; // FIXME */


    EgGraphForm(QWidget *parent = nullptr);
    ~EgGraphForm();

    inline void clearButtons();

    void LoadSampleGraph();
    void LoadImages();

    // void ShowGraphNodes() {}
    // void ShowGraphLinks() {}

protected:
    // void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    //    void on_loadButton_clicked();
    //    void on_saveButton_clicked();
    //    void on_clearButton_clicked();

    void on_moveModeButton_clicked();
    void on_connectsModeButton_clicked();
    void on_linkEditModeButton_clicked();
    void on_deleteModeButton_clicked();

    void on_lvlUpButton_clicked();
    // void on_lvlDownButton_clicked();

    void on_settingsButton_clicked();

    void on_detailsButton_clicked();

private:
    QVBoxLayout* verticalLayout;
    QHBoxLayout* horizontalLayout;

    TemplatesDragWidget* DragWidget;
    EgGraphWidget* GraphWidget;

    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QSpacerItem* spacer3;

};


#endif // EGGRAPHFORM_H
