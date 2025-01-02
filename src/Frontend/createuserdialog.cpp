#include "Frontend/createuserdialog.h"
#include "Frontend/selectuser.h"
#include "ui_createuserdialog.h"

#include "Backend/Classes/User.hpp"

#include <QAbstractButton>
#include <QTableWidget>
#include <QTableWidgetItem>

CreateUserDialog::CreateUserDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateUserDialog) {
    ui->setupUi(this);
    setWindowTitle("Create User");
}

CreateUserDialog::~CreateUserDialog() { delete ui; }

void CreateUserDialog::on_buttonBox_clicked(QAbstractButton *button) {
    if(!ui->buttonBox->button(QDialogButtonBox::Ok)) return this->reject();

    User user("n_" + ui->lineEdit->text());

    const bool status = user.create();
    if(!status) return ui->label->setText("Error: User was not created.");

    selectuser *parent = qobject_cast<selectuser*>(parentWidget());
    if(!parent) return ui->label->setText("Error: Parent not found.");

    this->accept();

    QListWidget* list = parent->get_listWidget();
    QListWidgetItem* item = new QListWidgetItem(ui->lineEdit->text());

    item->setData(Qt::UserRole, QVariant(user.getID()));
    list->addItem(item);
}
