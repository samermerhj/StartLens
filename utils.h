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
#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QSettings>

inline QString categoryForApp(const QString &iconPath, const QString &appName = QString()) {
    QSettings desktop(iconPath, QSettings::IniFormat);
    desktop.beginGroup("Desktop Entry");
    QStringList cats = desktop.value("Categories").toString().split(';', Qt::SkipEmptyParts);
    desktop.endGroup();

    QString lowerName = appName.toLower();
    if (lowerName.contains("firefox") || lowerName.contains("chrome") || lowerName.contains("brave") ||
        lowerName.contains("edge") || lowerName.contains("opera") || lowerName.contains("vivaldi"))
        return "Internet";
    if (lowerName.contains("game") || lowerName.contains("steam")) return "Games";
    if (lowerName.contains("settings") || lowerName.contains("preferences") || lowerName.contains("config"))
        return "Settings";
    if (lowerName.contains("terminal") || lowerName.contains("console")) return "System Tools";
    if (lowerName.contains("vlc") || lowerName.contains("media") || lowerName.contains("player") ||
        lowerName.contains("audio") || lowerName.contains("video")) return "Multimedia";

    if (cats.contains("Network") || cats.contains("WebBrowser") || cats.contains("Email")) return "Internet";
    if (cats.contains("Game")) return "Games";
    if (cats.contains("Graphics") || cats.contains("Photography")) return "Graphics";
    if (cats.contains("Audio") || cats.contains("Video") || cats.contains("AudioVideo") || cats.contains("Player"))
        return "Multimedia";
    if (cats.contains("Office") || cats.contains("WordProcessor") || cats.contains("Presentation")) return "Office";
    if (cats.contains("System") || cats.contains("Utility") || cats.contains("TerminalEmulator")) return "System Tools";
    if (cats.contains("Development") || cats.contains("IDE") || cats.contains("Editor")) return "Development";
    if (cats.contains("Education") || cats.contains("Science")) return "Education";
    if (cats.contains("Settings") || cats.contains("DesktopSettings") || cats.contains("HardwareSettings"))
        return "Settings";
    if (cats.contains("Utility")) return "Utilities";

    return "Other";
}

#endif // UTILS_H
