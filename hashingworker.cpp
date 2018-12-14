#include "hashingworker.h"

#include <QDirIterator>

HashingWorker::HashingWorker(QObject *parent) : interrupt_flag(0), hash(QCryptographicHash::Algorithm::Sha256) {}

HashingWorker::~HashingWorker() {}

void HashingWorker::process(QString const& directory) {
    interrupt_flag = 0;

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
            return;
        }
    }

    emit scan_ended();
}

void HashingWorker::stop_scan() {
    interrupt_flag = 1;
}
