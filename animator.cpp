#include "animator.h"

Animator::Animator()
    : _skel (nullptr)
    , _anim (nullptr)
    , _speed (1.0)
{

}

void Animator::advance (float tick)
{
    if (nullptr == _anim)
    {
        return;
    }

    _time += _speed*tick;
    if (_time >= _anim->length())
    {
        _time = _time - _anim->length();
    }
}
void Animator::bindpose (Matrix *pose, uint32_t num)
{
    if (nullptr == _skel)
    {
        return;
    }

    auto& bones = _skel->bones ();
    if (num <= bones.size ())
    {
        return;
    }

    auto& indices = _skel->indices ();
    for (auto i = 0u; i < bones.size (); i++)
    {
        auto& index = indices[i];
        auto r = Vector (bones[index.id]->r[0],
                         bones[index.id]->r[1],
                         bones[index.id]->r[2],
                         bones[index.id]->r[3]);
        auto t = Vector (bones[index.id]->t[0],
                         bones[index.id]->t[1],
                         bones[index.id]->t[2],
                         1.0);
        if (index.parent < 0)
        {
            pose[index.id] = Matrix::from_rotation_position (r, t);
            continue;
        }

        Matrix tmp = Matrix::from_rotation_position (r, t);
        pose[index.id] = pose[index.parent]*tmp;
    }
}
void Animator::pose (Matrix *pose, uint32_t num)
{
    if (nullptr == _skel)
    {
        return;
    }
    if (nullptr == _anim)
    {
        return;
    }

    if (num <= _skel->bones ().size ())
    {
        return;
    }
}
