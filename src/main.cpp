#include <QApplication>
#include "Frontend/mainwindow.h"
#include "Backend/Utilities/DiscordManager.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    DiscordManager::initialize();
    DiscordManager::updatePresence("Browsing Decks", "", "view");

    // Create and display the MainWindow
    MainWindow mainWindow;
    mainWindow.show();

    int result = app.exec();
    
    DiscordManager::shutdown();
    return result;
}