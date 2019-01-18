#ifndef SEARCHINGWORKER_H
#define SEARCHINGWORKER_H

#include "indexingworker.h"

#include <QObject>

class SearchingWorker : public QObject
{
    Q_OBJECT
public:
    explicit SearchingWorker(QObject *parent = nullptr);

signals:
    void filesModified(QVector<FileData>);
    void finishedSearching();

public slots:
    void search(QString query, QVector<FileData> data);
};

#endif // SEARCHINGWORKER_H
