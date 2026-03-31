#pragma once

#include <QDialog>
#include <QSlider>
#include <QLabel>
#include <QWindow>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QCheckBox>
#include <optional>


class Slider: public QDialog{
    Q_OBJECT
public:
    Slider(QWidget *parent = nullptr): QDialog(parent){}
    int get_value() const { return m_slider->value(); }

signals:
    void valueChanged(double value);

protected:
    QSlider *m_slider;
    QLabel *m_label;
};

class SliderStep : public Slider
{
    Q_OBJECT
public:
    explicit SliderStep(QWidget *parent = nullptr);
};

class SliderAlfa : public Slider
{
    Q_OBJECT

public:
    explicit SliderAlfa(QWidget *parent = nullptr);
};


// another window to get axis of rotation
class InputCoors: public QDialog{
    Q_OBJECT
public:
    enum class Mode { Mode2D, Mode3D };

    explicit InputCoors(Mode mode_, QWidget *parent = nullptr): QDialog(parent), mode(mode_) {}

    bool ask_coors(const QString& info);

    float get_x() const{return m_resultX;}
    float get_y() const{return m_resultY;}
    std::optional<float> get_z() const{return m_resultZ;}

private:
    // x and y musn always be
    QLineEdit* x{};
    QLineEdit* y{};
    std::optional<QLineEdit*> z{};

    float m_resultX = 0;
    float m_resultY = 0;
    std::optional<float> m_resultZ;

    Mode mode{};

    QPushButton *m_okButton{};

    void onOkClicked();
};


class LightSources: public QObject{
    Q_OBJECT
public:
    void set_color();
    void set_pos();
    QVector3D get_color(){
        return color;
    }
    void set_direction();

    void load_image();

    virtual void start_func(){
        set_color();
    }

    void hide(){if(label){label->hide();}}
    void show(){if(label){label->show();}}

    QVector3D get_pos() const{return pos;}
    QVector3D get_direction() const{return direction;}
    QWindow* get_cube_window() const{return m_mainWindow;}

    void setLabelScreenPosition(const QPoint& new_screen_pos){
        if(label){label->move(new_screen_pos);}
    }

    virtual QString who_is() = 0;

protected:
    QVector3D pos{};
    QVector3D color{};
    QVector3D direction{};

    QString path{};

    QLabel* label{};
    QWindow* m_mainWindow;
};

class Lamp: public LightSources{
    Q_OBJECT
public:
    explicit Lamp(QWindow* mainWindow=nullptr){
        m_mainWindow = mainWindow;
        path = ":/new/prefix1/lamp.jpg";
    }

    QString who_is() override{
        return QString("Lamp");
    }

    void start_func() override{
        LightSources::start_func();
        set_pos();
    }
};


class SpotLight: public LightSources{
    Q_OBJECT
public:
    explicit SpotLight(QWindow* mainWindow=nullptr){
        m_mainWindow = mainWindow;
        path = ":/new/prefix1/spotlight.jpg";
    }

    void start_func() override{
        LightSources::start_func();
        set_pos();
        set_direction();
    }

    QString who_is() override{
        return QString("SpotLight");
    }
};

class DirectionalLight: public LightSources{
    Q_OBJECT
public:
    explicit DirectionalLight(QWindow* mainWindow=nullptr){
        m_mainWindow = mainWindow;
        path = ""; // no path, it only has direction
    }

    void start_func() override{
        LightSources::start_func();
        set_direction();
    }

    QString who_is() override{
        return QString("DirectionalLight");
    }
};


