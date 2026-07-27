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
#ifndef STARTLENS_SETTINGS_H
#define STARTLENS_SETTINGS_H

#include <QObject>
#include <QSet>
#include <QMap>
#include <QString>
#include <QColor>

class StartLensSettings : public QObject
{
    Q_OBJECT
public:
    explicit StartLensSettings(QObject *parent = nullptr);

    // Hidden apps
    QSet<QString> hiddenApps() const;
    void hideApp(const QString &appName);
    void unhideApp(const QString &appName);
    bool isHidden(const QString &appName) const;

    // Custom categories
    QMap<QString, QString> customCategories() const;
    void setCustomCategory(const QString &appName, const QString &category);
    QString customCategory(const QString &appName) const;

    // Custom icons
    QMap<QString, QString> customIcons() const;
    void setCustomIcon(const QString &appName, const QString &iconText);
    QString customIcon(const QString &appName) const;
    void load();

    // Background settings
    QString backgroundMode() const;
    void setBackgroundMode(const QString &mode);

    QColor solidColor() const;
    void setSolidColor(const QColor &color);

    int colorIntensity() const;
    void setColorIntensity(int intensity);

    // ===== إضافة دوال شدة الصورة =====
    int imageIntensity() const;
    void setImageIntensity(int intensity);
    // ================================

    QString backgroundImagePath() const;
    void setBackgroundImage(const QString &path);

    bool showAdvancedCategories() const;
    void setShowAdvancedCategories(bool show);

    bool showHiddenToggle() const;
    void setShowHiddenToggle(bool show);

signals:
    void iconChanged(const QString &appName, const QString &newIcon);
    void backgroundChanged();

private:
    void save();

    QSet<QString> m_hiddenApps;
    QMap<QString, QString> m_customCategories;
    QMap<QString, QString> m_customIcons;

    QString m_backgroundMode = "wave";
    QColor m_solidColor = QColor("#00C9FF");
    int m_colorIntensity = 80;
    int m_imageIntensity = 80;  // <-- أضف هذا المتغير
    QString m_backgroundImagePath;
    bool m_showAdvancedCategories = false;
    bool m_showHiddenToggle = false;
};

#endif // STARTLENS_SETTINGS_H
