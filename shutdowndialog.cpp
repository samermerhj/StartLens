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
#include "shutdowndialog.h"
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>

ShutdownDialog::ShutdownDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("System Control");
    setFixedSize(350, 250);
    setModal(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QLabel *titleLabel = new QLabel("⏻ System Control");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(titleLabel, 0, Qt::AlignCenter);

    QGridLayout *gridLayout = new QGridLayout();

    m_shutdownBtn = new QPushButton("⏻ Shutdown");
    m_shutdownBtn->setStyleSheet("QPushButton { background-color: #ef4444; color: white; padding: 10px; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #dc2626; }");

    m_restartBtn = new QPushButton("🔄 Restart");
    m_restartBtn->setStyleSheet("QPushButton { background-color: #f59e0b; color: white; padding: 10px; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #d97706; }");

    m_logoutBtn = new QPushButton("🚪 Logout");
    m_logoutBtn->setStyleSheet("QPushButton { background-color: #3b82f6; color: white; padding: 10px; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #2563eb; }");

    m_suspendBtn = new QPushButton("💤 Suspend");
    m_suspendBtn->setStyleSheet("QPushButton { background-color: #8b5cf6; color: white; padding: 10px; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #7c3aed; }");

    m_hibernateBtn = new QPushButton("💾 Hibernate");
    m_hibernateBtn->setStyleSheet("QPushButton { background-color: #6b7280; color: white; padding: 10px; border-radius: 6px; font-weight: bold; } QPushButton:hover { background-color: #4b5563; }");

    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setStyleSheet("QPushButton { padding: 8px; border-radius: 4px; } QPushButton:hover { background-color: #e5e7eb; }");

    gridLayout->addWidget(m_shutdownBtn, 0, 0);
    gridLayout->addWidget(m_restartBtn, 0, 1);
    gridLayout->addWidget(m_logoutBtn, 1, 0);
    gridLayout->addWidget(m_suspendBtn, 1, 1);
    gridLayout->addWidget(m_hibernateBtn, 2, 0, 1, 2);
    mainLayout->addLayout(gridLayout);

    mainLayout->addWidget(m_cancelBtn, 0, Qt::AlignCenter);

    connect(m_shutdownBtn, &QPushButton::clicked, this, &ShutdownDialog::onShutdown);
    connect(m_restartBtn, &QPushButton::clicked, this, &ShutdownDialog::onRestart);
    connect(m_logoutBtn, &QPushButton::clicked, this, &ShutdownDialog::onLogout);
    connect(m_suspendBtn, &QPushButton::clicked, this, &ShutdownDialog::onSuspend);
    connect(m_hibernateBtn, &QPushButton::clicked, this, &ShutdownDialog::onHibernate);
    connect(m_cancelBtn, &QPushButton::clicked, this, &ShutdownDialog::closeDialog);
}

void ShutdownDialog::executeCommand(const QString &command, const QStringList &args)
{
    QProcess::startDetached(command, args);
    closeDialog();
}

void ShutdownDialog::closeDialog()
{
    accept();
}

void ShutdownDialog::onShutdown()
{
    if (QMessageBox::question(this, "Shutdown", "Shut down the system?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (QProcess::startDetached("systemctl", {"poweroff"})) {
            closeDialog();
        } else if (QProcess::startDetached("loginctl", {"poweroff"})) {
            closeDialog();
        } else {
            QStringList args;
            args << "--system" << "--dest=org.freedesktop.login1"
                 << "--type=method_call" << "--print-reply"
                 << "/org/freedesktop/login1"
                 << "org.freedesktop.login1.Manager.PowerOff"
                 << "boolean:true";
            if (QProcess::startDetached("dbus-send", args)) {
                closeDialog();
            } else {
                QMessageBox::warning(this, "Error", "Could not shutdown the system.");
            }
        }
    }
}

void ShutdownDialog::onRestart()
{
    if (QMessageBox::question(this, "Restart", "Restart the system?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (QProcess::startDetached("systemctl", {"reboot"})) {
            closeDialog();
        } else if (QProcess::startDetached("loginctl", {"reboot"})) {
            closeDialog();
        } else {
            QStringList args;
            args << "--system" << "--dest=org.freedesktop.login1"
                 << "--type=method_call" << "--print-reply"
                 << "/org/freedesktop/login1"
                 << "org.freedesktop.login1.Manager.Reboot"
                 << "boolean:true";
            if (QProcess::startDetached("dbus-send", args)) {
                closeDialog();
            } else {
                QMessageBox::warning(this, "Error", "Could not restart the system.");
            }
        }
    }
}

void ShutdownDialog::onLogout()
{
    if (QMessageBox::question(this, "Logout", "Logout from the system?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        // محاولة loginctl أولاً
        QString username = qgetenv("USER");
        if (QProcess::startDetached("loginctl", {"terminate-user", username})) {
            closeDialog();
        } else if (QProcess::startDetached("pkill", {"-KILL", "-u", username})) {
            closeDialog();
        } else {
            // محاولة gnome-session-quit أو qdbus
            if (QProcess::startDetached("gnome-session-quit", {"--force"})) {
                closeDialog();
            } else {
                QMessageBox::warning(this, "Error", "Could not logout.");
            }
        }
    }
}

void ShutdownDialog::onSuspend()
{
    if (QProcess::startDetached("systemctl", {"suspend"})) {
        closeDialog();
    } else if (QProcess::startDetached("loginctl", {"suspend"})) {
        closeDialog();
    } else {
        QMessageBox::warning(this, "Error", "Could not suspend the system.");
    }
}

void ShutdownDialog::onHibernate()
{
    if (QProcess::startDetached("systemctl", {"hibernate"})) {
        closeDialog();
    } else if (QProcess::startDetached("loginctl", {"hibernate"})) {
        closeDialog();
    } else {
        QMessageBox::warning(this, "Error", "Could not hibernate the system.");
    }
}
