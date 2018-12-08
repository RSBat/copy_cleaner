#include "samefilesmodel.h"

#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QElapsedTimer>

#include <iostream>

SameFilesModel::SameFilesModel() : QAbstractItemModel(nullptr), worker_thread(), unique_group(new Node("Unique files", QByteArray())), maxTime(0), total_files(0) {
    worker = new HashingWorker();
    worker->moveToThread(&worker_thread);
    connect(this, &SameFilesModel::scan_directory, worker, &HashingWorker::process);
    connect(worker, &HashingWorker::file_processed, this, &SameFilesModel::add_file);
    connect(worker, &HashingWorker::scan_ended, this, &SameFilesModel::no_more_files);
    worker_thread.start();
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
        return QString::number(ptr->children.size()) + " same files";
    }
}

QVariant SameFilesModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

QModelIndex SameFilesModel::index(int row, int column, const QModelIndex &parent) const {
    if (!parent.isValid()) {
        if (row == grouped_files.size()) {
            return createIndex(row, column, unique_group);
        } else {
            return createIndex(row, column, grouped_files[row]);
        }
    }

    auto parent_ptr = static_cast<Node*>(parent.internalPointer());
    return createIndex(row, column, parent_ptr->children[row]);
}

QModelIndex SameFilesModel::parent(const QModelIndex &index) const {
    auto ptr = static_cast<Node*>(index.internalPointer());
    auto parent_ptr = ptr->parent;

    if (parent_ptr == nullptr) {
        return QModelIndex();
    }

    if (parent_ptr == unique_group) {
        return createIndex(grouped_files.size(), 0, unique_group);
    } else {
        int parents_row = hash_to_id[parent_ptr->hash];
        return createIndex(parents_row, 0, grouped_files[parents_row]);
    }
}

int SameFilesModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return grouped_files.size() + 1;
    }
    return static_cast<Node*>(parent.internalPointer())->children.size();
}

int SameFilesModel::columnCount(const QModelIndex &parent) const {
    return 1;
}

void SameFilesModel::add_file(Node* file) {
    // for DEBUG
    QElapsedTimer timer;
    timer.start();

    auto pos = hash_to_id.find(file->hash);
    int parent_pos;
    Node* group;

    if (pos == hash_to_id.end()) {
        auto unique_pos = unique_id.find(file->hash);
        if (unique_pos == unique_id.end()) {
            group = unique_group;
            parent_pos = grouped_files.size();

            unique_id[file->hash] = unique_group->children.size();
        } else {
            group = new Node(file->hash, file->hash);
            group->isFile = false;
            parent_pos = grouped_files.size();
            hash_to_id[file->hash] = parent_pos;

            beginInsertRows(QModelIndex(), parent_pos, parent_pos);
            grouped_files.push_back(group);
            endInsertRows();

            // move file from unique to group
            // some files in front of us could be deleted, need to recalculate position
            int old_file_pos = unique_pos.value();
            if (old_file_pos >= unique_group->children.size()) { old_file_pos = unique_group->children.size() - 1; }
            while (unique_group->children[old_file_pos]->hash != file->hash) { old_file_pos--; }
            std::cerr << unique_pos.value() - old_file_pos << std::endl; // for DEBUG

            Node* old_file = unique_group->children[old_file_pos];
            old_file->parent = group;
            beginRemoveRows(index(grouped_files.size(), 0, QModelIndex()), old_file_pos, old_file_pos);
            unique_group->children.erase(unique_group->children.begin() + old_file_pos);
            unique_id.erase(unique_pos);
            endRemoveRows();

            beginInsertRows(index(parent_pos, 0, QModelIndex()), 0, 0);
            group->children.push_back(old_file);
            endInsertRows();
        }
    } else {
        parent_pos = pos.value();
        group = grouped_files[parent_pos];
    }

    file->parent = group;
    beginInsertRows(index(parent_pos, 0, QModelIndex()), parent_pos, parent_pos);
    group->children.push_back(file);
    endInsertRows();

    total_files++;
    emit scan_update(total_files);

    // for DEBUG
    auto timed = timer.nsecsElapsed();
    if (timed > maxTime) {
        maxTime = timed;
        std::cerr << maxTime << std::endl;
    }
}

void SameFilesModel::no_more_files() {
    emit scan_ended(total_files);
}

void SameFilesModel::start_scan(QString const& directory) {
    beginResetModel();
    for (auto ptr: unique_group->children) {
        delete ptr;
    }
    unique_group->children.clear();
    unique_id.clear();

    for (auto ptr: grouped_files) {
        delete ptr;
    }
    grouped_files.clear();
    hash_to_id.clear();

    total_files = 0;
    endResetModel();

    emit scan_directory(directory);
    emit scan_started();
}

void SameFilesModel::stop_scan() {
    worker->stop_scan();
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
            std::cerr << "Worker stopped" << std::endl;
            delete file_node;
            return;
        }
    }

    emit scan_ended();
}

void HashingWorker::stop_scan() {
    interrupt_flag = 1;
}
