#include <QApplication>
#include <QSurfaceFormat>
#include <QColorDialog>
#include <QColor>

#include "cubewindow.h"
#include "CubeUIManager.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CubeWindow window;

    CubeUIManager manager(&window);

    // sourses
    auto lamp = new Lamp(&window);
    auto spotlight = new SpotLight(&window);
    auto dirlight = new DirectionalLight(&window);

    // set axis
    manager.set_axis_rot();

    // alfa slider
    manager.get_alfa_slider(QVector2D(10, 200))->show();

    // N slider
    manager.get_step_slider(QVector2D(10, 30))->show();

    // all check boxes
    manager.get_check_box(lamp, QVector2D(1000, 100), QVector2D(30, 40))->show();
    manager.get_check_box(spotlight, QVector2D(1000, 200), QVector2D(30, 40))->show();
    manager.get_check_box(dirlight, QVector2D(1000, 300), QVector2D(30, 40))->show();

    window.resize(640, 480);
    window.show();
    window.setAnimating(true);

    return app.exec();
}
