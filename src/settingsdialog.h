#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(int batteryPercent, bool charging, double energyRate = 0.0,
                            QWidget *parent = nullptr);

private slots:
    void onPowerProfileChanged(int index);
    void openGitHubIssues();
    void openConfigEditor();

private:
    QLabel *batteryIconLabel;
    QLabel *batteryTextLabel;
    QComboBox *profileCombo;
};

#endif  // SETTINGSDIALOG_H
