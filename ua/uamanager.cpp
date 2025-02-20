#include "uamanager.h"

#include <QDebug>
#include <QStringList>

#include "ua.h"
#include "mycall.h"
#include "calldialog.h"

const static uint16_t sSIPPortStarted = 5090;
const static int sCodecDefaultPriority = 168; // 0-255, where zero means to disable
const static int sCodecDisable = 0;
const static QString sSIPPrefix = "sip:";
const static QString sSIPSecPrefix = "sips:";

UserAgentManager::UserAgentManager(QObject *parent)
		: QObject(parent) {
    initEp();
}

UserAgentManager::~UserAgentManager() {
	//  ep->libDestroy();
	//  delete ep;
}

void UserAgentManager::startEp(uint16_t port) {
	try {
        mEpTsCfg.port = (unsigned)port;

        mEp->transportCreate(PJSIP_TRANSPORT_UDP, mEpTsCfg);
        mEp->transportCreate(PJSIP_TRANSPORT_TCP, mEpTsCfg);
        //mEp->transportCreate(PJSIP_TRANSPORT_TLS, mEpTsCfg); // todo

        mEp->libStart();

		qDebug() << "*** PJSUA2 STARTED ***";
    } catch (pj::Error &err) {
        qDebug() << "Error starting " << QString::fromStdString(err.reason);
        startEp(port + 1); // try another port
        return;
	}
}

pj::AccountConfig UserAgentManager::getAccountConfig(Telephone *mTelephone) {
    QString domain(sSIPPrefix);

    QString username;
	if (!mTelephone->username.split('@').isEmpty()) {
		domain.append(mTelephone->username.split('@').takeLast());
		username.append(mTelephone->username.split('@').takeFirst());
	} else {
		username.append(mTelephone->username);
	}

	if (mTelephone->transport == PJSIP_TRANSPORT_TCP) {
		domain.append(";transport=tcp");
	} else if (mTelephone->transport == PJSIP_TRANSPORT_TLS) {
		domain.append(";transport=tls");
	}

    int authCredDataType = 0; // plain text
    auto authCredData = mTelephone->password.toStdString();
    pj::AuthCredInfo cred("digest", "*", username.toStdString(), authCredDataType, authCredData);
    pj::AccountConfig acfg;
    acfg.idUri = QString(sSIPPrefix).append(mTelephone->username).toStdString();
	acfg.regConfig.registrarUri = domain.toStdString();
	acfg.regConfig.registerOnAdd = mTelephone->should_register_startup;
	acfg.regConfig.timeoutSec = mTelephone->registration_expiry_delay;
    if (!mTelephone->proxy.isEmpty()) {
        if (!mTelephone->proxy.contains(sSIPPrefix))
            acfg.sipConfig.proxies.push_back((QString(sSIPPrefix) + mTelephone->proxy).toStdString());
        else
            acfg.sipConfig.proxies.push_back(mTelephone->proxy.toStdString());
    }
	acfg.natConfig.udpKaIntervalSec = mTelephone->keep_alive_expiry_delay;
	acfg.sipConfig.authCreds.push_back(cred);

// *  0: SRTP does not require secure signaling
// *  1: SRTP requires secure transport such as TLS
// *  2: SRTP requires secure end-to-end transport (SIPS)
    acfg.mediaConfig.srtpSecureSignaling = 0;
    // 0 - disable, 1 - optional, 2 - mandatory
    acfg.mediaConfig.srtpUse = pjmedia_srtp_use(mTelephone->srtp_use);

	return acfg;
}

void UserAgentManager::newUserAgent(TelephoneMainWindow *telephone, QString username, pj::AccountConfig acfg) {
	UserAgent *acc = new UserAgent;
	try {
		acc->setInstance(telephone);
		acc->create(acfg);
		mAccounts[username] = acc;
    } catch (pj::Error &err) {
        qDebug() << "account create error " << QString::fromStdString(err.reason);
		delete acc;
	}
}

