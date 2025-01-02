#include "Frontend/selectuser.h"
#include "Frontend/createuserdialog.h"
#include <Frontend/renameuserdialog.h>
#include "Frontend/mainwindow.h"
#include "ui_selectuser.h"

#include <QApplication>
#include <QListWidget>
#include <QListWidgetItem>

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
    main->show();
    main->setAttribute(Qt::WA_DeleteOnClose); // Delete Main Window on close
}

void selectuser::on_Create_clicked() {
    CreateUserDialog* dialog = new CreateUserDialog(this);
    dialog->exec(); // Open the dialog and pause execution

    dialog->setAttribute(Qt::WA_DeleteOnClose); // Delete the dialog on close
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

    RenameUserDialog *dialog = new RenameUserDialog(id, this);
    dialog->exec(); // Open the dialog and pause execution
}

