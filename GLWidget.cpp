#include "GLWidget.h"

GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent) {
    setFocusPolicy(Qt::StrongFocus);

    // start parameters of camera
    m_camX = 0.0f;
    m_camY = 0.2f;
    m_camZ = 2.0f;

    m_anisotropy = 1.0;
    m_multiTexture = true;
}

GLWidget::~GLWidget() {
    makeCurrent();

    delete m_texRoadBase;
    delete m_texRoadMarkup;
    delete m_texRoadCracks;
    delete m_program;

    doneCurrent();
}


void GLWidget::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

    // load main texture
    m_texRoadBase = new QOpenGLTexture(QImage("C:/Users/USER/Documents/Road/textures/road_2.jpg").flipped(Qt::Vertical));
    m_texRoadBase->setWrapMode(QOpenGLTexture::Repeat);

    // to generate Mipmap
    m_texRoadBase->bind();
    glGenerateMipmap(GL_TEXTURE_2D);

    // another textures for mixing
    m_texRoadMarkup = new QOpenGLTexture(QImage("C:/Users/USER/Documents/Road/textures/road_1.jpg").flipped(Qt::Vertical));
    m_texRoadMarkup->setWrapMode(QOpenGLTexture::Repeat);
    glGenerateMipmap(GL_TEXTURE_2D);

    m_texRoadCracks = new QOpenGLTexture(QImage("C:/Users/USER/Documents/Road/textures/road_3.jpg").flipped(Qt::Vertical));
    m_texRoadCracks->setWrapMode(QOpenGLTexture::Repeat);
    glGenerateMipmap(GL_TEXTURE_2D);

    // load shaders
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, "C:/Users/USER/Documents/Road/road.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, "C:/Users/USER/Documents/Road/road.frag");
    m_program->link();
}

void GLWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // matrices
    QMatrix4x4 model;
    QMatrix4x4 view;
    QMatrix4x4 projection;

    view.lookAt(QVector3D(m_camX, m_camY, m_camZ),
                QVector3D(m_camX, m_camY, m_camZ - 1.0f),
                QVector3D(0, 1, 0));

    projection.perspective(45.0f, float(width()) / height(), 0.1f, 200.0f);

    // activate shaders and send matrix
    m_program->bind();
    m_program->setUniformValue("mvp", projection * view * model);

    m_program->setUniformValue("multiTexture", m_multiTexture);

    // Stone
    glActiveTexture(GL_TEXTURE0);
    m_texRoadBase->bind();
    // use anisotropy
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_anisotropy);
    m_program->setUniformValue("texBase", 0);

    // marking
    glActiveTexture(GL_TEXTURE1);
    m_texRoadMarkup->bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_anisotropy);
    m_program->setUniformValue("texMarkup", 1);

    // cracks
    glActiveTexture(GL_TEXTURE2);
    m_texRoadCracks->bind();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_anisotropy);
    m_program->setUniformValue("texCracks", 2);

    // filters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_minFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_magFilter);

    // Geometry
    GLfloat vertices[] = {
        -0.5f, 0.0f,  0.0f,      0.0f, 0.0f,
        0.5f, 0.0f,  0.0f,      1.0f, 0.0f,
        0.5f, 0.0f, -100.0f,    1.0f, 100.0f,
        -0.5f, 0.0f, -100.0f,    0.0f, 100.0f
    };

    int posLoc = m_program->attributeLocation("pos");
    int texLoc = m_program->attributeLocation("tex");

    m_program->enableAttributeArray(posLoc);
    m_program->setAttributeArray(posLoc, GL_FLOAT, vertices, 3, 5 * sizeof(GLfloat));

    m_program->enableAttributeArray(texLoc);
    m_program->setAttributeArray(texLoc, GL_FLOAT, &vertices[3], 2, 5 * sizeof(GLfloat));

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    m_program->disableAttributeArray(posLoc);
    m_program->disableAttributeArray(texLoc);

    m_program->release();
}

void GLWidget::keyPressEvent(QKeyEvent *event) {
    float step = 0.2f;
    if (event->key() == Qt::Key_W) m_camZ -= step; // forward
    if (event->key() == Qt::Key_S) m_camZ += step; // backward
    if (event->key() == Qt::Key_A) m_camX -= step; // left
    if (event->key() == Qt::Key_D) m_camX += step; // right
    if (event->key() == Qt::Key_Q) m_camY += step; // up
    if (event->key() == Qt::Key_E) m_camY -= step; // down

    update();
}

void GLWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
}
