#include <fstream>
#include "anim.h"

Anim::~Anim ()
{
    for (auto i = 0u; i < _nbones; i++)
    {
        delete [] _rotation[i];
    }
    delete [] _rotation;
    delete [] _position;
}

Anim *Anim::from_file (std::string& path)
{
    /*Load these from the .mi file*/
    uint32_t nframes = 0;
    uint32_t fps = 30;

    /*Key frames are stored in the .mc file*/
    FILE *fp = fopen (path.c_str (), "rb");
    if (NULL == fp)
    {
        return nullptr;
    }

    auto anim = new Anim;
    anim->_length = (float)nframes/fps;

    const uint32_t MAX_BONES = 64;
    uint32_t nbones = 0;
    int32_t offsets[MAX_BONES];
    fread (&nbones, sizeof (nbones), 1, fp);
    fread (offsets, sizeof (offsets[0]), nbones, fp);

    anim->_position = new Vector[nframes];
    fseek (fp, offsets[0], SEEK_SET);
    for (uint32_t i = 0; i < nframes; i++)
    {
        float v[3];
        fread (v, sizeof (v[0]), 3, fp);
        anim->_position[i] = Vector (v[0], v[1], v[2]);
    }

    anim->_rotation = new Vector*[nbones];
    for (uint32_t i = 1; i < nbones; i++)
    {
        Vector *frames = new Vector[nframes];
        fseek (fp, offsets[i], SEEK_SET);
        for (uint32_t j = 0; j < nframes; j++)
        {
            float v[4];
            fread (v, sizeof (v[0]), 4, fp);
            frames[j] = Vector (v[0], v[1], v[2], v[3]);
        }
        anim->_rotation[i] = frames;
    }

    fclose (fp);

    return anim;
}
