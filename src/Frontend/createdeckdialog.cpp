#include "Frontend/createdeckdialog.h"
#include "Frontend/mainwindow.h"
#include "ui_createdeckdialog.h"

#include "Backend/Classes/Deck.hpp"

#include <QAbstractButton>
#include <QTableWidget>
#include <QTableWidgetItem>

CreateDeckDialog::CreateDeckDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateDeckDialog) {
    ui->setupUi(this);
    setWindowTitle("Create Deck");
}

CreateDeckDialog::~CreateDeckDialog() {
    delete ui;
}

void CreateDeckDialog::on_buttonBox_clicked(QAbstractButton *button) {
    if(!ui->buttonBox->button(QDialogButtonBox::Ok)) return this->reject();

    Deck deck("n_" + ui->lineEdit->text());

    const bool status = deck.create();
    if(!status) return ui->label->setText("Error: Deck was not created.");

    MainWindow *parent = qobject_cast<MainWindow*>(parentWidget());
    if(!parent) return ui->label->setText("Error: Parent not found.");

    parent->get_tableWidget()->setVisible(false);

    this->accept();

    QTableWidget* list = parent->get_tableWidget();
    QTableWidgetItem* item = new QTableWidgetItem(ui->lineEdit->text());

    int row = list->rowCount();
    list->insertRow(row);

    item->setData(Qt::UserRole, QVariant(deck.getID()));
    list->setItem(row, 0, item);
}

