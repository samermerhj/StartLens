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
#include "startlens.h"
#include "utils.h"
#include "startlens_settings.h"
#include "startlens_settingsdialog.h"
#include "shutdowndialog.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QScrollBar>
#include <QMessageBox>
#include <algorithm>
#include <QEasingCurve>
#include <QRegularExpression>
#include <QSettings>
#include <QRadialGradient>
#include <QMenu>
#include <QProcess>
#include <QPropertyAnimation>
#include <QAbstractAnimation>
#include <QPainter>
#include <QToolButton>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QButtonGroup>
#include <QLineEdit>
#include <QScrollArea>
#include <QGridLayout>
#include <cmath>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QTimer>   // [تعديل] للتأكد من وجود QTimer

StartLens::StartLens(QWidget *parent) : QDialog(parent),
    m_scanner(new StartLensScanner(this)),
    m_usage(new StartLensUsage(this)),
    m_view(new StartLensView(this)),
    m_settings(new StartLensSettings(this)),
    m_settingsDialog(nullptr),
    m_shutdownDialog(nullptr),
    m_keepOpen(false)   // [تعديل] تهيئة m_keepOpen
{
    // [تعديل] تغيير Qt::Popup إلى Qt::Tool للتحكم اليدوي بالإخفاء
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setWindowTitle("Start Lens");
    setAttribute(Qt::WA_TranslucentBackground);
    setFocusPolicy(Qt::StrongFocus);   // [تعديل] منع فقدان التركيز تلقائياً

    // تأثير التلاشي
    m_fadeInAnim = new QPropertyAnimation(this, "windowOpacity");
    m_fadeInAnim->setDuration(150);
    m_fadeInAnim->setStartValue(0.0);
    m_fadeInAnim->setEndValue(1.0);

    m_fadeOutAnim = new QPropertyAnimation(this, "windowOpacity");
    m_fadeOutAnim->setDuration(200);
    m_fadeOutAnim->setStartValue(1.0);
    m_fadeOutAnim->setEndValue(0.0);
    connect(m_fadeOutAnim, &QPropertyAnimation::finished, this, &QDialog::hide);

    // [تعديل] تأثير الحركة (صعود وهبوط)
    m_slideInAnim = new QPropertyAnimation(this, "pos");
    m_slideInAnim->setDuration(200);
    m_slideInAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_slideOutAnim = new QPropertyAnimation(this, "pos");
    m_slideOutAnim->setDuration(200);
    m_slideOutAnim->setEasingCurve(QEasingCurve::InCubic);

    // حساب المواضع
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();
    int w = screenRect.width() * 5 / 6;
    int topMargin = 180;
    int h = screenRect.height() - topMargin;
    resize(w, h);
    int x = (screenRect.width() - w) / 2;
    m_originalPos = QPoint(x, topMargin);
    m_hiddenPos = QPoint(x, screenRect.height() + 50);   // [تعديل] أسفل الشاشة

    move(m_hiddenPos);   // [تعديل] تبدأ النافذة مخفية

    m_bgTimer = new QTimer(this);
    connect(m_bgTimer, &QTimer::timeout, this, &StartLens::animateBackground);
    m_bgTimer->start(50);

    setupUI();
    m_allApps = m_scanner->loadAppsAndGroups();
    refreshDisplayList();

    m_hotkey = new QHotkey(this);
    m_hotkey->registerShortcut(QKeySequence("Super+Space"));
    connect(m_hotkey, &QHotkey::activated, this, &StartLens::toggleWindow);

    connect(m_settings, &StartLensSettings::iconChanged, this, [this](const QString &appName, const QString &newIcon) {
        for (int i = 0; i < m_gridLayout->count(); ++i) {
            QWidget *w = m_gridLayout->itemAt(i)->widget();
            if (w && w->inherits("QToolButton")) {
                QToolButton *btn = qobject_cast<QToolButton*>(w);
                if (btn && btn->text() == appName) {
                    btn->setIcon(createIconForApp(appName, newIcon));
                    break;
                }
            }
        }
    });

    connect(m_settings, &StartLensSettings::backgroundChanged, this, &StartLens::onBackgroundChanged);
    onBackgroundChanged();

    m_showHidden = m_settings->showHiddenToggle();
}

