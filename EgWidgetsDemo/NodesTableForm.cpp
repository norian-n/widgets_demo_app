#include "NodesTableForm.h"
#include "EgNodeWidget.h"
#include "ui_NodesTableForm.h"
#include "qtinterface/egQtInterface.h"
#include "EgGraphWidget.h"

using namespace std;

NodesTableForm::NodesTableForm(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NodesTableForm)
{
    ui->setupUi(this);

    setFixedSize(700, 460);

    ui->nodesView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->nodesView->setSelectionMode(QAbstractItemView::SingleSelection);

    nodesModel = new QStandardItemModel();
    connect(nodesModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(model_data_changed(const QModelIndex&, const QModelIndex&)));
}

NodesTableForm::~NodesTableForm()
{
    delete ui;
}

void NodesTableForm::SetModelHeaders(QStandardItemModel* model)
{
    // model->setHorizontalHeaderLabels({"Node name"}); // , "Column 2", "Column 3"});
    model->setHorizontalHeaderItem(0, new QStandardItem("Node name"));

    /*
    int i = 0;
    for (QList<EgBasicControlDesc>::iterator curDesc = basicControlDescs.begin(); curDesc != basicControlDescs.end(); ++curDesc)
    {
        model->setHorizontalHeaderItem(i, new QStandardItem((*curDesc).controlLabel));
        i++;
    } */
}

void NodesTableForm::DataToModel(QStandardItemModel* model)
{
    QList<QStandardItem*> items;
    // QStandardItem *parentItem = model-> invisibleRootItem();

    model->clear();
    SetModelHeaders(model);  // fill headers

    if (! GraphWidget-> graphNodes) //  && graphNodes-> isConnected))
    {
        cout << "ERROR DataToModel(): bad graphNodes " << endl;
        return;
    }

    for (auto nodesIter : GraphWidget-> graphNodes-> dataMap)
    {
        QString tmpStr;
        (*(nodesIter.second))["name"] >> tmpStr;

        items.clear();
        items << new QStandardItem(tmpStr);
        items[0]->setData(QVariant(nodesIter.first), dataNodeID);   // ID
        items[0]->setData(QVariant(0), dataUpdFlag);                // not updated
        // int myID = model->item(row,0)->data(data_id).toInt();
        model-> insertRow(0, items);
    }
}

void NodesTableForm::showEvent(QShowEvent *event)
{
    ui->nodesView-> setModel(nullptr); // detach model
    DataToModel(nodesModel);    // move data to model
    ui->nodesView-> setModel(nodesModel); // attach model
    ui->nodesView-> setColumnWidth(0, ui->nodesView->width() - 36); // FIXME literal
}

void NodesTableForm::on_okButton_clicked()
{
    for (int row = 0; row < nodesModel-> rowCount(); row++) // iterate table rows
    {
        if (nodesModel->item(row,0)->data(dataUpdFlag).toInt()) // check if updated
        {
            EgNodeWidget* nodeWidget = static_cast<EgNodeWidget*> ((*(GraphWidget-> graphNodes))[nodesModel->item(row,0)->data(dataNodeID).toInt()].serialDataPtr);
            nodeWidget->labelText = nodesModel->item(row,0)->text();
            (*(GraphWidget-> graphNodes))[nodesModel->item(row,0)->data(dataNodeID).toInt()]["name"] << nodeWidget->labelText;
            GraphWidget->graphNodes-> MarkUpdatedDataNode(nodesModel->item(row,0)->data(dataNodeID).toInt());
            nodeWidget-> repaint();
        }
    }
    close();
}

void NodesTableForm::on_cancelButton_clicked()
{
    close();
}

void NodesTableForm::model_data_changed(const QModelIndex & topLeft, const QModelIndex & bottomRight)
{
    // update current index
    // Projects.GUI-> model_current_row = topLeft.row();
        // modify row in dataclass and model
    // Projects.GUI-> ModifyRowOfModel(model);
    nodesModel->item(topLeft.row(),0)->setData(QVariant(1), dataUpdFlag);
    // cout << "model_data_changed() row: " << topLeft.row() << endl;
}

