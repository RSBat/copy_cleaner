#ifndef SAMEFILESMODEL_H
#define SAMEFILESMODEL_H

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include <QAbstractItemModel>
#include <QByteArray>
#include <QVector>
#include <QMap>
#include <QThread>
#include <QAtomicInt>

struct FileEntry;

struct Node {
    QVector<Node*> children;
    QString name;
    QByteArray hash;
    Node* parent;
    bool isFile;

    Node() {}
    Node(QString const& name, QByteArray const& hash) : name(name), hash(hash), parent(nullptr), isFile(true) {}
};


class HashingWorker : public QObject {
    Q_OBJECT
public:
    HashingWorker();
    ~HashingWorker();

    void stop_scan();

public slots:
    void process(QString const& directory);

signals:
    void file_processed(Node* file);
    void scan_ended();

private:
    QAtomicInt interrupt_flag;
};

class SameFilesModel :public QAbstractItemModel
{
    Q_OBJECT
public:
    SameFilesModel();
    ~SameFilesModel();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

public slots:
    void add_file(Node* file);
    void no_more_files();
    void start_scan(QString const& directory);
    void stop_scan();

signals:
    void scan_directory(QString const& directory);
    void scan_started();
    void scan_ended(int files_scanned);
    void scan_update(int files_scanned);

private:
    QMap<QByteArray, int> hash_to_id;
    QVector<Node*> grouped_files;
    QThread worker_thread;
    HashingWorker* worker;

    Node* unique_group;
    QMap<QByteArray, int> unique_id;

    qint64 maxTime;
    int total_files;
};

#endif // SAMEFILESMODEL_H
