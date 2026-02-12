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

    // Directly get stats from the user object
    UserStats stats = user.getTotalUserStats();
    populateUserData(stats);
    populateDeckData();
}

StatsDialog::~StatsDialog(){ delete ui; }

void StatsDialog::populateUserData(const UserStats& stats) {
    ui->CardsSeenCount->setText(formatNumber(stats.getCardsSeen()));
    ui->PressedAgainCount->setText(formatNumber(stats.getPressedAgain()));
    ui->PressedHardCount->setText(formatNumber(stats.getPressedHard()));
    ui->PressedGoodCount->setText(formatNumber(stats.getPressedGood()));
    ui->PressedEasyCount->setText(formatNumber(stats.getPressedEasy()));
    ui->TimeSpentCount->setText(formatDuration(stats.getTimeSpentSeconds()));
}

void StatsDialog::populateDeckData() {
    User user;
    if (!user.fetchSelected()) return;

    ui->label_2->setText("Deck Stats (Total)");

    std::vector<Deck> decks = user.listDecks();
    int totalCardsAdded = 0;
    int totalCardsSeen = 0;
    qint64 totalTimeSpent = 0;

    for (const auto& deck : decks) {
        DeckStats ds = deck.getTotalDeckStats();
        totalCardsAdded += ds.getCardsAdded();
        totalCardsSeen += ds.getCardsSeen();
        totalTimeSpent += ds.getTimeSpent();
    }

    ui->CardsAddedCount->setText(formatNumber(totalCardsAdded));
    ui->CardsSeenCount_2->setText(formatNumber(totalCardsSeen));
    ui->TimeSpentCount_3->setText(formatDuration(totalTimeSpent));
}

QString StatsDialog::formatDuration(qint64 seconds) {
    if (seconds < 60) return QString("%1s").arg(seconds);
    
    qint64 minutes = seconds / 60;
    qint64 hours = minutes / 60;
    qint64 remainingSeconds = seconds % 60;
    qint64 remainingMinutes = minutes % 60;

    if (hours > 0) {
        return QString("%1h %2m %3s").arg(hours).arg(remainingMinutes).arg(remainingSeconds);
    }
    return QString("%1m %2s").arg(remainingMinutes).arg(remainingSeconds);
}

QString StatsDialog::formatNumber(int number) {
    return QLocale(QLocale::English).toString(number);
}
