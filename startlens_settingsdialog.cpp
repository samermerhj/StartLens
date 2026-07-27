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
#include "startlens_settingsdialog.h"
#include <QSettings>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QPainter>
#include <QPushButton>
#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QCheckBox>
#include <QListWidget>
#include <QTabWidget>
#include <QScrollArea>
#include <QApplication>
#include <QSlider>
#include <QPainter>
#include <QProcess>
#include <QMenu>
#include <QAction>
#include "applistmodel.h"
#include "startlens_scanner.h"

StartLensSettingsDialog::StartLensSettingsDialog(StartLensSettings *settings, StartLensUsage *usage, QWidget *parent)
    : QDialog(parent), m_settings(settings), m_usage(usage), m_intensityValue(80)
{
    setWindowTitle("⚙️ StartLens Settings");
    setMinimumSize(600, 550);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_tabWidget = new QTabWidget(this);

    // ---------- Appearance ----------
    QWidget *appearanceTab = new QWidget();
    QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceTab);

    m_bgGroup = new QGroupBox("Background Style");
    QVBoxLayout *bgLayout = new QVBoxLayout(m_bgGroup);

    m_waveRadio = new QRadioButton("🌊 Dynamic Waves (Animated)");
    m_solidRadio = new QRadioButton("🎨 Solid Color");
    m_imageRadio = new QRadioButton("🖼 Image (Static)");

    bgLayout->addWidget(m_waveRadio);
    bgLayout->addWidget(m_solidRadio);
    bgLayout->addWidget(m_imageRadio);

    // شدة اللون
    QHBoxLayout *intensityLayout = new QHBoxLayout();
    intensityLayout->addSpacing(20);
    intensityLayout->addWidget(new QLabel("Color Intensity:"));
    m_intensitySlider = new QSlider(Qt::Horizontal);
    m_intensitySlider->setRange(10, 100);
    m_intensitySlider->setValue(80);
    m_intensitySlider->setFixedWidth(200);
    m_intensitySlider->setTickPosition(QSlider::TicksBelow);
    m_intensitySlider->setTickInterval(10);
    m_intensityPreview = new QLabel();
    m_intensityPreview->setFixedSize(40, 30);
    m_intensityPreview->setStyleSheet("border: 1px solid #ccc; border-radius: 4px; background-color: #00C9FF;");
    intensityLayout->addWidget(m_intensitySlider);
    intensityLayout->addWidget(m_intensityPreview);
    intensityLayout->addStretch();
    bgLayout->addLayout(intensityLayout);

    // صورة الخلفية
    QHBoxLayout *imageLayout = new QHBoxLayout();
    imageLayout->addSpacing(20);
    m_imageBrowseBtn = new QPushButton("Browse Image...");
    m_imageRemoveBtn = new QPushButton("Remove Image");
    m_imagePreview = new QLabel();
    m_imagePreview->setFixedSize(120, 80);
    m_imagePreview->setStyleSheet("border: 1px solid #ccc; border-radius: 4px; background-color: #f0f0f0;");
    m_imagePreview->setScaledContents(true);
    m_imagePreview->setAlignment(Qt::AlignCenter);
    imageLayout->addWidget(m_imageBrowseBtn);
    imageLayout->addWidget(m_imageRemoveBtn);
    imageLayout->addWidget(m_imagePreview);
    imageLayout->addStretch();
    bgLayout->addLayout(imageLayout);

    // شدة الصورة
    QHBoxLayout *imageIntensityLayout = new QHBoxLayout();
    imageIntensityLayout->addSpacing(20);
    imageIntensityLayout->addWidget(new QLabel("Image Intensity:"));
    m_imageIntensitySlider = new QSlider(Qt::Horizontal);
    m_imageIntensitySlider->setRange(10, 100);
    m_imageIntensitySlider->setValue(80);
    m_imageIntensitySlider->setFixedWidth(200);
    m_imageIntensitySlider->setTickPosition(QSlider::TicksBelow);
    m_imageIntensitySlider->setTickInterval(10);
    m_imageIntensityPreview = new QLabel();
    m_imageIntensityPreview->setFixedSize(40, 30);
    m_imageIntensityPreview->setStyleSheet("border: 1px solid #ccc; border-radius: 4px; background-color: rgba(0, 201, 255, 0.8);");
    imageIntensityLayout->addWidget(m_imageIntensitySlider);
    imageIntensityLayout->addWidget(m_imageIntensityPreview);
    imageIntensityLayout->addStretch();
    bgLayout->addLayout(imageIntensityLayout);

    QLabel *infoLabel = new QLabel("💡 Intensity controls how strongly the category color appears in the background.");
    infoLabel->setStyleSheet("color: #6c757d; font-size: 10px; margin-top: 5px;");
    bgLayout->addWidget(infoLabel);

    m_bgGroup->setLayout(bgLayout);
    appearanceLayout->addWidget(m_bgGroup);
    appearanceLayout->addStretch();
    appearanceTab->setLayout(appearanceLayout);
    m_tabWidget->addTab(appearanceTab, "Appearance");

    // ---------- Behavior ----------
    QWidget *behaviorTab = new QWidget();
    QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorTab);

    m_showAdvancedCheck = new QCheckBox("Show advanced categories (Graphics, Development, Education, Utilities)");
    m_showHiddenCheck = new QCheckBox("Show hidden apps in main view (toggle)");

    behaviorLayout->addWidget(m_showAdvancedCheck);
    behaviorLayout->addWidget(m_showHiddenCheck);
    behaviorLayout->addStretch();
    behaviorTab->setLayout(behaviorLayout);
    m_tabWidget->addTab(behaviorTab, "Behavior");

    // ---------- Hidden Apps (معدل) ----------
    QWidget *hiddenTab = new QWidget();
    QVBoxLayout *hiddenLayout = new QVBoxLayout(hiddenTab);

    hiddenLayout->addWidget(new QLabel("Hidden applications (right-click for options):"));
    m_hiddenList = new QListWidget();
    m_hiddenList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_hiddenList->setContextMenuPolicy(Qt::CustomContextMenu);
    hiddenLayout->addWidget(m_hiddenList);

    QPushButton *unhideAllBtn = new QPushButton("👁 Unhide All");
    connect(unhideAllBtn, &QPushButton::clicked, [this]() {
        for (const QString &name : m_settings->hiddenApps()) {
            m_settings->unhideApp(name);
        }
        loadSettings();
        emit settingsApplied(); // تحديث الواجهة الرئيسية
    });
    hiddenLayout->addWidget(unhideAllBtn);
    hiddenTab->setLayout(hiddenLayout);
    m_tabWidget->addTab(hiddenTab, "Hidden Apps");

    // ---------- Installed Apps (معدل) ----------
    QWidget *installedTab = new QWidget();
    QVBoxLayout *installedLayout = new QVBoxLayout(installedTab);

    QLabel *installedLabel = new QLabel("All installed applications (right-click for options):");
    installedLayout->addWidget(installedLabel);

    m_installedList = new QListWidget();
    m_installedList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_installedList->setContextMenuPolicy(Qt::CustomContextMenu);
    installedLayout->addWidget(m_installedList);

    QPushButton *refreshBtn = new QPushButton("🔄 Refresh List");
    connect(refreshBtn, &QPushButton::clicked, [this]() {
        populateInstalledApps();
    });
    installedLayout->addWidget(refreshBtn);

    installedTab->setLayout(installedLayout);
    m_tabWidget->addTab(installedTab, "Installed Apps");

    // ---------- أزرار ----------
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    m_addAppBtn = new QPushButton("➕ Add Application");
    m_resetBtn = new QPushButton("🔄 Reset All");
    m_applyBtn = new QPushButton("Apply");
    m_applyBtn->setStyleSheet("QPushButton { background-color: #28a745; color: white; font-weight: bold; padding: 6px 20px; border-radius: 4px; }");
    m_closeBtn = new QPushButton("Close");

    buttonLayout->addWidget(m_addAppBtn);
    buttonLayout->addWidget(m_resetBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_applyBtn);
    buttonLayout->addWidget(m_closeBtn);

    mainLayout->addWidget(m_tabWidget);
    mainLayout->addLayout(buttonLayout);

    // ---------- الاتصالات ----------
    connect(m_waveRadio, &QRadioButton::toggled, this, &StartLensSettingsDialog::onBackgroundModeChanged);
    connect(m_solidRadio, &QRadioButton::toggled, this, &StartLensSettingsDialog::onBackgroundModeChanged);
    connect(m_imageRadio, &QRadioButton::toggled, this, &StartLensSettingsDialog::onBackgroundModeChanged);

    connect(m_intensitySlider, &QSlider::valueChanged, this, &StartLensSettingsDialog::onIntensityChanged);
    connect(m_imageIntensitySlider, &QSlider::valueChanged, this, &StartLensSettingsDialog::onImageIntensityChanged);
    connect(m_imageBrowseBtn, &QPushButton::clicked, this, &StartLensSettingsDialog::onChooseImage);
    connect(m_imageRemoveBtn, &QPushButton::clicked, this, &StartLensSettingsDialog::onRemoveImage);
    connect(m_resetBtn, &QPushButton::clicked, this, &StartLensSettingsDialog::onReset);
    connect(m_applyBtn, &QPushButton::clicked, this, &StartLensSettingsDialog::onApply);
    connect(m_closeBtn, &QPushButton::clicked, this, &StartLensSettingsDialog::onClose);
    connect(m_addAppBtn, &QPushButton::clicked, this, &StartLensSettingsDialog::onAddApp);

    // قوائم السياق
    connect(m_installedList, &QListWidget::customContextMenuRequested, this, &StartLensSettingsDialog::showInstalledContextMenu);
    connect(m_hiddenList, &QListWidget::customContextMenuRequested, this, &StartLensSettingsDialog::showHiddenContextMenu);

    // النقر المزدوج على Installed -> تشغيل (احتفظنا بالوظيفة السابقة)
    connect(m_installedList, &QListWidget::itemDoubleClicked, this, &StartLensSettingsDialog::onInstalledItemClicked);

    // النقر على Hidden -> إظهار (نحتفظ بها كسلوك سريع، لكننا نضيف القائمة أيضاً)
    connect(m_hiddenList, &QListWidget::itemClicked, this, &StartLensSettingsDialog::onHiddenItemClicked);

    loadSettings();
}

