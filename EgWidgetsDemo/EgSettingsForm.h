#ifndef EGSETTINGSFORM_H
#define EGSETTINGSFORM_H

#include <QWidget>
#include <QMainWindow>

#include "metainfo/egDatabase.h"

const int defaultCanvasW   {600};
const int defaultCanvasH   {400};

namespace Ui {
class EgSettingsForm;
}

class EgSettingsForm : public QWidget
{
    Q_OBJECT

public:

    QMainWindow* closeAppAfterInit;

    EgSettingsForm(QWidget *parent = nullptr);
    ~EgSettingsForm();

    void createNodesBlueprint(const std::string& name, EgDatabase& graphDB);
    void createLinksBlueprint(const std::string& linksName, EgDatabase& graphDB);

    void initDatabase(EgDatabase& graphDB);
    void addSampleDataNode(EgDataNodesSet& dataNodes, const std::string& name, int X, int Y, int H, int W);
    void initData(EgDatabase& graphDB);

private slots:
    void on_initButton_clicked();

private:
    Ui::EgSettingsForm *ui;
};

#endif // EGSETTINGSFORM_H
