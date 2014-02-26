// This file is part of RSS Guard.
//
// Copyright (C) 2011-2014 by Martin Rotter <rotter.martinos@gmail.com>
//
// RSS Guard is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// RSS Guard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with RSS Guard. If not, see <http://www.gnu.org/licenses/>.

#ifndef FORMSETTINGS_H
#define FORMSETTINGS_H

#include "ui_formsettings.h"

#include <QDialog>


namespace Ui {
  class FormSettings;
}

// Structure holding some initial values.
struct TemporarySettings {

  public:
    TemporarySettings()
      : m_webBrowserProgress(QColor()),
        m_mysqlDataStorageChanged(false) {
    }

    QColor m_webBrowserProgress;
    bool m_mysqlDataStorageChanged;
};

class FormSettings : public QDialog {
    Q_OBJECT
    
  public:
    // Constructors and destructors.
    explicit FormSettings(QWidget *parent = 0);
    virtual ~FormSettings();

  protected:
    // Does check of controls before dialog can be submitted.
    bool doSaveCheck();

  protected slots:
    // Displays "restart" dialog if some critical settings changed.
    void promptForRestart();

    // Saves settings into global configuration.
    void saveSettings();

    void loadInterface();
    void saveInterface();
    void onSkinSelected(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void loadGeneral();
    void saveGeneral();

    void loadDataStorage();
    void saveDataStorage();
    void mysqlTestConnection();
    void onMysqlHostnameChanged(const QString &new_hostname);
    void onMysqlUsernameChanged(const QString &new_username);
    void onMysqlPasswordChanged(const QString &new_password);
    void onMysqlDataStorageEdited();

    void loadLanguage();
    void saveLanguage();

    void loadShortcuts();
    void saveShortcuts();

    void loadBrowser();
    void saveBrowser();
    void changeDefaultBrowserArguments(int index);
    void selectBrowserExecutable();
    void changeBrowserProgressColor();
    void loadWebBrowserColor(const QColor &color);

    void loadProxy();
    void saveProxy();
    void displayProxyPassword(int state);
    void onProxyTypeChanged(int index);

    void loadFeedsMessages();
    void saveFeedsMessages();
    
  private:
    Ui::FormSettings *m_ui;
    TemporarySettings m_initialSettings;
    QStringList m_changedDataTexts;
};

#endif // FORMSETTINGS_H
