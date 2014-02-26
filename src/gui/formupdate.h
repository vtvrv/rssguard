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

#ifndef FORMUPDATE_H
#define FORMUPDATE_H

#include <QDialog>

#include "ui_formupdate.h"


namespace Ui {
  class FormUpdate;
}

class FormUpdate : public QDialog {
    Q_OBJECT

  public:
    // Constructors and destructors.
    explicit FormUpdate(QWidget *parent = 0);
    virtual ~FormUpdate();

  protected slots:
    // Check for updates and interprets the results.
    void checkForUpdates();

  private:
    Ui::FormUpdate *m_ui;
};

#endif // FORMUPDATE_H
