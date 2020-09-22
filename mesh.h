#ifndef MESH_H
#define MESH_H

#include <cstdint>
#include <string>
#include <vector>
#include "skel.h"

class Model
{
public:
    struct MyMaterial
    {
        std::string path;
        float alpha;
        float emissive;
    };
    struct Vertex
    {
        float xyz[3];
        float nom[3];
        float uv[2];
        uint32_t id;
    };
    struct Mesh
    {
        uint32_t slot;
        Vertex *verts;
        uint16_t *indices;
        uint32_t nverts;
        uint32_t nindices;
    };

    static Model *from_file (std::string& path, std::string& prefix, Skel *skel);

    ~Model ()
    {
        for (auto& mesh : _meshes)
        {
            delete [] mesh->verts;
            delete [] mesh->indices;
        }
    }

    std::vector<MyMaterial *>& materials () {return _materials;}
    std::vector<Mesh *>& meshes () {return _meshes;}
private:
    explicit Model() {};
    std::vector<MyMaterial *> _materials;
    std::vector<Mesh *> _meshes;

private: /*Dark structs*/
    struct Header
    {
        char id[4];
        uint32_t version;

        uint32_t unk1;
        uint32_t unk2;
        uint32_t unk3;

        uint8_t unk4;
        uint8_t nsegs;
        uint8_t nmats;
        uint8_t nchunks;
        uint16_t ntris;
        uint16_t nverts;
        uint32_t unk5;

        uint32_t map;
        uint32_t segs;
        uint32_t mats;
        uint32_t chunks;
        uint32_t tris;
        uint32_t norms;
        uint32_t points;
        uint32_t uvs;
        uint32_t unk6;
    };
    struct Segment
    {
        uint32_t unk1;
        uint8_t id;
        uint8_t unk2;
        uint8_t unk3;
        uint8_t unk4;
        uint32_t unk5[3];
    };
    struct Chunk
    {
        uint16_t ntris;
        uint16_t tris;
        uint16_t nverts;
        uint16_t verts;
        uint16_t unk1;
        uint16_t unk2;
        uint16_t unk3;
        uint16_t bone;
    };
    struct Material
    {
        char name[16];
        uint32_t caps;
        float alpha;
        float emissive;
        uint8_t unk3[0x1c];
    };
    struct UV
    {
        float u, v;
        uint32_t n;
    };
    struct Tri
    {
        uint16_t indices[3];
        uint16_t slot;
        uint8_t unk[8];
    };
};

#endif // MESH_H
