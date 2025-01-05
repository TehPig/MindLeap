#ifndef SELECTUSER_H
#define SELECTUSER_H

#include "Backend/Classes/User.hpp"
#include <QMainWindow>
#include <QListWidget>

namespace Ui {
    class selectuser;
}

class selectuser : public QMainWindow {
    Q_OBJECT

public:
    explicit selectuser(QWidget *parent = nullptr);
    ~selectuser();

    QListWidget* get_listWidget();

    private slots:
        void on_Quit_clicked();
        void on_Open_clicked();
        void on_Create_clicked();
        void on_Delete_clicked();
        void on_Rename_clicked();

private:
    void populateListWidget(const std::vector<User> &users);

    Ui::selectuser *ui;
};

#endif // SELECTUSER_H