void StartLens::applySettings()
{
    m_showHidden = m_settings->showHiddenToggle();
    refreshDisplayList();
    onBackgroundChanged();
    if (m_settings->backgroundMode() == "solid") {
        m_settings->setSolidColor(m_waveColor);
    }
}

void StartLens::onSettingsApplied()
{
    applySettings();
    if (m_settingsDialog) {
        m_settingsDialog->accept();
    }
    m_keepOpen = false;
}

// [تعديل] دالة جديدة لإعادة m_keepOpen عند إغلاق الإعدادات
void StartLens::onSettingsClosed()
{
    m_keepOpen = false;
}

void StartLens::onBackgroundChanged()
{
    QString imgPath = m_settings->backgroundImagePath();
    if (!imgPath.isEmpty() && QFile::exists(imgPath)) {
        if (m_backgroundImage.load(imgPath)) {
            m_hasBackgroundImage = true;
        } else {
            m_hasBackgroundImage = false;
        }
    } else {
        m_hasBackgroundImage = false;
        m_backgroundImage = QImage();
    }

    update();

    if (m_settings->backgroundMode() == "image") {
        m_bgTimer->stop();
    } else if (m_settings->backgroundMode() == "wave" && !m_bgTimer->isActive()) {
        m_bgTimer->start(50);
    }
}

void StartLens::animateBackground()
{
    if (m_settings->backgroundMode() == "wave") {
        m_wavePhase += 0.015f;
        if (m_wavePhase > 2.0f * M_PI) {
            m_wavePhase -= 2.0f * M_PI;
        }
        update();
    }
}

