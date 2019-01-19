#ifndef INDEXINGWORKER_H
#define INDEXINGWORKER_H

#include <QObject>
#include <QSet>
#include <QFileSystemWatcher>

struct FileData {
    QSet<quint32> trigrams;
    QString name;
    bool indexed;
    bool found;
    bool searched;

    FileData() : trigrams(), name(), indexed(false), found(false), searched(false) {}
    explicit FileData(QString name) : trigrams(), name(name), indexed(false), found(false), searched(false) {}
    FileData(QString name, QSet<quint32> trigrams) : trigrams(trigrams), name(name), indexed(true), found(false), searched(false) {}
};

Q_DECLARE_METATYPE(FileData);

class IndexingWorker : public QObject {
    Q_OBJECT
public:
    IndexingWorker();

public slots:
    void watch(QString dir);

signals:
    void filesModified(QVector<FileData>);
    void finishedIndexing();

private:
    static constexpr int MAGIC_SIZE = 200000;

    QFileSystemWatcher watcher;

    FileData indexFile(QString name);

private slots:
    void getModification(QString name);
    void getDirModification(QString name);
};

#endif // INDEXINGWORKER_H
