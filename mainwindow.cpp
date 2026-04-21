#include "mainwindow.h"
#include "GLWidget.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    GLWidget *glWidget = new GLWidget(this);

    ui->multiTexCheck->setChecked(glWidget->isMultiTexture());
    ui->anisoSlider->setValue(glWidget->getAnisotropy());

    glWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ui->verticalLayout->insertWidget(0, glWidget, 1);

    // Button for two modes
    connect(ui->multiTexCheck, &QCheckBox::toggled, [=](bool checked){
        glWidget->setMultiTexture(checked);
    });

    connect(ui->anisoSlider, &QSlider::valueChanged, [=](int value){
        glWidget->setAnisotropy(static_cast<float>(value));
    });
}


MainWindow::~MainWindow()
{
    delete ui;
}
