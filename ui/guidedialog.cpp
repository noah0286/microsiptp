#include "guidedialog.h"
#include "ui_guidedialog.h"

#include "managermainwindow.h"
#include "telephone.h"

#include <QDebug>
#include <QPushButton>
#include <QLineEdit>

GuideDialog::GuideDialog(QWidget *parent)
    : QDialog(parent), mUi(new Ui::Guide) {
    mUi->setupUi(this);

    connect(mUi->buttonBox,
            SIGNAL(accepted()),
            this,
            SLOT(accept()),
            Qt::UniqueConnection);
    connect(mUi->usernameEdit,
            SIGNAL(textEdited(const QString &)),
            this,
			SLOT(usernameChanged(const QString &)));

    //mUi->usernameEdit->setPlaceholderText("extension@domain");
    mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

GuideDialog::~GuideDialog() {
    delete mUi;
}

void GuideDialog::setManager(ManagerMainWindow *manager) {
	mManager = manager;
}

void GuideDialog::usernameChanged(const QString &text) {
	if (text.contains('@')) {
        mUi->descriptionEdit->setText(text.split('@').takeFirst());
        mUi->nameEdit->setText(text.split('@').takeFirst());
        mUi->domainEdit->setText(text.split('@').takeLast());
		if (!text.split('@').takeLast().isEmpty()) {
            mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
		} else {
            mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
		}
	} else {
        mUi->domainEdit->setText("");
        mUi->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	}
}

void GuideDialog::accept() {
    auto description = mUi->descriptionEdit->text().trimmed();
    auto name = mUi->nameEdit->text().trimmed();
    auto domain = mUi->domainEdit->text().trimmed();
    auto username = mUi->usernameEdit->text().trimmed();
    auto password = mUi->passwordEdit->text().trimmed();
    auto proxy = mUi->proxyEdit->text().trimmed();

    if (username.isEmpty() ||
        password.isEmpty() ) {
        return;
    }

    Telephone tel;
    tel.id = QString("0");
    tel.description = description;
    tel.name = name;
    tel.domain = domain;
    tel.username = username;
    tel.password = password;
    tel.active = 1; // active
    tel.proxy = proxy;

    // prefs
    tel.should_register_startup = 1;
    tel.should_subscribe_presence = 0;
    tel.should_publish_presence = 0;
    tel.should_use_blf = 0;
    tel.should_disable_ringback_tone = 0;
    tel.custom_ringtone = QString("");

    tel.transport = 1;
    tel.subscription_expiry_delay = 0;
    tel.keep_alive_expiry_delay = 15;
    tel.registration_expiry_delay = 300;
    tel.use_stun = 0;
    tel.stun_server = QString("");

    mManager->saveTelephone(&tel);
    mManager->newMTelephone(tel);

    close();
}
