#ifndef EXPORTFUNCS_H
#define EXPORTFUNCS_H

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
#include "bezierdata.h"

using namespace std;

void writeBytes(iostream *stream, const char* data, size_t length);
void writeNulls(iostream *stream , size_t length);

void readBytes(iostream *stream, void* _ptr, size_t length);
bool readNulls(iostream *stream, size_t length);
bool readBool(iostream *stream);
int readInt(iostream *stream);
float readFloat(iostream *stream);
string readString(iostream *stream, size_t length);
glm::vec3 readVec3(iostream *stream);

void writeToExportFile(fstream *file, QList<BezierData*> &bezList);

#endif // EXPORTFUNCS_H
