#include "mainwindow.h"
#include <QApplication>
#include <QGst/Init>
#include <QShortcut>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QGst::init(&argc, &argv);

    MainWindow w;
    w.show();

    return a.exec();
}