// ========== دوال التعبئة ==========

void StartLensSettingsDialog::populateInstalledApps()
{
    m_installedList->clear();

    StartLensScanner scanner;
    QList<SystemItem> apps = scanner.loadAppsAndGroups();

    for (const auto &app : apps) {
        if (app.referenceVersion == "group" || app.referenceVersion == "section" || app.referenceVersion == "spacer")
            continue;

        QString displayName = app.name;
        if (app.package.isEmpty()) {
            displayName += " (custom)";
        }

        QListWidgetItem *item = new QListWidgetItem(displayName);
        // تخزين البيانات: name, package, iconPath
        item->setData(Qt::UserRole, app.name);
        item->setData(Qt::UserRole + 1, app.package);
        item->setData(Qt::UserRole + 2, app.iconPath);

        // محاولة تحميل الأيقونة
        QIcon icon;
        if (!app.iconPath.isEmpty()) {
            if (app.iconPath.endsWith(".desktop")) {
                QSettings desktop(app.iconPath, QSettings::IniFormat);
                desktop.beginGroup("Desktop Entry");
                QString iconName = desktop.value("Icon").toString();
                desktop.endGroup();
                icon = QIcon::fromTheme(iconName);
            } else if (app.iconPath.endsWith(".png") || app.iconPath.endsWith(".svg")) {
                icon = QIcon(app.iconPath);
            }
        }
        if (icon.isNull()) {
            // محاولة من اسم الحزمة
            QString pkg = app.package;
            if (pkg.endsWith(".desktop")) pkg.chop(8);
            icon = QIcon::fromTheme(pkg);
        }
        if (icon.isNull()) {
            icon = QIcon::fromTheme("application-x-executable");
        }
        item->setIcon(icon);

        m_installedList->addItem(item);
    }

    if (m_installedList->count() == 0) {
        QListWidgetItem *empty = new QListWidgetItem("(No applications found)");
        empty->setFlags(Qt::NoItemFlags);
        m_installedList->addItem(empty);
    }
}

