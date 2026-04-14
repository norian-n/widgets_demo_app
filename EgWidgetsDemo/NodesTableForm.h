#ifndef NODESTABLEFORM_H
#define NODESTABLEFORM_H

#include "metainfo/egDatabase.h"

#include <QDialog>
#include <QStandardItemModel>

namespace Ui {
class NodesTableForm;
}

class EgGraphWidget;

class NodesTableForm : public QDialog
{
    Q_OBJECT

public:
    const int dataNodeID      = Qt::UserRole + 1;
    const int dataUpdFlag = Qt::UserRole + 2;

    EgGraphWidget* GraphWidget;
    QStandardItemModel* nodesModel;
    // EgDataNodesSet* graphNodes  {nullptr};

    explicit NodesTableForm(QWidget *parent = nullptr);
    ~NodesTableForm();

    void SetModelHeaders(QStandardItemModel* model);
    void DataToModel(QStandardItemModel* model);

private:
    Ui::NodesTableForm *ui;

private slots:
    void model_data_changed(const QModelIndex&, const QModelIndex&);
protected:
    void showEvent(QShowEvent *event) override;
private slots:
    void on_okButton_clicked();
    void on_cancelButton_clicked();
};

#endif // NODESTABLEFORM_H
