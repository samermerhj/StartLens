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
#include "applistmodel.h"

QVariant AppListModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_items.size()) return QVariant();
    const SystemItem &item = m_items.at(index.row());
    if (role == Qt::DisplayRole || role == NameRole) return item.name;
    if (role == IconPathRole) return item.iconPath;
    if (role == PackageRole) return item.package;
    if (role == TypeRole) return static_cast<int>(item.type);
    if (role == ReferenceVersionRole) return item.referenceVersion;
    if (role == CleaningStatusRole) return item.cleaningStatus;
    return QVariant();
}