// ========== قائمة السياق لـ Installed Apps ==========

void StartLensSettingsDialog::showInstalledContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_installedList->itemAt(pos);
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;

    QString appName = item->data(Qt::UserRole).toString();
    if (appName.isEmpty()) return;

    QMenu menu;

    // Run
    menu.addAction("▶ Run", [this, appName]() {
        QProcess::startDetached("gtk-launch", {appName});
    });

    // Pin / Unpin
    if (m_usage->isPinned(appName)) {
        menu.addAction("📌 Unpin", [this, appName]() {
            m_usage->unpinApp(appName);
            emit settingsApplied(); // تحديث StartLens
        });
    } else {
        menu.addAction("📌 Pin", [this, appName]() {
            m_usage->pinApp(appName);
            emit settingsApplied();
        });
    }

    // Hide / Unhide
    if (m_settings->isHidden(appName)) {
        menu.addAction("👁 Unhide", [this, appName]() {
            m_settings->unhideApp(appName);
            loadSettings(); // تحديث Hidden tab و Installed tab (لأننا سنعيد populate)
            emit settingsApplied();
        });
    } else {
        menu.addAction("👁 Hide", [this, appName]() {
            m_settings->hideApp(appName);
            loadSettings();
            emit settingsApplied();
        });
    }

    // Move to Category
    QMenu *moveMenu = menu.addMenu("📂 Move to Category");
    QStringList cats = {"Internet", "Games", "Multimedia", "System Tools", "Office", "Settings", "Other"};
    for (const QString &cat : cats) {
        moveMenu->addAction(cat, [this, appName, cat]() {
            m_settings->setCustomCategory(appName, cat);
            emit settingsApplied();
        });
    }
    moveMenu->addAction("↺ Auto", [this, appName]() {
        m_settings->setCustomCategory(appName, QString());
        emit settingsApplied();
    });

    menu.exec(m_installedList->mapToGlobal(pos));
}

