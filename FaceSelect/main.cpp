#include "faceattend.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    FaceAttend w;
    w.show();
    return a.exec();
}
