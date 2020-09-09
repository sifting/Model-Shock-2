#include "viewport.h"
#include "scene.h"

static const char *simple_vtx = R"gl(
    #version 450 core

    uniform mat4 pmv;

    in vec3 xyz;

    void main ()
    {
        gl_Position = pmv*vec4 (xyz, 1.0);
    }
)gl";

static const char *simple_frg = R"gl(
    #version 450 core

    layout(location = 0) out vec4 colour;

    void main ()
    {
        colour = vec4 (1.0, 0.0, 0.0, 1.0);
    }
)gl";

Viewport::Viewport (QWidget *parent)
: QOpenGLWidget (parent)
{
}

void Viewport::initializeGL ()
{
    initializeOpenGLFunctions();

    makeCurrent ();

    _colour[0] = 0;
    _colour[1] = 0;
    _colour[2] = 0.5;

    GLuint vtxs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vtxs, 1, &simple_vtx, NULL);
    glCompileShader (vtxs);

    GLuint frgs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (frgs, 1, &simple_frg, NULL);
    glCompileShader (frgs);

    _simple = glCreateProgram ();
    glAttachShader (_simple, vtxs);
    glAttachShader (_simple, frgs);
    glLinkProgram (_simple);
    glUseProgram (_simple);

    {
        glGenVertexArrays (1, &_skel_vao);
        glBindVertexArray (_skel_vao);

        glGenBuffers (1, &_skel);
        glBindBuffer (GL_ARRAY_BUFFER, _skel);

        auto id = glGetAttribLocation (_simple, "xyz");
        glEnableVertexAttribArray (id);
        glVertexAttribPointer (id, 3, GL_FLOAT, GL_FALSE, sizeof (Vector), NULL);
    }

    draw ();
}

void Viewport::draw ()
{
    glClearColor (_colour[0], _colour[1], _colour[2], 1.0);
    glClear (GL_COLOR_BUFFER_BIT);


    static float time = 0.0;
    time += 0.16;

    Matrix mdl;
    float th = time/10.0;
    float s = sinf (th);
    float c = cosf (th);
    mdl[0].x = c; mdl[0].z =-s;
    mdl[2].x = s; mdl[2].z = c;
    mdl[2].w =-12;

    Matrix flip;
    s = sinf (-3.1415*90.0/180.0);
    c = cosf (-3.1415*90.0/180.0);
    flip[1].y = c; flip[1].z =-s;
    flip[2].y = s; flip[2].z = c;

    Matrix pmv = _proj*mdl*flip;

    auto id = glGetUniformLocation (_simple, "pmv");
    glUniformMatrix4fv (id, 1, GL_TRUE, pmv.as_ptr ());

    if (g_scene->skel () != nullptr)
    {
        Matrix pose[32];
        std::vector<Vector> pts;
        g_scene->animator ()->bindpose (pose, 32);
        for (auto& n : g_scene->skel ()->indices ())
        {
            if (n.parent < 0)
            {
                continue;
            }
            pts.push_back (pose[n.parent].col (3));
            pts.push_back (pose[n.id].col (3));
        }
        glBindVertexArray (_skel_vao);
        glBindBuffer (GL_ARRAY_BUFFER, _skel);
        glBufferData (GL_ARRAY_BUFFER, sizeof (Vector)*pts.size (), pts.data (), GL_DYNAMIC_DRAW);
        glDrawArrays (GL_LINES, 0, pts.size ());
    }

    if (glGetError () != GL_NO_ERROR)
    {
        _colour[0] = 1.0;
        _colour[1] = 1.0;
        _colour[2] = 0.0;
    }
}

