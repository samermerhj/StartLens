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
#include "startlens_settings.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

StartLensSettings::StartLensSettings(QObject *parent) : QObject(parent) { load(); }

QSet<QString> StartLensSettings::hiddenApps() const { return m_hiddenApps; }
void StartLensSettings::hideApp(const QString &appName) { m_hiddenApps.insert(appName); save(); }
void StartLensSettings::unhideApp(const QString &appName) { m_hiddenApps.remove(appName); save(); }
bool StartLensSettings::isHidden(const QString &appName) const { return m_hiddenApps.contains(appName); }

QMap<QString, QString> StartLensSettings::customCategories() const { return m_customCategories; }
void StartLensSettings::setCustomCategory(const QString &appName, const QString &category) {
    m_customCategories[appName] = category;
    save();
}
QString StartLensSettings::customCategory(const QString &appName) const {
    return m_customCategories.value(appName);
}

QMap<QString, QString> StartLensSettings::customIcons() const { return m_customIcons; }
void StartLensSettings::setCustomIcon(const QString &appName, const QString &iconText) {
    m_customIcons[appName] = iconText;
    save();
    emit iconChanged(appName, iconText);
}
QString StartLensSettings::customIcon(const QString &appName) const {
    return m_customIcons.value(appName);
}

QString StartLensSettings::backgroundMode() const { return m_backgroundMode; }
void StartLensSettings::setBackgroundMode(const QString &mode) {
    m_backgroundMode = mode;
    save();
    emit backgroundChanged();
}

QColor StartLensSettings::solidColor() const { return m_solidColor; }
void StartLensSettings::setSolidColor(const QColor &color) {
    m_solidColor = color;
    save();
    emit backgroundChanged();
}

int StartLensSettings::colorIntensity() const { return m_colorIntensity; }
void StartLensSettings::setColorIntensity(int intensity) {
    m_colorIntensity = intensity;
    save();
    emit backgroundChanged();
}

int StartLensSettings::imageIntensity() const { return m_imageIntensity; }
void StartLensSettings::setImageIntensity(int intensity) {
    m_imageIntensity = intensity;
    save();
    emit backgroundChanged();
}

QString StartLensSettings::backgroundImagePath() const { return m_backgroundImagePath; }
void StartLensSettings::setBackgroundImage(const QString &path) {
    m_backgroundImagePath = path;
    save();
    emit backgroundChanged();
}

bool StartLensSettings::showAdvancedCategories() const { return m_showAdvancedCategories; }
void StartLensSettings::setShowAdvancedCategories(bool show) {
    m_showAdvancedCategories = show;
    save();
}

bool StartLensSettings::showHiddenToggle() const { return m_showHiddenToggle; }
void StartLensSettings::setShowHiddenToggle(bool show) {
    m_showHiddenToggle = show;
    save();
}

void StartLensSettings::load() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens";
    QDir().mkpath(dir);

    QFile hiddenFile(dir + "/hidden.json");
    if (hiddenFile.open(QIODevice::ReadOnly)) {
        QJsonArray arr = QJsonDocument::fromJson(hiddenFile.readAll()).array();
        for (const auto &v : arr) m_hiddenApps.insert(v.toString());
        hiddenFile.close();
    }

    QFile catFile(dir + "/custom_categories.json");
    if (catFile.open(QIODevice::ReadOnly)) {
        QJsonObject obj = QJsonDocument::fromJson(catFile.readAll()).object();
        for (auto it = obj.begin(); it != obj.end(); ++it)
            m_customCategories[it.key()] = it.value().toString();
        catFile.close();
    }

    QFile iconFile(dir + "/custom_icons.json");
    if (iconFile.open(QIODevice::ReadOnly)) {
        QJsonObject obj = QJsonDocument::fromJson(iconFile.readAll()).object();
        for (auto it = obj.begin(); it != obj.end(); ++it)
            m_customIcons[it.key()] = it.value().toString();
        iconFile.close();
    }

    QFile settingsFile(dir + "/settings.json");
    if (settingsFile.open(QIODevice::ReadOnly)) {
        QJsonObject obj = QJsonDocument::fromJson(settingsFile.readAll()).object();
        m_backgroundMode = obj["background_mode"].toString("wave");
        if (obj.contains("solid_color")) {
            m_solidColor = QColor(obj["solid_color"].toString());
        }
        m_colorIntensity = obj["color_intensity"].toInt(80);
        m_imageIntensity = obj["image_intensity"].toInt(80);   // <-- أضف هذا
        m_backgroundImagePath = obj["background_image"].toString();
        m_showAdvancedCategories = obj["show_advanced"].toBool(false);
        m_showHiddenToggle = obj["show_hidden_toggle"].toBool(false);
        settingsFile.close();
    }
}

void StartLensSettings::save() {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens";
    QDir().mkpath(dir);

    QJsonArray hiddenArr;
    for (const auto &app : m_hiddenApps) hiddenArr.append(app);
    QFile hiddenFile(dir + "/hidden.json");
    if (hiddenFile.open(QIODevice::WriteOnly)) {
        hiddenFile.write(QJsonDocument(hiddenArr).toJson());
        hiddenFile.close();
    }

    QJsonObject catObj;
    for (auto it = m_customCategories.begin(); it != m_customCategories.end(); ++it)
        catObj[it.key()] = it.value();
    QFile catFile(dir + "/custom_categories.json");
    if (catFile.open(QIODevice::WriteOnly)) {
        catFile.write(QJsonDocument(catObj).toJson());
        catFile.close();
    }

    QJsonObject iconObj;
    for (auto it = m_customIcons.begin(); it != m_customIcons.end(); ++it)
        iconObj[it.key()] = it.value();
    QFile iconFile(dir + "/custom_icons.json");
    if (iconFile.open(QIODevice::WriteOnly)) {
        iconFile.write(QJsonDocument(iconObj).toJson());
        iconFile.close();
    }

    QJsonObject settingsObj;
    settingsObj["background_mode"] = m_backgroundMode;
    settingsObj["solid_color"] = m_solidColor.name();
    settingsObj["color_intensity"] = m_colorIntensity;
    settingsObj["image_intensity"] = m_imageIntensity;  // <-- أضف هذا
    settingsObj["background_image"] = m_backgroundImagePath;
    settingsObj["show_advanced"] = m_showAdvancedCategories;
    settingsObj["show_hidden_toggle"] = m_showHiddenToggle;

    QFile settingsFile(dir + "/settings.json");
    if (settingsFile.open(QIODevice::WriteOnly)) {
        settingsFile.write(QJsonDocument(settingsObj).toJson());
        settingsFile.close();
    }
}
