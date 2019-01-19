#include "searchingworker.h"

#include <QVector>
#include <QByteArray>
#include <QThread>
#include <QFile>
#include <QDataStream>

SearchingWorker::SearchingWorker(QObject *parent) : QObject(parent) {

}

void SearchingWorker::search(QString query, QVector<FileData> data) {
    QByteArray arr(query.toUtf8());
    QVector<quint32> pref(arr.size());

    for (int i = 1; i < arr.size(); i++) {
        quint32 j = pref[i - 1];
        while (j > 0 && arr[i] != arr[j]) {
            j = pref[j - 1];
        }
        if (arr[i] == arr[j]) { ++j; }
        pref[i] = j;
    }

    for (auto& fileData : data) {
        fileData.searched = true;
        if (!fileData.indexed) { fileData.found = false; continue; }
        bool ok = true;

        quint8 buf[3];
        buf[1] = static_cast<quint8>(arr[0]);
        buf[2] = static_cast<quint8>(arr[1]);
        for (int i = 2; i < arr.size(); i++) {
            buf[0] = buf[1];
            buf[1] = buf[2];
            buf[2] = static_cast<quint8>(arr[i]);

            quint32 trig = static_cast<quint32>(buf[0] << 16) + (buf[1] << 8) + buf[2];

            if (!fileData.trigrams.contains(trig)) {
                ok = false;
                break;
            }

            if (thread()->isInterruptionRequested()) {
                ok = false;
                break;
            }
        }

        if (ok) {
            ok = false;

            QFile file(fileData.name);
            if (file.open(QFile::ReadOnly)) {
                quint32 j = 0;
                QDataStream in(&file);

                while (!in.atEnd()) {
                    quint8 ch;
                    in >> ch;

                    while (j > 0 && ch != arr[j]) {
                        j = pref[j - 1];
                    }
                    if (ch == arr[j]) { ++j; }

                    if (j == arr.size()) { ok = true; break; }
                }
            }
        }

        fileData.found = ok;

        if (thread()->isInterruptionRequested()) {
            break;
        }
    }

    emit filesModified(data);
    emit finishedSearching();
}
