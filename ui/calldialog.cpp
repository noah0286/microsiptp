#include "calldialog.h"
#include "transfercalldialog.h"
#include "telephonemainwindow.h"

#include "ui_calldialog.h"

#include "mycall.h"

#include <ctime>

#include <QShortcut>
#include <QRegularExpression>
#include <QDebug>
#include <QFileDialog>

CallDialog::CallDialog(TelephoneMainWindow *parent,
                       CallDirection direction,
                       QString username,
                       int disableRingback,
                       QString customRingtone) :
        QDialog(parent),
        mUi(new Ui::Call) {
    mUi->setupUi(this);

    mCallDirection = direction;
    mCalleeUsername = username;
    mIsDisableRingback = (bool) disableRingback;
    mCustomRingtone = customRingtone;

    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_A), this,
				  SLOT(actionAnswer()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_W), this,
				  SLOT(actionHangup()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_H), this,
				  SLOT(actionHold()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_M), this,
				  SLOT(actionMute()));
    new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_T), this,
				  SLOT(openTransferCallDialog()));

    connect(mUi->answerButton,
			SIGNAL(clicked()), this,
			SLOT(actionAnswer()));
    connect(mUi->cancelButton,
			SIGNAL(clicked()), this,
			SLOT(actionHangup()));

    connect(mUi->muteButton,
			SIGNAL(clicked()), this,
			SLOT(actionMute()));
    connect(mUi->holdButton,
			SIGNAL(clicked()), this,
			SLOT(actionHold()));
    connect(mUi->transferButton,
			SIGNAL(clicked()), this,
            SLOT(openTransferCallDialog()));

	connect(this,
			SIGNAL(rejected()), this,
			SLOT(onWindowClose()));

    connect(this,
            SIGNAL(sigCallDestroy()),
            this,
            SLOT(callDestroy()));

    connect(this,
            SIGNAL(sigCallbackAnswer()),
            this,
            SLOT(callbackAnswer()));

    mUi->dtmfInput->hide();
    mUi->callAction->hide();
    mUi->dtmfInput->setReadOnly(true);
    mUi->dtmfInput->setEnabled(false);
    mUi->dtmfInput->setFrame(false);
    if (mCallDirection == CallDirection::outbound) {
        mUi->answerButton->deleteLater();
	}

    mStartTimestamp = time(nullptr);
}

CallDialog::~CallDialog() {
    delete mUi;
}

void CallDialog::setInstance(MyCall *telephoneCall) {
    mCall = telephoneCall;
    auto ci = mCall->getInfo();

	QString whoLabel;
    mContact = getNumberFromURI(QString::fromStdString(ci.remoteUri));

    if (mCallDirection == CallDirection::outbound) {
		whoLabel.append("To: ");
		whoLabel.append(QString::fromStdString(ci.remoteUri));

        auto outboundAudio = new QSoundEffect();
        outboundAudio->setSource(QUrl::fromLocalFile(":/sound/outbound-ring.wav"));
        outboundAudio->setLoopCount(QSoundEffect::Infinite);
        outboundAudio->setVolume(1.0f);
        mOutboundAudio = outboundAudio;

        if (!mIsDisableRingback)
            outboundAudio->play();
	} else {
        auto inboundAudio = new QSoundEffect();
		whoLabel.append("From: ");
        //whoLabel.append(QString::fromStdString(ci.localContact));
        whoLabel.append(QString::fromStdString(ci.remoteContact));

        if (mCustomRingtone.isEmpty()) {
            inboundAudio->setSource(QUrl::fromLocalFile(":/sound/inbound-ring.wav"));
		} else {
            inboundAudio->setSource(QUrl::fromLocalFile(mCustomRingtone));
		}

        inboundAudio->setLoopCount(QSoundEffect::Infinite);
        inboundAudio->setVolume(1.0f);
        mInboundAudio = inboundAudio;
        if (!mIsDisableRingback)
			inboundAudio->play();
	}
    mUi->whoLabel->setText(whoLabel);
}

void CallDialog::doCallDestroy() {
    emit sigCallDestroy();
}

void CallDialog::doCallbackAnswer() {
    emit sigCallbackAnswer();
}

void CallDialog::callDestroy() {
    if (mCall) {
        delete mCall;
        mCall = nullptr;
	}

    close();
}

void CallDialog::closeEvent(QCloseEvent *event) {
    onWindowClose();
}

void CallDialog::keyPressEvent(QKeyEvent *event) {
    actionDtmf(event->text());
}

void CallDialog::onWindowClose() {
    if (mOutboundAudio) {
        mOutboundAudio->stop();
	}
    if (mInboundAudio) {
        mInboundAudio->stop();
	}
    if (mCall) {
        mCall->doHangup();
        delete mCall;
	}
    if (mTransferCallDiablog) {
        mTransferCallDiablog->close();
	}
}

void CallDialog::callbackAnswer() {
    mUi->dtmfInput->show();
    mUi->callAction->show();
    mIsAnswered = true;

    if (mOutboundAudio) {
        mOutboundAudio->stop();
        delete mOutboundAudio;
        mOutboundAudio = nullptr;
	}
}

