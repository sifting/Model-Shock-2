#ifndef ANIM_H
#define ANIM_H

#include <cstdint>
#include "vector.h"
#include "skel.h"

class Anim
{
private:
    explicit Anim () {};

public:
    ~Anim ();

    static Anim *from_file (std::string& path, Skel *skel);

    uint32_t fps () {return _fps;}
    uint32_t nbones () {return _nbones;}
    uint32_t nframes () {return _nframes;}
    float length () {return _length;}

    uint32_t frame_from_time (float time)
    {
        return (uint32_t)(_fps*time)%_nframes;
    }
    Vector *position () {return _position;}
    Vector **rotation () {return _rotation;}
    uint32_t *remap;
private: /*Dark structs*/
    struct Metadata
    {
        uint32_t unk1;
        uint32_t unk2;
        float nframes;
        uint32_t fps;
        uint32_t unk4;
        char name[12];
        uint8_t unk5[64];
        uint32_t ncomp;
        uint32_t unk6;
        uint32_t unk7;
        uint32_t unk8;
    };
    struct Component
    {
        uint32_t unk1;
        int32_t bone;
        uint32_t unk2;
    };
private:
    uint32_t _fps;
    uint32_t _nbones;
    float _length;
    uint32_t _nframes;
    Vector *_position;
    Vector **_rotation;
};

#endif // ANIM_H
