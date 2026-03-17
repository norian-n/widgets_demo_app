#include "EgGraphForm.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EgGraphForm w;
    w.show();
    return a.exec();
}
