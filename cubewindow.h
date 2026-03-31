#pragma once

#include <QWindow>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDialog>
#include <QLabel>
#include <vector>
#include <QSlider>

#include "ui_widgets.h"

class QOpenGLBuffer;
class QOpenGLShaderProgram;

class OpenGLWindow : public QWindow, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit OpenGLWindow(QWindow *parent = nullptr);
    ~OpenGLWindow();

    virtual void render();
    virtual void initialize();

    void setAnimating(bool animating);

public slots:
    void renderLater();
    void renderNow();

protected:
    bool event(QEvent *event) override;

    void exposeEvent(QExposeEvent *event) override;

private:
    bool m_animating = false;

    QOpenGLContext *m_context = nullptr;
};


class CubeWindow : public OpenGLWindow
{
    Q_OBJECT
public:
    using OpenGLWindow::OpenGLWindow;

    void initialize() override;
    void render() override;

    void set_color(const QColor& color_){
        color = QVector3D(
            color_.redF(),
            color_.greenF(),
            color_.blueF()
            );
    }

    void set_axis(float x, float y, float z){
        axis.setX(x);
        axis.setY(y);
        axis.setZ(z);
    }

    void set_material(float k_a_, float k_d_, float k_s_){
        k_a = k_a_;
        k_d = k_d_;
        k_s = k_s_;
    }

    float get_k_a()const{return k_a;}
    float get_k_d()const{return k_d;}
    float get_k_s()const{return k_s;}

    void add_light_source(LightSources* source){
        m_lights.append(source);
    }

    void delete_light_source(LightSources* source){
        m_lights.removeAll(source);
    }

    void onButtonClicked();

    std::vector<QVector3D> get_grid(QVector3D v1, QVector3D v2,
                                    QVector3D v3, QVector3D v4, size_t N) const;

    std::vector<GLfloat> add_colors(const std::vector<QVector3D>& grid, const std::vector<QVector3D>& colors);


    ~CubeWindow();

public slots:
    void setSubdivision(int N_){
        N = N_;
        m_needsRebuild = true;
        renderLater();
    }

    void setAlfa(float alfa_){
        alfa = alfa_;
        m_needsRebuild = true;
        renderLater();
    }

private:
    GLint m_matrixUniform = -1;
    GLint m_morphFactorUniform = -1;
    GLint m_lampPosUniform = -1;
    GLint m_SpotLightPosUniform = -1;
    GLint m_SpotLightDirUniform = -1;
    GLint m_CameraPos = -1;
    GLint m_DirLightDirUniform = -1;
    GLint m_matrixRotUniform = -1;
    GLint m_SourceColorUniform = -1;

    // use
    GLint m_lampIsUse = -1;
    GLint m_SpotLightIsUse = -1;
    GLint m_DirectionalLightIsUse = -1;

    QOpenGLBuffer* m_vbo = nullptr;
    QOpenGLBuffer* m_ibo = nullptr;
    QOpenGLShaderProgram* m_program = nullptr;

    QList<LightSources*> m_lights{}; // buffer to save all sourses

    size_t index_buff_size{};

    QVector3D color{};
    QVector3D axis{};

    int m_frame = 0;

    int N = 20; // step of grid
    bool m_needsRebuild = false;

    float alfa{}; // for transform to sphere

    void rebuildBuffers();

    std::vector<GLfloat> add_colors_and_normals(
        const std::vector<QVector3D>& grid,
        const std::vector<QVector3D>& colors,
        const QVector3D& normal,
        size_t color_size);

    float k_a{};
    float k_d{};
    float k_s{};
};