// ========== قائمة السياق لـ Hidden Apps ==========

void StartLensSettingsDialog::showHiddenContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_hiddenList->itemAt(pos);
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;

    QString appName = item->text();
    if (appName.isEmpty() || appName == "(No hidden apps)") return;

    QMenu menu;
    menu.addAction("👁 Unhide", [this, appName]() {
        m_settings->unhideApp(appName);
        loadSettings();
        emit settingsApplied();
    });
    // يمكن إضافة خيارات أخرى مثل "Pin" أو "Move to Category" هنا إذا أردت
    // ولكن حسب الطلب، فقط "Unhide" يكفي.
    menu.exec(m_hiddenList->mapToGlobal(pos));
}

// ========== بقية الدوال (مع تعديلات بسيطة) ==========

void StartLensSettingsDialog::onInstalledItemClicked(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    QString appName = item->data(Qt::UserRole).toString();
    if (appName.isEmpty()) return;
    QProcess::startDetached("gtk-launch", {appName});
}

void StartLensSettingsDialog::onHiddenItemClicked(QListWidgetItem *item)
{
    // النقر السريع: إظهار التطبيق (نفس السلوك السابق)
    if (!item || !(item->flags() & Qt::ItemIsSelectable)) return;
    QString name = item->text();
    if (name.isEmpty() || name == "(No hidden apps)") return;
    m_settings->unhideApp(name);
    loadSettings();
    emit settingsApplied();
}

// باقي الدوال (updateColorPreview, updateIntensityPreview, onIntensityChanged,
// onImageIntensityChanged, onBackgroundModeChanged, loadSettings, saveSettings,
// onApply, onClose, onReset, onChooseImage, onRemoveImage, onAddApp) تبقى كما هي
// مع إضافة emit settingsApplied() في الأماكن المناسبة لتحديث StartLens عند التغيير.

// ===== الدوال المذكورة أعلاه تبقى دون تغيير، لكن سأضمنها هنا للاكتمال =====

void StartLensSettingsDialog::updateColorPreview(const QColor &color)
{
    int intensity = m_intensitySlider->value();
    QColor previewColor = color;
    previewColor.setAlpha(intensity * 255 / 100);
    m_intensityPreview->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 4px;")
                                      .arg(previewColor.name(QColor::HexArgb)));
}

void StartLensSettingsDialog::updateIntensityPreview()
{
    QColor baseColor = m_settings->solidColor();
    if (!baseColor.isValid()) {
        baseColor = QColor("#00C9FF");
    }
    int intensity = m_intensitySlider->value();
    QColor previewColor = baseColor;
    previewColor.setAlpha(intensity * 255 / 100);
    m_intensityPreview->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 4px;")
                                      .arg(previewColor.name(QColor::HexArgb)));
}

void StartLensSettingsDialog::onIntensityChanged(int value)
{
    m_intensityValue = value;
    updateIntensityPreview();
}

void StartLensSettingsDialog::onImageIntensityChanged(int value)
{
    m_imageIntensityValue = value;
    QColor previewColor = QColor(0, 201, 255);
    previewColor.setAlpha(value * 255 / 100);
    m_imageIntensityPreview->setStyleSheet(QString("background-color: %1; border: 1px solid #ccc; border-radius: 4px;")
                                          .arg(previewColor.name(QColor::HexArgb)));
}

