#include "Frontend/invalidinputbox.h"

void showStyledMessageBox(const QString &input, const QString &title, QMessageBox::Icon icon) {
    QMessageBox msgBox;
    msgBox.setStyleSheet(R"(
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
    )");
    msgBox.setText(title);
    msgBox.setIcon(icon);
    msgBox.setWindowTitle(input);
    msgBox.exec();
}
