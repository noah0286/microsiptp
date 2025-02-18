#include "ua.h"

#include <QStatusBar>
#include "telephonemainwindow.h"
#include "mycall.h"

#include <QDebug>

UserAgent::UserAgent(QObject *parent)
		: QObject(parent) {
}

UserAgent::~UserAgent() {
}

void UserAgent::onRegState(pj::OnRegStateParam &prm) {
    auto accountInfo = getInfo();

    qDebug() << "account " << getId()
             << ", regIsActive " << (accountInfo.regIsActive ? "Registed" : "Unregisted")
             << ", status " << prm.status
             << ", sip code " << prm.code
             << ", sip text " << QString::fromStdString(prm.reason);

    emit sRegisterStatusChanged(accountInfo.regIsActive);
}

void UserAgent::onIncomingCall(pj::OnIncomingCallParam &iprm) {
    MyCall *call = new MyCall(*this, iprm.callId);
	emit sNewCall(call);
}

void UserAgent::setInstance(TelephoneMainWindow *telephone) {
    mTelephone = telephone;
    QObject::connect(this,
                     SIGNAL(sNewCall(MyCall * )),
                     mTelephone,
                     SLOT(actionInboundCall(MyCall * ))
                    );
    QObject::connect(this,
                     SIGNAL(sRegisterStatusChanged(bool)),
                     mTelephone,
                     SLOT(changeRegistrationStatus(bool))
                    );
}

void UserAgent::dispatchUiMessage(QString message) {
    QStatusBar *status = mTelephone->statusBar();
	status->showMessage(message);
}
