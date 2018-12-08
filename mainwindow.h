#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void set_progress_complete(int count);
    void set_progress_working();
    void set_progress_update(int count);
    void click_start();
    void click_stop();

signals:
    void scan_directory(QString const& directory);
    void abort_scan();

private:
    Ui::MainWindow *ui;
    QLabel* total_label;
};

#endif // MAINWINDOW_H
