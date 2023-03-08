#include "loadwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    loadWidget w;
    w.show();
    return a.exec();
}
