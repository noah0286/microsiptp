#include "telephonemainwindow.h"

#include "managermainwindow.h"
#include "ui_telephonemainwindow.h"
#include "preferencesmainwindow.h"

#include "mycall.h"
#include "calldialog.h"
#include "uamanager.h"

#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QShortcut>

TelephoneMainWindow::TelephoneMainWindow(QWidget *parent) :
		QMainWindow(parent),
        mUi(new Ui::Telephone) {
    mUi->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    mStatusLabel = new QLabel(this);
    mUi->statusBar->addPermanentWidget(mStatusLabel);

	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Comma), this,
				  SLOT(actionPreferences()));
	new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this,
				  SLOT(close()));

    connect(mUi->actionPreferences,
			SIGNAL(triggered()), this,
			SLOT(actionPreferences()));
    connect(mUi->actionAbout,
			SIGNAL(triggered()), this,
			SLOT(actionAbout()));
    connect(mUi->statusComboBox,
			SIGNAL(currentIndexChanged(int)), this,
			SLOT(changeStatus(int)));
    connect(mUi->sipInput,
			SIGNAL(returnPressed()), this,
			SLOT(actionOutboundCall()));
}

TelephoneMainWindow::~TelephoneMainWindow() {
    mManager->removeMTelephone(&mTelephone);
    delete mUi;
}

void TelephoneMainWindow::closeEvent(QCloseEvent *event) {
    hide();
	event->ignore();
}

void TelephoneMainWindow::setManager(ManagerMainWindow *manager) {
	mManager = manager;
}

ManagerMainWindow *TelephoneMainWindow::getManager() {
	return mManager;
}

void TelephoneMainWindow::changeRegistrationStatus(bool status) {
	static const char *statusNames[] = {"Unregistered",
										"Registered",
										nullptr};
    disconnect(mUi->statusComboBox,
               SIGNAL(currentIndexChanged(int)),
               0,
               0);
	statusMessage(statusNames[status]);
    mUi->statusComboBox->setCurrentIndex(!status);
    connect(mUi->statusComboBox,
            SIGNAL(currentIndexChanged(int)),
            this,
			SLOT(changeStatus(int)));
}

void TelephoneMainWindow::statusMessage(QString message) {
    mStatusLabel->setText(message);
}

void TelephoneMainWindow::setTelephone(Telephone telephone) {
	mTelephone = telephone;
	QString title = "";
	title.append(mTelephone.username);
	if (!mTelephone.description.isEmpty()) {
		title.append(" - ");
		title.append(mTelephone.description);
	}
    setWindowTitle(title);
}

void TelephoneMainWindow::actionInboundCall(MyCall *call) {
    auto dialog = new CallDialog(this,
                                 CallDialog::CallDirection::inbound,
                                 mTelephone.username,
                                 mTelephone.should_disable_ringback_tone,
                                 mTelephone.custom_ringtone);
	qApp->alert(this);
	dialog->setInstance(call);
	call->setInstance(dialog);
	dialog->show();
}

void TelephoneMainWindow::actionOutboundCall() {
    const auto dest = mUi->sipInput->text().trimmed();
    if (dest.isEmpty())
        return;

    try {
        auto cd = new CallDialog(this,
                                 CallDialog::CallDirection::outbound,
                                 mTelephone.username,
                                 mTelephone.should_disable_ringback_tone,
                                 mTelephone.custom_ringtone);
        auto call = mManager->getUserAgentManager()->placeCall(mTelephone.username, dest, cd);
        cd->setInstance(call);
        //call->setInstance(cd);
        cd->show();
    } catch (pj::Error &err) {
        mUi->statusBar->showMessage(QString::fromStdString(err.reason), 2000);
        qDebug() << "Cannot place call";
    }
}

void TelephoneMainWindow::actionPreferences() {
	mManager->openPreferences();
}

void TelephoneMainWindow::actionAbout() {
	mManager->openAbout();
}

void TelephoneMainWindow::changeStatus(int index) {
	switch (index) {
		case 0:
			mManager->getUserAgentManager()->setRegister(mTelephone.username, true);
			break;
		case 1:
			mManager->getUserAgentManager()->setRegister(mTelephone.username, false);
			break;
	}
}
