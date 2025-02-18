#pragma once

#include <QDialog>

namespace Ui {
    class About;
}

class AboutDialog : public QDialog {

Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = 0);
    ~AboutDialog();

private:
    Ui::About *mUi;
};
