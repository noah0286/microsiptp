#pragma once

#include <QMainWindow>

class QListWidgetItem;
class ManagerMainWindow;

namespace Ui {
    class Preferences;
}

class PreferencesMainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit PreferencesMainWindow(QWidget *parent = 0);
    ~PreferencesMainWindow();

	void show();
	void reload();
    void setManager(ManagerMainWindow *manager);

protected:
	void closeEvent(QCloseEvent *event) override;

public slots:
	void itemChanged(QListWidgetItem *current, QListWidgetItem *previous);
	void saveChanges();
	void newItem();
	void removeItem();
	void findRingtone();

private slots:
    void on_removeCodecPushButton_clicked();

    void on_addCodecPushButton_clicked();

private:
    Ui::Preferences *mUi;

    ManagerMainWindow *mManager{nullptr};
    QListWidgetItem *mCurrentTelephone{nullptr};
};
