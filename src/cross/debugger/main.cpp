#include "gui/MainWindow.h"
#include "CrossAccessible.h"

int main(int argc, char* argv[])
{
    qRegisterMetaType<REGDUMP>("REGDUMP");

#ifndef QT_NO_ACCESSIBILITY
    QAccessible::installFactory(crossAccessibleInterfaceFactory);
#endif

    QApplication app(argc, argv);
    MainWindow::loadTheme();

    MainWindow w;
    w.show();
    return QApplication::exec();
}
