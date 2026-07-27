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
#ifndef STARTLENS_USAGE_H
#define STARTLENS_USAGE_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QList>
#include <QString>

class StartLensUsage : public QObject
{
    Q_OBJECT
public:
    explicit StartLensUsage(QObject *parent = nullptr);

    void load();
    void save();
    void recordUsage(const QString &appName);

    QMap<QString, int> usageCount() const { return m_usageCount; }
    QMap<QString, QDateTime> lastUsed() const { return m_lastUsed; }

    // ===== الدوال الجديدة =====
    void removeApp(const QString &appName);           // حذف تطبيق من usage
    void setUsageCount(const QMap<QString, int> &counts);
    void setLastUsed(const QMap<QString, QDateTime> &last);
    // ========================

    QList<QString> pinnedApps() const { return m_pinnedApps; }
    void pinApp(const QString &appName);
    void unpinApp(const QString &appName);
    bool isPinned(const QString &appName) const;

private:
    void loadPinned();
    void savePinned();

    QMap<QString, int> m_usageCount;
    QMap<QString, QDateTime> m_lastUsed;
    QList<QString> m_pinnedApps;
};

#endif
