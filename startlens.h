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
#ifndef STARTLENS_H
#define STARTLENS_H

#include <QDialog>
#include <QToolButton>
#include <QPushButton>
#include <QLineEdit>
#include <QButtonGroup>
#include <QProcess>
#include <QPropertyAnimation>
#include <QMenu>
#include <QMessageBox>
#include <QResizeEvent>
#include <QColor>
#include <QTimer>
#include <QScrollArea>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QFrame>
#include <QtMath>
#include <QImage>
#include "applistmodel.h"
#include "startlens_scanner.h"
#include "startlens_usage.h"
#include "startlens_view.h"
#include "qhotkey.h"
#include "startlens_settings.h"
#include "startlens_settingsdialog.h"
#include "shutdowndialog.h"

class StartLens : public QDialog
{
    Q_OBJECT
public:
    explicit StartLens(QWidget *parent = nullptr);
    void showWindow();
    void hideWindow();
    void applySettings();
    void toggleWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    bool event(QEvent *event) override;  // [تعديل] إضافة معالج الأحداث

private slots:
    
    void refreshDisplayList();
    void onSearchTextChanged(const QString &text);
    void animateBackground();
    void onBackgroundChanged();
    void onSettingsApplied();
    void onSettingsClosed();  // [تعديل] عند إغلاق الإعدادات

    void showSystemCategory();
    void showPinnedAndGroups();
    void showShutdownDialog();

private:
    void setupUI();
    void applyCategoryColor(const QString &colorName);
    void runItem(const SystemItem &item);
    void onCustomContextMenu(const QPoint &globalPos, const SystemItem &item);
    QIcon createIconForApp(const QString &appName, const QString &customIconText = QString());
    void addCustomApp();
    void shutdownSystem();

    QList<SystemItem> m_allApps;
    QScrollArea *m_scrollArea;
    QWidget *m_scrollContent;
    QGridLayout *m_gridLayout;
    QLineEdit *m_searchEdit;
    QButtonGroup *m_categoryGroup;
    QString m_currentCategory = "All";
    QHotkey *m_hotkey;

    // [تعديل] إضافة مؤشرات الرسوم المتحركة للحركة
    QPropertyAnimation *m_fadeInAnim;
    QPropertyAnimation *m_fadeOutAnim;
    QPropertyAnimation *m_slideInAnim;   // للصعود
    QPropertyAnimation *m_slideOutAnim;  // للهبوط

    StartLensScanner *m_scanner;
    StartLensUsage *m_usage;
    StartLensView *m_view;
    StartLensSettings *m_settings;
    StartLensSettingsDialog *m_settingsDialog;
    ShutdownDialog *m_shutdownDialog;

    QPoint m_originalPos;   // الموضع الطبيعي (وسط الشاشة)
    QPoint m_hiddenPos;     // [تعديل] الموضع المخفي (أسفل الشاشة)
    bool m_keepOpen;        // [تعديل] منع الإخفاء أثناء فتح الإعدادات

    QColor m_waveColor = QColor("#00C9FF");
    QColor m_borderColor = QColor("#00C9FF");
    QTimer *m_bgTimer;
    float m_wavePhase = 0.0f;

    bool m_showHidden = false;
    QImage m_backgroundImage;
    bool m_hasBackgroundImage = false;

    QPushButton *m_settingsBtn = nullptr;
    QPushButton *m_shutdownBtn = nullptr;
};

#endif // STARTLENS_H
