#ifndef SKELMODEL_H
#define SKELMODEL_H

#include <QObject>
#include <QStandardItemModel>
#include <QStandardItem>
#include "skel.h"

class skelmodel : public QStandardItemModel
{
    Q_OBJECT
public:
    skelmodel (Skel *skel);
    void set_skel (Skel *skel);
private:
    QList<QStandardItem *> _bones;
    Skel *_skel;
};

#endif // SKELMODEL_H
