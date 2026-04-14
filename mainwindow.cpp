#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "MeshObjFile.h"

MainWindow::MainWindow(MeshObjFile& model, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    model.loadFile();

    QString info = QString("Загружен куб!\nВершин: %1").arg(model.getVertices().size());
    ui->plainTextEdit->setPlainText(info);
}

MainWindow::~MainWindow()
{
    delete ui;
}
