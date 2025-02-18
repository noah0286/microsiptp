#pragma once

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>

#include <pjsua2.hpp>

#include "telephone.h"

class UserAgent;
class TelephoneMainWindow;
class MyCall;
class CallDialog;

class UserAgentManager : public QObject {
Q_OBJECT
public:
    explicit UserAgentManager(QObject *parent = 0);
	~UserAgentManager();

    pj::AccountConfig getAccountConfig(Telephone *mTelephone);
    void newUserAgent(TelephoneMainWindow *telephone, QString username, pj::AccountConfig acfg);
	void removeUserAgent(QString domain);
	void setRegister(QString domain, bool status);
    MyCall *placeCall(QString &username, const QString &dest, CallDialog* cd);

    void updateAudioCodecs(const QString &activeAudioCodecs);
    QStringList getAvailableAudioCodecs(const QString &activeAudioCodecs);
    QStringList getActiveAudioCodecs(const QString &activeAudioCodecs);

private:
    void initEp();
    void startEp(uint16_t port);
    void enumDev();

private:
	QHash<QString, UserAgent *> mAccounts;
    pj::Endpoint *mEp{nullptr};
    pj::EpConfig mEpCfg;
    pj::TransportConfig mEpTsCfg;

};