void CallDialog::actionAnswer() {
    mUi->answerButton->setEnabled(false);
    mCall->doAnswer();

    mUi->dtmfInput->show();
    mUi->callAction->show();

    if (mInboundAudio) {
        mInboundAudio->stop();
        delete mInboundAudio;
        mInboundAudio = nullptr;
	}
}

void CallDialog::actionHangup() {
    if (mOutboundAudio) {
        mOutboundAudio->stop();
        delete mOutboundAudio;
        mOutboundAudio = nullptr;
	}
    if (mInboundAudio) {
        mInboundAudio->stop();
        delete mInboundAudio;
        mInboundAudio = nullptr;
	}
    mCall->doHangup();
    close();
}

void CallDialog::actionHold() {
    mUi->holdButton->setText(mIsHold ? "Hold" : "Release");
    mCall->doHold(mIsHold);
    mIsHold = !mIsHold;
}

void CallDialog::actionMute() {
    mUi->muteButton->setText(mIsMute ? "Mute" : "Unmute");
    mCall->doMute(mIsMute);
    mIsMute = !mIsMute;
}

void CallDialog::actionDtmf(QString digit) {
    if (!mIsAnswered) return;

    try {
        mCall->doDtmf(digit);
        mUi->dtmfInput->insert(digit);
    } catch (pj::Error &err) {
        qDebug() << "DTMF Error " << QString::fromStdString(err.reason);
    }
}

void CallDialog::actionTransfer(QString destinationNumber) {
	if (destinationNumber.isEmpty() || destinationNumber.isNull()) {
		return;
	}
    mCall->doTransfer(destinationNumber, mCalleeUsername);
    mTransferCallDiablog->close();
}

void CallDialog::openTransferCallDialog() {
    if (mTransferCallDiablog) {
        mTransferCallDiablog->show();
        return;
	}

    const auto onClickTransfer = std::bind(&CallDialog::actionTransfer, this, std::placeholders::_1);
    mTransferCallDiablog = new TransferCallDialog(onClickTransfer);
    mTransferCallDiablog->setWindowTitle(mContact);
}

QString CallDialog::getNumberFromURI(QString uri) {
    QRegularExpression re("sip:(?<number>.+)@.+");
    QRegularExpressionMatch match = re.match(uri);
    QString number = match.captured("number");
    return number;
}

void CallDialog::on_fileSelectPushButton_clicked() {
    auto f = QFileDialog::getOpenFileName(this,
                                        tr("Select Audio(8k,16k,32k,48k, 16bit PCM monochannel"),
                                        QString(),
                                        tr("Audio Files (*.wav)")
                                        );

    if (f.isEmpty()) {
        qDebug() << "select empty audio file";
        return;
    }

    mUi->fileSelectedLabel->setText(f);
}

void CallDialog::on_startPlayToRemotePushButton_clicked() {
    auto f = mUi->fileSelectedLabel->text().trimmed();
    if (f.isEmpty())
        return;

    mCall->startPlayFileToRemote(f, mUi->loopPlayCheckBox->isChecked());
}

void CallDialog::on_stopPlayToRemotePushButton_clicked() {
    mCall->stopPlayFileToRemote();
}

void CallDialog::on_recordFileStartPushButton_clicked() {
    static const QString defaultExt = ".wav";
    auto d = mUi->recordFileStoreDirLabel->text().trimmed();
    if (d.isEmpty()) return;

    // 0 - local, 1 - remote, 2 - both
    auto mode = MyCall::RecordMode(mUi->recordFileAudioModeComboBox->currentIndex());

    auto cinfo = mCall->getInfo();
    auto localUri = QString::fromStdString(cinfo.localUri).toUtf8().toBase64(QByteArray::Base64Option::Base64UrlEncoding);
    auto remoteUri = QString::fromStdString(cinfo.remoteUri).toUtf8().toBase64(QByteArray::Base64Option::Base64UrlEncoding);

    QString fileName = (mCallDirection == CallDirection::outbound) ? "out_" : "in_";

    fileName.append(localUri + "_");
    fileName.append(remoteUri + "_");

    switch (mode) {
    case MyCall::RecordMode::local:
        fileName.append("l_");
        break;
    case MyCall::RecordMode::remote:
        fileName.append("r_");
        break;
    case MyCall::RecordMode::both:
        fileName.append("b_");
        break;
    }

    fileName.append(QString::number(mStartTimestamp) + "_");
    fileName.append(QString::number(time(nullptr)));
    fileName.append(defaultExt);

    QDir dir(d);
    auto f = dir.absoluteFilePath(fileName);

    mCall->startRecord(f, mode);
}

void CallDialog::on_recordFileStopPushButton_clicked() {
    mCall->stopRecord();
}

void CallDialog::on_recordFileStoreDirPushButton_clicked()
{
    auto f = QFileDialog::getExistingDirectory(this,
                                          tr("Record Store Directory"),
                                          QString(),
                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
                                          );

    if (f.isEmpty()) {
        qDebug() << "select empty directory";
        return;
    }

    mUi->recordFileStoreDirLabel->setText(f);
}

