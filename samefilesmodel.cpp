#include "samefilesmodel.h"

#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>

SameFilesModel::SameFilesModel() : QAbstractItemModel(nullptr) {
    QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);

    QDirIterator it("/home/rsbat/Downloads", QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto name = it.next();
        QFile file(name);
        file.open(QIODevice::ReadOnly);

        hash.reset();
        hash.addData(&file);
        auto file_hash = hash.result().toHex();

        auto pos = hash_to_id.find(file_hash);
        if (pos == hash_to_id.end()) {
            Node* group = new Node(file_hash, nullptr);
            Node* file_node = new Node(name.toUtf8(), group);
            group->children.push_back(file_node);

            hash_to_id[file_hash] = grouped_files.size();
            grouped_files.push_back(group);
        } else {
            Node* group = grouped_files[pos.value()];
            Node* file_node = new Node(name.toUtf8(), group);
            group->children.push_back(file_node);
        }
    }
}

SameFilesModel::~SameFilesModel() {

}

QVariant SameFilesModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    auto ptr = static_cast<Node*>(index.internalPointer());
    return QString(ptr->name);
}

QVariant SameFilesModel::headerData(int section, Qt::Orientation orientation, int role) const {
    return QVariant();
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

    int parents_row = hash_to_id[parent_ptr->name];
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
