#include "aboutdialog.h"

#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
        QDialog(parent),
        mUi(new Ui::About) {
    mUi->setupUi(this);
}

AboutDialog::~AboutDialog() {
    delete mUi;
}
