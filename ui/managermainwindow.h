#pragma once

#include <QApplication>
#include <QMainWindow>
#include <QObject>
#include <QHash>
#include <QSystemTrayIcon>
#include <QMenu>

#include "telephone.h"
#include "global.h"

class TelephoneMainWindow;
class GuideDialog;
class PreferencesMainWindow;
class AboutDialog;
class UserAgentManager;

class ManagerMainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit ManagerMainWindow(QWidget *parent = 0);
    ~ManagerMainWindow();

    void newMTelephone(Telephone);
    void updateMTelephone(QString, Telephone *);
    void removeMTelephone(Telephone *);
    void unloadMTelephones();

    void saveTelephone(Telephone *);
    void updateTelephone(Telephone *);
    void deleteTelephone(Telephone *);

    QHash<QString, TelephoneMainWindow *> getTelephones();

	UserAgentManager *getUserAgentManager();

    Global getGlobal();
    void updateGlobals(const Global& global);

	QStringList getAvailableCodecs();
	QStringList getActiveCodecs();

	void openPreferences();
	void openAbout();

	void bootstrap();
	void bootstrapGlobal();

    void startGuideDialog();

    void connectDatabase();
    static void bootstrapDatabase();
    void loadFromDatabase();

    bool hasActiveAccounts();

private slots:
	void open();
	void openPreferencesSlot();

private:
    void initTray();

private:
    QSystemTrayIcon *mTrayIcon{nullptr};
    QMenu *mTrayIconMenu{nullptr};

    QHash<QString, TelephoneMainWindow *> mTelephoneMainWins;
    TelephoneMainWindow *mTelephoneMainWin{nullptr};
    GuideDialog *mGuideDialog{nullptr};
    PreferencesMainWindow *mPreferences{nullptr};
    AboutDialog *mAboutDialog{nullptr};

    UserAgentManager *mUAManager{nullptr};
    Global mGlobal;
};
