#include "viewport.h"
#include "scene.h"

static const char *simple_vtx = R"gl(
    #version 450 core

    uniform mat4 pmv;

    layout (location = 0) in vec3 xyz;

    void main ()
    {
        gl_Position = pmv*vec4 (xyz, 1.0);
    }
)gl";

static const char *simple_frg = R"gl(
    #version 450 core

    layout (location = 0) out vec4 colour;

    void main ()
    {
        colour = vec4 (1.0, 0.0, 0.0, 1.0);
    }
)gl";

static const char *textured_frg = R"gl(
    #version 450 core

    uniform sampler2D map;

    in vec2 uv;

    layout (location = 0) out vec4 colour;

    void main ()
    {
        colour = texture (map, uv);
    }
)gl";

static const char *skinned_vtx = R"gl(
    #version 450 core

    uniform mat4 pmv;
    uniform mat4 bones[64];

    layout (location = 0) in vec3 _xyz;
    layout (location = 1) in vec3 _nom;
    layout (location = 2) in vec2 _uv;
    layout (location = 3) in int _id;

    out vec2 uv;

    void main ()
    {
        vec4 pos = bones[_id]*vec4 (_xyz, 1.0);
        vec3 n = mat3 (bones[_id])*_nom;
        gl_Position = pmv*pos;
        uv = _uv;
    }
)gl";


Viewport::Viewport (QWidget *parent)
: QOpenGLWidget (parent)
, _dirty (false)
{
}

GLuint Viewport::compile (const char *vtx, const char *frg)
{
    GLuint vtxs = glCreateShader (GL_VERTEX_SHADER);
    glShaderSource (vtxs, 1, &vtx, NULL);
    glCompileShader (vtxs);

    GLuint frgs = glCreateShader (GL_FRAGMENT_SHADER);
    glShaderSource (frgs, 1, &frg, NULL);
    glCompileShader (frgs);

    auto prog = glCreateProgram ();
    glAttachShader (prog, vtxs);
    glAttachShader (prog, frgs);
    glLinkProgram (prog);
    glUseProgram (prog);
    return prog;
}
void Viewport::initializeGL ()
{
    initializeOpenGLFunctions();

    makeCurrent ();

    _colour[0] = 0;
    _colour[1] = 0;
    _colour[2] = 0.5;

    _simple = compile (simple_vtx, simple_frg);
    _skin = compile (skinned_vtx, textured_frg);
    {
        glGenVertexArrays (1, &_skel_vao);
        glBindVertexArray (_skel_vao);

        glGenBuffers (1, &_skel);
        glBindBuffer (GL_ARRAY_BUFFER, _skel);

        glEnableVertexAttribArray (0);
        glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof (Vector), NULL);
    }

    glDisable (GL_CULL_FACE);
    glEnable (GL_DEPTH_TEST);
    draw ();
}

void Viewport::draw ()
{
    glClearColor (_colour[0], _colour[1], _colour[2], 1.0);
    glClear (GL_COLOR_BUFFER_BIT);
    /*Upload data if it's been changed*/
    if (_dirty)
    {
        Model *mdl = g_scene->model ();
        /*Upload geometry*/
        glGenVertexArrays (mdl->meshes ().size (), _vaos);
        glGenBuffers (mdl->meshes ().size (), _buffers);
        glGenBuffers (mdl->meshes ().size (), _elements);
        for (auto i = 0u; i < mdl->meshes ().size (); i++)
        {
            auto mesh = mdl->meshes()[i];

            glBindVertexArray (_vaos[i]);
            glBindBuffer (GL_ARRAY_BUFFER, _buffers[i]);
            glBufferData (GL_ARRAY_BUFFER,
                          mesh->nverts*sizeof (mesh->verts[0]),
                          mesh->verts,
                          GL_STATIC_DRAW);

            glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, _elements[i]);
            glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                          2*mesh->nindices,
                          mesh->indices,
                          GL_STATIC_DRAW);

            glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE,
                                   sizeof (Model::Vertex),
                                   (void *)offsetof(Model::Vertex, xyz));
            glEnableVertexAttribArray (0);
            glVertexAttribPointer (1, 3, GL_FLOAT, GL_FALSE,
                                   sizeof (Model::Vertex),
                                   (void *)offsetof(Model::Vertex, nom));
            glEnableVertexAttribArray (1);
            glVertexAttribPointer (2, 2, GL_FLOAT, GL_FALSE,
                                   sizeof (Model::Vertex),
                                   (void *)offsetof(Model::Vertex, uv));
            glEnableVertexAttribArray (2);
            glVertexAttribIPointer (3, 1, GL_UNSIGNED_INT,
                                   sizeof (Model::Vertex),
                                   (void *)offsetof(Model::Vertex, id));
            glEnableVertexAttribArray (3);

        }
        /*Load textures*/
        for (auto i = 0u; i < mdl->materials ().size (); i++)
        {
            auto mat = mdl->materials()[i];
            auto pix = new QOpenGLTexture (
                        QImage (mat->path.c_str ()));
            pix->setMinificationFilter (QOpenGLTexture::LinearMipMapLinear);
            pix->setMagnificationFilter (QOpenGLTexture::Linear);
            _textures[i] = pix;
        }
        /*Mark that we're clean*/
        _dirty = false;
    }

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

    Matrix pose[64];
    g_scene->animator ()->bindpose (pose, 64);

    if (g_scene->model () != nullptr)
    {
        glUseProgram (_skin);

        auto id = glGetUniformLocation (_skin, "pmv");
        glUniformMatrix4fv (id, 1, GL_TRUE, pmv.as_ptr ());
        id = glGetUniformLocation (_skin, "bones");
        glUniformMatrix4fv (id, 64, GL_TRUE, pose[0].as_ptr ());
        id = glGetUniformLocation (_skin, "map");
        glUniform1i (id, 0);

        auto meshes = g_scene->model ()->meshes ();
        for (auto i = 0u; i < meshes.size (); i++)
        {
            auto mesh = meshes[i];

            glActiveTexture (GL_TEXTURE0);
            _textures[mesh->slot]->bind ();

            glBindVertexArray (_vaos[i]);
            glDrawElements (GL_TRIANGLES,
                            mesh->nindices,
                            GL_UNSIGNED_SHORT,
                            0);
        }
    }

    if (g_scene->skel () != nullptr)
    {
        std::vector<Vector> pts;
        for (auto& n : g_scene->skel ()->indices ())
        {
            if (n.parent < 0)
            {
                continue;
            }
            pts.push_back (pose[n.parent].col (3));
            pts.push_back (pose[n.id].col (3));
        }
        glUseProgram (_simple);

        auto id = glGetUniformLocation (_simple, "pmv");
        glUniformMatrix4fv (id, 1, GL_TRUE, pmv.as_ptr ());

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

