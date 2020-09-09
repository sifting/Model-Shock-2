#ifndef MESH_H
#define MESH_H

#include <cstdint>
#include <string>

class Mesh
{
public:
    static Mesh *from_file (std::string& prefix);
private:
    explicit Mesh() {};
private:
};

#endif // MESH_H
