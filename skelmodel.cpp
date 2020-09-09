#include "skelmodel.h"

skelmodel::skelmodel (Skel *skel)
{
    set_skel (skel);
}

void skelmodel::set_skel (Skel *skel)
{
    invisibleRootItem ()->clearData();
    invisibleRootItem ()->removeRows(0, invisibleRootItem()->rowCount ());
    _bones.clear ();

    if (nullptr == skel)
    {
        return;
    }

    for (auto& bone : skel->bones ())
    {
        auto item = new QStandardItem ();
        item->setText (QString (bone->name.c_str ()));
        _bones.push_back (item);
    }

    invisibleRootItem ()->appendRow (_bones[0]);
    for (auto& ndx : skel->indices ())
    {
        if (ndx.parent < 0)
        {
            invisibleRootItem ()->appendRow (_bones[ndx.id]);
            continue;
        }
        _bones[ndx.parent]->appendRow (_bones[ndx.id]);
    }
    _skel = skel;
}

