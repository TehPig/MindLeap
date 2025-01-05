#include <QApplication>
#include "Frontend/mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Create and display the MainWindow
    MainWindow mainWindow;
    mainWindow.show();

    // Enter the event loop
    return app.exec();
}