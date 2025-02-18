#include "managermainwindow.h"

#include "telephonemainwindow.h"
#include "guidedialog.h"
#include "aboutdialog.h"
#include "preferencesmainwindow.h"
#include "uamanager.h"

#include <QSqlDatabase>
#include <QSqlDriver>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>

#include <QDebug>

const char *TELEPHONE_CONF_DIR = std::getenv("TELEPHONE_CONF_DIR");

ManagerMainWindow::ManagerMainWindow(QWidget *parent) :
		QMainWindow(parent) {

    initTray();

    connectDatabase();
    bootstrapDatabase();

    bootstrapGlobal();

    mUAManager = new UserAgentManager(this);
    mUAManager->updateAudioCodecs(mGlobal.mActiveCodecs);

    bootstrap();
}

ManagerMainWindow::~ManagerMainWindow() {
    unloadMTelephones();

    if (mTelephoneMainWin) {
        delete mTelephoneMainWin;
	}
    if (mGuideDialog) {
        delete mGuideDialog;
	}
	if (mUAManager) {
		delete mUAManager;
	}
}

void ManagerMainWindow::bootstrapGlobal() {
    QSqlQuery queryGlb;
    queryGlb.prepare(Global::sqlTableGlbSelect());

    if (!queryGlb.exec()) {
        qWarning() << "ERROR: " << queryGlb.lastError().text();
		return;
	}

    while (queryGlb.next()) {
        mGlobal.mActiveCodecs = queryGlb.value(0).toString();
	}
}

void ManagerMainWindow::bootstrap() {
    loadFromDatabase();
    if (mTelephoneMainWins.isEmpty()) {
        startGuideDialog();
		return;
	}

    if (!hasActiveAccounts()) {
        openPreferences();
		return;
	}
}

void ManagerMainWindow::openPreferencesSlot() {
    openPreferences();
}

UserAgentManager *ManagerMainWindow::getUserAgentManager() {
	return mUAManager;
}

void ManagerMainWindow::newMTelephone(Telephone telephone) {
    if (mTelephoneMainWins.contains(telephone.username)) {
        mTelephoneMainWin = mTelephoneMainWins[telephone.username];
	} else {
        mTelephoneMainWin = new TelephoneMainWindow();
	}
    mTelephoneMainWin->setManager(this);
    mTelephoneMainWin->setTelephone(telephone);
    mTelephoneMainWins[telephone.username] = mTelephoneMainWin;
	if (telephone.active == 1) {
        mTelephoneMainWin->show();
        auto accountConfig = mUAManager->getAccountConfig(&telephone);
        mUAManager->newUserAgent(mTelephoneMainWin,
		                         telephone.username,
                                 accountConfig);
        mTelephoneMainWin->statusMessage("Registering...");
	}
    mTelephoneMainWin = nullptr;

    qDebug() << mTelephoneMainWins;
}

void ManagerMainWindow::updateMTelephone(QString oldUsername, Telephone *telephone) {
    mTelephoneMainWin = mTelephoneMainWins[oldUsername];
    mTelephoneMainWin->setTelephone(*telephone);
	if (QString::compare(telephone->username, oldUsername)) {
        mTelephoneMainWins.remove(oldUsername);
	}
    mTelephoneMainWins[telephone->username] = mTelephoneMainWin;
    updateTelephone(telephone);

	mUAManager->removeUserAgent(oldUsername);
	if (mPreferences) {
		mPreferences->reload();
	}
	if (telephone->active == 1) {
        mUAManager->newUserAgent(mTelephoneMainWin,
		                         telephone->username,
		                         mUAManager->getAccountConfig(telephone));
        mTelephoneMainWin->statusMessage("Registering...");
	}
    mTelephoneMainWin = nullptr;
}

void ManagerMainWindow::removeMTelephone(Telephone *telephone) {
    auto removedTelephone = mTelephoneMainWins.value(telephone->username);
    mTelephoneMainWins.remove(telephone->username);
	if (telephone->active) {
		removedTelephone->close();
	}
	mUAManager->removeUserAgent(telephone->username);
	if (mPreferences) {
		mPreferences->reload();
	}
    mTelephoneMainWin = nullptr;
}

QHash<QString, TelephoneMainWindow *> ManagerMainWindow::getTelephones() {
    return mTelephoneMainWins;
}

void ManagerMainWindow::connectDatabase() {
    const QString d(Global::sqlDriverName());
    if (QSqlDatabase::isDriverAvailable(d)) {
        QSqlDatabase db = QSqlDatabase::addDatabase(d);
        QDir confDir = QDir(TELEPHONE_CONF_DIR);

		if (!confDir.exists()) {
			confDir.mkdir(confDir.path());
		}

        db.setDatabaseName(confDir.filePath("telephone.db"));
		if (!db.open()) {
			qWarning() << "ERROR: " << db.lastError();
		}
	}
}

