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
#include "startlens_scanner.h"
#include "SafeFileWriter.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QHash>
#include <QSet>

StartLensScanner::StartLensScanner(QObject *parent) : QObject(parent) {}

// ==========================================================
// دوال التخزين المؤقت (Cache)
// ==========================================================

QString StartLensScanner::cacheFilePath()
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/StartLens";
    QDir().mkpath(cacheDir);
    return cacheDir + "/menu_cache.json";
}

void StartLensScanner::clearCache()
{
    m_cacheValid = false;
    m_cachedApps.clear();
    QFile::remove(cacheFilePath());
}

void StartLensScanner::saveToCache(const QList<SystemItem> &items)
{
    QJsonArray arr;
    for (const auto &item : items) {
        QJsonObject obj;
        obj["name"] = item.name;
        obj["package"] = item.package;
        obj["iconPath"] = item.iconPath;
        obj["type"] = item.type;
        obj["version"] = item.version;
        obj["referenceVersion"] = item.referenceVersion;
        obj["status"] = item.status;
        obj["cleaningStatus"] = item.cleaningStatus;
        QJsonArray filesArray;
        for (const auto &f : item.files) filesArray.append(f);
        obj["files"] = filesArray;
        arr.append(obj);
    }
    SafeFileWriter::writeAtomic(cacheFilePath(), QJsonDocument(arr).toJson());
}

QList<SystemItem> StartLensScanner::loadFromCache()
{
    QList<SystemItem> items;
    QFile file(cacheFilePath());
    if (!file.open(QIODevice::ReadOnly)) return items;
    QJsonArray arr = QJsonDocument::fromJson(file.readAll()).array();
    for (const auto &val : arr) {
        QJsonObject obj = val.toObject();
        SystemItem item;
        item.name = obj["name"].toString();
        item.package = obj["package"].toString();
        item.iconPath = obj["iconPath"].toString();
        item.type = (SystemItem::Type)obj["type"].toInt();
        item.version = obj["version"].toString();
        item.referenceVersion = obj["referenceVersion"].toString();
        item.status = obj["status"].toString();
        item.cleaningStatus = obj["cleaningStatus"].toString();
        QJsonArray filesArray = obj["files"].toArray();
        for (const auto &f : filesArray) item.files.append(f.toString());
        items.append(item);
    }
    return items;
}

// ==========================================================
// دوال الملفات القديمة (للتوافق)
// ==========================================================

QString StartLensScanner::appsFilePath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens";
    QDir().mkpath(dir);
    return dir + "/startlens_apps.json";
}

QList<SystemItem> StartLensScanner::loadFromAppsFile()
{
    QList<SystemItem> items;
    QFile file(appsFilePath());
    if (!file.open(QIODevice::ReadOnly)) return items;
    QJsonArray arr = QJsonDocument::fromJson(file.readAll()).array();
    file.close();
    for (const auto &val : arr) {
        QJsonObject obj = val.toObject();
        SystemItem item;
        item.name = obj["name"].toString();
        item.package = obj["package"].toString();
        item.iconPath = obj["icon_path"].toString();
        item.type = SystemItem::Application;
        item.version = obj["version"].toString();
        items.append(item);
    }
    return items;
}

void StartLensScanner::saveToAppsFile(const QList<SystemItem> &items)
{
    QJsonArray arr;
    for (const auto &item : items) {
        QJsonObject obj;
        obj["name"] = item.name;
        obj["package"] = item.package;
        obj["icon_path"] = item.iconPath;
        obj["type"] = "application";
        obj["version"] = item.version;
        arr.append(obj);
    }
    SafeFileWriter::writeAtomic(appsFilePath(), QJsonDocument(arr).toJson());
}

void StartLensScanner::syncFromSharedMap()
{
    QString mapPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/LensClean/current_map.json";
    QFile mapFile(mapPath);
    if (!mapFile.open(QIODevice::ReadOnly)) return;
    QJsonObject mapObj = QJsonDocument::fromJson(mapFile.readAll()).object();
    mapFile.close();
    QList<SystemItem> apps;
    QJsonArray itemsArr = mapObj["items"].toArray();
    for (const auto &val : itemsArr) {
        QJsonObject obj = val.toObject();
        if (obj["type"].toString() == "application") {
            SystemItem item;
            item.name = obj["name"].toString();
            item.package = obj["package"].toString();
            item.iconPath = obj["icon_path"].toString();
            item.type = SystemItem::Application;
            item.version = obj["version"].toString();
            apps.append(item);
        }
    }
    if (!apps.isEmpty()) {
        saveToAppsFile(apps);
    }
}

