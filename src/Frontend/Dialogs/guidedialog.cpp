#include "Frontend/Dialogs/guidedialog.h"
#include "Dialogs/ui_guidedialog.h"

GuideDialog::GuideDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::GuideDialog) {
    ui->setupUi(this);
    ensurePolished();
}

GuideDialog::~GuideDialog() {
    delete ui;
}
