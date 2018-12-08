#include "samefilesmodel.h"

#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>

SameFilesModel::SameFilesModel() : QAbstractItemModel(nullptr), worker_thread() {
    HashingWorker* worker = new HashingWorker();
    worker->moveToThread(&worker_thread);
    connect(this, &SameFilesModel::scan_directory, worker, &HashingWorker::process);
    connect(worker, &HashingWorker::file_processed, this, &SameFilesModel::add_file);
    worker_thread.start();

    emit scan_directory("/home/rsbat/Downloads");
}

SameFilesModel::~SameFilesModel() {

}

QVariant SameFilesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    auto ptr = static_cast<Node*>(index.internalPointer());
    if (ptr->isFile) {
        return ptr->name;
    } else {
        return QString::fromStdString(std::to_string(ptr->children.size()) + " same files");
    }
}

QVariant SameFilesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation != Qt::Horizontal) {
        return QVariant();
    }

    return "Name";
}

QModelIndex SameFilesModel::index(int row, int column, const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return createIndex(row, column, grouped_files[row]);
    }

    auto ptr = static_cast<Node*>(parent.internalPointer());
    return createIndex(row, column, ptr->children[row]);
}

QModelIndex SameFilesModel::parent(const QModelIndex &index) const {
    auto ptr = static_cast<Node*>(index.internalPointer());
    auto parent_ptr = ptr->parent;

    if (parent_ptr == nullptr) {
        return QModelIndex();
    }

    int parents_row = hash_to_id[parent_ptr->hash];
    return createIndex(parents_row, 0, grouped_files[parents_row]);
}

int SameFilesModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return grouped_files.size();
    }
    return static_cast<Node*>(parent.internalPointer())->children.size();
}

int SameFilesModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

void SameFilesModel::add_file(Node* file) {

    auto pos = hash_to_id.find(file->hash);
    int parent_pos;
    Node* group;

    if (pos == hash_to_id.end()) {
        group = new Node(file->hash, file->hash);
        group->isFile = false;
        parent_pos = grouped_files.size();
        hash_to_id[file->hash] = parent_pos;

        beginInsertRows(QModelIndex(), parent_pos, parent_pos);
        grouped_files.push_back(group);
        endInsertRows();
    } else {
        parent_pos = pos.value();
        group = grouped_files[parent_pos];
    }

    file->parent = group;
    beginInsertRows(index(parent_pos, 0, QModelIndex()), parent_pos, parent_pos);
    group->children.push_back(file);
    endInsertRows();
}

HashingWorker::HashingWorker() : interrupt_flag(0) {

}

HashingWorker::~HashingWorker() {

}

void HashingWorker::process(QString const& directory) {
    interrupt_flag = 0;

    QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);
    QDirIterator it(directory, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto name = it.next();
        QFile file(name);
        file.open(QIODevice::ReadOnly);

        hash.reset();
        hash.addData(&file);
        auto file_hash = hash.result().toHex();

        Node* file_node = new Node(name, file_hash);
        if (interrupt_flag == 0) {
            emit file_processed(file_node);
        } else {
            delete file_node;
            break;
        }
    }
}
