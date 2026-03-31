#include "cubewindow.h"
#include "ui_widgets.h"
#include "tools.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QOpenGLContext>
#include <QScreen>
#include <QMessageBox>
#include <QColorDialog>


// ctor
OpenGLWindow::OpenGLWindow(QWindow *parent)
    : QWindow(parent)
{
    setSurfaceType(QWindow::OpenGLSurface);
}

// render
void OpenGLWindow::render()
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

}

void OpenGLWindow::renderLater()
{
    requestUpdate();
}

// get event from OS
bool OpenGLWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void OpenGLWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void OpenGLWindow::renderNow()
{
    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    m_context->makeCurrent(this);

    if (needsInitialize) {
        initializeOpenGLFunctions();
        initialize();
    }

    render();

    m_context->swapBuffers(this);

    if (m_animating)
        renderLater();
}


void OpenGLWindow::setAnimating(bool animating)
{
    m_animating = animating;

    if (animating)
        renderLater();
}

void OpenGLWindow::initialize(){}

OpenGLWindow::~OpenGLWindow(){delete m_context;}

std::vector<GLfloat> CubeWindow::add_colors(const std::vector<QVector3D>& grid, const std::vector<QVector3D>& colors){
    std::vector<GLfloat> vertices_colors{};
    size_t color_size = colors.size();

    QVector<float> concatenated{};

    for(size_t i = 0; i < grid.size(); ++i){
        concatenated << grid[i].x() << grid[i].y() << grid[i].z();
        concatenated << colors[i % color_size].x() << colors[i % color_size].y() << colors[i % color_size].z();

        vertices_colors.insert(vertices_colors.end(), concatenated.begin(), concatenated.end());

        concatenated.clear();
    }

    return vertices_colors;
}


void CubeWindow::initialize()
{
    if (!m_vbo) {
        m_vbo = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    }

    if(!m_ibo){
        m_ibo = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
    }

    m_vbo->create();
    m_ibo->create();

    rebuildBuffers();

    m_program = new QOpenGLShaderProgram(this);

    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "../../shaders/vertex_shader.vsh");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "../../shaders/fragment_shader.fsh");

    m_program->bindAttributeLocation("posAttr", 0);
    m_program->bindAttributeLocation("colAttr", 1);
    m_program->bindAttributeLocation("normalAttr", 2);

    m_program->link();

    if (!m_program->link()) {
        qCritical() << "Shader program link failed:" << m_program->log();
        return;
    }

    glEnable(GL_DEPTH_TEST);

    m_program->bind();

    // all uniform to shader
    m_matrixUniform = m_program->uniformLocation("matrix");
    m_matrixRotUniform = m_program->uniformLocation("rot_matrix");
    m_morphFactorUniform = m_program->uniformLocation("morphFactor");
    m_SourceColorUniform = m_program->uniformLocation("source_color");

    if (m_matrixUniform == -1 || m_morphFactorUniform == -1) {
        qWarning() << "Failed to get uniform location!";
    }

    m_lampPosUniform = m_program->uniformLocation("posLamp");
    m_lampIsUse = m_program->uniformLocation("is_use_lamp");
    m_SpotLightPosUniform = m_program->uniformLocation("PosSpotLight");

    if (m_lampPosUniform == -1 || m_SpotLightPosUniform == -1) {
        qWarning() << "Failed to get Lights pos!";
    }

    // hand over direction
    m_SpotLightDirUniform = m_program->uniformLocation("SpotLightDir");
    m_SpotLightIsUse = m_program->uniformLocation("is_use_SpotLight");

    m_DirLightDirUniform = m_program->uniformLocation("DirLightDir");
    m_DirectionalLightIsUse = m_program->uniformLocation("is_use_DirectionalLight");

    m_CameraPos = m_program->uniformLocation("CameraPos");
}

