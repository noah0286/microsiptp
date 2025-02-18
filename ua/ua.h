#pragma once

#include <QObject>
#include <QString>
#include <QHash>

#include <pjsua2.hpp>

class MyCall;
class TelephoneMainWindow;

class UserAgent : public QObject, public pj::Account {
Q_OBJECT

public:
	explicit UserAgent(QObject *parent = 0);
	~UserAgent();

    void onRegState(pj::OnRegStateParam &prm) override;
    void onIncomingCall(pj::OnIncomingCallParam &iprm) override;

    void setInstance(TelephoneMainWindow *telephone);
	void dispatchUiMessage(QString message);

signals:
    void sRegisterStatusChanged(bool state);
	void sNewCall(MyCall *call);

private:
    TelephoneMainWindow *mTelephone{nullptr};
    //QHash<QString, MyCall *> mCalls;
};
