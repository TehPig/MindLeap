#include "Frontend/addcarddialog.h"
#include "Frontend/invalidinputbox.h"
#include "ui_addcarddialog.h"
#include <qmessagebox.h>

AddCardDialog::AddCardDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddCardDialog) {
    ui->setupUi(this);
    setWindowTitle("Add Card");

    QFont labelFont = ui->label_2->font();
    labelFont.setPointSize(14); // Set a larger point size

    ui->label_2->setFont(labelFont);
    ui->label_3->setFont(labelFont);

    // Set size policies for QLabel widgets
    ui->label_2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->label_3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Connect textChanged signals to adjustTextEditSize slot
    connect(ui->textEdit, &QTextEdit::textChanged, this, &AddCardDialog::adjustTextEditSize);
    connect(ui->textEdit_2, &QTextEdit::textChanged, this, &AddCardDialog::adjustTextEditSize);

    adjustTextEditSize();
}

QString AddCardDialog::getQuestion() { return ui->textEdit->toPlainText(); };
QString AddCardDialog::getAnswer() { return ui->textEdit_2->toPlainText(); };

AddCardDialog::~AddCardDialog() { delete ui; }

// Note to myself: These checks can also be done using events and disabling buttons
// Same applies for all dialogs
bool AddCardDialog::validateEntries() {
    QString questionText = getQuestion();
    QString answerText = getQuestion();

    if (questionText.trimmed().isEmpty()) {
        showStyledMessageBox("Question field has an invalid value.\nEmpty text is not allowed.\n\nAdd some text and resubmit.", "Invalid Question", QMessageBox::Warning);
        return false;
    }
    if (answerText.trimmed().isEmpty()) {
        showStyledMessageBox("Answer field has an invalid value.\nEmpty text is not allowed.\n\nAdd some text and resubmit.", "Invalid Answer", QMessageBox::Warning);
        return false;
    }

    return true;
}

void AddCardDialog::onAccepted() {
    if(validateEntries()) accept();
}

void AddCardDialog::onRejected() { reject(); }

void AddCardDialog::adjustTextEditSize() {
    auto adjustHeight = [](QTextEdit* textEdit) {
        textEdit->setFixedHeight(textEdit->document()->size().height() + 10);
    };

    adjustHeight(ui->textEdit);
    adjustHeight(ui->textEdit_2);
}
