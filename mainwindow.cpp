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

    ui->progressBar->reset();
    statusBar()->addWidget(total_label);
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
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);

    total_label->setText("Files scanned: 0");
}

void MainWindow::set_progress_update(int count) {
    total_label->setText("Files scanned: " + QString::number(count));
}
