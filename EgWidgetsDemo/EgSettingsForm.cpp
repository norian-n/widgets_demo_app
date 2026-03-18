#include "EgSettingsForm.h"
#include "ui_EgSettingsForm.h"

#include <iostream>
#include <cstring>
#include <QByteArray>
#include <QVariant>
#include <QList>

#include "nodes/egDataNodesSet.h"
#include "metainfo/egLayers.h"
#include "metainfo/egLiterals.h"

using namespace std;

EgSettingsForm::EgSettingsForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EgSettingsForm)
{
    ui->setupUi(this);
}

EgSettingsForm::~EgSettingsForm()
{
    delete ui;
}

void EgSettingsForm::on_initButton_clicked()
{
    // std::filesystem::path currentPath = std::filesystem::current_path();
    // std::cout << "Current path: " << currentPath << std::endl;

    // delete all data and blueprint files
    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(fs::current_path())) { // iterate all files
        std::cout << "File ext: " << entry.path().extension() << std::endl;
        if (fs::is_regular_file(entry.status()) && ( entry.path().extension() == ".dnl" || entry.path().extension() == ".gdn" )) {
            fs::remove(entry.path()); // Delete the file
            std::cout << "Deleted: " << entry.path().filename() << std::endl;
        }
    }

    cout << "===== Init sample data " << " =====" << endl;
    EgDatabase graphDB;
    EgLayers   initLayers;

    createNodesBlueprint("layerNodesBlueprint", graphDB);
    graphDB.CreateNodesSetByBlueprint("topLayerNodes", "layerNodesBlueprint");
    createLinksBlueprint("layerLinksBlueprint", graphDB);
    graphDB.CreateLinksSetByBlueprint("topLayerLinks", "layerLinksBlueprint", "topLayerNodes", "topLayerNodes");

    graphDB.CreateLayersSet ("demoAppLayers");
    initLayers.ConnectLayers("demoAppLayers", graphDB);

    EgDataNodeIDType topLayerID;
    initLayers.createBlankLayer(topLayerID, 0, 1111, 787, "topLayerNodes", "topLayerLinks"); // "layer1links");  // create top layer

    cout << "top LayerID: " << topLayerID << endl;
    initLayers.StoreLayers();

    cout << "===== Init complete =====" << endl;

    close();
    closeAppAfterInit-> close();
}

void EgSettingsForm::createNodesBlueprint(const std::string& name, EgDatabase& graphDB)
{
    graphDB.CreateNodeBlueprint(name);

    graphDB.AddNodeDataField("name");
    graphDB.AddNodeDataField("cornerX");
    graphDB.AddNodeDataField("cornerY");
    graphDB.AddNodeDataField("rectH");
    graphDB.AddNodeDataField("rectW");
    graphDB.AddNodeDataField("detailsLayerID");

    graphDB.CommitNodeBlueprint();
}

void EgSettingsForm::createLinksBlueprint(const std::string& linksName, EgDatabase& graphDB)
{
    graphDB.CreateLinkWithDataBlueprint(linksName);

    graphDB.AddLinkDataField("startPointX");
    graphDB.AddLinkDataField("startPointY");
    graphDB.AddLinkDataField("endPointX");
    graphDB.AddLinkDataField("endPointY");
    graphDB.AddLinkDataField("portFrom"); // portSideFrom
    graphDB.AddLinkDataField("portTo");   // portSideTo
    graphDB.AddLinkDataField("lineType");

    graphDB.CommitLinkBlueprint();
}


void EgSettingsForm::initDatabase(EgDatabase& graphDB) {
    createNodesBlueprint("layerNodesBlueprint", graphDB);

    graphDB.CreateNodesSetByBlueprint("layer1nodes", "layerNodesBlueprint");
    graphDB.CreateNodesSetByBlueprint("layer2nodes", "layerNodesBlueprint");

    createLinksBlueprint("layer1links", graphDB);
    createLinksBlueprint("layer2links", graphDB);
}

void EgSettingsForm::addSampleDataNode(EgDataNodesSet& dataNodes, const std::string& name, int X, int Y, int H, int W) // , QList<QVariant>& addValues) {
{
    EgDataNode* newNode = new EgDataNode(dataNodes.dataNodeBlueprint);
    (*newNode)["name"]    << name;
    (*newNode)["cornerX"] << X;
    (*newNode)["cornerY"] << Y;
    (*newNode)["rectH"]   << H;
    (*newNode)["rectW"]   << W;

    dataNodes << newNode;
}

void EgSettingsForm::initData(EgDatabase& graphDB) {
    EgDataNodesSet locationsNodesType;
    locationsNodesType.Connect("nodes", graphDB);
    // QList<QVariant> addValues;

    // addValues.clear();
    // addValues << "one" << 250 << 150 << 150 <<  100 ; // 0x41 << 0x42 << 0x43 ;
    addSampleDataNode(locationsNodesType, "one", 250, 150, 150, 100);

    // addValues.clear();
    // addValues << "two" << 370 << 170 << 80 <<  120 ; // int ('D') << int ('E') << int ('F') ;
    addSampleDataNode(locationsNodesType, "two", 370, 170, 80, 120);

    // addValues.clear();
    // addValues << "three" << 100 << 250 << 100 <<  100 ;
    addSampleDataNode(locationsNodesType, "three", 100, 250, 100, 100);

    // addValues.clear();
    // addValues << "four" << 50 << 100 << 80 <<  160 ;
    addSampleDataNode(locationsNodesType, "four", 50, 100, 80, 160);

    locationsNodesType.Store();
}