void StartLens::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setClipRect(rect());

    QString mode = m_settings->backgroundMode();

    if (mode == "image" && m_hasBackgroundImage && !m_backgroundImage.isNull()) {
    // رسم الصورة في الخلفية
    painter.drawImage(rect(), m_backgroundImage.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

    // الحصول على شدة الصورة من الإعدادات
    int intensity = m_settings->imageIntensity();
    if (intensity < 10) intensity = 80;

    // حساب شفافية الطبقة البيضاء (كلما زادت الشدة، قلت الشفافية)
    // intensity 100% -> طبقة بيضاء شفافة قليلاً (20)
    // intensity 10%  -> طبقة بيضاء شفافة كثيراً (230)
    int whiteLayerAlpha = 230 - (intensity * 210 / 100);
    if (whiteLayerAlpha < 20) whiteLayerAlpha = 20;
    if (whiteLayerAlpha > 230) whiteLayerAlpha = 230;

    painter.fillRect(rect(), QColor(255, 255, 255, whiteLayerAlpha));
        
        
    } else if (mode == "solid") {
        // ===== الخلفية الصلبة مع شدة اللون =====
        QColor solidColor = m_settings->solidColor();
        if (!solidColor.isValid()) solidColor = m_waveColor;

        // الحصول على شدة اللون من الإعدادات
        int intensity = m_settings->colorIntensity();
        if (intensity < 10) intensity = 80;

        // تطبيق الشدة على اللون
        QColor finalColor = solidColor;
        finalColor.setAlpha(intensity * 255 / 100);

        // رسم الخلفية باللون مع الشدة المحددة
        painter.fillRect(rect(), finalColor);

        // طبقة بيضاء شفافة لتفتيح الخلفية قليلاً (اختياري)
        painter.fillRect(rect(), QColor(255, 255, 255, 40));

    } else {
        // ===== الخلفية المتحركة (تبقى كما هي دون تغيير) =====
        painter.fillRect(rect(), QColor(255, 255, 255, 230));

        QRadialGradient gradient(width()/2, height(), height());
        gradient.setColorAt(0, m_waveColor.lighter(150));
        gradient.setColorAt(0.3, m_waveColor);
        gradient.setColorAt(1, QColor(255, 255, 255, 0));
        painter.fillRect(rect(), gradient);

        QPen sunPen(m_waveColor);
        sunPen.setWidth(1);
        painter.setPen(sunPen);

        int w = width();
        int h = height();
        int cx = w / 2;
        int cy = h;
        float innerRadius = 250.0f;
        float maxLength = std::hypot(w, h) * 1.2f;

        int halfRays = 30;
        float angleStep = (M_PI / 2.0f) / halfRays;

        for (int i = 0; i <= halfRays; ++i) {
            float angle = i * angleStep;

            QPainterPath pathRight;
            pathRight.moveTo(cx, cy);
            for (float r = 0; r <= maxLength; r += 12.0f) {
                float baseX = cx + r * cos(angle);
                float baseY = cy - r * sin(angle);
                float amplitude = (r / maxLength) * 14.0f;
                float speedFactor;
                if (r < innerRadius) speedFactor = 3.5f;
                else if (r > innerRadius * 2.0f) speedFactor = 0.5f;
                else {
                    float t = (r - innerRadius) / innerRadius;
                    speedFactor = 3.5f - (t * 3.0f);
                }
                float wave = qSin(r * 0.04f + (m_wavePhase * speedFactor)) * amplitude;
                float perpX = -sin(angle) * wave;
                float perpY = -cos(angle) * wave;
                pathRight.lineTo(baseX + perpX, baseY + perpY);
            }
            painter.drawPath(pathRight);

            if (i != 0) {
                QPainterPath pathLeft;
                pathLeft.moveTo(cx, cy);
                for (float r = 0; r <= maxLength; r += 6.0f) {
                    float baseX = cx - r * cos(angle);
                    float baseY = cy - r * sin(angle);
                    float amplitude = (r / maxLength) * 14.0f;
                    float speedFactor;
                    if (r < innerRadius) speedFactor = 3.5f;
                    else if (r > innerRadius * 2.0f) speedFactor = 0.5f;
                    else {
                        float t = (r - innerRadius) / innerRadius;
                        speedFactor = 3.5f - (t * 3.0f);
                    }
                    float wave = qSin(r * 0.04f + (m_wavePhase * speedFactor)) * amplitude;
                    float perpX = sin(angle) * wave;
                    float perpY = -cos(angle) * wave;
                    pathLeft.lineTo(baseX + perpX, baseY + perpY);
                }
                painter.drawPath(pathLeft);
            }
        }
    }

    QPen borderPen(m_borderColor, 4);
    painter.setPen(borderPen);
    painter.drawRect(rect().adjusted(2, 2, -2, -2));
}

void StartLens::toggleWindow()
{
    isVisible() ? hideWindow() : showWindow();
}

void StartLens::showWindow()
{
    m_fadeOutAnim->stop();
    m_slideOutAnim->stop();

    m_allApps = m_scanner->loadAppsAndGroups();
    refreshDisplayList();

    // ضبط الموضع الأولي للحركة (أسفل)
    move(m_hiddenPos);
    setWindowOpacity(0.0);

    // إظهار النافذة (لكنها مخفية تماماً)
    QDialog::show();
    activateWindow();

    // بدء حركة الصعود والتلاشي
    m_slideInAnim->setStartValue(m_hiddenPos);
    m_slideInAnim->setEndValue(m_originalPos);
    m_slideInAnim->start();

    m_fadeInAnim->start();

    m_searchEdit->clear();
    m_searchEdit->setFocus();
}

// [تعديل] دالة hideWindow المعدلة (هبوط + تلاشي)
void StartLens::hideWindow()
{
    m_fadeInAnim->stop();
    m_slideInAnim->stop();

    // بدء حركة الهبوط والتلاشي
    m_slideOutAnim->setStartValue(pos());
    m_slideOutAnim->setEndValue(m_hiddenPos);
    m_slideOutAnim->start();

    m_fadeOutAnim->start(); // عند الانتهاء، يستدعي QDialog::hide
}