void ManagerMainWindow::bootstrapDatabase() {
    QSqlQuery query(Global::sqlTableTelCreate());
	if (!query.isActive())
        qWarning() << query.lastError().text();

    QSqlQuery queryGlb(Global::sqlTableGlbCreate());
    if (!queryGlb.isActive())
        qWarning() << queryGlb.lastError().text();

    QSqlQuery queryGlbInsert;
    queryGlbInsert.prepare(Global::sqlTableGlbInsert());
    //queryGlbInsert.bindValue(":active_codecs", "PCMU/8000/1,PCMA/8000/1,opus/48000/2");
    queryGlbInsert.bindValue(":active_codecs", "PCMU/8000/1,PCMA/8000/1");

    if (!queryGlbInsert.exec()) {
        qWarning() << queryGlbInsert.lastError().text();
		return;
	}
}

void ManagerMainWindow::loadFromDatabase() {
	QSqlQuery query;
    query.prepare(Global::sqlTableTelSelect());

	if (!query.exec()) {
        qWarning() << "ERROR: " << query.lastError().text();
		return;
	}

	while (query.next()) {
        int idx = 0;
        Telephone telephone{
            query.value(idx++).toString(),
            query.value(idx++).toString(),
            query.value(idx++).toString(),
            query.value(idx++).toString(),
            query.value(idx++).toString(),
            query.value(idx++).toString(),
            query.value(idx++).toInt(),
            query.value(idx++).toString(),

            query.value(idx++).toInt(),
            query.value(idx++).toInt(),
            query.value(idx++).toInt(),
            query.value(idx++).toInt(),
            query.value(idx++).toInt(),
            query.value(idx++).toString(),
            query.value(idx++).toInt(),

            query.value(idx++).toInt(),
            query.value(idx++).toInt(),
            query.value(idx++).toInt(),
            query.value(idx++).toInt(),
            query.value(idx++).toString()
		};

        newMTelephone(telephone);
	}

    QSqlQuery queryGlb;
    queryGlb.prepare(Global::sqlTableGlbSelect());

    if (!queryGlb.exec()) {
        qWarning() << queryGlb.lastError().text();
		return;
	}

    while (queryGlb.next()) {
        mGlobal.mActiveCodecs = queryGlb.value(0).toString();
	}

    qDebug() << "codecs support " << mGlobal.mActiveCodecs;
}

Global ManagerMainWindow::getGlobal() {
    return mGlobal;
}

void ManagerMainWindow::updateGlobals(const Global& global) {
    QSqlQuery queryGlbUpdate;
    queryGlbUpdate.prepare(Global::sqlTableGlbUpdate());
    queryGlbUpdate.bindValue(":active_codecs", global.mActiveCodecs);

    if (!queryGlbUpdate.exec()) {
        qWarning() << queryGlbUpdate.lastError().text();
		return;
	}

    mGlobal.mActiveCodecs = global.mActiveCodecs;

    mUAManager->updateAudioCodecs(mGlobal.mActiveCodecs);
}

bool ManagerMainWindow::hasActiveAccounts() {
	QSqlQuery query;
    query.prepare(Global::sqlTableTelActiveCount());

	if (!query.exec()) {
        qWarning() << "ERROR: " << query.lastError().text();
		return false;
	}

	query.first();
	return query.value(0).toInt() > 0;
}

void ManagerMainWindow::unloadMTelephones() {
    foreach(QString item, mTelephoneMainWins.keys()) {
		mUAManager->removeUserAgent(item);
	}
}

void ManagerMainWindow::saveTelephone(Telephone *telephone) {
	QSqlQuery query;
    query.prepare(Global::sqlTableTelInsert());
	query.bindValue(":description", telephone->description);
	query.bindValue(":name", telephone->name);
	query.bindValue(":domain", telephone->domain);
	query.bindValue(":username", telephone->username);
	query.bindValue(":password", telephone->password);
	query.bindValue(":active", telephone->active);
    query.bindValue(":proxy", telephone->proxy);
	query.bindValue(":should_register_startup", telephone->should_register_startup);
	query.bindValue(":should_subscribe_presence", telephone->should_subscribe_presence);
	query.bindValue(":should_publish_presence", telephone->should_publish_presence);
	query.bindValue(":should_use_blf", telephone->should_use_blf);
	query.bindValue(":should_disable_ringback_tone", telephone->should_disable_ringback_tone);
	query.bindValue(":custom_ringtone", telephone->custom_ringtone);
	query.bindValue(":transport", telephone->transport);
	query.bindValue(":subscription_expiry_delay", telephone->subscription_expiry_delay);
	query.bindValue(":keep_alive_expiry_delay", telephone->keep_alive_expiry_delay);
	query.bindValue(":registration_expiry_delay", telephone->registration_expiry_delay);
	query.bindValue(":use_stun", telephone->use_stun);
	query.bindValue(":stun_server", telephone->stun_server);

	if (!query.exec()) {
        qWarning() << "ERROR: " << query.lastError().text();
		return;
	}

	if (query.lastInsertId().canConvert(QMetaType::QString)) {
		telephone->id = query.lastInsertId().toString();
	}
}

