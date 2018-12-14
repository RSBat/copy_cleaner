#ifndef HASHINGWORKER_H
#define HASHINGWORKER_H

#include <QObject>
#include <QCryptographicHash>
#include <QVector>

struct Node {
    QVector<Node*> children;
    QString name;
    QByteArray hash;
    Node* parent;
    bool isFile;

    Node() {}
    Node(QString const& name, QByteArray const& hash) : children(), name(name), hash(hash), parent(nullptr), isFile(true) {}
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

signals:
    void file_processed(Node* file);
    void scan_ended();

private:
    QAtomicInt interrupt_flag;
    QCryptographicHash hash;
};

#endif // HASHINGWORKER_H
