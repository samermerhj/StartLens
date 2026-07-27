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
#ifndef QHOTKEY_H
#define QHOTKEY_H

#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>

class QHotkey : public QObject
{
    Q_OBJECT
public:
    explicit QHotkey(QObject *parent = nullptr);
    ~QHotkey();

    bool registerShortcut(const QKeySequence &shortcut);
    void unregisterShortcut();

signals:
    void activated();

private:
    class Private;
    Private *d;
};

#endif
