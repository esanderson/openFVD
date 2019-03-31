#ifndef SECNLCSV_H
#define SECNLCSV_H

#include <QMap>
#include "track.h"
#include "section.h"

class secnlcsv : public section
{
public:
    secnlcsv(track* getParent, mnode* first);
    virtual int updateSection(int node = 0);

    virtual void saveSection(iostream& stream);
    virtual void loadSection(iostream& stream);
    virtual void legacyLoadSection(iostream& stream);

    virtual float getMaxArgument();
    virtual bool isLockable(func* _func);
    virtual bool isInFunction(int index, subfunc* func);
    void loadTrack(QString filename);

private:
    QList<mnode> csvNodes;
    void initDistances();
    mnode getNodeAtDistance(float distance);
};

#endif // SECNLCSV_H