void StartLens::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hideWindow();
        return;
    }
    QDialog::keyPressEvent(event);
}

// [تعديل] معالجة تغيير الحالة مع مراعاة m_keepOpen
void StartLens::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        if (!isActiveWindow() && !m_keepOpen) {
            hideWindow();
        }
    }
    QDialog::changeEvent(event);
}

// [تعديل] إضافة معالج الأحداث لالتقاط فقدان التركيز
bool StartLens::event(QEvent *event)
{
    if (event->type() == QEvent::WindowDeactivate) {
        if (!m_keepOpen) {
            // إخفاء بعد تأخير بسيط لتجنب التعارض مع أحداث أخرى
            QTimer::singleShot(50, this, &StartLens::hideWindow);
        }
    }
    return QDialog::event(event);
}

void StartLens::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    refreshDisplayList();
    update();
}


void StartLens::applyCategoryColor(const QString &colorName)
{
    m_waveColor = QColor(colorName);
    m_borderColor = m_waveColor;

    // إذا كانت الخلفية في وضع Solid، نحدث اللون في الإعدادات
    if (m_settings->backgroundMode() == "solid") {
        m_settings->setSolidColor(m_waveColor);
        // تحديث المعاينة في نافذة الإعدادات إذا كانت مفتوحة
        if (m_settingsDialog) {
            m_settingsDialog->updateColorPreview(m_waveColor);
        }
    }
    update();
}

