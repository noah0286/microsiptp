#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
    class TransferCall;
}

class TransferCallDialog : public QDialog {
Q_OBJECT

public:
    explicit TransferCallDialog(std::function<void(QString)> onTransferClick);
    ~TransferCallDialog();

protected:
    void closeEvent(QCloseEvent *);

private slots:
    void actionTransfer();
    void actionCancel();
    void resetState();

private:
    Ui::TransferCall *mUi;
    std::function<void(QString)> mOnTransferClick;
};
