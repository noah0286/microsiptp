#include "mycall.h"
#include "calldialog.h"

#include <QDebug>

MyCall::MyCall(pj::Account &acc, int callId, CallDialog *cd)
    :Call(acc, callId), mCallDialog(cd) {
}

void MyCall::onCallState(pj::OnCallStateParam &prm) {
    auto callInfo = getInfo();
    qDebug() << "sid " << getId() << ", state " << callInfo.state;

    if (callInfo.state == PJSIP_INV_STATE_DISCONNECTED) {
        if (mCallDialog)
            mCallDialog->doCallDestroy();
    } else if (callInfo.state == PJSIP_INV_STATE_CONFIRMED) {
        if (mCallDialog)
            mCallDialog->doCallbackAnswer();
    }
}

void MyCall::onCallMediaState(pj::OnCallMediaStateParam &prm) {
    auto callInfo = getInfo();
    qDebug() << "sid " << getId() << ", media size " << callInfo.media.size();

    for (unsigned idx = 0; idx < callInfo.media.size(); idx++) {
        if (callInfo.media[idx].type == PJMEDIA_TYPE_AUDIO && getMedia(idx)) {
            auto audMed = (pj::AudioMedia *) getMedia(idx);

            auto &audDevMgr = pj::Endpoint::instance().audDevManager();
            audMed->startTransmit(audDevMgr.getPlaybackDevMedia());
            audDevMgr.getCaptureDevMedia().startTransmit(*audMed);
		}
	}
}

void MyCall::setInstance(CallDialog *telephoneCall) {
    mCallDialog = telephoneCall;
}

void MyCall::doHold(bool isHold) {
    qDebug() << "sid " << getId() << ", hold " << isHold;

    if (!isHold) {
        auto prm = pj::CallOpParam(isHold);

        setHold(prm);
    } else {
        pj::CallOpParam prm;
        prm.opt.audioCount = 1;
        prm.opt.videoCount = 0;
        prm.opt.flag = PJSUA_CALL_UNHOLD;

        reinvite(prm);
    }
}

void MyCall::doMute(bool mute) {
    qDebug() << "sid " << getId() << ", mute " << mute;

    auto callInfo = getInfo();

    for (unsigned idx = 0; idx < callInfo.media.size(); idx++) {
        if (callInfo.media[idx].type == PJMEDIA_TYPE_AUDIO && getMedia(idx)) {
            auto audMed = (pj::AudioMedia *) getMedia(idx);

            auto &audDevMgr = pj::Endpoint::instance().audDevManager();
            if (!mute) {
                audDevMgr.getCaptureDevMedia().stopTransmit(*audMed);
            } else {
                audDevMgr.getCaptureDevMedia().startTransmit(*audMed);
            }
        }
    }
}

void MyCall::doDtmf(QString dtmf) {
    qDebug() << "sid " << getId() << ", dtmf " << dtmf;

    dialDtmf(dtmf.toStdString());
}

void MyCall::doTransfer(QString destination, QString callee) {
    qDebug() << "sid " << getId() << ", desinition " << destination << ", callee " << callee;

    pj::CallOpParam prm;

    QString sip = "sip:";
    QString sipUri = QString();

    sipUri.append(sip);
    sipUri.append(destination);
    sipUri.append('@');
    sipUri.append(callee.split('@').takeLast());

    xfer(sipUri.toStdString(), prm);
}

void MyCall::doAnswer() {
    qDebug() << "sid " << getId();

    pj::CallOpParam prm;
	prm.statusCode = PJSIP_SC_OK;

    answer(prm);
}

void MyCall::doHangup() {
    qDebug() << "sid " << getId();

    pj::CallOpParam prm;
	prm.statusCode = PJSIP_SC_OK;

    hangup(prm);
}

void MyCall::startPlayFileToRemote(const QString &file, bool loop) {
    auto callInfo = getInfo();

    unsigned playOptions = (loop ? 0 : PJMEDIA_FILE_NO_LOOP);

    stopPlayFileToRemote();

    try {
        for (unsigned idx = 0; idx < callInfo.media.size(); idx++) {
            if (callInfo.media[idx].type == PJMEDIA_TYPE_AUDIO && getMedia(idx)) {
                auto audMed = (pj::AudioMedia *) getMedia(idx);

                auto &audDevMgr = pj::Endpoint::instance().audDevManager();
                audDevMgr.getCaptureDevMedia().stopTransmit(*audMed);
                try {
                    mAudioPlayer = std::make_shared<pj::AudioMediaPlayer>();
                    mAudioPlayer->createPlayer(file.toUtf8().toStdString(), playOptions);
                    mAudioPlayer->startTransmit(*audMed);
                    mIsPlayingToRemote = true;
                } catch (pj::Error &e) {
                    qWarning() << "sid " << getId() << ",failed to play " << file
                               << ", err " << QString::fromStdString(e.reason);
                    audDevMgr.getCaptureDevMedia().startTransmit(*audMed);
                }
            }
        }
    } catch (pj::Error &e) {
        qWarning() << "sid " << getId()
        << ", err " << QString::fromStdString(e.reason);
    }
}

void MyCall::stopPlayFileToRemote() {
    if (mAudioPlayer)
        mAudioPlayer.reset();

    mIsPlayingToRemote = false;
}

void MyCall::startRecord(const QString &file, RecordMode mode)
{
    if (mIsRecord) return;

    auto callInfo = getInfo();

    try {
        for (unsigned idx = 0; idx < callInfo.media.size(); idx++) {
            if (callInfo.media[idx].type == PJMEDIA_TYPE_AUDIO && getMedia(idx)) {
                try {
                    mAudioRecorder = std::make_shared<pj::AudioMediaRecorder>();
                    mAudioRecorder->createRecorder(file.toStdString());

                    if (mode == RecordMode::both || mode ==RecordMode::remote) {
                        auto audMed = (pj::AudioMedia *) getMedia(idx);
                        audMed->startTransmit(*mAudioRecorder);
                    }

                    if (mode == RecordMode::both || mode ==RecordMode::local) {
                        if (mIsPlayingToRemote && mAudioPlayer) {
                            mAudioPlayer->startTransmit(*mAudioRecorder);
                        } else {
                            auto &audDevMgr = pj::Endpoint::instance().audDevManager();
                            audDevMgr.getCaptureDevMedia().startTransmit(*mAudioRecorder);
                        }
                    }

                    mIsRecord = true;
                } catch (pj::Error &e) {
                    qWarning() << "sid " << getId() << ",failed to record " << file
                               << ", err " << QString::fromStdString(e.reason);
                }
            }
        }
    } catch (pj::Error &e) {
        qWarning() << "sid " << getId()
        << ", err " << QString::fromStdString(e.reason);
    }
}

void MyCall::stopRecord()
{
    if (mAudioRecorder)
        mAudioRecorder.reset();

    mIsRecord = false;
}
