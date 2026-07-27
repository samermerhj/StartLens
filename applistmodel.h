/*
 * Copyright 2026 Samer Merhj <mjosak7@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef APPLISTMODEL_H
#define APPLISTMODEL_H

#include <QAbstractListModel>
#include <QList>

struct SystemItem {
    enum Type { Application, Library, Leftover };
    QString name;
    QString package;
    QString iconPath;
    Type type;
    QStringList files;
    QStringList leftovers;
    int usageCount = 0;
    QStringList usedByApps;
    QString version;
    QString referenceVersion;
    QString status;
    QString cleaningStatus;
};
Q_DECLARE_METATYPE(SystemItem)

class AppListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IconPathRole,
        PackageRole,
        TypeRole,
        ReferenceVersionRole,
        CleaningStatusRole
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (parent.isValid()) return 0;
        return m_items.size();
    }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void setItems(const QList<SystemItem> &items) { beginResetModel(); m_items = items; endResetModel(); }
    SystemItem itemAt(int row) const { return m_items.value(row); }

private:
    QList<SystemItem> m_items;
};

#endif // APPLISTMODEL_H
