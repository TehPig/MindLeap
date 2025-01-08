#include "Frontend/confirmationdialog.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

ConfirmationDialog::ConfirmationDialog(const QString &additionalText, QWidget *parent)
    : QDialog(parent) {
    this->setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Confirmation");

    // Set the custom stylesheet
    setCustomStyleSheet();

    // Create layout and widget elements
    QVBoxLayout *layout = new QVBoxLayout(this);

    // Set the full question with the customizable part
    QString question = "Are you sure you want to " + additionalText + "?";

    QLabel *label = new QLabel(question, this);
    label->setStyleSheet("color: #a9b7c6; font-size: 14px; margin-bottom: 20px;");
    layout->addWidget(label);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    layout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &ConfirmationDialog::onConfirm);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ConfirmationDialog::onCancel);
}

void ConfirmationDialog::setCustomStyleSheet() {
    QString stylesheet = R"(
                            QDialog {
                                background-color: #1e1d23;
                                color: #a9b7c6;
                                font-family: Segoe UI, sans-serif;
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

void ConfirmationDialog::onConfirm() { accept(); }

void ConfirmationDialog::onCancel() { reject(); }
