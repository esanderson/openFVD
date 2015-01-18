/*
#    FVD++, an advanced coaster design tool for NoLimits
#    Copyright (C) 2012-2014, Stephan "Lenny" Alt <alt.stephan@web.de>
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

#include "subfunction.h"
#include "function.h"
#include "section.h"
#include "track.h"
#include "mnode.h"

#include "exportfuncs.h"

using namespace std;

float interpolate(float t, float x1, float x2)
{
    // (t * (x2 - x1)) + x1
    return (t * x2) + x1 - (t * x1);
}

float interpolate(float t, float x1, float x2, float x3)
{
    // ((1-t)^2 * x1) + (2 * t * (1-t) * x2) + (t^2 * x3)
    float t1 = 1-t;
    return t1*t1*x1 + 2*t*t1*x2 + t*t*x3;
}

float interpolate(float t, float x1, float x2, float x3, float x4)
{
    // ((1-t)^3 * x1) + (3 * t * (1-t)^2 * x2) + (3 * t^2 * (1-t) * x3) + (t^3 * x4)
    float t1 = 1-t;
    return t1*t1*t1*x1 + 3*t*t1*t1*x2 + 3*t*t*t1*x3 + t*t*t*x4;
}

subfunction::subfunction(float xStart, float xEnd, float yStart, float symArg, function* parent)
{
    _xStart = xStart;
    _xEnd = xEnd;
    _yStart = yStart;

    _symArg = symArg;

    _centerArg = 0.f;
    _tensionArg = 0.f;

    _parent = parent;
    lenAssert(parent != NULL);

    if (_parent->type == funcNormal)
        changeDegree(cubic);
    else
        changeDegree(quartic);

    _isLocked = false;
}

void subfunction::update(float xStart, float xEnd, float symArg)
{
    _xStart = xStart;
    _xEnd = xEnd;

    _symArg = symArg;

    _parent->translateValues(this);
}

void subfunction::updateBez()
{
    valueList.clear();

    float t = 0.0f;
    float nextT = 0.0f;
    float gotT;

    for (int i = 0; i < 100; ++i)
    {
        valueList.append(interpolate(t, 0, pointList[0].y, pointList[1].y, 1));

        nextT += 0.01f;
        gotT = interpolate(t, 0, pointList[0].x, pointList[1].x, 1);
        t += (nextT - gotT) / (3 * interpolate(t, pointList[0].x, pointList[1].x - pointList[0].x, 1.0f - pointList[1].x));
    }

    valueList.append(1.0f);
}

void subfunction::changeDegree(eDegree newDegree)
{
    _degree = newDegree;

    switch (newDegree)
    {
    case linear:
        break;
    case quadratic:
        break;
    case cubic:
        break;
    case quartic:
        _arg1 = -10.0f;
        break;
    case quintic:
        _arg1 = 0.0f;
        break;
    case sinusoidal:
        break;
    case plateau:
        _arg1 = 1.0f;
        break;
    case freeform:
        pointList.clear();
        bez_t b;
        b.x = 0.3f;
        b.y = 0.0f;
        pointList.append(b);
        b.x = 0.7f;
        b.y = 1.0f;
        pointList.append(b);
        updateBez();
        break;
    case tozero:
        _centerArg = 0.0f;
        _tensionArg = 0.0f;
        _symArg = -_yStart;
        break;
    default:
        lenAssert(0 && "unknown degree");
    }
    return;
}

float subfunction::getValue(float x)
{
    if (_isLocked)
    {
        _parent->changeLength(_parent->secParent->getMaxArgument() - _xStart, _parent->getSubfunctionNumber(this));
        //maxArgument = parent->secParent->getMaxArgument();
    }
    else if (x > _xEnd)
    {
        qWarning("Function got parameter out of bounds: x = %f", x);
        x = _xEnd;
    }
    else if (x < _xStart)
    {
        qWarning("Function got parameter out of bounds: x = %f", x);
        x = _xStart;
    }

    // Scale x between the start and end
    x = (x - _xStart) / (_xEnd - _xStart);

    x = applyCenter(x);
    x = applyTension(x);

    switch (_degree)
    {
    case linear:
        return _symArg * x + _yStart;

    case quadratic:
        if (isSymmetric())
        {
            // symArg * (1 - (2x - 1)^2)         + yStart
            // symArg * (1 - (4x^2 - 4x + 1))    + yStart
            // symArg * (4x - 4x^2)              + yStart
            // 4 * symArg * x - 4 * symArg * x^2 + yStart
            // @ 0.00, return                 yStart
            // @ 0.25, return 0.75 * symArg + yStart
            // @ 0.50, return        symArg + yStart
            // @ 0.75, return 0.75 * symArg + yStart
            // @ 1.00, return                 yStart
            x = 2.0f * x - 1.0f;
            return _symArg * (1.0f - x * x) + _yStart;
        }
        else if (_arg1 < 0.f)
        {
            // Lead out
            // symArg * (1 - (1 - x)^2)      + yStart
            // symArg * (1 - (1 - 2x + x^2)) + yStart
            // symArg * (2x - x^2)           + yStart
            // 2 * symArg * x - symArg * x^2 + yStart
            // @ 0.00, return                    yStart
            // @ 0.25, return (7/16)  * symArg + yStart
            // @ 0.50, return 0.75    * symArg + yStart
            // @ 0.75, return (15/16) * symArg + yStart
            // @ 1.00, return           symArg + yStart
            return _symArg * (1.0f - (1.0f - x) * (1.0f - x)) + _yStart;
        }
        else
        {
            // Lead in
            // symArg * x^2 + yStart
            // @ 0.00, return                   yStart
            // @ 0.25, return (1/16) * symArg + yStart
            // @ 0.50, return 0.25   * symArg + yStart
            // @ 0.75, return (9/16) * symArg + yStart
            // @ 1.00, return          symArg + yStart
            return _symArg * x * x + _yStart;
        }

    case cubic:
        return _symArg*x*x*(3 + x*(-2)) + _yStart;

    case quartic:
        if(!isSymmetric())
        {
            return x*x*(-(6*_symArg*_arg1)/(1-2*_arg1)+x*(_symArg*(4*_arg1+4)/(1-2*_arg1)+x*((-3*_symArg/(1-2*_arg1)))))+_yStart;
        }
        else
        {
            return _symArg*x*x*(16+x*(-32+x*16))+_yStart;
        }

    case quintic: 
        if (fabs(_arg1) < 0.005)
        {
            return _symArg*x*x*x*(10+x*(-15+x*6))+_yStart;
        }
        else if(_arg1 < 0)
        {
            float root = -sqrt(9+fabs(_arg1/10.f)*(-16+16*fabs(_arg1/10.f)));
            float max = 0.01728+0.00576*root + fabs(_arg1/10.f)*(-0.0288-0.00448*root + fabs(_arg1/10.f)*(0.0032-0.00576*root + fabs(_arg1/10.f)*(-0.0704+0.02048*root + fabs(_arg1/10.f)*(0.1024-0.01024*root + _arg1/10.f*0.04096))));
            return _symArg/max*x*x*(x-1)*(x-1)*(x+_arg1/10.f)+_yStart;
        }
        else
        {
            float root = sqrt(9+_arg1/10.f*(-16+16*_arg1/10.f));
            float max = 0.01728+0.00576*root + _arg1/10.f*(-0.0288-0.00448*root + _arg1/10.f*(0.0032-0.00576*root + _arg1/10.f*(-0.0704+0.02048*root + _arg1/10.f*(0.1024-0.01024*root - _arg1/10.f*0.04096))));
            return _symArg/max*x*x*(x-1)*(x-1)*(x-_arg1/10.f)+_yStart;
        }

    case sinusoidal:
        return 0.5f*_symArg*(1-cos(F_PI*x))+_yStart;

    case plateau:
        //return symArg*(1.f-exp(-20.f*pow(sin(F_PI*x), 4))) +startValue;
        return _symArg*(1.f-(exp(-_arg1*15.f*(pow(1.f-fabs(2.f*x-1.f), 3)))))+_yStart;

    case freeform:
    {
        float root = (x*(valueList.size()-2));
        float max = floor(root)+0.01;
        root = root-floor(root);
        if((int)max == valueList.size()-1)
        {
            return root*_symArg*valueList[(int)max]+_yStart;
        }
        else
        {
           return (1-root)*_symArg*valueList[(int)max]+root*_symArg*valueList[(int)(max+1)] +_yStart;
        }
    }

    case tozero:
    {
        float d, e;

        section* secParent = _parent->secParent;
        track* trackParent = _parent->secParent->parent;

        mnode* curNode  = trackParent->getPoint(trackParent->getNumPoints(secParent) + (_xStart * 1000.f) - 0.5f);
        mnode* prevNode = trackParent->getPoint(trackParent->getNumPoints(secParent) + (_xStart * 1000.f) - 1.5f);

        if (secParent->bOrientation == EULER)
        {
            d = (curNode->fRollSpeed + glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f)) * curNode->fYawFromLast
                - prevNode->fRollSpeed
                - glm::dot(prevNode->vDir, glm::vec3(0.f, -1.f, 0.f)) * prevNode->fYawFromLast) * F_HZ;
            e = _yStart;
        }
        else
        {
            d = (curNode->fRollSpeed + glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f)) * curNode->fYawFromLast
                - prevNode->fRollSpeed
                - glm::dot(prevNode->vDir, glm::vec3(0.f, -1.f, 0.f)) * prevNode->fYawFromLast) * F_HZ;
            e = -glm::dot(curNode->vDir, glm::vec3(0.f, -1.f, 0.f)) * curNode->fYawFromLast * F_HZ;
            e += _yStart;
        }

        _arg1 = -curNode->fRoll / (_xEnd - _xStart);

        float a = -2.5f * (d + 6.f * (e - 2.f * _arg1));
        float b = 6.f * d + 32.f * e - 60.f * _arg1;
        float c = -d * 4.5f - 18.f * e + 30.f * _arg1;
        return x * (d + x * (c + x * (b + x * a))) + e;
    }

    default:
        qWarning("unknown degree");
        return -1.0f;
    }
}

// relic, doesn't get used at all at this time
//float subfunction::getMinValue()
//{
//    return startValue < endValue() ? startValue : endValue();
//}

// relic, doesn't get used at all at this time
//float subfunction::getMaxValue()
//{
//    return startValue > endValue() ? startValue : endValue();
//}

void subfunction::translateValues(float newStart)
{
    _yStart = newStart;

    if(_degree == tozero)
        _symArg = -_yStart;
}

bool subfunction::isSymmetric()
{
    if(_degree == quadratic && fabs(_arg1) < 0.5f)
        return true;
    if(_degree == quartic && _arg1 < 0)
        return true;
    if(_degree == quintic && fabs(_arg1) > 0.005f)
        return true;
    if(_degree == plateau)
        return true;
    return false;
}

void subfunction::saveSubFunc(fstream& file)
{
    writeBytes(&file, (const char*)&_degree,     sizeof(enum eDegree));
    writeBytes(&file, (const char*)&_xStart,     sizeof(float));
    writeBytes(&file, (const char*)&_xEnd,       sizeof(float));
    writeBytes(&file, (const char*)&_yStart,     sizeof(float));
    writeBytes(&file, (const char*)&_arg1,       sizeof(float));
    writeBytes(&file, (const char*)&_symArg,     sizeof(float));
    writeBytes(&file, (const char*)&_centerArg,  sizeof(float));
    writeBytes(&file, (const char*)&_tensionArg, sizeof(float));
    writeBytes(&file, (const char*)&_isLocked,   sizeof(bool));
}

void subfunction::saveSubFunc(stringstream& file)
{
    writeBytes(&file, (const char*)&_degree,     sizeof(enum eDegree));
    writeBytes(&file, (const char*)&_xStart,     sizeof(float));
    writeBytes(&file, (const char*)&_xEnd,       sizeof(float));
    writeBytes(&file, (const char*)&_yStart,     sizeof(float));
    writeBytes(&file, (const char*)&_arg1,       sizeof(float));
    writeBytes(&file, (const char*)&_symArg,     sizeof(float));
    writeBytes(&file, (const char*)&_centerArg,  sizeof(float));
    writeBytes(&file, (const char*)&_tensionArg, sizeof(float));
    writeBytes(&file, (const char*)&_isLocked,   sizeof(bool));
}

void subfunction::loadSubFunc(fstream& file)
{
    _degree     = (eDegree)readInt(&file);
    _xStart     = readFloat(&file);
    _xEnd       = readFloat(&file);
    _yStart     = readFloat(&file);
    _arg1       = readFloat(&file);
    _symArg     = readFloat(&file);
    _centerArg  = readFloat(&file);
    _tensionArg = readFloat(&file);
    _isLocked   = readBool(&file);
}

void subfunction::legacyLoadSubFunc(fstream& file)
{
    _degree     = (eDegree)readInt(&file);
    _xStart     = readFloat(&file);
    _xEnd       = readFloat(&file);
    _yStart     = readFloat(&file);
    _arg1       = readFloat(&file);
    _symArg     = readFloat(&file);
    _centerArg  = readFloat(&file);
    _tensionArg = readFloat(&file);
    _isLocked   = readBool(&file);
}

void subfunction::loadSubFunc(stringstream& file)
{
    _degree     = (eDegree)readInt(&file);
    _xStart     = readFloat(&file);
    _xEnd       = readFloat(&file);
    _parent->changeLength(_xEnd - _xStart, _parent->getSubfunctionNumber(this));
    _yStart     = readFloat(&file);
    _arg1       = readFloat(&file);
    _symArg     = readFloat(&file);
    _centerArg  = readFloat(&file);
    _tensionArg = readFloat(&file);
    _isLocked   = readBool(&file);
}

eDegree subfunction::degree() { return _degree; }
void subfunction::setDegree(eDegree value) { _degree = value; }

float subfunction::xStart() { return _xStart; }
float subfunction::xEnd() { return _xEnd; }
float subfunction::yStart() { return _yStart; }

float subfunction::_arg1() { return _arg1; }
void  subfunction::setArg1(float value) { _arg1 = value; }

float subfunction::symArg() { return _symArg; }
void  subfunction::setSymArg(float value) { _symArg = value; }

bool subfunction::isLocked() { return _isLocked; }
void subfunction::lock() { _isLocked = true; }
void subfunction::unlock() { _isLocked = false; }

function* subfunction::parent() { return _parent; }

float subfunction::applyTension(float x)
{
    if (fabs(_tensionArg) < 0.0005)
    {
        return x;
    }
    else if (_tensionArg > 0.f)
    {
        x = 2.0f * _tensionArg * (x - 0.5f);
        x = sinh(x) / sinh(_tensionArg);
        x = 0.5f * (x + 1.0f);
    }
    else
    {
        x = 2.0f * sinh(_tensionArg) * (x - 0.5f);
        x = asinh(x) / _tensionArg;
        x = 0.5f * (x + 1.0f);
    }

    return x;
}

float subfunction::applyCenter(float x)
{
    if (_centerArg > 0.f)
    {
        //x = sinh(x*centerArg)/sinh(centerArg);
        x = pow(x, pow(2.0f, _centerArg / 2.0f));
    }
    else if (_centerArg < 0.f)
    {
        //x = sinh((x-1.f)*centerArg)/sinh(centerArg)+1;
        x = 1.0f - pow(1.0f - x, pow(2, -_centerArg / 2.0f));
    }

    return x;
}

float subfunction::endValue()
{
    if (isSymmetric())
        return _yStart;
    else
        return _yStart + _symArg;
}

int subfunction::getNodeIndex()
{
    return (int)((_xStart * F_HZ) - 1.5f);
}
