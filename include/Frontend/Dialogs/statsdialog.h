#ifndef STATSDIALOG_H
#define STATSDIALOG_H

#include <QDialog>

#include "Backend/Classes/Stats/UserStats.hpp"

namespace Ui {
class StatsDialog;
}

class StatsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatsDialog(QWidget *parent = nullptr);
    ~StatsDialog();

private:
    Ui::StatsDialog *ui;

    void populateUserData(const UserStats& stats);
    void populateDeckData();

    QString formatDuration(qint64 seconds);
    QString formatNumber(int number);
};

#endif // STATSDIALOG_H
