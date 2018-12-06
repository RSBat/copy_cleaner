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
