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
#include "carddelegate.h"
#include "startlens_settings.h"
#include <QPainter>
#include <QApplication>
#include <QIcon>
#include <QSettings>

CardDelegate::CardDelegate(StartLensSettings *settings, QObject *parent)
    : QStyledItemDelegate(parent), m_settings(settings) {}

CardDelegate::CardDelegate(QObject *parent)
    : QStyledItemDelegate(parent), m_settings(nullptr) {}

void CardDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QString name = index.data(Qt::DisplayRole).toString();
    QString iconPath = index.data(Qt::UserRole + 2).toString();
    QString package = index.data(Qt::UserRole + 3).toString();
    QString refVersion = index.data(Qt::UserRole + 5).toString();
    QString cleaningStatus = index.data(Qt::UserRole + 4).toString();
    bool isGroup = (refVersion == "group");

    QRect rect = option.rect.adjusted(3, 3, -3, -3);
    bool hovered = option.state & QStyle::State_MouseOver;

    if (refVersion == "spacer") {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(rect);
        painter->restore();
        return;
    }

    if (refVersion == "section") {
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::transparent);
        painter->drawRect(rect);
        QFont font = QApplication::font();
        font.setBold(true);
        font.setPixelSize(14);
        painter->setFont(font);
        painter->setPen(QColor("#2c3e50"));
        painter->drawText(rect.adjusted(5, 5, -5, -5), Qt::AlignLeft | Qt::AlignVCenter, name);
        painter->restore();
        return;
    }

    QColor bgColor;
    if (isGroup) {
        bgColor = (!cleaningStatus.isEmpty() && QColor(cleaningStatus).isValid())
                      ? QColor(cleaningStatus) : QColor("#DAA520");
    } else {
        bgColor = QColor("#1E88E5");
    }
    if (hovered) {
        bgColor = bgColor.lighter(115);
        rect.adjust(-4, -4, 4, 4);
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 0, 60));
    painter->drawRoundedRect(rect.translated(1, 1), 12, 12);
    painter->setBrush(bgColor);
    painter->drawRoundedRect(rect, 12, 12);

    // أيقونة نصية أم عادية؟
    QString textIcon = m_settings ? m_settings->customIcon(name) : QString();

    if (!textIcon.isEmpty()) {
        // رسم أيقونة نصية
        painter->setBrush(QColor("#3b82f6"));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(rect.center().x(), rect.top() + 40), 24, 24);
        painter->setPen(Qt::white);
        QFont font = painter->font();
        font.setPixelSize(16);
        font.setBold(true);
        painter->setFont(font);
        painter->drawText(QRectF(rect.center().x() - 24, rect.top() + 16, 48, 48),
                          Qt::AlignCenter, textIcon.left(2));
    } else {
        // أيقونة عادية (نفس الكود القديم)
        QIcon icon;
        if (isGroup) {
            if (!iconPath.isEmpty()) {
                if (iconPath.endsWith(".svg") || iconPath.endsWith(".png")) {
                    icon = QIcon(iconPath);
                } else {
                    QSettings desktop(iconPath, QSettings::IniFormat);
                    desktop.beginGroup("Desktop Entry");
                    QString iconName = desktop.value("Icon").toString();
                    desktop.endGroup();
                    icon = QIcon::fromTheme(iconName);
                }
            }
        } else {
            if (!iconPath.isEmpty()) {
                QSettings desktop(iconPath, QSettings::IniFormat);
                desktop.beginGroup("Desktop Entry");
                QString iconName = desktop.value("Icon").toString();
                desktop.endGroup();
                icon = QIcon::fromTheme(iconName);
            }
        }
        if (icon.isNull()) icon = QIcon::fromTheme(package);
        if (icon.isNull()) icon = QIcon::fromTheme("application-x-executable");

        int iconSize = hovered ? 62 : 56;
        painter->drawPixmap(rect.center().x() - iconSize / 2, rect.top() + 12,
                            icon.pixmap(iconSize, iconSize));
    }

    // اسم التطبيق
    QFont nameFont = QApplication::font();
    nameFont.setBold(true);
    nameFont.setPixelSize(10);
    painter->setFont(nameFont);
    painter->setPen(Qt::white);
    QRectF textRect(rect.left() + 5, rect.top() + 70, rect.width() - 10, 35);
    painter->drawText(textRect, Qt::AlignHCenter | Qt::TextWordWrap, name);

    painter->restore();
}

QSize CardDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    QString refVersion = index.data(Qt::UserRole + 5).toString();
    if (refVersion == "section") return QSize(130, 30);
    if (refVersion == "spacer")  return QSize(130, 20);
    return QSize(140, 165);
}
