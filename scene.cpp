#include "scene.h"

Scene *g_scene;

Scene::Scene()
    : _skel (nullptr)
    , _animator (new Animator ())
{
    g_scene = this;
}

Scene::~Scene ()
{
    delete _skel;
    delete _animator;
}
