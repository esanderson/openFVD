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

#include "function.h"
#include "section.h"

#include "exportfuncs.h"
#include "lenassert.h"

function::~function()
{
    while(funcList.size() != 0) {
        delete funcList.at(0);
        funcList.removeAt(0);
    }
}

function::function(float min, float max, float start, float end, section* _parent, enum eFunctype newtype)
    : activeSubfunction(-1), type(newtype), secParent(_parent), startValue(start)
{
    funcList.append(new subfunction(min, max, start, end - start, this));
}

// Finds the subfunction that contains x by using the maxArgument of each
// subfunction; then, calls getValue on that subfunction
float function::getValue(float x)
{
    int i = 0;
    /*while((i < funcList.size()-1 && x > funcList[i+1]->minArgument) || (i < funcList.size() && x > funcList[i]->maxArgument)) {
        i++;
    }*/
    const int s = funcList.size();
    subfunction* cur = NULL;
    for(; i < s; ++i) {
        cur = funcList[i];
        if(cur->maxArgument >= x) {
            break;
        }
    }
    lenAssert(cur);
    return cur->getValue(x);
}

void function::appendSubFunction(float length, int i)
{
    const int index = funcList.size();
    subfunction *temp, *prev;

    if(i == -1) {
        // Prepend the subfunction

        if(index == 0) {
            // Subfunction list is empty; length param is not used?
            temp = new subfunction(0.f, 1.f, startValue, 0.f, this);
        } else {
            prev = this->funcList.at(0);

            // min: zero
            // max: length
            // startValue: prev startValue (same as function startValue because
            // this is the first subfunction?)
            // prev min, max, and start value will be updated below
            temp = new subfunction(0.f, length, prev->startValue, 0.f, this);
        }

        this->funcList.prepend(temp);
        activeSubfunction = index; // activeSubFunction is never used
    } else {
        // Add after specified index
        subfunction* pred = funcList[i];

        // min: preceding max
        // max: preceding max + length
        // startValue: end value of preceding subfunction (if symmetric
        // subfunction, same as startValue of subfunction; else startValue of
        // subfunction + symArg)
        this->funcList.insert(i+1, new subfunction(pred->maxArgument, pred->maxArgument+length, pred->endValue(), 0.f, this));
        activeSubfunction = i+1; // activeSubFunction is never used
    }

    // Update all subfunction min, max, and symArg values except for the first
    // item.
    const int s = funcList.size();
    for(i = 1; i < s; ++i) {
        subfunction* prev = funcList[i-1];
        subfunction* cur = funcList[i];

        // min: preceding max
        // max: preceding max + (current max - min)
        // diff: current diff
        cur->update(prev->maxArgument, prev->maxArgument + cur->maxArgument - cur->minArgument, cur->symArg);
    }
}

void function::removeSubFunction(int i)
{
    int index = this->funcList.size();
    lenAssert(index > 1);
    if(index <= 1) {
        // Function list is empty
        return;
    }

    // Remove at the specified index
    delete funcList[i];
    this->funcList.removeAt(i);

    subfunction* cur;
    if(i == 0) { // removed from beginning
        cur = funcList[i];
        cur->update(0, cur->maxArgument - cur->minArgument, cur->symArg);
        ++i;
    }
    for(; i < funcList.size(); ++i) {
        subfunction* prev = funcList[i-1];
        cur = funcList[i];
        translateValues(prev);
        cur->update(prev->maxArgument, prev->maxArgument + cur->maxArgument - cur->minArgument, cur->symArg);
    }
}

void function::setMaxArgument(float newMax)
{
    float scale = newMax/getMaxArgument();
    for(int i = 0; i < funcList.size(); i++) {
        subfunction* cur = funcList[i];
        cur->update(cur->minArgument*scale, cur->maxArgument*scale, cur->symArg);
    }
}

