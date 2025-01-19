#include <QApplication>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStatusBar>

#include "Frontend/selectuser.h"
#include "Frontend/mainwindow.h"
#include "Frontend/Dialogs/customdialog.h"
#include "forms/ui_selectuser.h"

selectuser::selectuser(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::selectuser) {
    ui->setupUi(this);
    setWindowTitle("Select User | MindLeap");

    // List existing users
    User user;

    // List the user's decks
    std::vector<User> users = user.listUsers();
    populateListWidget(users);
}

selectuser::~selectuser() { delete ui; }

QListWidget* selectuser::get_listWidget() { return ui->listWidget; }

void selectuser::populateListWidget(const std::vector<User> &users) {
    for(const auto user : users){
        QString itemText = QString("%1").arg(user.getUsername());
        QListWidgetItem *item = new QListWidgetItem(itemText);

        item->setData(Qt::UserRole, QVariant(user.getID()));
        ui->listWidget->addItem(item);
    }
}

void selectuser::on_Quit_clicked() { QApplication::quit(); }

void selectuser::on_Open_clicked() {
    QListWidgetItem* item = ui->listWidget->currentItem();
    if(!item) return;

    QString id = item->data(Qt::UserRole).toString();

    User user(id);
    user.select();

    this->close();

    MainWindow *main = new MainWindow();
    main->setAttribute(Qt::WA_DeleteOnClose); // Delete Main Window on close
    main->show();
}

void selectuser::on_Create_clicked() {
    CustomDialog* dialog = new CustomDialog(this);
    dialog->setWindowTitleText("Create User");
    dialog->setMessageText("Specify username:");

    if(dialog->exec() == QDialog::Accepted){
        User user("n_" + dialog->getEnteredText());

        const bool status = user.create();
        if(!status){
            this->statusBar()->showMessage("Error: User was not created.");
            delete dialog;
            return;
        }

        QListWidget* list = this->get_listWidget();
        QListWidgetItem* item = new QListWidgetItem(dialog->getEnteredText().trimmed());

        item->setData(Qt::UserRole, QVariant(user.getID()));
        list->addItem(item);
    }
}

void selectuser::on_Delete_clicked() {
    QListWidgetItem *selectedItem = ui->listWidget->currentItem();
    if (!selectedItem) return;

    QString id = selectedItem->data(Qt::UserRole).toString(); // Get the ID

    User user(id);
    user._delete();

    ui->listWidget->removeItemWidget(selectedItem);
    delete selectedItem;
}

void selectuser::on_Rename_clicked() {
    QListWidgetItem *selectedItem = ui->listWidget->currentItem();
    if (!selectedItem) return;

    QString id = selectedItem->data(Qt::UserRole).toString(); // Get the ID

    CustomDialog* dialog = new CustomDialog(this);
    dialog->setWindowTitleText("Rename User");
    dialog->setMessageText("Specify new username:");

    if(dialog->exec() == QDialog::Accepted){
        User user(id);

        const bool status = user.rename(dialog->getEnteredText());
        if(!status){
            this->statusBar()->showMessage("Error: User was not renamed.");
            delete dialog;
            return;
        }

        QListWidget* list = this->get_listWidget();
        QListWidgetItem *selectedItem = list->currentItem();

        selectedItem->setText(dialog->getEnteredText().trimmed());
    }
}

