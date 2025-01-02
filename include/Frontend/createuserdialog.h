#ifndef CREATEUSERDIALOG_H
#define CREATEUSERDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class CreateUserDialog;
}

class CreateUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateUserDialog(QWidget *parent = nullptr);
    ~CreateUserDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::CreateUserDialog *ui;
};

#endif // CREATEUSERDIALOG_H
