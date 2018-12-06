#include "samefilesmodel.h"

#include <QCryptographicHash>

SameFilesModel::SameFilesModel() : QAbstractItemModel(nullptr) {
    QCryptographicHash hash(QCryptographicHash::Algorithm::Sha256);

    hash.addData("hello", 5);
    Node* group = new Node(hash.result().toHex(), nullptr);
    group->children.push_back(new Node("file #1", group));
    group->children.push_back(new Node("file #2", group));
    group->children.push_back(new Node("file #3", group));
    grouped_files.push_back(group);

    hash.reset();
    hash.addData("world", 5);
    Node* group2 = new Node(hash.result().toHex(), nullptr);
    group2->children.push_back(new Node("file #1", group2));
    group2->children.push_back(new Node("file #2", group2));
    group2->children.push_back(new Node("file #3", group2));
    grouped_files.push_back(group2);
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
    return QString();
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

    int parents_row = hash_to_id[ptr->name];
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
