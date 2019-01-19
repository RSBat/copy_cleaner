#include "indexingworker.h"

#include <QDirIterator>
#include <QFile>
#include <QVector>
#include <QDataStream>
#include <QThread>

IndexingWorker::IndexingWorker() : watcher() {
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &IndexingWorker::getModification);
}

void IndexingWorker::watch(QString dir) {
    connect(&watcher, &QFileSystemWatcher::fileChanged, this, &IndexingWorker::getModification);
    connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &IndexingWorker::getDirModification);

    QVector<FileData> files;
    QDirIterator iter(dir, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (iter.hasNext()) {
        auto name = iter.next();

        files.push_back(indexFile(name));
        watcher.addPath(name);
        watcher.addPath(QFileInfo(name).canonicalPath());

        if (thread()->isInterruptionRequested()) { break; }
    }

    watcher.addPath(dir);

    emit filesModified(files);
    emit finishedIndexing();
}

void IndexingWorker::getModification(QString name) {
    watcher.addPath(name);

    QVector<FileData> files;
    files.push_back(indexFile(name));

    emit filesModified(files);
}

void IndexingWorker::getDirModification(QString dir) {
    watcher.addPath(dir);

    QDirIterator iter(dir, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::NoIteratorFlags);
    while (iter.hasNext()) {
        auto name = iter.next();

        if (watcher.addPath(name)) {
            QVector<FileData> files;
            files.push_back(indexFile(name));

            emit filesModified(files);
        }
    }
}

FileData IndexingWorker::indexFile(QString name) {
    QFile file(name);

    if (file.open(QFile::ReadOnly)) {
        QSet<quint32> trigrams;
        QDataStream ins(&file);

        quint8 buf[3];
        quint8 ch;
        while (!ins.atEnd()) {
            ins >> ch;
            buf[0] = buf[1];
            buf[1] = buf[2];
            buf[2] = ch;

            quint32 trig = static_cast<quint32>(buf[0] << 16) + (buf[1] << 8) + buf[2];
            trigrams.insert(trig);

            if (trigrams.size() > MAGIC_SIZE) { break; }
            if (thread()->isInterruptionRequested()) { return FileData(name); }
        }

        if (trigrams.size() <= MAGIC_SIZE) {
            return FileData(name, trigrams);
        } else {
            return FileData(name);
        }
    } else {
        return FileData(name);
    }
}

