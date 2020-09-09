#ifndef SKEL_H
#define SKEL_H

#include <unordered_map>
#include <cstdint>
#include <string>
#include <vector>

#define MAX_SOCKETS     16
struct Bone
{
    std::vector<Bone *> children;
    int32_t handle;
    int32_t parent;
    std::string name;
    float t[4];
    float r[4];
};
class Skel
{
public:
    struct Indexed
    {
        int32_t id;
        int32_t parent;
    };
public:
    ~Skel ();

    static Skel *from_file (std::string& path);
    Bone *bone_create (int32_t handle);
    Bone *bone_by_handle (int32_t handle);
    Bone *bone_child_add (Bone *parent, Bone *child);

    const std::vector<Bone *>& bones () {return _bones;};
    const std::vector<Indexed>& indices () {return _indices;};

private:
    explicit Skel () {};
private:
    std::vector<Bone *> _bones;
    std::vector<Indexed> _indices;
    std::unordered_map<int32_t, Bone *> _handle2bone;
private: /*Dark structs*/
    typedef float Dv3[3];
    struct Torso
    {
        int32_t handle;
        int32_t parent;
        int32_t nsockets;
        int32_t children[MAX_SOCKETS];
        Dv3 sockets[MAX_SOCKETS];
    };
    struct Limb
    {
        int32_t torso;
        int32_t unk;
        int32_t nsockets;
        int16_t limb;
        int16_t children[MAX_SOCKETS];
        Dv3 sockets[MAX_SOCKETS];
        float lengths[MAX_SOCKETS];
    };
};

#endif // SKEL_H
