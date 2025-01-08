#ifndef CONFIRMATIONDIALOG_H
#define CONFIRMATIONDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QString>

class ConfirmationDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfirmationDialog(const QString &additionalText, QWidget *parent = nullptr);

private slots:
    void onConfirm();
    void onCancel();

private:
    void setCustomStyleSheet();
};

#endif // CONFIRMATIONDIALOG_H
