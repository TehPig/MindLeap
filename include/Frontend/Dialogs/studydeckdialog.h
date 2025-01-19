#ifndef STUDYDECKDIALOG_H
#define STUDYDECKDIALOG_H

#include <QDialog>

#include "Frontend/mainwindow.h"

namespace Ui {
class StudyDeckDialog;
}

class StudyDeckDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StudyDeckDialog(MainWindow *parent = nullptr);
    ~StudyDeckDialog();

signals:
    void studySessionRequested(const QString& deckID);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::StudyDeckDialog *ui;
    Ui::MainWindow* main_ui;
};

#endif // STUDYDECKDIALOG_H
