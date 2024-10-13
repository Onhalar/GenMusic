#include "playermain.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PlayerMain w;
    w.show();
    return a.exec();
}
