#pragma once

#include "ui_widgets.h"
#include "cubewindow.h"

#include <QCheckBox>

class CubeUIManager: public QObject{
    Q_OBJECT
public:
    explicit CubeUIManager(CubeWindow *window, QObject *parent = nullptr): QObject(parent), m_window(window){}

    void connect_window_hide(QWidget* obj){
        // close, when CubeWindow became invisible
        QObject::connect(m_window, &QWindow::visibleChanged,
                         [this, obj](bool visible) {
                             if (!visible && m_window->isExposed()) {
                                 obj->close();
                             }
        });
    }

    SliderNums* get_nums_slider(QVector2D pos){
        // alfa slider
        auto slider_nums = new SliderNums();
        createSlider(slider_nums, pos);

        // set a new alfa, when the slider will change
        QObject::connect(slider_nums, &SliderNums::valueChanged,
                         m_window, &CubeWindow::set_nums);

        return slider_nums;
    }

    SliderDists* get_dists_slider(QVector2D pos){
        // alfa slider
        auto slider_dists = new SliderDists();
        createSlider(slider_dists, pos);

        // set a new alfa, when the slider will change
        QObject::connect(slider_dists, &SliderNums::valueChanged,
                         m_window, &CubeWindow::set_dists);

        return slider_dists;
    }

    SliderAlfa* get_alfa_slider(QVector2D pos){
        // alfa slider
        SliderAlfa* slider_alfa = new SliderAlfa();
        createSlider(slider_alfa, pos);

        // set a new alfa, when the slider will change
        QObject::connect(slider_alfa, &SliderAlfa::valueChanged,
                         m_window, &CubeWindow::setAlfa);

        return slider_alfa;
    }

    SliderStep* get_step_slider(QVector2D pos){
        // alfa slider
        SliderStep* slider_step = new SliderStep();
        createSlider(slider_step, pos);

        // set new scale of grid, when the slider will change
        QObject::connect(slider_step, &SliderStep::valueChanged,
                         m_window, &CubeWindow::setSubdivision);

        return slider_step;
    }

    void set_axis_rot(){
        // set axis
        InputCoors coors_dialog(InputCoors::Mode::Mode3D);

        if (coors_dialog.ask_coors("Ось")) {
            auto x = coors_dialog.get_x();
            auto y = coors_dialog.get_y();
            auto z = coors_dialog.get_z();

            if(z.has_value()){m_window->set_axis(x, y, *z);}
        }
    }

    QCheckBox* get_check_box(LightSources* sourses, QVector2D pos, QVector2D dim){
        QCheckBox* checkBox = new QCheckBox("Show " + sourses->who_is(), nullptr);

        checkBox->setGeometry(pos.x(), pos.y(), dim.x(), dim.y());  // x, y, width, height
        checkBox->setChecked(false);

        auto baseWindow = sourses->get_cube_window();
        CubeWindow* cubeWindow = qobject_cast<CubeWindow*>(baseWindow);

        // include signal
        connect(checkBox, &QCheckBox::toggled, this, [sourses, this, cubeWindow](bool checked) {
            if (checked && cubeWindow) {
                sourses->load_image();
                cubeWindow->add_light_source(sourses);
                cubeWindow->renderNow();
            } else {
                if(cubeWindow){
                    cubeWindow->delete_light_source(sourses);
                    sourses->hide();
                    cubeWindow->renderNow();
                }
            }
        });

        connect_window_hide(checkBox);

        return checkBox;
    }


private:
    CubeWindow *m_window;
    void createSlider(QWidget* slider, QVector2D offset);
};
