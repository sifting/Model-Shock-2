#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "matrix.h"
#include "skel.h"
#include "anim.h"

class Animator
{
public:
    Animator ();

    void set_skel (Skel *skel) {_skel = skel;}
    void set_anim (Anim *anim) {_anim = anim;}
    Anim *anim () {return _anim;}

    void set_speed (float speed) {_speed = speed;}
    float speed () {return _speed;}

    void advance (float tick);
    void bindpose (Matrix *pose, uint32_t num);
    void pose (Matrix *pose, uint32_t num);

public:
    float head_yaw;
    float head_pitch;
    float body_yaw;
    float body_pitch;

private:
    Skel *_skel;
    Anim *_anim;
    float _speed;
    float _time;
};

#endif // ANIMATOR_H
