#ifndef VIEWPORT_H
#define VIEWPORT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_4_5_Core>
#include <QOpenGLTexture>
#include "matrix.h"

class Viewport : public QOpenGLWidget, protected QOpenGLFunctions_4_5_Core
{
    Q_OBJECT
public:
    Viewport (QWidget *parent);
    void mark_dirty () {_dirty = true;}
private:
    struct _Vertex
    {
        float xyz[3];
        float nom[3];
    };
    float _colour[3];
    GLuint _simple;
    GLuint _skin;
    Matrix _proj;
    GLuint _skel;
    GLuint _skel_vao;

    static const uint32_t MAX_HANDLES = 64;
    GLuint _vaos[MAX_HANDLES];
    GLuint _buffers[MAX_HANDLES];
    GLuint _elements[MAX_HANDLES];
    QOpenGLTexture *_textures[MAX_HANDLES];
    bool _dirty;

    GLuint compile (const char *vtx, const char *frg);
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
