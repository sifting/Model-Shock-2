#ifndef ANIM_H
#define ANIM_H

#include <cstdint>
#include "vector.h"

class Anim
{
private:
    explicit Anim () {};

public:
    ~Anim ();

    static Anim *from_file (std::string& path);

    uint32_t fps () {return _fps;}
    uint32_t nbones () {return _nbones;}
    uint32_t nframes () {return _nframes;}
    float length () {return _length;}

    uint32_t frame_from_time (float time)
    {
        return (uint32_t)(_fps*time)%_nframes;
    }

private:
    uint32_t _fps;
    uint32_t _nbones;
    float _length;
    uint32_t _nframes;
    Vector *_position;
    Vector **_rotation;
};

#endif // ANIM_H
