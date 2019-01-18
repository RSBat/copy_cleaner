#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "fileindexingmodel.h"

#include <QDesktopServices>
#include <QFileInfo>
#include <QUrl>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    isScanning(false),
    ui(new Ui::MainWindow),
    no_directory_message(new QErrorMessage(this))
{
    ui->setupUi(this);

    // base model
    model = new FileIndexingModel(this);
    connect(model, &FileIndexingModel::finishedIndexing, this, &MainWindow::set_progress_complete);
    connect(model, &FileIndexingModel::finishedSearching, this, &MainWindow::set_progress_complete);

    // filters
    QSortFilterProxyModel *proxyFound = new QSortFilterProxyModel(this);
    proxyFound->setSourceModel(model);
    proxyFound->setFilterFixedString("Found");
    proxyFound->setFilterKeyColumn(1);
    ui->view_found->setModel(proxyFound);
    ui->view_found->header()->hideSection(1);

    QSortFilterProxyModel *proxyNotFound = new QSortFilterProxyModel(this);
    proxyNotFound->setSourceModel(model);
    proxyNotFound->setFilterFixedString("Not found");
    proxyNotFound->setFilterKeyColumn(1);
    ui->view_not_found->setModel(proxyNotFound);
    ui->view_not_found->header()->hideSection(1);

    QSortFilterProxyModel *proxyNotIndexed = new QSortFilterProxyModel(this);
    proxyNotIndexed->setSourceModel(model);
    proxyNotIndexed->setFilterFixedString("Not indexed");
    proxyNotIndexed->setFilterKeyColumn(1);
    ui->view_not_indexed->setModel(proxyNotIndexed);
    ui->view_not_indexed->header()->hideSection(1);

    // progress bar and stuff
    ui->progressBar->reset();

    total_label = new QLabel(statusBar());
    statusBar()->addWidget(total_label);

    // buttons and input
    connect(ui->btn_start, &QPushButton::clicked, this, &MainWindow::click_start_index);
    connect(ui->lineEdit_dir, &QLineEdit::returnPressed, this, &MainWindow::click_start_index);
    connect(ui->btn_stop_index, &QPushButton::clicked, this, &MainWindow::click_stop_index);

    connect(ui->btn_search, &QPushButton::clicked, this, &MainWindow::click_start_search);
    connect(ui->lineEdit_search, &QLineEdit::returnPressed, this, &MainWindow::click_start_search);
    connect(ui->btn_stop_search, &QPushButton::clicked, this, &MainWindow::click_stop_search);

    connect(ui->btn_dir_dialog, &QPushButton::clicked, this, [this](){
        QString tmp = QFileDialog::getExistingDirectory(this, "Directory to scan");
        this->ui->lineEdit_dir->setText(tmp);
    });

    // hack!
    enable_buttons(ModelStatus::IDLE);
    ui->btn_search->setEnabled(false);

    ui->lineEdit_dir->setText(QDir::currentPath());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::enable_buttons(ModelStatus status) {
    bool state = status == ModelStatus::IDLE;
    ui->btn_start->setEnabled(state);
    ui->lineEdit_dir->setEnabled(state);
    ui->btn_dir_dialog->setEnabled(state);
    ui->btn_search->setEnabled(state);

    ui->btn_stop_index->setEnabled(status == ModelStatus::INDEXING);
    ui->btn_stop_search->setEnabled(status == ModelStatus::SEARCHING);
}

void MainWindow::set_progress_complete() {
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(1);
    ui->progressBar->setValue(1);

    enable_buttons(ModelStatus::IDLE);

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
        enable_buttons(ModelStatus::INDEXING);

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

    enable_buttons(ModelStatus::IDLE);

    isScanning = false;
}

void MainWindow::click_start_search() {
    //TODO
    model->search(ui->lineEdit_search->text());

    enable_buttons(ModelStatus::SEARCHING);
}

void MainWindow::click_stop_search() {
    model->stopSearching();
    //TODO
    enable_buttons(ModelStatus::IDLE);
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
