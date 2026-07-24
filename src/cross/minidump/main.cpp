#include <QDarkApplication.h>
#include "MainWindow.h"
#include "CrossAccessible.h"

int main(int argc, char* argv[])
{
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::installFactory(crossAccessibleInterfaceFactory);
#endif

    QDarkApplication app(argc, argv);
    MainWindow w;
    w.show();
    QDarkApplication::applyDarkTitleBar(&w);

    // Load the dump provided on the command line
    if(argc > 1)
    {
        w.loadFile(QString::fromUtf8(argv[1]));
    }

    return app.exec();
}
