#include "MeshObjFile.h"
#include "MyGLWidget.h"

#include <QDebug>
#include <string>

#include <QApplication>

int main(int argc, char *argv[])
{
    std::string fileCube = "C:/Users/USER/Documents/Mesh/models/MeshCube.obj";
    std::string fileHouse = "C:/Users/USER/Documents/Mesh/models/MeshHouse.obj";
    std::string fileSphere = "C:/Users/USER/Documents/Mesh/models/MeshSphere.obj";

    QApplication a(argc, argv);

    MeshObjFile* cube = new MeshObjFile(fileCube);
    qDebug() << "Finish loading!";

    MyGLWidget* w = new MyGLWidget(cube);

    w->resize(800, 600);
    w->setWindowTitle("3D Viewer");
    w->show();

    qDebug() << "Show called";

    return a.exec();
}
