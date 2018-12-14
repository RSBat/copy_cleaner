#ifndef HASHINGWORKER_H
#define HASHINGWORKER_H

#include <QObject>
#include <QCryptographicHash>
#include <QVector>

struct Node {
    QVector<Node*> children;
    QString name;
    QByteArray hash;
    qint64 size;
    Node* parent;
    bool isFile;
    bool hasHash;

    Node() {}
    Node(QString const& name, qint64 const& size) : children(), name(name), hash(QString::number(size).toUtf8()), size(size), parent(nullptr), isFile(true), hasHash(false) {}
    ~Node() {
        for (auto ptr: children) {
            delete ptr;
        }
    }
};

class HashingWorker : public QObject {
    Q_OBJECT
public:
    explicit HashingWorker(QObject *parent = nullptr);
    ~HashingWorker();

    void stop_scan();

public slots:
    void process(QString const& directory);
    void calc_hash(Node* file);

signals:
    void file_processed(Node* file);
    void hash_calculated(Node* file);
    void scan_ended();

private:
    QAtomicInt interrupt_flag;
    QCryptographicHash hash;
};

#endif // HASHINGWORKER_H