void StartLens::shutdownSystem()
{
    if (QMessageBox::question(this, "Shutdown", "Shut down the system?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        bool success = false;
        if (QProcess::startDetached("systemctl", {"poweroff"})) {
            success = true;
        } else if (QProcess::startDetached("loginctl", {"poweroff"})) {
            success = true;
        } else {
            QStringList args;
            args << "--system" << "--dest=org.freedesktop.login1"
                 << "--type=method_call" << "--print-reply"
                 << "/org/freedesktop/login1"
                 << "org.freedesktop.login1.Manager.PowerOff"
                 << "boolean:true";
            if (QProcess::startDetached("dbus-send", args)) {
                success = true;
            }
        }
        if (success) {
            hideWindow();
        } else {
            QMessageBox::warning(this, "Error", "Could not shutdown the system.");
        }
    }
}

// ===== الدوال الجديدة =====
void StartLens::showSystemCategory()
{
    m_currentCategory = "System";
    // اللون اخضر (#49f116)
    applyCategoryColor("#49f116");
    refreshDisplayList();
}

void StartLens::showPinnedAndGroups()
{
    m_currentCategory = "Pinned & Groups";
    // اللون الذهبي (#F1C40F)
    applyCategoryColor("#F1C40F");
    refreshDisplayList();
}

void StartLens::showShutdownDialog()
{
    if (!m_shutdownDialog) {
        m_shutdownDialog = new ShutdownDialog(this);
    }
    m_shutdownDialog->exec();
}

// ===== setupUI مع تعديل زر الإعدادات =====
void StartLens::setupUI()
{
    QVBoxLayout *mainLay = new QVBoxLayout(this);
    mainLay->setContentsMargins(20, 16, 20, 12);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollContent = new QWidget();
    m_scrollContent->setStyleSheet("background: transparent;");
    m_gridLayout = new QGridLayout(m_scrollContent);
    m_gridLayout->setSpacing(10);
    m_gridLayout->setContentsMargins(5, 5, 5, 5);
    m_scrollArea->setWidget(m_scrollContent);
    mainLay->addWidget(m_scrollArea, 1);

    QVBoxLayout *bottomLay = new QVBoxLayout;
    bottomLay->setSpacing(8);

    QHBoxLayout *catLay = new QHBoxLayout;
    catLay->setSpacing(8);
    catLay->setContentsMargins(0, 0, 0, 0);

    // ===== أزرار التصنيفات الجديدة =====
    m_categoryGroup = new QButtonGroup(this);
    QStringList categories = {"All", "Internet", "Graphics", "Multimedia", "Office", "Pinned & Groups", "System"};
    QStringList colorNames = {
        "#2fb62a", // All    (أخضر)
        "#e8a600", // Internet
        "#ff5500", // Graphics (أخضر)
        "#F59E0B", // Multimedia (البرتقالي)
        "#8B5CF6", // Office
        "#F1C40F", // Pinned & Groups (ذهبي)
        "#60ca09"  // System (أخضر)
    };

    for (int i = 0; i < categories.size(); ++i) {
        QPushButton *btn = new QPushButton(categories[i]);
        btn->setCheckable(true);
        btn->setFixedSize(110, 36);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(QString(
            "QPushButton { background-color: #f3f4f6; color: #374151; border: none; border-radius: 18px; font-weight: bold; font-size: 11px; }"
            "QPushButton:checked { background-color: %1; color: white; }"
        ).arg(colorNames[i]));
        catLay->addWidget(btn);
        m_categoryGroup->addButton(btn, i);
        if (i == 0) btn->setChecked(true);
    }
    catLay->addStretch();

    // زر الإعدادات (مسنن)
    m_settingsBtn = new QPushButton("⚙️");
    m_settingsBtn->setFixedSize(44, 36);
    m_settingsBtn->setCursor(Qt::PointingHandCursor);
    m_settingsBtn->setStyleSheet(
        "QPushButton { background-color: #6c757d; color: white; border: none; border-radius: 18px; font-weight: bold; font-size: 16px; }"
        "QPushButton:hover { background-color: #5a6268; }"
    );

    // [تعديل] تعديل سلوك زر الإعدادات لتعيين m_keepOpen
    connect(m_settingsBtn, &QPushButton::clicked, [this]() {
        if (!m_settingsDialog) {
            m_settingsDialog = new StartLensSettingsDialog(m_settings, m_usage, this);
            connect(m_settingsDialog, &StartLensSettingsDialog::settingsApplied, this, &StartLens::onSettingsApplied);
            connect(m_settingsDialog, &StartLensSettingsDialog::addAppRequested, this, &StartLens::addCustomApp);
            connect(m_settingsDialog, &QDialog::finished, this, &StartLens::onSettingsClosed);
        }
        // منع الإخفاء أثناء عرض الإعدادات
        m_keepOpen = true;
        m_settingsDialog->exec();
        // عند إغلاق الإعدادات، يتم إعادة m_keepOpen عبر الإشارة finished
    });
    catLay->addWidget(m_settingsBtn);

    m_shutdownBtn = new QPushButton("⏻");
    m_shutdownBtn->setFixedSize(44, 36);
    m_shutdownBtn->setCursor(Qt::PointingHandCursor);
    m_shutdownBtn->setStyleSheet(
        "QPushButton { background-color: #ef4444; color: white; border: none; border-radius: 18px; font-weight: bold; font-size: 16px; }"
        "QPushButton:hover { background-color: #dc2626; }"
    );
    connect(m_shutdownBtn, &QPushButton::clicked, this, &StartLens::showShutdownDialog);
    catLay->addWidget(m_shutdownBtn);

    bottomLay->addLayout(catLay);

    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("🔍 Search applications...");
    m_searchEdit->setStyleSheet("QLineEdit { padding: 12px 18px; font-size: 15px; border: 1px solid #d1d5db; border-radius: 26px; background: white; }");
    bottomLay->addWidget(m_searchEdit);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &StartLens::onSearchTextChanged);
    mainLay->addLayout(bottomLay);

    connect(m_categoryGroup, QOverload<int>::of(&QButtonGroup::idClicked), [this, colorNames](int id) {
        QStringList cats = {"All", "Internet", "Graphics", "Multimedia", "Office", "Pinned & Groups", "System"};
        m_currentCategory = cats[id];
        applyCategoryColor(colorNames[id]);
        refreshDisplayList();
    });
}

