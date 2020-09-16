#include <fstream>
#include <cstring>
#include <unordered_map>
#include "mesh.h"

Model *
Model::from_file (std::string& path, std::string& prefix, Skel *skel)
{
    FILE *fp = fopen (path.c_str (), "rb");
    if (!fp)
    {
        return nullptr;
    }

    Header h;
    fread (&h, 1, sizeof (h), fp);
    if (memcmp ("LGMM", h.id, 4) != 0)
    {
        return nullptr;
    }
    if (h.version != 2)
    {
        return nullptr;
    }

    auto map = new uint8_t[2*h.nsegs];
    fseek (fp, h.map, SEEK_SET);
    fread (map, sizeof (map[0]), 2*h.nsegs, fp);

    auto segs = new Segment[h.nsegs];
    fseek (fp, h.segs, SEEK_SET);
    fread (segs, sizeof (segs[0]), h.nsegs, fp);

    auto mats = new Material[h.nmats];
    fseek (fp, h.mats, SEEK_SET);
    fread (mats, sizeof (mats[0]), h.nmats, fp);

    auto tris = new Tri[h.ntris];
    fseek (fp, h.tris, SEEK_SET);
    fread (tris, sizeof (tris[0]), h.ntris, fp);

    auto verts = new float[3*h.nverts];
    fseek (fp, h.points, SEEK_SET);
    fread (verts, sizeof (verts[0]), 3*h.nverts, fp);

    auto uvs = new UV[h.nverts];
    fseek (fp, h.uvs, SEEK_SET);
    fread (uvs, sizeof (uvs[0]), h.nverts, fp);

    auto chunks = new Chunk[h.nchunks];
    fseek (fp, h.chunks, SEEK_SET);
    fread (chunks, sizeof (chunks[0]), h.nchunks, fp);
    fclose (fp);

    /*Digest materials into something usable*/
    std::vector<MyMaterial *> mymats;
    for (auto i = 0u; i < h.nmats; i++)
    {
        MyMaterial *mat = new MyMaterial;
        mat->path = prefix + "/txt16/" + mats[i].name;
        mat->alpha = mats[i].alpha;
        mat->emissive = mats[i].emissive;
        mymats.push_back (mat);
    }

    /*Digest model data into something usable*/
    std::vector<Mesh *> meshes;
    auto i2v = new int32_t[h.nverts];
    auto v2i = new int32_t[h.nverts];
    for (auto i = 0u; i < h.nmats; i++)
    {
        uint32_t nverts = 0;
        uint32_t nindices = 0;
        for (auto j = 0u; j < h.nverts; j++)
        {
            i2v[j] = -1;
            v2i[j] = -1;
        }

        /*Count and remap index values*/
        for (auto j = 0u; j < h.ntris; j++)
        {
            Tri *t = tris + j;
            if (t->slot != i)
            {
                continue;
            }
            for (auto k = 0u; k < 3; k++)
            {
                if (i2v[t->indices[k]] != -1)
                {
                    continue;
                }
                i2v[t->indices[k]] = nverts;
                v2i[nverts] = t->indices[k];
                nverts++;
            }
            nindices += 3;
        }

        /*Create canonical vertex from LGS format*/
        auto mesh = new Mesh;
        mesh->verts = new Vertex[nverts];
        for (auto j = 0u; j < nverts; j++)
        {
            uint32_t index = v2i[j];
            Vertex *v = mesh->verts + j;
            const float *xyz = verts + 3*index;
            const UV *uv = uvs + index;
            v->xyz[0] = xyz[0];
            v->xyz[1] = xyz[1];
            v->xyz[2] = xyz[2];
            /*30 bit normals, 10 bits each component*/
            v->nom[0] = ((uv->n>>22)&0x3ff)/512.0 - 1.0;
            v->nom[1] = ((uv->n>>12)&0x3ff)/512.0 - 1.0;
            v->nom[2] = ((uv->n>>2)&0x3ff)/512.0 - 1.0;
            v->uv[0] = uv->u;
            v->uv[1] = uv->v;
        }

        /*Create remapped indices*/
        mesh->indices = new uint16_t[nindices];
        uint32_t index = 0;
        for (auto j = 0u; j < h.ntris; j++)
        {
            Tri *t = tris + j;
            if (t->slot != i)
            {
                continue;
            }
            mesh->indices[index + 0] = i2v[t->indices[0]];
            mesh->indices[index + 1] = i2v[t->indices[1]];
            mesh->indices[index + 2] = i2v[t->indices[2]];
            index += 3;
        }

        /*Assign parent bone*/
        for (auto j = 0u; j < h.nchunks; j++)
        {
            Chunk *c = chunks + j;
            uint8_t *remap = map + h.nsegs;
            for (auto k = 0u; k < c->nverts; k++)
            {
                uint32_t n = 0;
                for (auto& b : skel->bones ())
                {
                    if (b->handle == segs[c->bone].id)
                    {
                        break;
                    }
                    n++;
                }
                mesh->verts[i2v[c->verts + k]].id = n;
            }
        }

        /*Assign material slot*/
        mesh->slot = i;
        mesh->nverts = nverts;
        mesh->nindices = nindices;
        /*Add to mesh list*/
        meshes.push_back (mesh);
    }

    auto mdl = new Model;
    mdl->_materials = mymats;
    mdl->_meshes = meshes;

    /*Clean up work area*/
    delete [] i2v;
    delete [] v2i;
    delete [] chunks;
    delete [] uvs;
    delete [] verts;
    delete [] tris;
    delete [] mats;

    return mdl;
}
