#include "CubeUIManager.h"


void CubeUIManager::createSlider(QWidget* slider, QVector2D offset){
    slider->setAttribute(Qt::WA_DeleteOnClose);
    QPoint Pos = m_window->geometry().topRight();

    // good position
    Pos.setX(Pos.x() + offset.x());
    Pos.setY(Pos.y() + offset.y());
    slider->move(Pos);

    connect_window_hide(slider);
}
