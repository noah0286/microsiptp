#include "preferencesmainwindow.h"

#include "ui_preferencesmainwindow.h"

#include "telephonemainwindow.h"
#include "managermainwindow.h"
#include "telephone.h"

#include <QFileDialog>
#include <QListWidgetItem>
#include <QDebug>

PreferencesMainWindow::PreferencesMainWindow(QWidget *parent) :
		QMainWindow(),
        mUi(new Ui::Preferences) {

    mUi->setupUi(this);

    connect(mUi->telephonesList,
            SIGNAL(currentItemChanged(QListWidgetItem * , QListWidgetItem * )),
            this,
	        SLOT(itemChanged(QListWidgetItem * , QListWidgetItem * )));

    connect(mUi->addButton,
            SIGNAL(clicked()),
            this,
	        SLOT(newItem()));

    connect(mUi->removeButton,
            SIGNAL(clicked()),
            this,
	        SLOT(removeItem()));

    connect(mUi->saveButton,
            SIGNAL(released()),
            this,
	        SLOT(saveChanges()));

    connect(mUi->customRingtoneButton,
            SIGNAL(released()),
            this,
	        SLOT(findRingtone()));
}

PreferencesMainWindow::~PreferencesMainWindow() {
    delete mUi;
}

void PreferencesMainWindow::closeEvent(QCloseEvent *event) {
    hide();
	event->ignore();
}

void PreferencesMainWindow::show() {
	QWidget::show();

    reload();
}

void PreferencesMainWindow::reload() {
    mCurrentTelephone = nullptr;
    mUi->telephonesList->clear();
    mUi->toolBox->hide();

    auto tels = mManager->getTelephones();

    foreach(QString item, tels.keys()) {
        mUi->telephonesList->addItem(tels[item]->mTelephone.username);
	}

    mUi->availableListWidget->clear();
    mUi->selectedListWidget->clear();

	foreach (QString codec, mManager->getAvailableCodecs()) {
        mUi->availableListWidget->addItem(codec);
	}

	foreach (QString codec, mManager->getActiveCodecs()) {
        mUi->selectedListWidget->addItem(codec);
	}
}

void PreferencesMainWindow::setManager(ManagerMainWindow *manager) {
	mManager = manager;

    reload();
}

void PreferencesMainWindow::itemChanged(QListWidgetItem *current, QListWidgetItem *previous) {
    auto tels = mManager->getTelephones();
    if (!current || !tels.contains(current->text())) {
		return;
	}

    mUi->toolBox->show();

    auto item = tels.value(current->text());
    const Telephone tel = item->mTelephone;

    mUi->descriptionEdit->setText(tel.description);
    mUi->descriptionEdit->setEnabled(!tel.active);
    mUi->nameEdit->setText(tel.name);
    mUi->nameEdit->setEnabled(!tel.active);
    mUi->domainEdit->setText(tel.domain);
    mUi->domainEdit->setEnabled(!tel.active);
    mUi->usernameEdit->setText(tel.username);
    mUi->usernameEdit->setEnabled(!tel.active);
    mUi->passwordEdit->setText(tel.password);
    mUi->passwordEdit->setEnabled(!tel.active);
    mUi->activeCheckbox->setChecked(tel.active);
    mUi->proxyEdit->setText(tel.proxy);
    mUi->proxyEdit->setEnabled(!tel.active);
    mUi->registerStartupCheckbox->setChecked(tel.should_register_startup);
    mUi->ringbackCheckbox->setChecked(tel.should_disable_ringback_tone);
    mUi->transportCombobox->setCurrentIndex(tel.transport - 1);
    mUi->keepAliveDelayEdit->setText(QString::number(tel.keep_alive_expiry_delay));
    mUi->registrationDelayEdit->setText(QString::number(tel.registration_expiry_delay));
    mUi->customRingtoneButton->setText(tel.custom_ringtone);

    mCurrentTelephone = current;
}

void PreferencesMainWindow::findRingtone() {
    if (!mCurrentTelephone)
        return;

    auto tels = mManager->getTelephones();
    if (!tels.contains(mCurrentTelephone->text())) {
		return;
	}

    TelephoneMainWindow *item = tels.value(mCurrentTelephone->text());
    Telephone tel = item->mTelephone;

    QUrl fileName = QFileDialog::getOpenFileUrl(this,
                                                tr("Open Audio"),
                                                QUrl(),
                                                tr("Audio Files (*.wav *.mp3 *.ogg)")
                                                );

	if (fileName.isEmpty() || !fileName.isValid()) {
        qDebug() << "select empty ringtone audio file";
		return;
	}

    tel.custom_ringtone = fileName.toString();

    mManager->updateMTelephone(tel.username, &tel);
    //tels = mManager->getTelephones();
}

