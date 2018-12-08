#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "samefilesmodel.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeView->setModel(new SameFilesModel());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::set_progress_complete() {
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);
}

void MainWindow::set_progress_working() {
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
}
