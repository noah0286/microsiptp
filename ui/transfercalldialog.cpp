#include "transfercalldialog.h"
#include <QDebug>

#include "ui_transfercalldialog.h"

TransferCallDialog::TransferCallDialog(std::function<void(QString)> onTransferClick) :
		QDialog(),
        mUi(new Ui::TransferCall),
        mOnTransferClick(onTransferClick) {
    mUi->setupUi(this);
    mUi->destinationNumberInput->setPlaceholderText(tr("Destination Number"));
    mUi->transferButton->setDefault(true);

    connect(mUi->transferButton,
            SIGNAL(clicked()),
            this,
			SLOT(actionTransfer()));

    connect(mUi->cancelButton,
            SIGNAL(clicked()),
            this,
			SLOT(actionCancel()));

	connect(this,
			SIGNAL(rejected()), this,
			SLOT(resetState()));
}

TransferCallDialog::~TransferCallDialog() {
    delete mUi;
}

void TransferCallDialog::actionTransfer() {
    mOnTransferClick(mUi->destinationNumberInput->text());
}

void TransferCallDialog::actionCancel() {
    close();
}

void TransferCallDialog::resetState() {
    mUi->destinationNumberInput->clear();
    mUi->destinationNumberInput->setFocus();
}

void TransferCallDialog::closeEvent(QCloseEvent *event) {
	resetState();
}
