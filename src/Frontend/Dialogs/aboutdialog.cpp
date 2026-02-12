#include "Frontend/Dialogs/aboutdialog.h"
#include "Dialogs/ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    ensurePolished();
}

AboutDialog::~AboutDialog() {
    delete ui;
}
