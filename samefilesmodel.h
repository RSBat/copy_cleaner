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

struct FileEntry;

struct Node {
    QVector<Node*> children;
    QByteArray name;
    Node* parent;

    Node() {}
    Node(QByteArray const& name, Node* parent) : name(name), parent(parent) {}
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

private:
    QMap<QByteArray, int> hash_to_id;
    QVector<Node*> grouped_files;
};

#endif // SAMEFILESMODEL_H
