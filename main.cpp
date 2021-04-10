#include "windowsnativeeventfilter.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    WindowsNativeEventFilter eventFilter;

    QObject::connect(&eventFilter, &WindowsNativeEventFilter::onDeviceChange, [](){
       qDebug() << "Device changed";
    });

    return a.exec();
}
