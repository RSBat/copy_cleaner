#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fileindexingmodel.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>
#include <QFileDialog>
#include <QFileSystemModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    isScanning(false),
    ui(new Ui::MainWindow),
    no_directory_message(new QErrorMessage(this))
{
    ui->setupUi(this);

    model = new FileIndexingModel(this);
    connect(model, &FileIndexingModel::finishedIndexing, this, &MainWindow::set_progress_complete);
    ui->view_not_found->setModel(model);

    ui->progressBar->reset();

    total_label = new QLabel(statusBar());
    statusBar()->addWidget(total_label);

    connect(ui->btn_start, &QPushButton::clicked, this, &MainWindow::click_start_index);
    connect(ui->lineEdit_dir, &QLineEdit::returnPressed, this, &MainWindow::click_start_index);
    connect(ui->btn_stop_index, &QPushButton::clicked, this, &MainWindow::click_stop_index);

    connect(ui->btn_dir_dialog, &QPushButton::clicked, this, [this](){
        QString tmp = QFileDialog::getExistingDirectory(this, "Directory to scan");
        this->ui->lineEdit_dir->setText(tmp);
    });

    ui->btn_stop_index->setEnabled(false);

    ui->lineEdit_dir->setText(QDir::currentPath());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::enable_buttons(bool state) {
    ui->btn_start->setEnabled(state);
    ui->lineEdit_dir->setEnabled(state);
    ui->btn_dir_dialog->setEnabled(state);

    ui->btn_stop_index->setEnabled(!state);
}

void MainWindow::set_progress_complete() {
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);

    enable_buttons(true);

    //TODO
    //total_label->setText("Files scanned: " + QString::number(count));

    isScanning = false;
}

void MainWindow::set_progress_update(int count) {
    total_label->setText("Files scanned: " + QString::number(count));
}

void MainWindow::click_start_index() {
    QString dir = ui->lineEdit_dir->text();

    if (QDir(dir).exists()) {
        ui->progressBar->setMinimum(0);
        ui->progressBar->setMaximum(0);

        total_label->setText("Files scanned: 0");
        enable_buttons(false);

        model->setDir(ui->lineEdit_dir->text());
        isScanning = true;
    } else {
        no_directory_message->showMessage("No such directory");
    }
}

void MainWindow::click_stop_index() {
    model->stopIndexing();

    ui->progressBar->setMaximum(1); // otherwise reset won't work
    ui->progressBar->reset();

    enable_buttons(true);

    isScanning = false;
}

void MainWindow::click_start_search() {
    //TODO
}

void MainWindow::click_stop_search() {
    //TODO
}

void MainWindow::getContextMenu(QPoint const& pos) {/*
    QModelIndex index = ui->view_found->indexAt(pos);
    Node* ptr = static_cast<Node*>(index.internalPointer());
    if (!ptr->isFile) { return; }

    QString filename = ptr->name;

    QMenu* menu = new QMenu(ui->view_found);
    QAction* act_open = menu->addAction("Open file");
    connect(act_open, &QAction::triggered, this, [filename](){
        QDesktopServices::openUrl(QUrl(filename));
    });

    QAction* act_folder = menu->addAction("Open folder");
    connect(act_folder, &QAction::triggered, this, [filename](){
        QFileInfo info(filename);
        QDesktopServices::openUrl(QUrl(info.absolutePath())); // or info.path() because filename includes absolute path
    });

    QAction* act_delete = menu->addAction("Delete file");
    connect(act_delete, &QAction::triggered, this, [this, index]() {
        emit this->delete_file(index);
    });


    QAction* act_delete_same = menu->addAction("Delete same except this");
    connect(act_delete_same, &QAction::triggered, this, [this, index]() {
        emit this->delete_same(index);
    });

    if (isScanning) {
        act_delete->setEnabled(false);
        act_delete_same->setEnabled(false);
    }

    menu->exec(ui->view_found->mapToGlobal(pos));*/
}
