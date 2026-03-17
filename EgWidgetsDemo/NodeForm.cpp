#include "NodeForm.h"
#include "EgNodeWidget.h"
// #include "EgGraphWidget.h"

#include <QtDebug>

NodeForm::NodeForm(QWidget *parent):
  ui(new Ui::NodeForm)
{
    ui->setupUi(this);
}

void NodeForm::openNode()
{
    ui->idLabel->  setText(QString::number(dataNodeWidget-> dataNodeID));
    ui->nameEdit-> setText(dataNodeWidget->labelText);
    ui->nameEdit-> setFocus();
}

void NodeForm::on_cancelButton_clicked()
{
    dataNodeWidget-> nodeFormOkFlag = false;
    close();
}

void NodeForm::on_okButton_clicked()
{
    if (dataNodeWidget-> labelText != ui->nameEdit-> text())
    {
        dataNodeWidget-> labelText = ui->nameEdit-> text();
        dataNodeWidget-> nodeFormOkFlag = true;
    }
    close();
}