// باقي الدوال (addCustomApp, createIconForApp, refreshDisplayList, onCustomContextMenu, runItem, onSearchTextChanged) تبقى كما هي دون تغيير
// لقد تم تضمينها أعلاه بالكامل، لكن سأكتبها مرة أخرى للاكتمال

void StartLens::addCustomApp()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Application (.desktop)",
        "/usr/share/applications",
        "Desktop Files (*.desktop);;All Files (*)"
    );

    if (filePath.isEmpty()) return;

    QSettings desktop(filePath, QSettings::IniFormat);
    desktop.beginGroup("Desktop Entry");
    QString name = desktop.value("Name").toString();
    QString noDisplay = desktop.value("NoDisplay").toString();
    QString hidden = desktop.value("Hidden").toString();
    desktop.endGroup();

    if (name.isEmpty() || noDisplay == "true" || hidden == "true") {
        QMessageBox::warning(this, "Error", "Invalid or hidden application.");
        return;
    }

    QString pkg = QFileInfo(filePath).completeBaseName();
    for (const auto &app : m_allApps) {
        if (app.package == pkg) {
            QMessageBox::information(this, "Already Exists", "This application is already in the list.");
            return;
        }
    }

    SystemItem newApp;
    newApp.name = name;
    newApp.package = pkg;
    newApp.iconPath = filePath;
    newApp.type = SystemItem::Application;
    newApp.version = "custom";
    newApp.referenceVersion = "custom";

    m_allApps.append(newApp);

    QString dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/StartLens";
    QDir().mkpath(dir);
    QFile file(dir + "/custom_apps.json");
    QJsonArray arr;
    if (file.open(QIODevice::ReadOnly)) {
        arr = QJsonDocument::fromJson(file.readAll()).array();
        file.close();
    }
    QJsonObject obj;
    obj["name"] = name;
    obj["package"] = pkg;
    obj["icon_path"] = filePath;
    obj["version"] = "custom";
    arr.append(obj);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(arr).toJson());
        file.close();
    }

    refreshDisplayList();
    QMessageBox::information(this, "Success", QString("Added '%1' successfully.").arg(name));
}

QIcon StartLens::createIconForApp(const QString &appName, const QString &customIconText)
{
    if (!customIconText.isEmpty()) {
        if (customIconText.startsWith("svg:")) {
            return QIcon(customIconText.mid(4));
        } else {
            QPixmap pix(48, 48);
            pix.fill(Qt::transparent);
            QPainter p(&pix);
            p.setBrush(QColor("#3b82f6"));
            p.setPen(Qt::NoPen);
            p.drawEllipse(0, 0, 48, 48);
            p.setPen(Qt::white);
            QFont f = p.font();
            f.setPixelSize(20);
            f.setBold(true);
            p.setFont(f);
            p.drawText(QRect(0, 0, 48, 48), Qt::AlignCenter, customIconText.left(2));
            p.end();
            return QIcon(pix);
        }
    }

    for (const auto &item : m_allApps) {
        if (item.name == appName && item.referenceVersion != "group") {
            QIcon icon;
            if (!item.iconPath.isEmpty()) {
                QSettings desktop(item.iconPath, QSettings::IniFormat);
                desktop.beginGroup("Desktop Entry");
                QString iconName = desktop.value("Icon").toString();
                desktop.endGroup();
                icon = QIcon::fromTheme(iconName);
            }
            if (icon.isNull()) icon = QIcon::fromTheme(item.package.isEmpty() ? appName.toLower().replace(' ', '-') : item.package);
            if (icon.isNull()) icon = QIcon::fromTheme("application-x-executable");
            return icon;
        }
    }
    return QIcon::fromTheme("application-x-executable");
}

