#ifndef SUBFUNCTION_H
#define SUBFUNCTION_H

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

#include <fstream>
#include <QList>

class function;

enum eDegree
{
    linear = 0,
    quadratic = 1,
    cubic = 2,
    quartic = 3,
    quintic = 4,
    sinusoidal = 5,
    plateau = 6,
    tozero = 7,
    freeform = 8
};

typedef struct bez_s
{
    float x;
    float y;
} bez_t;

class subfunction
{
public:
    subfunction();
    subfunction(float min, float max, float start, float diff, function* getparent = 0);
    void update(float min, float max, float diff);

    float getValue(float x);

    void changeDegree(eDegree newDegree);
    void updateBez();

    // relic, doesn't get used at all at this time
    //float getMinValue();
    //float getMaxValue();

    void translateValues(float newStart);

    bool isSymmetric();
    float endValue();

    void saveSubFunc(std::fstream& file);
    void loadSubFunc(std::fstream& file);
    void legacyLoadSubFunc(std::fstream& file);
    void saveSubFunc(std::stringstream& file);
    void loadSubFunc(std::stringstream& file);

    // Set to value passed into constructor or update method
    // When constructing a function, min passed to function is passed to sub
    // Set to zero for the first subfunction
    // Set to the max value of the preceding subfunction
    float minArgument;

    // Set to value passed into constructor or update method
    // When constructing a function, max passed to function is passed to sub
    // Set to 1 for geometic and force sections; set to 10 for the roll function
    // of section types other than Bezier
    // Updated to previous sub max argument + the difference between the old
    // values of max and min
    // Constructed to previous sub max argument + a specified length
    float maxArgument;

    // Set to value passed into constructor or translateValues
    // When prepending this sub, set to startValue of sub which will come after
    // When constructing a function, start passed to function is passed to sub
    // Set to normal or lateral force for forced sections
    // Set to deltaPitch or deltaYaw for geometric sections
    // Set to 0 for the roll function of section types other than Bezier
    // When appending, set to the endValue of the preceding sub
    float startValue;

    // Set in changeDegree: to -10 for quartic, 0 for quintic, 1 for plateau
    // Set in getValue for the toZero sub type
    // Set in the transition widget
    // For quadratic, 1 for type 0, -1 for type 1, and 0 for type 2
    // For quartic, -10 for type 0; for other types, set to 1 - arg1 or
    // something like 0.5f +/- 0.5f / (1 + 5 * ui->quarticSpin->value())
    // For quintic, 0 for type 0, -spinBox for type 1, and +spinBox for type 2
    float arg1;

    // Set to value passed into constructor or update method
    // When constructing a function, set to end - start arguments but these are
    // always the same value so the result is 0
    // When appending or prepending, set to 0
    // Set in the transition widget when change spinbox is changed
    float symArg;

    // Initialized to false in the constructor and set in the unlock/lock
    // methods of the parent function
    bool locked;

    //timewarp arguments
    // Set to zero in constructor and when degree is changed to toZero type
    // Set in doUndo and doRedo and by transition widget
    float centerArg;
    float tensionArg;

    // Set in changeDegree and doUndo and doRedo
    // Set to cubic if parent function is funcNormal type; else set to quartic
    // changeDegree called by transtion widget
    enum eDegree degree;

    function* parent; // Set in constructor

    // Cleared and 2 points are appended in changeDegree for the freeform type
    // Set in the graph widget when dragging points
    QList<bez_t> pointList;

    // Generating in updateBez; updateBez is called in changeDegree for the
    // freeform type and is called by the graph widget
    QList<float> valueList;

private:
    float applyTension(float x);
    float applyCenter(float x);
};


#endif // SUBFUNCTION_H