void UserAgentManager::removeUserAgent(QString username) {
    if (!mAccounts.contains(username))
        return;

    delete mAccounts.value(username); // remove Account in order to unregister
    mAccounts.remove(username);
}

void UserAgentManager::setRegister(QString username, bool status) {
    if (!mAccounts.contains(username))
        return;

    try {
        mAccounts[username]->setRegistration(status);
    } catch (pj::Error &err) {
        mAccounts[username]->dispatchUiMessage(QString::fromStdString(err.reason));
    }
}

MyCall *UserAgentManager::placeCall(QString &username, const QString &dest, CallDialog* cd) {
    if (!mAccounts.contains(username)) {
        qDebug() << "account not found for username " << username;
        return nullptr;
    }

    MyCall *call = new MyCall(*mAccounts[username], PJSUA_INVALID_ID, cd);

    qDebug() << " account " << username << " try call " << dest << " sid " << call->getId();

    QString sipUri(sSIPPrefix);
	sipUri.append(dest);
	if (!dest.split('@').isEmpty()) {
		sipUri.append('@');
		sipUri.append(username.split('@').takeLast());
	}

    call->makeCall(sipUri.toStdString(), pj::CallOpParam(true));
	return call;
}

void UserAgentManager::updateAudioCodecs(const QString &activeAudioCodecs)
{
    auto activeCodecs = getActiveAudioCodecs(activeAudioCodecs);
    auto disabledCodecs = getAvailableAudioCodecs(activeAudioCodecs);

    qDebug() << activeCodecs << "/" << disabledCodecs;


    int priority = sCodecDefaultPriority;
    foreach(auto codec, activeCodecs) {
        try {
            mEp->codecSetPriority(codec.toStdString(), priority);
        } catch (pj::Error &err) {
            qWarning() << "cannot import audio codec " << codec << " " << QString::fromStdString(err.reason);
            continue;
        }
        qDebug() << " add audio codec " << codec << ", priority " << priority;
        --priority;
    }

    foreach(auto codec, disabledCodecs) {
        try {
            mEp->codecSetPriority(codec.toStdString(), sCodecDisable);
        } catch (pj::Error &err) {
            qDebug() << "cannot disable " << codec << " " << QString::fromStdString(err.reason);
            continue;
        }
        qDebug() << " disable audio codec " << codec;
    }
}

void UserAgentManager::initEp()
{
    mEp = new pj::Endpoint;
    mEp->libCreate();
    mEp->libInit(mEpCfg);

    startEp(sSIPPortStarted);
    enumDev();
}

void UserAgentManager::enumDev() {
    auto& audMgr = mEp->audDevManager();
    auto& vidMgr = mEp->vidDevManager();

    auto audDevs = audMgr.enumDev2();
    for (auto& dev: audDevs) {
        qDebug() << "audio/" << dev.id << " " << dev.name
                 << " " << dev.inputCount
                 << "/" << dev.outputCount;
    }

    auto vidDevs = vidMgr.enumDev2();
    for (auto& dev: vidDevs) {
        qDebug() << "video/" << dev.id << " " << dev.name;
    }
}

QStringList UserAgentManager::getAvailableAudioCodecs(const QString &activeAudioCodecs) {
    QStringList availableCodecs;
    const auto codecEnum = pj::Endpoint::instance().codecEnum2();
    std::for_each(codecEnum.begin(), codecEnum.end(), [&](const auto& item){
        availableCodecs.append(QString::fromStdString(item.codecId));
    });

    auto activeCodecs = getActiveAudioCodecs(activeAudioCodecs);
    foreach (QString codec, activeCodecs) {
        availableCodecs.removeAll(codec);
    }

    return availableCodecs;
}

QStringList UserAgentManager::getActiveAudioCodecs(const QString &activeAudioCodecs) {
    if (activeAudioCodecs.isEmpty())
        return QStringList();

    QStringList codecs = activeAudioCodecs.split(",");
    return codecs;
}
