#pragma once

#include <QDialog>
#include <QSoundEffect>
#include <QKeyEvent>

class MyCall;
class TelephoneMainWindow;
class TransferCallDialog;

namespace Ui {
    class Call;
}

class CallDialog : public QDialog {

Q_OBJECT

public:
    enum class CallDirection {
        inbound,
        outbound
    };

    explicit CallDialog(TelephoneMainWindow *parent = 0,
                            CallDirection direction = CallDirection::inbound,
                            QString username = "",
                            int disableRingback = 0,
                            QString customRingtone = "");
    ~CallDialog();

	void setInstance(MyCall *telephoneCall);

    void doCallDestroy();
    void doCallbackAnswer();

signals:
    void sigCallDestroy();
    void sigCallbackAnswer();

public slots:
    void actionHangup();
    void actionAnswer();
    void actionMute();
    void actionHold();
    void actionDtmf(QString);
    void actionTransfer(QString);

private slots:
    void callDestroy();
    void callbackAnswer();

protected:
	void closeEvent(QCloseEvent *);
	void keyPressEvent(QKeyEvent *);

private slots:
	void openTransferCallDialog();
	void onWindowClose();

private:
    QString getNumberFromURI(QString uri);

private:
    Ui::Call *mUi;

    MyCall *mCall{nullptr};
    CallDirection mCallDirection;
    QString mCalleeUsername;
    QString mCustomRingtone;
    QString mContact;

    QSoundEffect *mInboundAudio{nullptr};
    QSoundEffect *mOutboundAudio{nullptr};
    bool mIsAnswered{false};
    bool mIsHold{false};
    bool mIsMute{false};
    bool mIsDisableRingback{false};

    TransferCallDialog *mTransferCallDiablog{nullptr};
};
