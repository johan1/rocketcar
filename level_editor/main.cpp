#include <QApplication>
#include "mainwindow.h"

#include "config.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    Config::getInstance();

    return a.exec();
}
