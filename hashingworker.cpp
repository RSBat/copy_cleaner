#include "hashingworker.h"

#include <QDirIterator>

HashingWorker::HashingWorker(QObject *parent) : interrupt_flag(0), hash(QCryptographicHash::Algorithm::Sha1), files_with_errors(-1) {}

HashingWorker::~HashingWorker() {}

void HashingWorker::process(QString const& directory) {
    interrupt_flag = 0;
    files_with_errors = -1;

    QDirIterator it(directory, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        auto name = it.next();
        QFile file(name);

        Node* file_node = new Node(name, file.size());

        if (!(file.permissions() & QFileDevice::Permission::ReadUser)) {
            file_node->hash = QString::number(files_with_errors).toUtf8();
            file_node->hasHash = true;
            files_with_errors--;
        }

        if (interrupt_flag == 0) {
            emit file_processed(file_node);
        } else {
            delete file_node;
            return;
        }
    }

    emit scan_ended();
}

void HashingWorker::calc_hash(Node* file_node) {
    if (interrupt_flag == 1) {
        delete file_node;
        return;
    }

    QFile file(file_node->name);
    file.open(QIODevice::ReadOnly);

    //auto file_hash = ("_" + QString::number(file.size())).toUtf8();//hash.result().toHex();

    hash.reset();
    hash.addData(&file);
    auto file_hash = hash.result().toHex();

    file_node->hash = file_hash;
    file_node->hasHash = true;
    if (interrupt_flag == 0) {
        emit file_processed(file_node);
    } else {
        delete file_node;
        return;
    }
}

void HashingWorker::stop_scan() {
    interrupt_flag = 1;
}
