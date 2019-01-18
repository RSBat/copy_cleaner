#include "searchingworker.h"

#include <QVector>
#include <QByteArray>

SearchingWorker::SearchingWorker(QObject *parent) : QObject(parent) {

}

void SearchingWorker::search(QString query, QVector<FileData> data) {
    QByteArray arr(query.toUtf8());
    for (auto& file : data) {
        if (!file.indexed) { file.found = false; continue; }
        bool ok = true;

        quint8 buf[3];
        buf[1] = static_cast<quint8>(arr[0]);
        buf[2] = static_cast<quint8>(arr[1]);
        for (int i = 2; i < arr.size(); i++) {
            buf[0] = buf[1];
            buf[1] = buf[2];
            buf[2] = static_cast<quint8>(arr[i]);

            quint32 trig = static_cast<quint32>(buf[0] << 16) + (buf[1] << 8) + buf[2];

            if (!file.trigrams.contains(trig)) {
                ok = false;
                break;
            }
        }

        if (ok) {
            //TODO make search
            file.found = true;
        } else {
            file.found = false;
        }
    }

    emit filesModified(data);
    emit finishedSearching();
}
