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
#ifndef SHUTDOWNDIALOG_H
#define SHUTDOWNDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>

class ShutdownDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ShutdownDialog(QWidget *parent = nullptr);

private slots:
    void onShutdown();
    void onRestart();
    void onLogout();
    void onSuspend();
    void onHibernate();

private:
    void executeCommand(const QString &command, const QStringList &args);
    void closeDialog();

    QPushButton *m_shutdownBtn;
    QPushButton *m_restartBtn;
    QPushButton *m_logoutBtn;
    QPushButton *m_suspendBtn;
    QPushButton *m_hibernateBtn;
    QPushButton *m_cancelBtn;
};

#endif // SHUTDOWNDIALOG_H
