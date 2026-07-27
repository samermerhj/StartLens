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
#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QIcon>
#include <QLocalSocket>
#include <QLocalServer>
#include "startlens.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("StartLens");
    app.setQuitOnLastWindowClosed(false);

    // ============================================================
    // 1. التحقق من وجود نسخة أخرى (Single Instance)
    // ============================================================
    QLocalSocket socket;
    socket.connectToServer("startlens_socket");
    if (socket.waitForConnected(500)) {
        // نسخة أخرى تعمل -> نرسل لها أمر "toggle" ونخرج
        socket.write("toggle");
        socket.flush();
        socket.waitForBytesWritten(500);
        socket.close();
        return 0;   // ننهي هذه النسخة
    }

    // ============================================================
    // 2. إنشاء الخادم المحلي للاستماع للأوامر (لهذه النسخة)
    // ============================================================
    QLocalServer server;
    // في حال بقاء السوكيت من نسخة سابقة لم تغلق بشكل نظيف
    server.removeServer("startlens_socket");
    if (!server.listen("startlens_socket")) {
        // فشل الاستماع (نادراً)، ولكن نستمر في التشغيل بدون هذه الميزة
        // (يمكنك إضافة رسالة خطأ هنا إذا أردت)
    }

    // ============================================================
    // 3. إنشاء الكائن الرئيسي StartLens
    // ============================================================
    StartLens startLens;
    startLens.setWindowIcon(QIcon::fromTheme("system-search"));

    // ============================================================
    // 4. ربط الخادم المحلي لاستدعاء toggleWindow()
    // ============================================================
    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket *client = server.nextPendingConnection();
        if (!client) return;

        // قراءة الأمر (يمكننا تجاهل محتواه، لكننا نقرأه لتنظيف البافر)
        client->readAll();
        client->disconnectFromServer();
        client->deleteLater();

        // تبديل حالة النافذة (الدالة موجودة بالفعل في startlens)
        startLens.toggleWindow();
    });

    // ============================================================
    // 5. أيقونة العلبة (System Tray)
    // ============================================================
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(QIcon::fromTheme("system-search"));
    trayIcon->setToolTip("Start Lens - Application Launcher (Super+Space)");
    trayIcon->show();

    QMenu *trayMenu = new QMenu();
    QAction *quitAction = trayMenu->addAction("Exit");
    QObject::connect(quitAction, &QAction::triggered, &app, &QApplication::quit);
    trayIcon->setContextMenu(trayMenu);

    // ربط أيقونة العلبة بنفس دالة التبديل
    QObject::connect(trayIcon, &QSystemTrayIcon::activated,
                     [&startLens](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
            startLens.toggleWindow();   // استخدام الدالة الموجودة
        }
    });

    // ============================================================
    // 6. تشغيل التطبيق
    // ============================================================
    return app.exec();
}
