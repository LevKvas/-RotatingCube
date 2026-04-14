#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "MeshObjFile.h"

class MyGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    MyGLWidget(MeshObjFile* model, QWidget *parent = nullptr)
        : QOpenGLWidget(parent), model(model)
    {
        startTimer(16);
    }
protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glEnable(GL_DEPTH_TEST); // Чтобы куб был объемным, а не прозрачным
    }

    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();

        // settings of camera
        glTranslatef(0.0f, 0.0f, -5.0f);
        glRotatef(angle, 1, 1, 0);

        if (!model) return;

        // draw triangles
        glBegin(GL_TRIANGLES);
        const auto& vs = model->getVertices();
        const auto& ids = model->getIndices();

        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glColor3f(0.0f, 1.0f, 0.0f);

        for(auto i : ids) {
            glVertex3f(vs[i].x, vs[i].y, vs[i].z);
        }
        glEnd();
    }

    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        // Простая перспектива
        float aspect = (float)w / (float)h;
        glFrustum(-aspect, aspect, -1.0, 1.0, 1.0, 100.0);
        glMatrixMode(GL_MODELVIEW);
    }

    void timerEvent(QTimerEvent *event) override {
        angle += 1.0f;
        update();
    }

private:
    MeshObjFile* model;
    float angle = 0.0f;
};
