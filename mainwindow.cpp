#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "samefilesmodel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    total_label(new QLabel)
{
    ui->setupUi(this);
    SameFilesModel* model = new SameFilesModel();
    ui->treeView->setModel(model);
    connect(model, &SameFilesModel::scan_started, this, &MainWindow::set_progress_working);
    connect(model, &SameFilesModel::scan_ended, this, &MainWindow::set_progress_complete);
    connect(model, &SameFilesModel::scan_update, this, &MainWindow::set_progress_update);
    connect(this, &MainWindow::scan_directory, model, &SameFilesModel::start_scan);
    connect(this, &MainWindow::abort_scan, model, &SameFilesModel::stop_scan);

    ui->progressBar->reset();
    statusBar()->addWidget(total_label);

    connect(ui->btn_start, &QPushButton::clicked, this, &MainWindow::click_start);
    connect(ui->btn_stop, &QPushButton::clicked, this, &MainWindow::click_stop);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_progress_complete(int count) {
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);

    total_label->setText("Files scanned: " + QString::number(count));
}

void MainWindow::set_progress_working() {
}

void MainWindow::set_progress_update(int count) {
    total_label->setText("Files scanned: " + QString::number(count));
}

void MainWindow::click_start() {
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);

    total_label->setText("Files scanned: 0");

    emit scan_directory(ui->lineEdit->text());
}

void MainWindow::click_stop() {
    emit abort_scan();
    ui->progressBar->setMaximum(1); // otherwise reset won't work
    ui->progressBar->reset();
}
