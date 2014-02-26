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

#ifndef MESSAGESPROXYMODEL_H
#define MESSAGESPROXYMODEL_H

#include <QSortFilterProxyModel>


class MessagesModel;

class MessagesProxyModel : public QSortFilterProxyModel {
    Q_OBJECT

  public:
    // Constructors and destructors.
    explicit MessagesProxyModel(QObject *parent = 0);
    virtual ~MessagesProxyModel();

    // Source model getter.
    inline MessagesModel *sourceModel() {
      return m_sourceModel;
    }

    // Maps list of indexes.
    QModelIndexList mapListToSource(const QModelIndexList &indexes);
    QModelIndexList mapListFromSource(const QModelIndexList &indexes, bool deep = false);

  protected:
    // Compares two rows of data.
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

  private:
    // Source model pointer.
    MessagesModel *m_sourceModel;
};

#endif // MESSAGESPROXYMODEL_H
