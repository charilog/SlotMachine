#include <QApplication>
#include "ui/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("SlotMachine"));
    app.setOrganizationName(QStringLiteral("OptimTeam"));

    MainWindow window;
    // MainWindow constructor shows the start dialog (modal, blocks)
    // then shows itself
    window.show();

    return app.exec();
}
