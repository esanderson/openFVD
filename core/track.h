#ifndef TRACK_H
#define TRACK_H

/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2015, Stephan "Lenny" Alt <alt.stephan@web.de>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <fstream>

#include <QList>
#include <QString>
#include "mnode.h"
#include "secstraight.h"
#include "seccurved.h"
#include "secforced.h"
#include "secgeometric.h"
#include "secbezier.h"
#include "secnlcsv.h"
#include "sectionhandler.h"

class optionsMenu;
class trackWidget;
class smoothUi;
class smoothHandler;
class trackHandler;

using namespace std;

enum trackStyle {
    generic = 0,    // 0,5m
    genericflat,    // 0,7m
    vekoma,         // 0,6m
    bm,             // 0,6m
    triangle,       // 0,5m
    box,            // 0,5m
    smallflat,      // 0,5m
    doublespine
};

class track
{
public:
    track();
    track(trackHandler* _parent, glm::vec3 startPos, float startYaw, float heartLine = 0.0);
    ~track();

    void removeSection(int index);
    void removeSection(section* fromSection);

    void removeSmooth(int fromNode = 0);
    void applySmooth(int fromNode = 0);

    void updateTrack(int index, int iNode);
    void updateTrack(section* fromSection, int iNode);
    void newSection(enum secType type, int index = -1);

    int exportTrack(fstream* file, float mPerNode, int fromIndex, int toIndex, float fRollThresh);
    int exportTrack2(fstream* file, float mPerNode, int fromIndex, int toIndex, float fRollThresh);
    int exportTrack3(fstream* file, float mPerNode, int fromIndex, int toIndex, float fRollThresh);
    int exportTrack4(fstream* file, float mPerNode, int fromIndex, int toIndex, float fRollThresh);

    void exportNL2Track(FILE *file, float mPerNode, int fromIndex, int toIndex);

    QString saveTrack(fstream& file, trackWidget* _widget);
    QString loadTrack(fstream& file, trackWidget* _widget);
    QString legacyLoadTrack(fstream& file, trackWidget* _widget);
    
    mnode* getPoint(int index);
    int getIndexFromDist(float dist);
    int getNumPoints(section* until = NULL);
    int getSectionNumber(section* _section);

    void getSecNode(int index, int *node, int *section);

    // Initialized to true (not persisted)
    // Set to true:
    // * In updateTrack if index is beyond the number of sections
    // * At the end of updateTrack
    // * At the end of newSection
    // * graphWidget#selectionChanged
    // * smoothUi#applyRollSmooth
    // * TrackProperites#ondefaultColor_received
    // * TrackProperties#onsectionColor_received
    // * TrackProperties#ontransitionColor_received
    // * TrackProperties#on_buttonBox_accepted when style or isWireFrame changes
    // * trackWidget#on_sectionListWidget_itemSelectionChanged
    // * trackWidget#on_ybox_valueChanged
    // Reset to false in the following after checking hasChanged and calling mMesh#recolorTrack:
    // * glviewwidget#drawOcclusion
    // * glviewWidget#paintGL
    // TODO "needsRecolor"?
    bool hasChanged;

    // Initialized to true and persisted by this class.
    // Used in glviewwidget when rendering and set in projectwidget.
    bool isDrawn;

    // If the track is drawn (isDrawn), draw the track, hearline, or both
    // Initilized to "Everything" and persisted by this class.
    // Used in glviewwidget
    // Set in trackproperties
    // Taken from trackproperties.ui (drawBox)
    // 0 = Everything (track and heartline)
    // 1 = Track only
    // 2 = Heartline only
    int drawMode;

    static const int DRAW_MODE_BOTH = 0;
    static const int DRAW_MODE_TRACK_ONLY = 1;
    static const int DRAW_MODE_HEARTLINE_ONLY = 2;

    mnode* anchorNode;

    glm::vec3 startPos;
    float startYaw;
    float startPitch;

    section* activeSection;
    float fHeart;
    float fFriction;
    float fResistance;
    QList<section*> lSections;

    optionsMenu* mOptions;
    QString name;

    smoothUi* smoother;

    QList<smoothHandler*> smoothList;

    trackHandler* mParent;

    int smoothedUntil;
    enum trackStyle style;
    glm::vec2 povPos;
};

#endif // TRACK_H
