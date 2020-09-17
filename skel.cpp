#include <functional>
#include <vector>
#include <cstdio>
#include <cstring>
#include "skel.h"

static const char *_biped[] = {

};

Skel::~Skel ()
{
    for (auto b : _bones)
    {
        delete b;
    }
    _bones.clear ();
    _indices.clear ();
    _handle2bone.clear ();
}

Skel *
Skel::from_file (std::string& path)
{
    FILE *fp = fopen (path.c_str (), "rb");
    if (!fp)
    {
        return nullptr;
    }

    /*Ensure the magick is there*/
    uint32_t magick;
    fread (&magick, sizeof (magick), 1, fp);
    if (magick != 1)
    {
        return nullptr;
    }

    /*Pull out the torso and limb counts*/
    uint32_t ntorsos, nlimbs;
    fread (&ntorsos, sizeof (ntorsos), 1, fp);
    if (ntorsos >= MAX_SOCKETS)
    {
        return nullptr;
    }
    fread (&nlimbs, sizeof (nlimbs), 1, fp);
    if (nlimbs >= MAX_SOCKETS)
    {
        return nullptr;
    }

    /*Pull out the torsos and limbs... gross*/
    auto torsos = new Torso[ntorsos];
    fread (torsos, sizeof (torsos[0]), ntorsos, fp);
    auto limbs = new Limb[nlimbs];
    for (auto i = 0u; i < nlimbs; i++)
    {/*Limb contains alignment padding...*/
        Limb *l = limbs + i;
        fread (&l->torso, sizeof (l->torso), 1, fp);
        fread (&l->unk, sizeof (l->unk), 1, fp);
        fread (&l->nsockets, sizeof (l->nsockets), 1, fp);
        fread (&l->limb, sizeof (l->limb), 1, fp);
        fread (&l->children, sizeof (l->children), 1, fp);
        fread (&l->sockets, sizeof (l->sockets), 1, fp);
        fread (&l->lengths, sizeof (l->lengths), 1, fp);
    }

    /*Convert weird LGS skeleton to canonical form
    Torsos and limbs are named by handle instead of strings, and,
    to make it more confusing, multiple torsos can share the same handle.
    LGS permits multiple roots in theory, but only the first root is given
    position data. The rest are treated like limbs and only keep rotation,
    so functionally the skeleton only ever has one root. Torsos may have
    multiple children. The name is a misnomer - human characters have a torso
    for their pelvis and their proper torso!

    Limbs are chains of bones, each bone in the chain has one child, except for
    the last, which has none. Weirdly, instead of points, each bone is stored
    as a direction + length combination. Otherwise these are straight forward.
    */
    auto sk = new Skel;
    sk->bone_create (torsos[0].handle);
    for (auto i = 0u; i < ntorsos; i++)
    {
        const Torso *t = torsos + i;
        Bone *tb = sk->bone_by_handle (t->handle);
        for (auto j = 0; j < t->nsockets; j++)
        {
            auto b = sk->bone_create (t->children[j]);
            b->t[0] = t->sockets[j][0];
            b->t[1] = t->sockets[j][1];
            b->t[2] = t->sockets[j][2];
            sk->bone_child_add (tb, b);
        }
    }
    for (auto i = 0u; i < nlimbs; i++)
    {
        const Limb *l = limbs + i;
        Bone *lb = sk->bone_by_handle (l->limb);
        for (auto j = 0; j < l->nsockets; j++)
        {
            auto b = sk->bone_create (l->children[j]);
            b->t[0] = l->lengths[j]*l->sockets[j][0];
            b->t[1] = l->lengths[j]*l->sockets[j][1];
            b->t[2] = l->lengths[j]*l->sockets[j][2];
            sk->bone_child_add (lb, b);
            lb = b;
        }
    }

    /*Digest the bone tree into a flat indexable array.
    Would be better to just sort $_bones and remap parent*/
    std::function<void (Bone *, int)> fold = [&](Bone *bone, int32_t parent)
    {
        Indexed data;
        /*Find index of bone in $_bones*/
        int32_t index = -1;
        for (auto i = 0u; i < sk->_bones.size (); i++)
        {
            if (sk->_bones[i] != bone) continue;
            index = (int32_t)i;
        }
        /*Fill out the indexed data and append it to the list*/
        data.id = index;
        data.parent = parent;
        sk->_indices.push_back (data);
        /*Recurse into children*/
        for (auto ch : bone->children)
        {
            fold (ch, index);
        }
    };
    fold (sk->_bones[0], -1);

    /*Clean up the work area*/
    delete [] torsos;
    delete [] limbs;
    fclose (fp);

    /*Create result*/
    return sk;
}

Bone *
Skel::bone_create (int32_t handle)
{   /*Ensure handle is unique*/
    if (bone_by_handle(handle))
    {
        return nullptr;
    }
    /*Okay, cool create the new bone and initialise it*/
    auto b = new Bone;
    b->handle = handle;
    b->parent = -1;
    b->name = "Bone " + std::to_string (handle);

    for (auto i = 0; i < 4; i++)
    {
        b->t[i] = 0.0f;
        b->r[i] = 0.0f;
    }
    b->r[3] = 1.0f;

    /*Insert the bone into the containers*/
    _handle2bone.insert ({handle, b});
    _bones.push_back (b);
    return b;
}
Bone *
Skel::bone_by_handle (int32_t handle)
{
    auto it = _handle2bone.find (handle);
    if (_handle2bone.end () == it)
    {
        return nullptr;
    }
    return it->second;
}
Bone *
Skel::bone_child_add (Bone *parent, Bone *child)
{
    parent->children.push_back (child);
    child->parent = parent->handle;
    return parent;
}
