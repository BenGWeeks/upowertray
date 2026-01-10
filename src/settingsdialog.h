#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QComboBox>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(int batteryPercent, bool charging, QWidget *parent = nullptr);

private slots:
    void onPowerProfileChanged(int index);
    void openGitHubIssues();
    void openConfigEditor();

private:
    QLabel *batteryIconLabel;
    QLabel *batteryTextLabel;
    QComboBox *profileCombo;
};

#endif // SETTINGSDIALOG_H
