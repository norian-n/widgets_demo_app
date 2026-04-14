#include "NodeForm.h"
#include "EgNodeWidget.h"
#include "EgGraphWidget.h"
#include "EgGraphForm.h"

// #include "qtinterface/egQtInterface.h"

#include <QFileDialog>
#include <QColorDialog>

NodeForm::NodeForm(QWidget *parent):
  ui(new Ui::NodeForm)
{
    ui->setupUi(this);
}

void NodeForm::openNode()
{
    ui->idLabel->  setText(QString::number(dataNodeWidget-> dataNodeID));
    ui->nameEdit-> setText(dataNodeWidget-> labelText);
    ui->descEdit-> setPlainText(dataNodeWidget-> descText); //  + "\n" + dataNodeWidget-> labelText);

    /* QString myString = ui->descEdit-> toPlainText();
    QStringList myList = myString.split('\n');
    for (const QString& str : myList)
        std::cout << str.toStdString() << std::endl;
    // ui->descEdit-> appendPlainText(dataNodeWidget-> labelText); */

    ui->nameEdit-> setFocus();
}

void NodeForm::on_cancelButton_clicked()
{
    dataNodeWidget-> nodeFormOkFlag = false;
    close();
}

void NodeForm::on_okButton_clicked()
{
    // if (dataNodeWidget-> labelText != ui->nameEdit-> text())
    {
        dataNodeWidget-> labelText = ui->nameEdit-> text();
        dataNodeWidget-> descText  = ui->descEdit-> toPlainText();
        dataNodeWidget-> nodeFormOkFlag = true;

        // EgDataNodesSet* graphNodesLocal = GraphWidget-> graphNodes;
        (*(GraphWidget-> graphNodes))[dataNodeWidget->dataNodeID]["name"] << dataNodeWidget-> labelText;
    }
    // loadImage();
    close();
}

// void NodeForm::loadImage()
// {
    // QString imagePath = "images/clover.png";

    /* QByteArray itemData;
    EgByteArrayAbstractType byteArray;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);

    QPixmap pixmap(imagePath);
    dataStream << pixmap.scaled(40,40);

    itemData >> byteArray; */

    //  bool res = dataNodeWidget->pixmap->load(imagePath);

    // *(dataNodeWidget->pixmap) =  dataNodeWidget->pixmap-> scaled(40,40);
// }


void NodeForm::on_imageButton_clicked()
{
    imagePath = QFileDialog::getOpenFileName(this,  tr("Open Image"), "images", tr("All Files (*)" ));
        // tr("Image Files (*.png *.jpg *.bmp);;All Files (*)"));

    if (!imagePath.isEmpty())
        dataNodeWidget->pixmap->load(imagePath); /* bool res = */
}


void NodeForm::on_fillButton_clicked()
{

#include <QColor>
#include <QDebug>

    // ... inside a function or slot ...
    QColor color = QColorDialog::getColor(Qt::white, this, "Select Color");

    if (color.isValid())
        dataNodeWidget->fillColor = color;
}