std::vector<GLfloat> CubeWindow::add_colors_and_normals(
    const std::vector<QVector3D>& grid,
    const std::vector<QVector3D>& colors,
    const QVector3D& normal,
    size_t color_size)
{
    std::vector<GLfloat> vertices_data{};

    for(size_t i = 0; i < grid.size(); ++i){
        vertices_data.push_back(grid[i].x());
        vertices_data.push_back(grid[i].y());
        vertices_data.push_back(grid[i].z());


        vertices_data.push_back(colors[i % color_size].x());
        vertices_data.push_back(colors[i % color_size].y());
        vertices_data.push_back(colors[i % color_size].z());

        vertices_data.push_back(normal.x());
        vertices_data.push_back(normal.y());
        vertices_data.push_back(normal.z());
    }

    return vertices_data;
}

void CubeWindow:: rebuildBuffers(){
    std::vector<QVector3D> faceNormals = {
        { 0,  0,  1},
        { 0,  0, -1},
        {-1,  0,  0},
        { 1,  0,  0},
        { 0, -1,  0},
        { 0,  1,  0}
    };

    std::vector<QVector3D> colors = {
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 0.0f},

        {1.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 1.0f},
        {1.0f, 0.5f, 0.0f},
        {0.0f, 1.0f, 1.0f}
    };

    static const GLfloat vertices_colors_cube[] = {
        -0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f,
        0.5f,  0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 1.0f,
        0.5f,  0.5f, -0.5f, 1.0f, 0.5f, 0.0f,
        -0.5f,  0.5f, -0.5f, 0.0f, 1.0f, 1.0f
    };

    // build grid for each face
    std::vector<GLfloat> vertices_colors{};
    std::vector<uint16_t> indices{};

    QVector<QVector3D> p;
    for (int i = 0; i < 8; ++i) {
        int base = i * 6;
        p.append(QVector3D(
            vertices_colors_cube[base + 0],
            vertices_colors_cube[base + 1],
            vertices_colors_cube[base + 2]
            ));
    }

    auto v_c_1 = add_colors_and_normals(get_grid(p[0], p[1], p[2], p[3], N), colors, faceNormals[0], colors.size());
    auto v_c_2 = add_colors_and_normals(get_grid(p[4], p[7], p[6], p[5], N), colors, faceNormals[1], colors.size());
    auto v_c_3 = add_colors_and_normals(get_grid(p[4], p[0], p[3], p[7], N), colors, faceNormals[2], colors.size());
    auto v_c_4 = add_colors_and_normals(get_grid(p[1], p[5], p[6], p[2], N), colors, faceNormals[3], colors.size());
    auto v_c_5 = add_colors_and_normals(get_grid(p[4], p[5], p[1], p[0], N), colors, faceNormals[4], colors.size());
    auto v_c_6 = add_colors_and_normals(get_grid(p[3], p[2], p[6], p[7], N), colors, faceNormals[5], colors.size());

    vertices_colors.insert(vertices_colors.end(), v_c_1.begin(), v_c_1.end());
    vertices_colors.insert(vertices_colors.end(), v_c_2.begin(), v_c_2.end());
    vertices_colors.insert(vertices_colors.end(), v_c_3.begin(), v_c_3.end());
    vertices_colors.insert(vertices_colors.end(), v_c_4.begin(), v_c_4.end());
    vertices_colors.insert(vertices_colors.end(), v_c_5.begin(), v_c_5.end());
    vertices_colors.insert(vertices_colors.end(), v_c_6.begin(), v_c_6.end());


    // fill indices
    for (size_t face = 0; face < 6; ++face) {
        size_t offset = 2 * face * (N + 1);
        for (size_t i = 0; i < N; ++i) {
            uint16_t top_left = offset + 2 * i;
            uint16_t bottom_left = offset + 2 * i + 1;
            uint16_t top_right = offset + 2 * (i + 1);
            uint16_t bottom_right = offset + 2 * (i + 1) + 1;

            indices.push_back(top_left);
            indices.push_back(top_right);
            indices.push_back(bottom_left);

            indices.push_back(bottom_left);
            indices.push_back(top_right);
            indices.push_back(bottom_right);
        }
    }

    m_vbo->bind();
    m_vbo->allocate(vertices_colors.data(), vertices_colors.size() * sizeof(GLfloat));

    m_ibo->bind();
    m_ibo->allocate(indices.data(), indices.size() * sizeof(uint16_t));

    index_buff_size = indices.size();
}


