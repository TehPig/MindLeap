#include <QProcess>

#include "Backend/Database/setup.hpp"
#include "Frontend/Dialogs/preferencesdialog.h"
#include "Dialogs/ui_preferencesdialog.h"

PreferencesDialog::PreferencesDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);

    setWindowTitle("Preferences | MindLeap");
}

PreferencesDialog::~PreferencesDialog() {
    delete ui;
}

void PreferencesDialog::on_pushButton_clicked() {
    Database* db = Database::getInstance();
    db->reset();

    QString applicationPath = QCoreApplication::applicationFilePath();
    QProcess::startDetached(applicationPath);

    // Exit the current application
    QApplication::exit();
}

