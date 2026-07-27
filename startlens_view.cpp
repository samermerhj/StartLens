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
#include "startlens_view.h"
#include "utils.h"
#include "startlens_settings.h"
#include <algorithm>
#include <QSet>

StartLensView::StartLensView(QObject *parent) : QObject(parent) {}

void StartLensView::buildDisplayList(QList<SystemItem> &allApps,
                                     QList<SystemItem> &displayItems,
                                     const QString &currentCategory,
                                     StartLensUsage *usage,
                                     StartLensSettings *settings)
{
    displayItems.clear();

    // جمع أسماء التطبيقات المجمعة
    QSet<QString> groupedApps;
    QList<SystemItem> groups;
    for (const auto &app : allApps) {
        if (app.referenceVersion == "group") {
            groups.append(app);
            for (const auto &name : app.files) {
                groupedApps.insert(name);
            }
        }
    }

    // ===== حالة خاصة: Pinned & Groups =====
    if (currentCategory == "Pinned & Groups") {
        const auto pinned = usage->pinnedApps();
        if (!pinned.isEmpty()) {
            SystemItem sec; sec.referenceVersion = "section"; sec.name = "📌 Pinned"; displayItems.append(sec);
            for (const auto &name : pinned) {
                for (const auto &app : allApps) {
                    if (app.name == name && app.referenceVersion != "group") {
                        displayItems.append(app);
                        break;
                    }
                }
            }
            SystemItem sp; sp.referenceVersion = "spacer"; displayItems.append(sp);
        }

        if (!groups.isEmpty()) {
            SystemItem sec; sec.referenceVersion = "section"; sec.name = "📦 Groups"; displayItems.append(sec);
            for (const auto &group : groups) {
                SystemItem groupCopy = group;
                if (!groupCopy.status.isEmpty()) {
                    for (const auto &realApp : allApps) {
                        if (realApp.name == groupCopy.status && realApp.referenceVersion != "group") {
                            groupCopy.iconPath = realApp.iconPath;
                            break;
                        }
                    }
                }
                displayItems.append(groupCopy);
            }
            SystemItem sp; sp.referenceVersion = "spacer"; displayItems.append(sp);
        }
        return;
    }

    // ===== حالة خاصة: System =====
    if (currentCategory == "System") {
        QStringList targetCats = {"System Tools", "Settings"};
        for (const auto &cat : targetCats) {
            bool hasItems = false;
            for (const auto &app : allApps) {
                if (app.referenceVersion == "group") continue;
                if (groupedApps.contains(app.name)) continue;
                if (usage->isPinned(app.name)) continue;

                QString appCategory = settings->customCategory(app.name);
                if (appCategory.isEmpty()) {
                    appCategory = categoryForApp(app.iconPath, app.name);
                }
                if (appCategory == cat) {
                    hasItems = true;
                    break;
                }
            }

            if (!hasItems) continue;

            SystemItem sec; sec.referenceVersion = "section"; sec.name = cat; displayItems.append(sec);
            for (const auto &app : allApps) {
                if (app.referenceVersion == "group") continue;
                if (groupedApps.contains(app.name)) continue;
                if (usage->isPinned(app.name)) continue;

                QString appCategory = settings->customCategory(app.name);
                if (appCategory.isEmpty()) {
                    appCategory = categoryForApp(app.iconPath, app.name);
                }

                if (appCategory == cat) {
                    displayItems.append(app);
                }
            }
            SystemItem sp; sp.referenceVersion = "spacer"; displayItems.append(sp);
        }
        return;
    }

    // ----- 1. Pinned -----
    const auto pinned = usage->pinnedApps();
    if (!pinned.isEmpty()) {
        SystemItem sec; sec.referenceVersion = "section"; sec.name = "📌 Pinned"; displayItems.append(sec);
        for (const auto &name : pinned) {
            for (const auto &app : allApps) {
                if (app.name == name && app.referenceVersion != "group") {
                    displayItems.append(app);
                    break;
                }
            }
        }
        SystemItem sp; sp.referenceVersion = "spacer"; displayItems.append(sp);
    }

    // ----- 2. Most Used -----
    const auto usageCount = usage->usageCount();
    QList<SystemItem> mostUsed;
    for (const auto &app : allApps) {
        if (app.referenceVersion != "group" &&
            !groupedApps.contains(app.name) &&
            usageCount.contains(app.name) && usageCount[app.name] > 0 &&
            !usage->isPinned(app.name)) {
            mostUsed.append(app);
        }
    }
    std::sort(mostUsed.begin(), mostUsed.end(), [&](const SystemItem &a, const SystemItem &b) {
        return usageCount[a.name] > usageCount[b.name];
    });

    // إزالة التكرار الاحتياطية
    QSet<QString> uniqueNames;
    QList<SystemItem> uniqueMostUsed;
    for (const auto &app : mostUsed) {
        if (!uniqueNames.contains(app.name)) {
            uniqueNames.insert(app.name);
            uniqueMostUsed.append(app);
        }
    }
    mostUsed = uniqueMostUsed;

    if (!mostUsed.isEmpty()) {
        SystemItem sec; sec.referenceVersion = "section"; sec.name = "★ Most Used"; displayItems.append(sec);
        for (int i = 0; i < qMin(6, mostUsed.size()); ++i) {
            displayItems.append(mostUsed[i]);
        }
        SystemItem sp; sp.referenceVersion = "spacer"; displayItems.append(sp);
    }

    // ----- 3. Recent -----
    const auto lastUsed = usage->lastUsed();
    QList<SystemItem> recent;
    for (const auto &app : allApps) {
        if (app.referenceVersion != "group" &&
            !groupedApps.contains(app.name) &&
            lastUsed.contains(app.name) &&
            !usage->isPinned(app.name)) {
            bool alreadyInMost = false;
            for (int i = 0; i < qMin(6, mostUsed.size()); ++i) {
                if (mostUsed[i].name == app.name) {
                    alreadyInMost = true;
                    break;
                }
            }
            if (!alreadyInMost) {
                recent.append(app);
            }
        }
    }
    std::sort(recent.begin(), recent.end(), [&](const SystemItem &a, const SystemItem &b) {
        return lastUsed[a.name] > lastUsed[b.name];
    });
    if (!recent.isEmpty()) {
        SystemItem sec; sec.referenceVersion = "section"; sec.name = "🕐 Recent"; displayItems.append(sec);
        for (int i = 0; i < qMin(3, recent.size()); ++i) {
            displayItems.append(recent[i]);
        }
        SystemItem sp; sp.referenceVersion = "spacer"; displayItems.append(sp);
    }

    // ----- 4. المجموعات -----
    if (!groups.isEmpty()) {
        SystemItem sec; sec.referenceVersion = "section"; sec.name = "📦 Groups"; displayItems.append(sec);
        for (const auto &group : groups) {
            SystemItem groupCopy = group;
            if (!groupCopy.status.isEmpty()) {
                for (const auto &realApp : allApps) {
                    if (realApp.name == groupCopy.status && realApp.referenceVersion != "group") {
                        groupCopy.iconPath = realApp.iconPath;
                        break;
                    }
                }
            }
            displayItems.append(groupCopy);
        }
        SystemItem sp; sp.referenceVersion = "spacer"; displayItems.append(sp);
    }

    // ----- 5. التصنيفات -----
    QStringList primaryCats = {"Internet", "Games", "Multimedia", "Office", "System Tools"};
    QStringList secondaryCats = {"Graphics", "Development", "Education", "Utilities", "Settings"};

    QStringList cats;
    if (currentCategory == "All") {
        cats = primaryCats;
        if (settings->showAdvancedCategories()) {
            cats << secondaryCats;
        }
    } else {
        cats << currentCategory;
    }

    for (const auto &cat : cats) {
        bool hasItems = false;
        for (const auto &app : allApps) {
            if (app.referenceVersion == "group") continue;
            if (groupedApps.contains(app.name)) continue;
            if (usage->isPinned(app.name)) continue;

            QString appCategory = settings->customCategory(app.name);
            if (appCategory.isEmpty()) {
                appCategory = categoryForApp(app.iconPath, app.name);
            }
            if (appCategory == cat) {
                hasItems = true;
                break;
            }
        }

        if (!hasItems) continue;

        SystemItem sec; sec.referenceVersion = "section"; sec.name = cat; displayItems.append(sec);
        for (const auto &app : allApps) {
            if (app.referenceVersion == "group") continue;
            if (groupedApps.contains(app.name)) continue;
            if (usage->isPinned(app.name)) continue;

            QString appCategory = settings->customCategory(app.name);
            if (appCategory.isEmpty()) {
                appCategory = categoryForApp(app.iconPath, app.name);
            }

            if (appCategory == cat) {
                displayItems.append(app);
            }
        }
        SystemItem sp; sp.referenceVersion = "spacer"; displayItems.append(sp);
    }
}