QList<SystemItem> StartLensScanner::scanWithGtkLaunch()
{
    QList<SystemItem> items;
    QProcess proc;
    proc.start("bash", {"-c", "ls /usr/share/applications/*.desktop | xargs -I{} basename {} .desktop"});
    proc.waitForFinished(3000);
    QStringList apps = QString(proc.readAllStandardOutput()).split('\n', Qt::SkipEmptyParts);
    for (const QString &app : apps) {
        SystemItem item;
        item.name = app;
        item.package = app + ".desktop";
        item.iconPath = "/usr/share/applications/" + app + ".desktop";
        item.type = SystemItem::Application;
        item.version = "unknown";
        items.append(item);
    }
    return items;
}

// ==========================================================
// طبقات القراءة الجديدة
// ==========================================================

QStringList StartLensScanner::getDesktopDirs()
{
    QStringList dirs;
    dirs << "/usr/share/applications";
    dirs << QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);

    QString flatpakDir = QDir::homePath() + "/.local/share/flatpak/exports/share/applications";
    if (QDir(flatpakDir).exists()) dirs << flatpakDir;

    QString snapDir = "/var/lib/snapd/desktop/applications";
    if (QDir(snapDir).exists()) dirs << snapDir;

    QByteArray dataDirs = qgetenv("XDG_DATA_DIRS");
    if (!dataDirs.isEmpty()) {
        QStringList extraDirs = QString::fromLocal8Bit(dataDirs).split(':', Qt::SkipEmptyParts);
        for (const QString &d : extraDirs) {
            QString appDir = d + "/applications";
            if (QDir(appDir).exists()) dirs << appDir;
        }
    }

    QStringList uniqueDirs;
    for (const QString &d : dirs) {
        if (!uniqueDirs.contains(d)) uniqueDirs << d;
    }
    return uniqueDirs;
}

bool StartLensScanner::isAppExecutable(const SystemItem &app)
{
    if (app.referenceVersion == "group") return true;

    QString cmd = app.package;
    if (cmd.isEmpty()) cmd = app.name;
    if (cmd.endsWith(".desktop")) cmd.chop(8);
    if (cmd.contains(' ')) cmd = cmd.split(' ').first();

    if (!QStandardPaths::findExecutable(cmd).isEmpty()) return true;

    if (!app.iconPath.isEmpty() && app.iconPath.endsWith(".desktop")) {
        QSettings desktop(app.iconPath, QSettings::IniFormat);
        desktop.beginGroup("Desktop Entry");
        QString exec = desktop.value("Exec").toString();
        desktop.endGroup();
        if (!exec.isEmpty()) {
            QString execCmd = exec.split(' ').first();
            if (!QStandardPaths::findExecutable(execCmd).isEmpty()) return true;
        }
    }
    return false;
}

QList<SystemItem> StartLensScanner::deduplicateApps(const QList<SystemItem> &apps)
{
    QHash<QString, SystemItem> bestApp;
    QHash<QString, int> bestPriority;

    for (const auto &app : apps) {
        QString key = app.name;
        int priority = 30;
        if (app.referenceVersion == "custom") {
            priority = 10;
        } else if (app.iconPath.contains(".local/share/applications")) {
            priority = 100;
        } else if (app.iconPath.contains("flatpak")) {
            priority = 80;
        } else if (app.iconPath.contains("snap")) {
            priority = 70;
        } else if (app.iconPath.startsWith("/usr/share/applications")) {
            priority = 50;
        }

        if (!bestApp.contains(key) || priority > bestPriority.value(key, -1)) {
            bestApp[key] = app;
            bestPriority[key] = priority;
        }
    }
    return bestApp.values();
}

QList<SystemItem> StartLensScanner::loadFromLxqtMenuCache()
{
    QList<SystemItem> items;
    if (QStandardPaths::findExecutable("lxqt-menu-cache").isEmpty()) return items;

    QProcess process;
    process.start("lxqt-menu-cache", {"--json"});
    if (!process.waitForFinished(3000)) return items;

    QByteArray output = process.readAllStandardOutput();
    if (output.isEmpty()) return items;

    QJsonDocument doc = QJsonDocument::fromJson(output);
    if (doc.isNull()) return items;

    QJsonArray appsArray = doc.array();
    for (const auto &val : appsArray) {
        QJsonObject obj = val.toObject();
        SystemItem item;
        item.name = obj["name"].toString();
        item.package = obj["exec"].toString();
        item.iconPath = obj["icon"].toString();
        item.type = SystemItem::Application;
        item.version = obj["version"].toString();
        items.append(item);
    }
    return items;
}

