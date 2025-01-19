#ifndef CUSTOMDIALOG_H
#define CUSTOMDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>

class CustomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CustomDialog(QWidget *parent = nullptr);
    ~CustomDialog();

    // Setters for customizing title and text
    void setWindowTitleText(const QString &title);
    void setMessageText(const QString &message);

    // Retrieve entered username or text
    QString getEnteredText() const;

    // New method for validating input before accepting the dialog
    bool validateEntry();  // New method for validation

private slots:
    void onAccepted();
    void onRejected();

private:
    QLineEdit* inputLineEdit;
    QLabel* messageLabel;
    QDialogButtonBox* buttonBox;

    void setupUI();
    void applyStyleSheet();
};

#endif // CUSTOMDIALOG_H
