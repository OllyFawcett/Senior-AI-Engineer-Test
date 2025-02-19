#include "LabTracking.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LabTracking w;
    w.show();
    return a.exec();
}
