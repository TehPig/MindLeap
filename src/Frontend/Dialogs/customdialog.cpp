#include <QVBoxLayout>
#include <QPushButton>
#include <QFormLayout>
#include <QMessageBox>

#include "Frontend/Dialogs/customdialog.h"
#include "Frontend/invalidinputbox.h"

CustomDialog::CustomDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    applyStyleSheet();
    resize(500, 125);
}

CustomDialog::~CustomDialog() {}

void CustomDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    messageLabel = new QLabel(this);
    inputLineEdit = new QLineEdit(this);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CustomDialog::onAccepted);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CustomDialog::onRejected);

    mainLayout->addWidget(messageLabel);
    mainLayout->addWidget(inputLineEdit);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    inputLineEdit->setFocus();
}

void CustomDialog::applyStyleSheet() {
    QString stylesheet = R"(
                            QDialog {
                                background-color: #1e1d23;
                                color: #a9b7c6;
                                font-family: Arial, sans-serif;
                            }

                            QLabel {
                                color: #a9b7c6;
                                font-size: 14px;
                            }

                            QPushButton {
                                border: 1px solid #04b97f;
                                color: #a9b7c6;
                                padding: 4px 10px;
                                background-color: #1e1d23;
                            }

                            QPushButton:hover {
                                border-bottom-color: #37efba;
                                color: #FFFFFF;
                            }

                            QPushButton:pressed {
                                color: #37efba;
                                background-color: #1e1d23;
                            }

                            QDialogButtonBox {
                                border: none;
                            }

                            QDialogButtonBox QPushButton {
                                margin: 5px;
                            }
    )";
    setStyleSheet(stylesheet);
}

void CustomDialog::setWindowTitleText(const QString &title) { setWindowTitle(title); }

void CustomDialog::setMessageText(const QString &message) { messageLabel->setText(message); }

QString CustomDialog::getEnteredText() const { return inputLineEdit->text(); }

bool CustomDialog::validateEntry() {
    QString enteredText = getEnteredText();

    if (enteredText.trimmed().isEmpty()) {
        showStyledMessageBox("Description has an invalid value.\nEmpty text is not allowed.\n\nAdd some text and resubmit.", "Invalid Description", QMessageBox::Warning);
        return false;
    }

    return true;
}

void CustomDialog::onAccepted() {
    if (validateEntry()) accept();
}

void CustomDialog::onRejected() { reject(); }
