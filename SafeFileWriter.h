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
#ifndef SAFEFILEWRITER_H
#define SAFEFILEWRITER_H

#include <QFile>
#include <QLockFile>
#include <QDir>
#include <QDebug>

namespace SafeFileWriter {

inline bool writeAtomic(const QString &filePath, const QByteArray &data) {
    QString lockPath = filePath + ".lock";
    QString tempPath = filePath + ".tmp";

    QLockFile lock(lockPath);
    lock.setStaleLockTime(5000);

    if (!lock.tryLock(3000)) {
        qWarning() << "SafeFileWriter: Cannot acquire lock for" << filePath;
        return false;
    }

    QFile tempFile(tempPath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "SafeFileWriter: Cannot open temp file" << tempPath;
        lock.unlock();
        return false;
    }
    tempFile.write(data);
    tempFile.close();

    QFile::remove(filePath);
    if (!QFile::rename(tempPath, filePath)) {
        qWarning() << "SafeFileWriter: Cannot rename temp file to" << filePath;
        QFile::remove(tempPath);
        lock.unlock();
        return false;
    }

    lock.unlock();
    return true;
}

} // namespace SafeFileWriter

#endif // SAFEFILEWRITER_H
