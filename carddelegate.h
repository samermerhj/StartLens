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
#ifndef CARDDELEGATE_H
#define CARDDELEGATE_H

#include <QStyledItemDelegate>
#include <QPainter>
#include <QIcon>
#include <QSettings>
#include <QApplication>

class StartLensSettings;  // إعلان مسبق

class CardDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CardDelegate(StartLensSettings *settings, QObject *parent = nullptr);
    explicit CardDelegate(QObject *parent = nullptr);   // للتوافق القديم

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    StartLensSettings *m_settings = nullptr;
};

#endif // CARDDELEGATE_H
