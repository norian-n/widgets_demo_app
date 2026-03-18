#include "EgGraphForm.h"
#include "EgLinkWidget.h"
#include "ui_EgGraphForm.h"

using namespace std;

EgGraphForm::EgGraphForm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EgGraphForm)
{
    ui->setupUi(this);

    verticalLayout = new QVBoxLayout(ui-> centralwidget);
    horizontalLayout = new QHBoxLayout();

    DragWidget = new TemplatesDragWidget(ui-> centralwidget);
    DragWidget-> myForm = this;

    DragWidget-> setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DragWidget-> setAcceptDrops(false);

    GraphWidget = new EgGraphWidget(ui-> centralwidget);
    GraphWidget-> myForm = this;

    GraphWidget-> setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    GraphWidget-> setAcceptDrops(true);
    GraphWidget-> setMinimumSize(500, 400);

    // GraphWidget-> showCanvasRectangle =  GraphWidget->globCanvasOrig.x() < 400 || GraphWidget->globCanvasOrig.y() < 300;

    scrollArea1 = new QScrollArea();

    scrollArea1-> setWidget(GraphWidget);
    scrollArea1-> setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollArea1-> setWidgetResizable(true);

    scrollArea1-> setMinimumSize(400, 300);

    spacer1 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    spacer2 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    verticalLayout-> addWidget(DragWidget);
    verticalLayout-> addItem(spacer1);
    verticalLayout-> addWidget(scrollArea1);

    spacer3 = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    horizontalLayout-> addWidget(ui-> groupBox);
    horizontalLayout-> addItem(spacer3);
    horizontalLayout-> addWidget(ui-> groupBox_2);

    verticalLayout-> addItem(spacer2);
    verticalLayout-> addItem(horizontalLayout);

    ui->moveModeButton-> setStyleSheet("background-color: lightgreen;");
    ui->connectsModeButton-> setStyleSheet("background-color: white;");

    this->resize(1200, 800);
}

EgGraphForm::~EgGraphForm()
{
    delete ui;

    delete DragWidget;
    delete scrollArea1;

    delete verticalLayout;
}

void EgGraphForm::LoadImages()
{

}

inline void EgGraphForm::clearButtons()
{
    ui->moveModeButton->     setStyleSheet("background-color: white;");
    ui->connectsModeButton-> setStyleSheet("background-color: white;");
    ui->linkEditModeButton-> setStyleSheet("background-color: white;");
    ui->deleteModeButton->   setStyleSheet("background-color: white;");
    ui->detailsButton->      setStyleSheet("background-color: white;");

    GraphWidget->arrowIcon-> hide();
    GraphWidget-> clearEditLink();
    GraphWidget-> setAcceptDrops(true);
}

/*
void EgGraphForm::on_loadButton_clicked()
{
    clearButtons();

    GraphWidget-> clearLayer();
    // GraphWidget-> LoadDataNodes();
    // GraphWidget-> LoadDataLinks();

    GraphWidget-> LoadLayer();

    GraphWidget-> actionMode = moveMode;
    ui->moveModeButton-> setStyleSheet("background-color: lightgreen;");
}
*/

/*
void EgGraphForm::on_saveButton_clicked()
{
    GraphWidget-> StoreDataNodes();
    GraphWidget-> StoreDataLinks();
}

void EgGraphForm::on_clearButton_clicked()
{
    GraphWidget-> clearLayer();
    clearButtons();
    GraphWidget-> actionMode = moveMode;
    ui->moveModeButton-> setStyleSheet("background-color: lightgreen;");
}
*/

void EgGraphForm::on_moveModeButton_clicked()
{
    clearButtons();
    GraphWidget-> actionMode = moveMode;
    ui->moveModeButton-> setStyleSheet("background-color: lightgreen;");
}

void EgGraphForm::on_connectsModeButton_clicked()
{
    clearButtons();
    GraphWidget-> actionMode = connectMode;
    ui->connectsModeButton-> setStyleSheet("background-color: lightgreen;");
}
/*
void EgGraphForm::dropEvent(QDropEvent *event)
{
    cout << " dropEvent()" << endl;
    if (dragDropAction) // && (event->source() != this))
        cout << " dropEvent() action" << endl;

}*/

void EgGraphForm::on_linkEditModeButton_clicked()
{
    clearButtons();
    GraphWidget-> actionMode = linkEditMode;
    ui->linkEditModeButton-> setStyleSheet("background-color: lightgreen;");
}

void EgGraphForm::on_deleteModeButton_clicked()
{
    clearButtons();
    GraphWidget-> actionMode = nodeDeleteMode;
    ui->deleteModeButton-> setStyleSheet("background-color: lightgreen;");
}


void EgGraphForm::on_lvlUpButton_clicked()
{
    // clearButtons();
    GraphWidget-> LayerUp();
    // GraphWidget-> actionMode = moveMode;
    // ui->moveModeButton-> setStyleSheet("background-color: lightgreen;");
}


/* void EgGraphForm::on_lvlDownButton_clicked()
{
    clearButtons();
    GraphWidget-> LayerDown();
    GraphWidget-> actionMode = moveMode;
    ui->moveModeButton-> setStyleSheet("background-color: lightgreen;");
} */

void EgGraphForm::closeEvent(QCloseEvent *event)
{
    // cout << " EgGraphForm::closeEvent() action" << endl;
    if (GraphWidget-> graphNodes) {
        if (GraphWidget->graphNodes-> isDataChanged)
            GraphWidget-> StoreDataNodes();
        if (GraphWidget->graphLinks-> linksDataStorage.isDataChanged)
            GraphWidget-> StoreDataLinks();
    }
}


void EgGraphForm::on_settingsButton_clicked()
{
    if (! settingsForm) {
        settingsForm = new EgSettingsForm(nullptr);
        settingsForm-> closeAppAfterInit = this;
    }

    settingsForm-> show();
}


void EgGraphForm::on_detailsButton_clicked()
{
    clearButtons();
    GraphWidget-> actionMode = detailsLayerMode;
    ui->detailsButton-> setStyleSheet("background-color: lightgreen;");
}

