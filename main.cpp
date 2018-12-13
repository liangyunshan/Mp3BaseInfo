#include <QApplication>

#include "mp3infowidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Mp3InfoWidget w;
    w.setWindowTitle(QString("Mp3ID3"));
    w.show();

    return a.exec();
}
