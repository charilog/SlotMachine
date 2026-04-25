#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SlotMachine"));
    app.setOrganizationName(QStringLiteral("SlotDev"));

    MainWindow window;
    window.show();

    return app.exec();
}
