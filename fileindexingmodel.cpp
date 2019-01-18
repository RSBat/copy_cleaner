#include "fileindexingmodel.h"

FileIndexingModel::FileIndexingModel(QObject *parent)
    : QAbstractListModel(parent), files(), root_dir(), query(), indexingWorker(nullptr), searchingWorker(nullptr), indexing_thread(nullptr), searching_thread(nullptr) {
    qRegisterMetaType<QVector<FileData>>();
}

FileIndexingModel::~FileIndexingModel() {
    if (indexing_thread != nullptr) {
        indexing_thread->quit();
        delete indexingWorker;
        delete indexing_thread;
    }

    if (searching_thread != nullptr) {
        searching_thread->quit();
        delete searchingWorker;
        delete searching_thread;
    }
}

QVariant FileIndexingModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    if (section == 0) {
        return "Name";
    } else {
        return QVariant();
    }
}

int FileIndexingModel::rowCount(const QModelIndex &parent) const {
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (parent.isValid())
        return 0;

    return files.size();
}

int FileIndexingModel::columnCount(const QModelIndex &parent) const {
    return 2;
}

QVariant FileIndexingModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (index.column() == 0) {
        return files[index.row()].name;
    } else {
        if (files[index.row()].indexed) {
            if (files[index.row()].found) {
                return "Found";
            } else {
                return "Not found";
            }
        } else {
            return "Not indexed";
        }
    }
}

void FileIndexingModel::setDir(QString dir) {
    stopIndexing();

    beginResetModel();
    files.clear();
    endResetModel();

    indexing_thread = new QThread();
    indexingWorker = new IndexingWorker();
    indexingWorker->moveToThread(indexing_thread);

    connect(this, &FileIndexingModel::start_indexing, indexingWorker, &IndexingWorker::watch);
    connect(indexingWorker, &IndexingWorker::filesModified, this, &FileIndexingModel::add_files);
    connect(indexingWorker, &IndexingWorker::finishedIndexing, this, [this](){ emit finishedIndexing(); });

    indexing_thread->start();
    emit start_indexing(dir);
}

void FileIndexingModel::stopIndexing() {
    if (indexing_thread != nullptr) {
        indexing_thread->requestInterruption();
        indexing_thread->quit();
        indexing_thread->wait();
        delete indexingWorker;
        delete indexing_thread;

        indexingWorker = nullptr;
        indexing_thread = nullptr;
    }
}

void FileIndexingModel::search(QString query) {
    stopSearching();

    beginResetModel();
    for (auto& file: files) {
        file.found = false;
    }
    endResetModel();

    searching_thread = new QThread();
    searchingWorker = new SearchingWorker();
    searchingWorker->moveToThread(searching_thread);

    connect(this, &FileIndexingModel::start_search, searchingWorker, &SearchingWorker::search);
    connect(searchingWorker, &SearchingWorker::filesModified, this, &FileIndexingModel::add_files);
    connect(searchingWorker, &SearchingWorker::finishedSearching, this, [this](){ emit finishedSearching(); });

    searching_thread->start();
    emit start_search(query, files);
}

void FileIndexingModel::stopSearching() {
    if (searching_thread != nullptr) {
        searching_thread->requestInterruption();
        searching_thread->quit();
        searching_thread->wait();
        delete searchingWorker;
        delete searching_thread;

        searchingWorker = nullptr;
        searching_thread = nullptr;
    }
}

void FileIndexingModel::add_files(QVector<FileData> data) {
    if (data.empty() || data.size() > 1) {
        beginResetModel();
        files = data;
        endResetModel();
    } else {
        auto file = data[0];
        for (int i = 0; i < files.size(); i++) {
            if (files[i].name == file.name) {
                files[i] = file;
                emit dataChanged(createIndex(i, 0), createIndex(i, 1));
                return;
            }
        }

        beginInsertRows(QModelIndex(), files.size(), files.size() + 1);
        files.push_back(file);
        endInsertRows();
    }
}
