#include "Frontend/Dialogs/statsdialog.h"
#include "Backend/Classes/User.hpp"
#include "Dialogs/ui_statsdialog.h"

StatsDialog::StatsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::StatsDialog) {
    ui->setupUi(this);

    setWindowTitle("Stats");

    User user;
    const bool user_status = user.fetchSelected();
    if (!user_status) {
        ui->statusbar->showMessage("Error: Could not fetch selected user.");
        return;
    }

    UserStats userStats(user.getID()); // Initialize UserStats with user ID
    Stats* stats = userStats.load(); // Load user stats (returns a Stats* pointer)

    if (!stats) {
        ui->statusbar->showMessage("Error: Could not load user stats.");
        return;
    }

    // Try to cast Stats* to UserStats*
    UserStats* userStatsPtr = dynamic_cast<UserStats*>(stats);
    if (!userStatsPtr) {
        ui->statusbar->showMessage("Error: Could not cast Stats to UserStats.");
        delete stats; // Clean up the allocated memory
        return;
    }

    // Now you can use UserStats functions
    populateUserData(*userStatsPtr);

    delete stats; // Clean up the allocated memory
}

StatsDialog::~StatsDialog(){ delete ui; }

void StatsDialog::populateUserData(const UserStats& stats) {
    ui->CardsSeenCount->setText(QString::number(stats.getCardsSeen()));
    ui->PressedAgainCount->setText(QString::number(stats.getPressedAgain()));
    ui->PressedHardCount->setText(QString::number(stats.getPressedHard()));
    ui->PressedGoodCount->setText(QString::number(stats.getPressedGood()));
    ui->PressedEasyCount->setText(QString::number(stats.getPressedEasy()));
    ui->TimeSpentCount->setText(QString::number(stats.getTimeSpentSeconds()));
}

void StatsDialog::populateDeckData() {
    // in progress
}
