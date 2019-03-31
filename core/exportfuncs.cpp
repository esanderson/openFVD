
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

#include "exportfuncs.h"

void writeBytes(iostream *stream, const char* data, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        stream->write(data + length - 1 - i, 1);
    }
}

void readBytes(iostream *stream, void* _ptr, size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        stream->read((char*)_ptr + length - 1 - i, 1);
    }
}

void writeNulls(iostream *stream, size_t length)
{
    char data[1] = {0x00};
    for (size_t i = 0; i < length; ++i) {
        writeBytes(stream, data, 1);
    }
}

bool readNulls(iostream *stream, size_t length)
{
    char c;
    for (size_t i = 0; i < length; ++i) {
        stream->get(c);
    }
    return true;
}

bool readBool(iostream *stream)
{
    char temp;
    stream->get(temp);
    return temp != 0;
}

int readInt(iostream *stream)
{
    union {
        char c[4];
        int i;
    } temp;
    stream->get(temp.c[3]);
    stream->get(temp.c[2]);
    stream->get(temp.c[1]);
    stream->get(temp.c[0]);
    return temp.i;
}

float readFloat(iostream *stream)
{
    union {
        char c[4];
        float f;
    } temp;
    stream->get(temp.c[3]);
    stream->get(temp.c[2]);
    stream->get(temp.c[1]);
    stream->get(temp.c[0]);
    return temp.f;
}

string readString(iostream *stream, size_t length)
{
    string temp = "";
    char c;
    for (size_t i = 0; i < length; ++i) {
        stream->get(c);
        temp.append(1, c);
    }
    return temp;
}

glm::vec3 readVec3(iostream *stream)
{
    glm::vec3 temp(readFloat(stream), readFloat(stream), readFloat(stream));
    return temp;
}

void writeToExportFile(fstream *file, QList<BezierData*> &bezList)
{
    for (int i = 0; i < bezList.size(); ++i) {
        writeBytes(file, (const char*)&bezList[i]->Kp1.x, 4);
        writeBytes(file, (const char*)&bezList[i]->Kp1.y, 4);
        writeBytes(file, (const char*)&bezList[i]->Kp1.z, 4);

        writeBytes(file, (const char*)&bezList[i]->Kp2.x, 4);
        writeBytes(file, (const char*)&bezList[i]->Kp2.y, 4);
        writeBytes(file, (const char*)&bezList[i]->Kp2.z, 4);

        writeBytes(file, (const char*)&bezList[i]->P1.x, 4);
        writeBytes(file, (const char*)&bezList[i]->P1.y, 4);
        writeBytes(file, (const char*)&bezList[i]->P1.z, 4);

        writeBytes(file, (const char*)&bezList[i]->roll, 4);

        char cTemp = 0xFF;
        writeBytes(file, &cTemp, 1); // CONT ROLL
        cTemp = bezList[i]->relRoll ? 0xFF : 0x00;
        writeBytes(file, &cTemp, 1); // REL ROLL
        cTemp = 0x00;
        writeBytes(file, &cTemp, 1); // equal dist CP
        writeNulls(file, 7); // were 5
    }
}