void function::translateValues(subfunction* caller)
{
    int i = 0;
    subfunction* prev, *cur;

    // Do nothing on subfunctions up to and including the caller
    while(i < funcList.size()) {
        cur = funcList[i++];
        if(cur == caller) break;
    }

    // Update the startValue of subsequent subfunctions
    for(;i < funcList.size(); ++i) {
        prev = cur;
        cur = funcList[i];

        // end value of preceding subfunction (if symmetric subfunction, same as
        // startValue of subfunction; else startValue of subfunction + symArg)
        cur->translateValues(prev->endValue());
    }
}

float function::changeLength(float newlength, int index)
{
    subfunction* cur = funcList[index];
    subfunction* prev;

    cur->update(cur->minArgument, cur->minArgument+newlength, cur->symArg);
    for(++index; index < funcList.size(); ++index) {
        prev = cur;
        cur = funcList[index];
        if(cur->locked) {
            // Function is locked to the section's maximum argument
            cur->update(prev->maxArgument, secParent->getMaxArgument(), cur->symArg);
        } else {
            cur->update(prev->maxArgument, prev->maxArgument + cur->maxArgument - cur->minArgument, cur->symArg);
        }
    }
    return getMaxArgument();
}

void function::saveFunction(std::fstream& file)
{
    file << "FUNC";
    int size = funcList.size();
    writeBytes(&file, (const char*)&size, sizeof(int));
    for(int i = 0; i < funcList.size(); ++i) {
        funcList[i]->saveSubFunc(file);
    }
}

void function::loadFunction(std::fstream& file)
{
    if(readString(&file, 4) != "FUNC") {
        lenAssert(0 && "Error Loading Function");
        return;
    }
    int size = readInt(&file);
    funcList[0]->loadSubFunc(file);
    for(int i = 1; i < size; ++i) {
        appendSubFunction(1, i-1);
        funcList[i]->loadSubFunc(file);
    }
}

void function::legacyLoadFunction(std::fstream& file)
{
    if(readString(&file, 4) != "FUNC") {
        lenAssert(0 && "Error Loading Function");
        return;
    }
    int size = readInt(&file);
    funcList[0]->legacyLoadSubFunc(file);
    for(int i = 1; i < size; ++i) {
        appendSubFunction(1, i-1);
        funcList[i]->legacyLoadSubFunc(file);
    }
}

void function::saveFunction(std::stringstream& file)
{
    file << "FUNC";
    int size = funcList.size();
    writeBytes(&file, (const char*)&size, sizeof(int));
    for(int i = 0; i < funcList.size(); ++i) {
        funcList[i]->saveSubFunc(file);
    }
}

void function::loadFunction(std::stringstream& file)
{
    if(readString(&file, 4) != "FUNC") {
        lenAssert(0 && "Error Loading Function");
        return;
    }
    int size = readInt(&file);
    funcList[0]->loadSubFunc(file);
    for(int i = 1; i < size; ++i) {
        appendSubFunction(1, i-1);
        funcList[i]->loadSubFunc(file);
    }
}

int function::getSubfunctionNumber(subfunction *_sub)
{
    int number = 0;
    while(funcList[number] != _sub && number < funcList.size()) ++number;
    if(number < funcList.size()) {
        return number;
    } else {
        lenAssert(0 && "invalid subfunction");
        return -1;
    }
}

bool function::unlock(int _id)
{
    lenAssert(funcList[_id]->locked);
    funcList[_id]->locked = false;
    return true;
}

bool function::lock(int _id)
{
    lenAssert(!funcList[_id]->locked);
    funcList[_id]->locked = true;
    return true;
}

int function::lockedFunc()
{
    for(int i = 0; i < funcList.size(); ++i) {
        if(funcList[i]->locked) return i;
    }
    return -1;
}

subfunction* function::getSubfunction(float x)
{
    int i = 0;
    subfunction* cur = NULL;
    const int s = funcList.size();
    for(; i < s; ++i) {
        cur = funcList[i];
        if(cur->maxArgument >= x) {
            break;
        }
    }
    lenAssert(cur);
    return cur;
}