void StartLensSettingsDialog::onBackgroundModeChanged()
{
    bool isSolid = m_solidRadio->isChecked();
    bool isImage = m_imageRadio->isChecked();

    m_intensitySlider->setEnabled(isSolid);
    m_intensityPreview->setEnabled(isSolid);
    m_imageBrowseBtn->setEnabled(isImage);
    m_imageRemoveBtn->setEnabled(isImage);
    m_imagePreview->setEnabled(isImage);
    m_imageIntensitySlider->setEnabled(isImage);
    m_imageIntensityPreview->setEnabled(isImage);

    if (isSolid) {
        updateIntensityPreview();
    }
    if (isImage) {
        onImageIntensityChanged(m_imageIntensitySlider->value());
    }
}

void StartLensSettingsDialog::loadSettings()
{
    QString mode = m_settings->backgroundMode();
    if (mode == "solid") m_solidRadio->setChecked(true);
    else if (mode == "image") m_imageRadio->setChecked(true);
    else m_waveRadio->setChecked(true);

    m_intensityValue = m_settings->colorIntensity();
    if (m_intensityValue < 10) m_intensityValue = 80;
    m_intensitySlider->setValue(m_intensityValue);

    m_imageIntensityValue = m_settings->imageIntensity();
    if (m_imageIntensityValue < 10) m_imageIntensityValue = 80;
    m_imageIntensitySlider->setValue(m_imageIntensityValue);
    onImageIntensityChanged(m_imageIntensityValue);

    updateIntensityPreview();

    QString imgPath = m_settings->backgroundImagePath();
    if (!imgPath.isEmpty() && QFile::exists(imgPath)) {
        QPixmap pix(imgPath);
        if (!pix.isNull()) {
            m_imagePreview->setPixmap(pix.scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_imagePreview->setText("Invalid Image");
        }
        m_imagePreview->setToolTip(imgPath);
    } else {
        m_imagePreview->clear();
        m_imagePreview->setText("No Image");
        m_imagePreview->setToolTip("");
    }

    m_showAdvancedCheck->setChecked(m_settings->showAdvancedCategories());
    m_showHiddenCheck->setChecked(m_settings->showHiddenToggle());

    // تعبئة قائمة المخفيين
    m_hiddenList->clear();
    QSet<QString> hidden = m_settings->hiddenApps();
    for (const QString &name : hidden) {
        m_hiddenList->addItem(name);
    }
    if (hidden.isEmpty()) {
        QListWidgetItem *empty = new QListWidgetItem("(No hidden apps)");
        empty->setFlags(Qt::NoItemFlags);
        m_hiddenList->addItem(empty);
    }

    // تحديث قائمة التطبيقات المثبتة
    populateInstalledApps();

    onBackgroundModeChanged();
}

void StartLensSettingsDialog::saveSettings()
{
    if (m_waveRadio->isChecked()) {
        m_settings->setBackgroundMode("wave");
    } else if (m_solidRadio->isChecked()) {
        m_settings->setBackgroundMode("solid");
    } else if (m_imageRadio->isChecked()) {
        m_settings->setBackgroundMode("image");
    }

    m_settings->setColorIntensity(m_intensitySlider->value());
    m_settings->setImageIntensity(m_imageIntensitySlider->value());
    m_settings->setShowAdvancedCategories(m_showAdvancedCheck->isChecked());
    m_settings->setShowHiddenToggle(m_showHiddenCheck->isChecked());
}

void StartLensSettingsDialog::onApply()
{
    saveSettings();
    emit settingsApplied();
    accept();
}

void StartLensSettingsDialog::onClose()
{
    accept();
}

void StartLensSettingsDialog::onReset()
{
    if (QMessageBox::question(this, "Reset Settings",
                              "Delete all custom settings?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens";
        QDir(dir).removeRecursively();
        m_settings->load();
        m_usage->load();
        loadSettings();
        emit settingsApplied();
    }
}

void StartLensSettingsDialog::onChooseImage()
{
    QString path = QFileDialog::getOpenFileName(this, "Select Background Image",
                                                QString(), "Images (*.png *.jpg *.jpeg *.bmp *.svg);;All Files (*)");
    if (!path.isEmpty()) {
        m_settings->setBackgroundImage(path);
        QPixmap pix(path);
        if (!pix.isNull()) {
            m_imagePreview->setPixmap(pix.scaled(120, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            m_imagePreview->setText("Invalid Image");
        }
        m_imagePreview->setToolTip(path);
    }
}

void StartLensSettingsDialog::onRemoveImage()
{
    m_settings->setBackgroundImage(QString());
    m_imagePreview->clear();
    m_imagePreview->setText("No Image");
    m_imagePreview->setToolTip("");
}

void StartLensSettingsDialog::onAddApp()
{
    emit addAppRequested();
}