QList<SystemItem> StartLensScanner::loadFromXdgMenu()
{
    QList<SystemItem> items;
    if (QStandardPaths::findExecutable("xdg-menu").isEmpty()) return items;

    QProcess process;
    process.start("xdg-menu", {"--format", "json"});
    if (!process.waitForFinished(3000)) return items;

    QByteArray output = process.readAllStandardOutput();
    if (output.isEmpty()) return items;

    QJsonDocument doc = QJsonDocument::fromJson(output);
    if (doc.isNull()) return items;

    QJsonArray appsArray = doc.array();
    for (const auto &val : appsArray) {
        QJsonObject obj = val.toObject();
        SystemItem item;
        item.name = obj["name"].toString();
        item.package = obj["exec"].toString();
        item.iconPath = obj["icon"].toString();
        item.type = SystemItem::Application;
        item.version = obj["version"].toString();
        items.append(item);
    }
    return items;
}

QList<SystemItem> StartLensScanner::loadFromDesktopFiles()
{
    QList<SystemItem> allApps;
    QStringList dirs = getDesktopDirs();
    QSet<QString> seen;

    for (const QString &dirPath : dirs) {
        QDir dir(dirPath);
        if (!dir.exists()) continue;
        const QFileInfoList files = dir.entryInfoList(QStringList("*.desktop"), QDir::Files);
        for (const QFileInfo &fi : files) {
            QSettings desktop(fi.absoluteFilePath(), QSettings::IniFormat);
            desktop.beginGroup("Desktop Entry");
            QString name = desktop.value("Name").toString();
            QString exec = desktop.value("Exec").toString();
            QString noDisplay = desktop.value("NoDisplay").toString();
            QString hidden = desktop.value("Hidden").toString();
            QString icon = desktop.value("Icon").toString();
            QString categories = desktop.value("Categories").toString();
            desktop.endGroup();

            if (name.isEmpty() || exec.isEmpty() || noDisplay == "true" || hidden == "true") continue;

            SystemItem item;
            item.name = name;
            item.package = fi.completeBaseName();
            item.iconPath = fi.absoluteFilePath();
            item.type = SystemItem::Application;
            item.version = "unknown";
            if (!categories.isEmpty()) item.cleaningStatus = categories;
            allApps.append(item);
        }
    }

    QList<SystemItem> deduped = deduplicateApps(allApps);
    QList<SystemItem> finalApps;
    for (const auto &app : deduped) {
        if (isAppExecutable(app)) finalApps.append(app);
    }
    return finalApps;
}

// ==========================================================
// الدالة الرئيسية
// ==========================================================

QList<SystemItem> StartLensScanner::loadAppsAndGroups()
{
    if (m_cacheValid && !m_cachedApps.isEmpty()) {
        return m_cachedApps;
    }

    QList<SystemItem> cached = loadFromCache();
    if (!cached.isEmpty()) {
        m_cachedApps = cached;
        m_cacheValid = true;
        return cached;
    }

    QList<SystemItem> allItems;

    // الطبقة الأولى: lxqt-menu-cache
    allItems = loadFromLxqtMenuCache();

    // الطبقة الثانية: xdg-menu
    if (allItems.isEmpty()) {
        allItems = loadFromXdgMenu();
    }

    // الطبقة الثالثة: يدوي
    if (allItems.isEmpty()) {
        allItems = loadFromDesktopFiles();
    }

    // ===== إضافة المجموعات من LensClean =====
    QString groupsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/LensClean/appgroups.json";
    QFile groupsFile(groupsPath);
    if (groupsFile.open(QIODevice::ReadOnly)) {
        QJsonArray groupsArr = QJsonDocument::fromJson(groupsFile.readAll()).array();
        for (const auto &val : groupsArr) {
            QJsonObject obj = val.toObject();
            SystemItem groupItem;
            groupItem.name = obj["name"].toString();
            groupItem.package = "__group__";
            groupItem.type = SystemItem::Application;
            groupItem.referenceVersion = "group";
            groupItem.cleaningStatus = obj["color"].toString();
            groupItem.version = obj["mainAppName"].toString();

            QString customIcon = obj["customIconPath"].toString();
            QString iconSourceApp = obj["iconSourceApp"].toString();
            QJsonArray appsArr = obj["apps"].toArray();

            if (!customIcon.isEmpty()) {
                groupItem.iconPath = customIcon;
            } else if (!iconSourceApp.isEmpty()) {
                groupItem.status = iconSourceApp;
            } else if (!appsArr.isEmpty()) {
                QJsonObject firstApp = appsArr.first().toObject();
                groupItem.iconPath = firstApp["iconPath"].toString();
            }
            for (const auto &appVal : appsArr) {
                QJsonObject appObj = appVal.toObject();
                groupItem.files.append(appObj["name"].toString());
            }
            allItems.append(groupItem);
        }
        groupsFile.close();
    }

    saveToCache(allItems);
    m_cachedApps = allItems;
    m_cacheValid = true;

    emit progress(100, 100);
    QCoreApplication::processEvents();
    return allItems;
}
