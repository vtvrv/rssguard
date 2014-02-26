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

#ifndef LOCALIZATION_H
#define LOCALIZATION_H

#include <QString>
#include <QObject>
#include <QPointer>


struct Language {
    QString m_name;
    QString m_code;
    QString m_version;
    QString m_author;
    QString m_email;
};

class Localization : public QObject {
    Q_OBJECT

  private:
    // Constructor.
    explicit Localization(QObject *parent = 0);

  public:
    // Destructor.
    virtual ~Localization();

    // Singleton getter.
    static Localization *instance();

    // Returns code of language that should
    // be loaded according to settings.
    QString desiredLanguage();

    // Loads currently active language.
    void load();

    // Returns list of installed application localizations.
    // This list is used ie. in settings dialog.
    QList<Language> installedLanguages();

    // Returns empty string or loaded language
    // name if it is really loaded.
    inline QString loadedLanguage() const {
      return m_loadedLanguage;
    }

  private:
    // Code of loaded language.
    QString m_loadedLanguage;

    // Singleton.
    static QPointer<Localization> s_instance;
};

#endif // LOCALIZATION_H
