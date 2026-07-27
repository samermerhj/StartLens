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
#include "startlens_usage.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

StartLensUsage::StartLensUsage(QObject *parent) : QObject(parent)
{
    load();
    loadPinned();
}

void StartLensUsage::load()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens/usage.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonObject obj = QJsonDocument::fromJson(file.readAll()).object();
    QJsonObject countObj = obj["counts"].toObject();
    for (auto it = countObj.begin(); it != countObj.end(); ++it) {
        m_usageCount[it.key()] = it.value().toInt();
    }
    QJsonObject lastObj = obj["lastUsed"].toObject();
    for (auto it = lastObj.begin(); it != lastObj.end(); ++it) {
        m_lastUsed[it.key()] = QDateTime::fromString(it.value().toString(), Qt::ISODate);
    }
    file.close();
}

void StartLensUsage::save()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens";
    QDir().mkpath(dir);
    QJsonObject obj;
    QJsonObject countObj;
    for (auto it = m_usageCount.begin(); it != m_usageCount.end(); ++it) countObj[it.key()] = it.value();
    QJsonObject lastObj;
    for (auto it = m_lastUsed.begin(); it != m_lastUsed.end(); ++it) lastObj[it.key()] = it->toString(Qt::ISODate);
    obj["counts"] = countObj;
    obj["lastUsed"] = lastObj;
    QFile file(dir + "/usage.json");
    if (file.open(QIODevice::WriteOnly)) { file.write(QJsonDocument(obj).toJson()); file.close(); }
}

void StartLensUsage::recordUsage(const QString &appName)
{
    m_usageCount[appName]++;
    m_lastUsed[appName] = QDateTime::currentDateTime();
    save();
}

// ===== الدوال الجديدة =====
void StartLensUsage::removeApp(const QString &appName) {
    if (m_usageCount.contains(appName)) {
        m_usageCount.remove(appName);
    }
    if (m_lastUsed.contains(appName)) {
        m_lastUsed.remove(appName);
    }
    save();
}

void StartLensUsage::setUsageCount(const QMap<QString, int> &counts) {
    m_usageCount = counts;
    save();
}

void StartLensUsage::setLastUsed(const QMap<QString, QDateTime> &last) {
    m_lastUsed = last;
    save();
}
// =====================

void StartLensUsage::loadPinned()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens/pinned.json";
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return;
    QJsonArray arr = QJsonDocument::fromJson(file.readAll()).array();
    for (const auto &v : arr) m_pinnedApps.append(v.toString());
    file.close();
}

void StartLensUsage::savePinned()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens";
    QDir().mkpath(dir);
    QJsonArray arr;
    for (const auto &app : m_pinnedApps) arr.append(app);
    QFile file(dir + "/pinned.json");
    if (file.open(QIODevice::WriteOnly)) { file.write(QJsonDocument(arr).toJson()); file.close(); }
}

void StartLensUsage::pinApp(const QString &appName)
{
    if (!m_pinnedApps.contains(appName)) { m_pinnedApps.append(appName); savePinned(); }
}

void StartLensUsage::unpinApp(const QString &appName)
{
    m_pinnedApps.removeAll(appName); savePinned();
}

bool StartLensUsage::isPinned(const QString &appName) const
{
    return m_pinnedApps.contains(appName);
}
