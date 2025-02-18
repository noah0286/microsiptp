#pragma once

#include <QMainWindow>
#include <QCloseEvent>
#include <QLabel>

#include "telephone.h"

class ManagerMainWindow;
class PreferencesMainWindow;
class MyCall;

namespace Ui {
    class Telephone;
}

class TelephoneMainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit TelephoneMainWindow(QWidget *parent = 0);
    ~TelephoneMainWindow();

    void setManager(ManagerMainWindow *manager);
    void setTelephone(Telephone telephone);
	void statusMessage(QString);
    ManagerMainWindow *getManager();

public slots:
    void actionPreferences();
    void actionAbout();
    void changeStatus(int index);
    void actionInboundCall(MyCall *call);
    void actionOutboundCall();
    void changeRegistrationStatus(bool status);

protected:
	void closeEvent(QCloseEvent *event) override;

public:
    Ui::Telephone *mUi;

    ManagerMainWindow *mManager{nullptr};
    PreferencesMainWindow *mPreferences{nullptr};
    QLabel *mStatusLabel{nullptr};
    Telephone mTelephone;
};
