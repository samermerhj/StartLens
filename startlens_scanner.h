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
#ifndef STARTLENS_SCANNER_H
#define STARTLENS_SCANNER_H

#include <QObject>
#include <QList>
#include "applistmodel.h"

class StartLensScanner : public QObject
{
    Q_OBJECT
public:
    explicit StartLensScanner(QObject *parent = nullptr);

    // الوظيفة الرئيسية
    QList<SystemItem> loadAppsAndGroups();

    // إعادة تعيين ذاكرة التخزين المؤقت
    void clearCache();

    // دوال عامة للاستخدام الخارجي (مثل الإضافة والإزالة)
    static QString appsFilePath();
    void syncFromSharedMap();
    void saveToAppsFile(const QList<SystemItem> &items);

signals:
    void progress(int current, int total);
    void error(const QString &message);

private:
    // طبقات القراءة المختلفة (مرتبة حسب الأولوية)
    QList<SystemItem> loadFromLxqtMenuCache();   // الطبقة الأولى
    QList<SystemItem> loadFromXdgMenu();         // الطبقة الثانية
    QList<SystemItem> loadFromDesktopFiles();    // الطبقة الثالثة (يدوي)

    // أدوات مساعدة
    QList<SystemItem> deduplicateApps(const QList<SystemItem> &apps);
    bool isAppExecutable(const SystemItem &app);
    QStringList getDesktopDirs();

    // التخزين المؤقت (Cache)
    QString cacheFilePath();
    void saveToCache(const QList<SystemItem> &items);
    QList<SystemItem> loadFromCache();

    // الطرق القديمة (احتفظ بها للتوافق)
    QList<SystemItem> scanWithGtkLaunch();
    QList<SystemItem> loadFromAppsFile();

    // متغيرات التخزين المؤقت
    QList<SystemItem> m_cachedApps;
    bool m_cacheValid = false;
};

#endif // STARTLENS_SCANNER_H
