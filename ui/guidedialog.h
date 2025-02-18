#pragma once

#include <QDialog>
#include <QString>

class ManagerMainWindow;

namespace Ui {
    class Guide;
}

class GuideDialog : public QDialog {
Q_OBJECT

public:
    explicit GuideDialog(QWidget *parent = 0);
    ~GuideDialog();

    void setManager(ManagerMainWindow *manager);

public slots:
	void accept();
	void usernameChanged(const QString &text);

private:
    Ui::Guide *mUi;
    ManagerMainWindow *mManager{nullptr};

};
