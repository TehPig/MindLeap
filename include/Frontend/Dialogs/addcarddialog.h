#ifndef ADDCARDDIALOG_H
#define ADDCARDDIALOG_H

#include <QDialog>

namespace Ui {
class AddCardDialog;
}

class AddCardDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddCardDialog(QWidget *parent = nullptr);
    ~AddCardDialog();

    QString getQuestion();
    QString getAnswer();

private slots:
    void onAccepted();
    void onRejected();

    bool validateEntries();

private:
    Ui::AddCardDialog *ui;

    void adjustTextEditSize();
};

#endif // ADDCARDDIALOG_H
