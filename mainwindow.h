#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <fileindexingmodel.h>

#include <QMainWindow>
#include <QLabel>
#include <QErrorMessage>

namespace Ui {
class MainWindow;
}

enum class ModelStatus
{
    INDEXING, SEARCHING, IDLE
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
    bool isScanning;
    Ui::MainWindow *ui;
    QLabel* total_label;
    QErrorMessage* no_directory_message;

    FileIndexingModel* model;

    void enable_buttons(ModelStatus status);
};

#endif // MAINWINDOW_H
