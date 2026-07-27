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
#ifndef STARTLENS_SETTINGSDIALOG_H
#define STARTLENS_SETTINGSDIALOG_H

#include <QDialog>
#include <QTabWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>
#include <QColor>
#include <QSlider>
#include "startlens_settings.h"
#include "startlens_usage.h"

class StartLensSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit StartLensSettingsDialog(StartLensSettings *settings, StartLensUsage *usage, QWidget *parent = nullptr);
    void updateColorPreview(const QColor &color);

signals:
    void settingsApplied();
    void addAppRequested();

private slots:
    void onApply();
    void onClose();
    void onReset();
    void onIntensityChanged(int value);
    void onImageIntensityChanged(int value);
    void onChooseImage();
    void onRemoveImage();
    void onHiddenItemClicked(QListWidgetItem *item);
    void onAddApp();
    void onBackgroundModeChanged();
    void onInstalledItemClicked(QListWidgetItem *item);  // نقر مزدوج لتشغيل التطبيق

    // ===== دوال جديدة لقوائم السياق =====
    void showInstalledContextMenu(const QPoint &pos);   // قائمة سياق التطبيقات المثبتة
    void showHiddenContextMenu(const QPoint &pos);      // قائمة سياق التطبيقات المخفية
    // ==================================

private:
    void loadSettings();
    void saveSettings();
    void updateIntensityPreview();
    void populateInstalledApps();  // تعبئة قائمة التطبيقات المثبتة مع الأيقونات

    StartLensSettings *m_settings;
    StartLensUsage *m_usage;

    QTabWidget *m_tabWidget;

    // Appearance
    QGroupBox *m_bgGroup;
    QRadioButton *m_waveRadio;
    QRadioButton *m_solidRadio;
    QRadioButton *m_imageRadio;
    QSlider *m_intensitySlider;
    QLabel *m_intensityPreview;
    QPushButton *m_imageBrowseBtn;
    QPushButton *m_imageRemoveBtn;
    QLabel *m_imagePreview;
    int m_intensityValue;
    QSlider *m_imageIntensitySlider;
    QLabel *m_imageIntensityPreview;
    int m_imageIntensityValue;

    // Behavior
    QCheckBox *m_showAdvancedCheck;
    QCheckBox *m_showHiddenCheck;

    // Hidden Apps
    QListWidget *m_hiddenList;

    // Installed Apps
    QListWidget *m_installedList;

    // Buttons
    QPushButton *m_applyBtn;
    QPushButton *m_closeBtn;
    QPushButton *m_resetBtn;
    QPushButton *m_addAppBtn;
};

#endif // STARTLENS_SETTINGSDIALOG_H