void ManagerMainWindow::updateTelephone(Telephone *telephone) {
	if (telephone->id.isEmpty() || telephone->id.isNull()) {
		qWarning() << "No id provided on update.";
		return;
	}

	QSqlQuery query;
    query.prepare(Global::sqlTableTelUpdate());
	query.bindValue(":description", telephone->description);
	query.bindValue(":name", telephone->name);
	query.bindValue(":domain", telephone->domain);
	query.bindValue(":username", telephone->username);
	query.bindValue(":password", telephone->password);
	query.bindValue(":active", telephone->active);
    query.bindValue(":proxy", telephone->proxy);
	query.bindValue(":should_register_startup", telephone->should_register_startup);
	query.bindValue(":should_subscribe_presence", telephone->should_subscribe_presence);
	query.bindValue(":should_publish_presence", telephone->should_publish_presence);
	query.bindValue(":should_use_blf", telephone->should_use_blf);
	query.bindValue(":should_disable_ringback_tone", telephone->should_disable_ringback_tone);
	query.bindValue(":custom_ringtone", telephone->custom_ringtone);
	query.bindValue(":transport", telephone->transport);
	query.bindValue(":subscription_expiry_delay", telephone->subscription_expiry_delay);
	query.bindValue(":keep_alive_expiry_delay", telephone->keep_alive_expiry_delay);
	query.bindValue(":registration_expiry_delay", telephone->registration_expiry_delay);
	query.bindValue(":use_stun", telephone->use_stun);
	query.bindValue(":stun_server", telephone->stun_server);
	query.bindValue(":id", telephone->id);

	qDebug() << telephone->id;

	if (!query.exec()) {
        qWarning() << query.lastError().text();
		return;
	}
}

void ManagerMainWindow::deleteTelephone(Telephone *telephone) {
	if (telephone->id.isEmpty() || telephone->id.isNull()) {
		qWarning() << "No id provided on delete.";
		return;
	}

	QSqlQuery query;
    query.prepare(Global::sqlTableTelDelete());
    query.bindValue(":id", telephone->id.toInt());

	if (!query.exec()) {
        qWarning() << query.lastError().text();
		return;
    }
}


QStringList ManagerMainWindow::getAvailableCodecs() {
    return mUAManager->getAvailableAudioCodecs(mGlobal.mActiveCodecs);
}

QStringList ManagerMainWindow::getActiveCodecs() {
    return mUAManager->getActiveAudioCodecs(mGlobal.mActiveCodecs);
}

void ManagerMainWindow::open() {
    bootstrap();
}

void ManagerMainWindow::openPreferences(void) {
	if (mPreferences) {
		mPreferences->show();
		mPreferences->activateWindow();
		mPreferences->raise();
		mPreferences->setFocus();
		return;
	}

    mPreferences = new PreferencesMainWindow(this);
	mPreferences->setManager(this);
	mPreferences->show();
}

void ManagerMainWindow::openAbout() {
    if (mAboutDialog) {
        mAboutDialog->show();
        mAboutDialog->activateWindow();
        mAboutDialog->raise();
        mAboutDialog->setFocus();
		return;
	}

    mAboutDialog = new AboutDialog(this);
    mAboutDialog->show();
}

void ManagerMainWindow::startGuideDialog() {
    mGuideDialog = new GuideDialog();
    mGuideDialog->setManager(this);
    mGuideDialog->show();
}

void ManagerMainWindow::initTray() {
    mTrayIcon = new QSystemTrayIcon(this);
    mTrayIcon->setToolTip(tr("MicroSIPTP"));

    mTrayIconMenu = new QMenu(this);
    const auto openAction = mTrayIconMenu->addAction(tr("Open"));
    const auto settingsAction = mTrayIconMenu->addAction(tr("Settings"));
    const auto exitAction = mTrayIconMenu->addAction(tr("Exit"));
    mTrayIcon->setContextMenu(mTrayIconMenu);
    mTrayIconMenu->show();

    const auto appIcon = QIcon(":/image/telephone.png");
    mTrayIcon->setIcon(appIcon);
    setWindowIcon(appIcon);

    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openPreferencesSlot()));
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    mTrayIcon->show();
}