void CubeWindow::render()
{
    if(m_needsRebuild){
        rebuildBuffers();
        m_needsRebuild = false;
    }

    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program->bind();

    m_vbo->bind();
    m_ibo->bind();

    QVector3D camera_pos = QVector3D(0, 0, -2);

    QMatrix4x4 matrix;
    QMatrix4x4 matrix_rot;

    matrix.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    matrix.translate(camera_pos);

    // mult single matrix on rotate matrix
    matrix_rot.rotate(100.0f * m_frame / screen()->refreshRate(), axis[0], axis[1], axis[2]);

    m_program->setUniformValue(m_matrixUniform, matrix);
    m_program->setUniformValue(m_matrixRotUniform, matrix_rot);
    m_program->setUniformValue(m_morphFactorUniform, alfa);

    // pos of camera
    m_program->setUniformValue("CameraPos", camera_pos);

    // set start values
    m_program->setUniformValue("is_use_lamp", 0);
    m_program->setUniformValue("is_use_SpotLight", 0);
    m_program->setUniformValue("is_use_DirectionalLight", 0);

    // to hand over position
    for (auto it = m_lights.begin(); it != m_lights.end(); ++it) {
        auto light = *it;
        m_program->setUniformValue(m_SourceColorUniform, light->get_color()); // set source color

        if(light->who_is() == QString("Lamp")){
            m_program->setUniformValue(m_lampPosUniform, light->get_pos());
            m_program->setUniformValue("is_use_lamp", 1);
            qDebug() << "Lamp added";
        }
        if(light->who_is() == QString("SpotLight")){
            m_program->setUniformValue(m_SpotLightPosUniform, light->get_pos());
            m_program->setUniformValue(m_SpotLightDirUniform, light->get_direction());
            m_program->setUniformValue("is_use_SpotLight", 1);
        }
        if(light->who_is() == QString("DirectionalLight")){
            m_program->setUniformValue(m_DirLightDirUniform, light->get_direction());
            m_program->setUniformValue("is_use_DirectionalLight", 1);

            qDebug() << "Directional added";
        }
    }

    // include attribure of vertix
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat), nullptr);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat),
                          reinterpret_cast<void *>(3 * sizeof(GLfloat))); // shift on 3 floats

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(GLfloat),
                                reinterpret_cast<void*>(6 * sizeof(GLfloat)));


    glEnable(GL_CULL_FACE); // include edge cutting
    glCullFace(GL_BACK); // do not draw back edges

    glDrawElements(GL_TRIANGLES,
                   index_buff_size,
                   GL_UNSIGNED_SHORT,
                   nullptr);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glDisable(GL_CULL_FACE);

    m_program->release();

    // draw light sourses

    QMatrix4x4 projection;
    projection.perspective(60.0f, 4.0f / 3.0f, 0.1f, 100.0f);

    QMatrix4x4 view;
    view.translate(0, 0, -2);

    QMatrix4x4 model;

    for (auto it = m_lights.begin(); it != m_lights.end(); ++it) {
        auto light = *it;
        QVector3D lightPos3D = light->get_pos();

        // project in 2D
        QPoint screenPos = projectWorldToScreen(
            lightPos3D,
            projection, view, model,
            width() * devicePixelRatio(),
            height() * devicePixelRatio()
            );

        if (screenPos.x() >= 0 && screenPos.y() >= 0) {
            QPoint globalPos = mapToGlobal(screenPos);
            light->setLabelScreenPosition(globalPos);
            light->show();
        }
    }

    ++m_frame;
}

CubeWindow::~CubeWindow()
{
    delete m_vbo;
    delete m_ibo;

    delete m_program;
}

std::vector<QVector3D> CubeWindow::get_grid(QVector3D v1, QVector3D v2,
                                            QVector3D v3, QVector3D v4, size_t N) const {
    std::vector<QVector3D> res{};

    for (size_t col = 0; col <= N; ++col) {
        float t = static_cast<float>(col) / N;

        QVector3D top = v1 + t * (v2 - v1);
        QVector3D bottom = v4 + t * (v3 - v4);

        res.push_back(top);
        res.push_back(bottom);
    }

    return res;
}









