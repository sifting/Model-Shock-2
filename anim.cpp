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

Anim *Anim::from_file (std::string& path, Skel *skel)
{
    /*Load these from the .mi file*/
    Component *comp;
    uint32_t nframes = 0;
    uint32_t fps = 30;
    {
        FILE *fp = fopen ((path + ".mi").c_str (), "rb");
        if (nullptr == fp)
        {
            return nullptr;
        }
        Metadata md;

        fread (&md, sizeof (md), 1, fp);
        nframes = (uint32_t)md.nframes;
        fps = md.fps;

        comp = new Component[md.ncomp];
        fread (comp, sizeof (comp[0]), md.ncomp, fp);
        fclose (fp);
    }

    /*Key frames are stored in the .mc file*/
    FILE *fp = fopen ((path + "_.mc").c_str (), "rb");
    if (nullptr == fp)
    {
        return nullptr;
    }

    auto anim = new Anim;
    anim->_length = (float)nframes/fps;
    anim->_fps = fps;
    anim->_nframes = nframes;
    anim->_nbones = skel->bones ().size ();

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

    anim->_rotation = new Vector*[nbones]{nullptr};
    anim->remap = new uint32_t[skel->bones().size()]{0};
    for (uint32_t i = 1; i < nbones; i++)
    {
        Vector *frames = new Vector[nframes];
        fseek (fp, offsets[i], SEEK_SET);
        for (uint32_t j = 0; j < nframes; j++)
        {
            float v[4];
            fread (v, sizeof (v[0]), 4, fp);
            frames[j] = Vector (-v[1], -v[2], -v[3], v[0]);
        }
        anim->_rotation[i] = frames;

        /*Map to handles... yuck*/
        for (auto& b : skel->bones ())
        {
            if (b->handle == comp[i].bone)
            {
                anim->remap[b->handle] = i;
                break;
            }
        }
    }

    delete [] comp;

    fclose (fp);

    return anim;
}
