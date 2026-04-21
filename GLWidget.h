#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QKeyEvent>


class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    GLWidget(QWidget *parent = nullptr);
    ~GLWidget();

    // toggle filtrations
    void setFiltering(GLint minFilter, GLint magFilter);

    void setAnisotropy(float value) {
        m_anisotropy = value;
        update();
    }

    bool isMultiTexture(){return m_multiTexture;}
    float getAnisotropy(){return m_anisotropy;}

    void setMultiTexture(bool enabled) {
        m_multiTexture = enabled;
        update();
    }

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    QOpenGLShaderProgram *m_program;
    QOpenGLTexture* m_texRoadBase;
    QOpenGLTexture* m_texRoadMarkup;
    QOpenGLTexture* m_texRoadCracks;

    // coordinates of camera
    float m_camX = 0.0f;
    float m_camZ = 2.0f;
    float m_camY = 0.5f;

    // parameters of filtration
    GLint m_minFilter = GL_LINEAR_MIPMAP_LINEAR;
    GLint m_magFilter = GL_LINEAR;

    float m_anisotropy;
    bool m_multiTexture; // use one texture or several
};