void StartLens::refreshDisplayList()
{
    while (m_gridLayout->count() > 0) {
        QLayoutItem *item = m_gridLayout->takeAt(0);
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    QList<SystemItem> displayItems;
    m_view->buildDisplayList(m_allApps, displayItems, m_currentCategory, m_usage, m_settings);

    QList<SystemItem> filteredItems;
    for (const auto &item : displayItems) {
        if (item.referenceVersion == "section" || item.referenceVersion == "spacer" || item.referenceVersion == "group") {
            filteredItems.append(item);
            continue;
        }
        bool isHidden = m_settings->isHidden(item.name);
        if (isHidden && !m_showHidden) {
            continue;
        }
        filteredItems.append(item);
    }
    displayItems = filteredItems;

    int maxCols = qMax(1, (m_scrollArea->width() - 20) / 130);
    int col = 0, row = 0;
    bool firstSection = true;

    for (const auto &item : displayItems) {
        if (item.referenceVersion == "section") {
            if (!firstSection) { row++; }
            QWidget *sectionContainer = new QWidget();
            QHBoxLayout *secLayout = new QHBoxLayout(sectionContainer);
            secLayout->setContentsMargins(0, 4, 0, 4);
            QLabel *secLabel = new QLabel(item.name);
            secLabel->setStyleSheet("color: #374151; font-weight: bold; font-size: 13px; background: transparent;");
            QFrame *dividerLine = new QFrame();
            dividerLine->setFrameShape(QFrame::HLine);
            dividerLine->setStyleSheet(QString("background-color: %1; border: none; height: 2px;").arg(m_waveColor.name()));
            secLayout->addWidget(secLabel);
            secLayout->addWidget(dividerLine, 1);
            m_gridLayout->addWidget(sectionContainer, row, 0, 1, -1);
            row++;
            col = 0;
            firstSection = false;
            continue;
        }
        if (item.referenceVersion == "spacer") { row++; col = 0; continue; }

        QToolButton *card = new QToolButton();
        card->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        card->setIconSize(QSize(48, 48));
        card->setFixedSize(120, 120);
        bool isGroup = (item.referenceVersion == "group");
        bool isHidden = m_settings->isHidden(item.name);

        QString baseColor = isGroup ? (item.cleaningStatus.isEmpty() ? "#DAA520" : item.cleaningStatus) : "#3b82f6";
        QString hoverColor = m_waveColor.name();

        QString style = QString(
            "QToolButton { background-color: %1; border-radius: 12px; color: white; font-weight: bold; font-size: 10px; }"
            "QToolButton:hover { background-color: %2; border: 2px solid white; }"
        ).arg(baseColor, hoverColor);

        if (isHidden && m_showHidden) {
            style += " QToolButton { opacity: 0.6; }";
        }
        card->setStyleSheet(style);

        QString textIcon = m_settings->customIcon(item.name);
        QIcon icon = createIconForApp(item.name, textIcon);
        if (icon.isNull() && isGroup && !item.iconPath.isEmpty()) {
            if (item.iconPath.endsWith(".svg") || item.iconPath.endsWith(".png"))
                icon = QIcon(item.iconPath);
            else {
                QSettings desktop(item.iconPath, QSettings::IniFormat);
                desktop.beginGroup("Desktop Entry"); QString iconName = desktop.value("Icon").toString(); desktop.endGroup();
                icon = QIcon::fromTheme(iconName);
            }
        }
        card->setIcon(icon);

        QString displayName = item.name;
        if (isHidden && m_showHidden) {
            displayName += " 👁";
        }
        card->setText(displayName);

        connect(card, &QToolButton::clicked, [this, item]() { runItem(item); });
        card->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(card, &QToolButton::customContextMenuRequested, [this, card, item](const QPoint &pos) {
            onCustomContextMenu(card->mapToGlobal(pos), item);
        });

        m_gridLayout->addWidget(card, row, col);
        col++;
        if (col >= maxCols) { col = 0; row++; }
    }
}

void StartLens::onCustomContextMenu(const QPoint &globalPos, const SystemItem &item)
{
    if (item.referenceVersion == "group" || item.referenceVersion == "section" || item.referenceVersion == "spacer")
        return;

    QMenu menu;

    if (m_usage->isPinned(item.name)) {
        menu.addAction("📌 Unpin", [this, name = item.name]() {
            m_usage->unpinApp(name);
            refreshDisplayList();
        });
    } else {
        menu.addAction("📌 Pin", [this, name = item.name]() {
            m_usage->pinApp(name);
            refreshDisplayList();
        });
    }

    if (m_settings->isHidden(item.name)) {
        menu.addAction("👁 Show", [this, name = item.name]() {
            m_settings->unhideApp(name);
            refreshDisplayList();
        });
    } else {
        menu.addAction("👁 Hide", [this, name = item.name]() {
            m_settings->hideApp(name);
            refreshDisplayList();
        });
    }

    QMenu *moveMenu = menu.addMenu("📂 Move to Category");
    QStringList cats = {"Internet", "Games", "Multimedia", "System Tools", "Office", "Settings", "Other"};
    for (const QString &cat : cats) {
        moveMenu->addAction(cat, [this, name = item.name, cat]() {
            m_settings->setCustomCategory(name, cat);
            refreshDisplayList();
        });
    }
    moveMenu->addAction("↺ Auto", [this, name = item.name]() {
        m_settings->setCustomCategory(name, QString());
        refreshDisplayList();
    });

    menu.addAction("✏ Set Text Icon", [this, name = item.name]() {
        QMenu iconMenu;
        iconMenu.addAction("📝 Enter Text", [this, name]() {
            bool ok;
            QString text = QInputDialog::getText(this, "Set Text Icon",
                                                 "Enter up to 2 characters:", QLineEdit::Normal, "", &ok);
            if (ok) {
                m_settings->setCustomIcon(name, text.isEmpty() ? "" : text.left(2));
            }
        });
        iconMenu.addAction("📂 Browse SVG", [this, name]() {
            QString filePath = QFileDialog::getOpenFileName(this, "Select SVG Icon",
                                                            QString(), "SVG Files (*.svg);;All Files (*)");
            if (!filePath.isEmpty()) {
                m_settings->setCustomIcon(name, "svg:" + filePath);
            }
        });
        iconMenu.exec(QCursor::pos());
    });

    menu.exec(globalPos);
}

void StartLens::runItem(const SystemItem &item)
{
    if (item.referenceVersion == "section" || item.referenceVersion == "spacer") return;

    auto execApp = [&](const SystemItem &app) -> bool {
        // استخراج launchName الموحد
        QString launchName = app.package.isEmpty() ? app.name : app.package;
        if (launchName.endsWith(".desktop"))
            launchName.chop(8);

        bool success = QProcess::startDetached("gtk-launch", {launchName});
        if (success) {
            // ✅ التسجيل بنفس المفتاح (launchName)
            m_usage->recordUsage(launchName);
        } else {
            // ❌ الحذف بنفس المفتاح (launchName)
            qDebug() << "Failed to launch:" << launchName << " - Removing from usage";
            m_usage->removeApp(launchName);
        }
        return success;
    };

    if (item.referenceVersion == "group") {
        QString mainAppName = item.version;
        if (!mainAppName.isEmpty()) {
            for (const auto &a : m_allApps) {
                if (a.name == mainAppName && a.referenceVersion != "group") {
                    if (execApp(a)) {
                        hideWindow();
                    }
                    return;
                }
            }
        }
        QMessageBox::information(this, "Group", "No main application set for this group.\nSet one in LensClean.");
        return;
    }

    if (execApp(item)) {
        hideWindow();
    }
}
void StartLens::onSearchTextChanged(const QString &text)
{
    for (int i = 0; i < m_gridLayout->count(); ++i) {
        QWidget *w = m_gridLayout->itemAt(i)->widget();
        if (w && w->inherits("QToolButton")) {
            QToolButton *btn = qobject_cast<QToolButton*>(w);
            if (btn) {
                btn->setVisible(btn->text().contains(text, Qt::CaseInsensitive));
            }
        }
    }
}
