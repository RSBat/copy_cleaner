#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <fileindexingmodel.h>

#include <QMainWindow>
#include <QLabel>
#include <QErrorMessage>
#include <QSortFilterProxyModel>

namespace Ui {
class MainWindow;
}

enum class ModelStatus
{
    INDEXING, SEARCHING, IDLE, READY
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void set_progress_complete();
    void set_progress_update(int count);
    void click_start_index();
    void click_stop_index();
    void click_start_search();
    void click_stop_search();
    void getContextMenu(QPoint const& pos);

private:
    Ui::MainWindow *ui;
    QLabel* total_label;
    QErrorMessage* no_directory_message;

    FileIndexingModel* model;

    void enableButtons(ModelStatus status);
    QSortFilterProxyModel* makeProxy(QString filter);
};

#endif // MAINWINDOW_H
