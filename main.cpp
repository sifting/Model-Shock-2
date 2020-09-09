#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Scene s;
    MainWindow w;
    w.show();
    return a.exec();
}
