#include "ui_widgets.h"
#include "tools.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QLineEdit>
#include <QColorDialog>

SliderStep::SliderStep(QWidget *parent): QDialog(parent){
    setWindowTitle("Set grid scale");
    setModal(false); // do not block another window

    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(1, 60);
    m_slider->setValue(10);

    m_label = new QLabel("N = 10", this);

    connect(m_slider, &QSlider::valueChanged, this, [this](int val) {
        m_label->setText(QString("N = %1").arg(val));
        emit valueChanged(val);
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_label);
    layout->addWidget(m_slider);
}

SliderAlfa::SliderAlfa(QWidget *parent): QDialog(parent){
    setWindowTitle("Set alfa");
    setModal(false); // do not block another window

    m_slider = new QSlider(Qt::Horizontal);
    m_slider->setRange(0, 100);
    m_slider->setValue(10);

    m_label = new QLabel("alfa = 0", this);

    connect(m_slider, &QSlider::valueChanged, this, [this](int val) {
        float floatValue = val / 100.0f;
        m_label->setText(QString("alfa = %1").arg(val));
        emit valueChanged(floatValue);
    });

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_label);
    layout->addWidget(m_slider);
}


void InputCoors::onOkClicked()
{

    bool xOk = false, yOk = false, zOk = true;

    if (x && !x->text().trimmed().isEmpty()) {
        m_resultX = x->text().trimmed().toFloat(&xOk);
    } else { xOk = false; }

    if (y && !y->text().trimmed().isEmpty()) {
        m_resultY = y->text().trimmed().toFloat(&yOk);
    } else { yOk = false; }

    if (z.has_value() && z.value() && !z.value()->text().trimmed().isEmpty()) {
        m_resultZ = z.value()->text().trimmed().toFloat(&zOk);
    } else if (mode == Mode::Mode3D) {
        zOk = false;
    } else {
        m_resultZ = std::nullopt;
    }

    if (xOk && yOk && zOk) {
        accept();
    } else {
        QMessageBox::warning(this, "Ошибка", "Введите корректные числа!");
    }
}


bool InputCoors::ask_coors(const QString& info){
    if(mode == Mode::Mode3D){setWindowTitle(info);}

    setModal(true);

    auto *mainLayout = new QVBoxLayout();

    auto *layoutX = new QHBoxLayout;
    layoutX->addWidget(new QLabel("X:", this));
    x = new QLineEdit(this);
    layoutX->addWidget(x);

    mainLayout->addLayout(layoutX);

    auto *layoutY = new QHBoxLayout;
    layoutY->addWidget(new QLabel("Y:", this));
    y = new QLineEdit(this);
    layoutY->addWidget(y);
    mainLayout->addLayout(layoutY);

    if (mode == Mode::Mode3D) {
        auto *layoutZ = new QHBoxLayout;
        layoutZ->addWidget(new QLabel("Z:", this));
        QLineEdit *zEdit = new QLineEdit(this);
        layoutZ->addWidget(zEdit);
        mainLayout->addLayout(layoutZ);

        z = zEdit;
    }

    m_okButton = new QPushButton("OK", this);
    m_okButton->setDefault(false);
    m_okButton->setAutoDefault(false);
    mainLayout->addWidget(m_okButton, 0, Qt::AlignCenter);

    setLayout(mainLayout);
    adjustSize();
    setMinimumSize(400, 250);

    connect(m_okButton, &QPushButton::clicked, this, &InputCoors::onOkClicked);
    connect(x, &QLineEdit::returnPressed, this, &InputCoors::onOkClicked);
    connect(y, &QLineEdit::returnPressed, this, &InputCoors::onOkClicked);

    if(z.has_value()){
        connect(z.value(), &QLineEdit::returnPressed, this, &InputCoors::onOkClicked);
    }

    return exec() == QDialog::Accepted;
}

void LightSources::set_direction(){
    auto input_coors = InputCoors(InputCoors::Mode::Mode3D);

    if (input_coors.ask_coors("Направление") && input_coors.get_z().has_value()) {
        direction = QVector3D(input_coors.get_x(), input_coors.get_y(), *input_coors.get_z()).normalized();
    } else {
        direction = QVector3D(1, 1, 1).normalized();
    }
}

void LightSources::set_pos(){
    auto input_coors = InputCoors(InputCoors::Mode::Mode3D);

    qDebug("input coors was created");

    if (input_coors.ask_coors("Позиция") && input_coors.get_z().has_value()) {
        float scale = 2.0f / 200.0f;

        pos = QVector3D(
            (input_coors.get_x() - 50.0f) * scale,
            (input_coors.get_y() - 50.0f) * scale,
            (input_coors.get_z().value_or(50.0f) - 50.0f) * scale
            );
        //pos = QVector3D(input_coors.get_x(), input_coors.get_y(), *input_coors.get_z());
    } else {
        pos = {0.5f, 0.5f, 0.5f};
        //pos = {100, 100, 100};
    }
}

void LightSources::set_color(){
    QColor choice_color = QColorDialog::getColor(Qt::white, nullptr, "Выберите цвет");

    if (choice_color.isValid()) {
        auto r = choice_color.red();
        auto g = choice_color.green();
        auto b = choice_color.blue();

        color = QVector3D(r, g, b);
    } else {
        color = QVector3D(0, 0, 0);
    }
}

void LightSources::load_image(){
    start_func();

    if(!label){
        label = new QLabel(nullptr);
        label->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        label->setAttribute(Qt::WA_TranslucentBackground);
    }

    QPixmap pixmap(path);

    if (!pixmap.isNull()) {
        QPixmap transparentPixmap = makeTransparent(pixmap, Qt::white, 20);
        label->setPixmap(transparentPixmap.scaled(50, 50, Qt::KeepAspectRatio));
    } else {
        label->setText("Не удалось загрузить изображение");
    }
}

