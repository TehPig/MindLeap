#include "Frontend/Dialogs/studydeckdialog.h"
#include "Backend/Classes/Deck.hpp"
#include "Backend/Classes/User.hpp"
#include "Dialogs/ui_studydeckdialog.h"
#include "Frontend/invalidinputbox.h"
#include "Frontend/mainwindow.h"

StudyDeckDialog::StudyDeckDialog(MainWindow *parent)
    : QDialog(parent)
    , ui(new Ui::StudyDeckDialog)
{
    ui->setupUi(this);

    setWindowTitle("Study Deck");

    User user;
    const bool user_status = user.fetchSelected();
    if (!user_status) {
        qDebug() << "Error: Could not fetch selected user.";
        return;
    }

    const std::vector<Deck> decks = user.listDecks();
    if (decks.empty()) {
        qDebug() << "Error: Selected user has no decks.";
        return;
    }

    for (const Deck& deck : decks) {
        QListWidgetItem* item = new QListWidgetItem(deck.getName());
        item->setData(Qt::UserRole, deck.getID());
        ui->listWidget->addItem(item);
    }
}

StudyDeckDialog::~StudyDeckDialog() {
    delete ui;
}

void StudyDeckDialog::on_buttonBox_accepted() {
    QListWidgetItem* selectedItem = ui->listWidget->currentItem();
    if (!selectedItem) {
        showStyledMessageBox("No deck selected.", "You did not select a deck to study.", QMessageBox::Warning);
        return;
    }

    QString deckID = selectedItem->data(Qt::UserRole).toString();
    emit studySessionRequested(deckID);

    accept();
}
