#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_5_Core>
#include "matrix.h"

class Viewport : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT
public:
    Viewport (QWidget *parent);
private:
    struct _Vertex
    {
        float xyz[3];
        float nom[3];
    };
    float _colour[3];
    GLuint _simple;
    Matrix _proj;
    GLuint _skel;
    GLuint _skel_vao;

    void draw ();
protected:
    void initializeGL () override;
    void paintGL () override
    {
        draw ();
    }
    void resizeGL (int w, int h) override
    {
        _proj = Matrix::from_perspective (60.0, (float)w/h);
        draw ();
    }
};

#endif // VIEWPORT_H