void PreferencesMainWindow::saveChanges() {
    Global global = mManager->getGlobal();

	QStringList activeCodecs;
    for (int i = 0; i < mUi->selectedListWidget->count(); i++) {
        activeCodecs.append(mUi->selectedListWidget->item(i)->text());
	}

    global.mActiveCodecs = activeCodecs.join(",");
	mManager->updateGlobals(global);

    if (mCurrentTelephone == nullptr) {
		return;
	}

    auto tels = mManager->getTelephones();
    if (!tels.contains(mCurrentTelephone->text())) {
		return;
	}

    auto item = tels.value(mCurrentTelephone->text());
    auto tel = item->mTelephone;

    const bool shouldUpdateInstance = tel.active != mUi->activeCheckbox->checkState();
    const QString oldUsername = tel.username;
    tel.description = mUi->descriptionEdit->text();
    tel.name = mUi->nameEdit->text();
    tel.username = mUi->usernameEdit->text();
    tel.password = mUi->passwordEdit->text();
    tel.domain = mUi->domainEdit->text();
    tel.active = mUi->activeCheckbox->isChecked();
    tel.proxy = mUi->proxyEdit->text().trimmed();
    tel.should_register_startup = mUi->registerStartupCheckbox->isChecked();
    tel.should_disable_ringback_tone = mUi->ringbackCheckbox->isChecked();
    tel.transport = mUi->transportCombobox->currentIndex() + 1;
    tel.keep_alive_expiry_delay = mUi->keepAliveDelayEdit->text().toInt();
    tel.registration_expiry_delay = mUi->registrationDelayEdit->text().toInt();

    mUi->descriptionEdit->setEnabled(!tel.active);
    mUi->nameEdit->setEnabled(!tel.active);
    mUi->domainEdit->setEnabled(!tel.active);
    mUi->usernameEdit->setEnabled(!tel.active);
    mUi->passwordEdit->setEnabled(!tel.active);
    mUi->proxyEdit->setEnabled(shouldUpdateInstance);

    mManager->updateMTelephone(oldUsername, &tel);
    tels = mManager->getTelephones();

    if (mCurrentTelephone) {
        mCurrentTelephone->setText(tel.username);
        mUi->telephonesList->editItem(mCurrentTelephone);
	}

    if (shouldUpdateInstance && tel.active) {
        tels.value(tel.username)->show();
    } else if (shouldUpdateInstance && !tel.active) {
        tels.value(tel.username)->hide();
	}
}

void PreferencesMainWindow::newItem() {
    Telephone tel;
    tel.id = QString("0");
    tel.description = QString("dummy");
    tel.name = QString("dummy");
    tel.domain = QString("0.0.0.0");
    tel.username = QString("dummy@0.0.0.0");
    tel.password = QString("dummypass");
    tel.active = 0; // active
    //tel.proxy = QString("");

    // prefs
    tel.should_register_startup = 1;
    tel.should_subscribe_presence = 0;
    tel.should_publish_presence = 0;
    tel.should_use_blf = 0;
    tel.should_disable_ringback_tone = 0;
    //tel.custom_ringtone = QString("");

    tel.transport = 1;
    tel.subscription_expiry_delay = 0;
    tel.keep_alive_expiry_delay = 15;
    tel.registration_expiry_delay = 300;
    tel.use_stun = 0;
    //tel.stun_server = QString("");

    mManager->saveTelephone(&tel);
    mManager->newMTelephone(tel);
    mUi->telephonesList->addItem(tel.username);
}

void PreferencesMainWindow::removeItem() {
    if (!mCurrentTelephone)
		return;

    auto tels = mManager->getTelephones();
    if (!tels.contains(mCurrentTelephone->text())) {
		qDebug() << "No item selected";
		return;
	}

    auto item = tels.value(mCurrentTelephone->text());
    auto tel = &item->mTelephone;

    mManager->deleteTelephone(tel);
    mManager->removeMTelephone(tel);
    delete mUi->telephonesList->takeItem(mUi->telephonesList->row(mCurrentTelephone));

    mCurrentTelephone = nullptr;
}

void PreferencesMainWindow::on_removeCodecPushButton_clicked()
{
    auto r = mUi->selectedListWidget->currentRow();
    auto i = mUi->selectedListWidget->takeItem(r);
    if (!i) return; // not select codec

    if (!i->text().trimmed().isEmpty())
        mUi->availableListWidget->addItem(i->text().trimmed());
}


void PreferencesMainWindow::on_addCodecPushButton_clicked()
{
    auto r = mUi->availableListWidget->currentRow();
    auto i = mUi->availableListWidget->takeItem(r);
    if (!i) return; // not select codec

    if (!i->text().trimmed().isEmpty())
        mUi->selectedListWidget->addItem(i->text().trimmed());
}

