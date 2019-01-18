#ifndef FILEINDEXINGMODEL_H
#define FILEINDEXINGMODEL_H

#include "indexingworker.h"
#include "searchingworker.h"

#include <QAbstractListModel>
#include <QMap>
#include <QSet>
#include <QVector>
#include <QThread>

class FileIndexingModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FileIndexingModel(QObject *parent = nullptr);
    ~FileIndexingModel();

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void setDir(QString dir);
    void stopIndexing();

    void search(QString query);
    void stopSearching();

private:
    QVector<FileData> files;
    QString root_dir;
    QString query;

    IndexingWorker* indexingWorker;
    SearchingWorker* searchingWorker;

    QThread* indexing_thread;
    QThread* searching_thread;

signals:
    void start_indexing(QString dir);
    void start_search(QString query, QVector<FileData> data);

    void finishedIndexing();
    void finishedSearching();

private slots:
    void add_files(QVector<FileData> data);
};

#endif // FILEINDEXINGMODEL_H
