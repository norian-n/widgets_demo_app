#include "EgGraphForm.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    EgGraphForm w;
    if (w.lockFileDesc < 0) {
        return 111;
    }
    w.show();
    return a.exec();
}
