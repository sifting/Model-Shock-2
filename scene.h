#ifndef SCENE_H
#define SCENE_H

#include "skel.h"
#include "anim.h"
#include "animator.h"
#include <vector>

class Scene
{
public:
    Scene();
    ~Scene ();

    void set_skel (Skel *skel) {_skel = skel;}
    Skel *skel () {return _skel;}
    Animator *animator () {return _animator;}

    void add_anim (Anim *anim) {_anims.push_back (anim);}
    const std::vector<Anim *>& anims() {return _anims;}

private:
    Skel *_skel;
    Animator *_animator;
    std::vector<Anim *> _anims;
};

extern Scene *g_scene;

#endif // SCENE_H
