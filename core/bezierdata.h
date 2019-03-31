#ifndef BEZIER_DATA_H
#define BEZIER_DATA_H

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

#include "glm.hpp"

struct BezierData
{
    glm::vec3 Kp1;
    glm::vec3 Kp2;
    glm::vec3 P1;
    float roll;
    bool contRoll;
    bool equalDist;
    bool relRoll;

    float ptf;
    float fvdRoll;
    float length;
    int numNodes;
    float fVel;
};

#endif // BEZIER_DATA_H
