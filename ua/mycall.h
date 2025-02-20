#pragma once

#include <memory>

#include <QString>

#include <pjsua2.hpp>

class CallDialog;

class MyCall : public pj::Call {
public:
    enum class RecordMode {
        local,
        remote,
        both
    };

    MyCall(pj::Account &acc, int callId = PJSUA_INVALID_ID, CallDialog *cd = nullptr);
    virtual ~MyCall() = default;

    void onCallState(pj::OnCallStateParam &prm) override;
    void onCallMediaState(pj::OnCallMediaStateParam &prm) override;

    void setInstance(CallDialog *telephoneCall);
	void doHold(bool);
	void doMute(bool);
	void doDtmf(QString);
	void doTransfer(QString, QString);
	void doAnswer();
	void doHangup();

    void startPlayFileToRemote(const QString &file, bool loop);
    void stopPlayFileToRemote();

    void startRecord(const QString &file, RecordMode mode = RecordMode::both);
    void stopRecord();

private:
    CallDialog *mCallDialog{nullptr};

    std::shared_ptr<pj::AudioMediaPlayer> mAudioPlayer;
    bool mIsPlayingToRemote{false};

    std::shared_ptr<pj::AudioMediaRecorder> mAudioRecorder;
    bool mIsRecord{false};
};
