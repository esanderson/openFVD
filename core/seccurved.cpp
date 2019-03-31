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

#include "seccurved.h"
#include "exportfuncs.h"
#include "mnode.h"
#include <cmath>

seccurved::seccurved(track* getParent, mnode* first, float getAngle, float getRadius): section(getParent, curved, first)
{
    fAngle = getAngle;
    fRadius = getRadius;
    fLeadIn = fAngle/3.f > 10.f ? 10.f : fAngle/3.f;
    fLeadOut = fAngle/3.f > 10.f ? 10.f : fAngle/3.f;
    fDirection = 90.0;
    length = 0.0;
    bOrientation = 0;
    bArgument = TIME;
    bSpeed = 0;
    fVel = 10;
    rollFunc->setMaxArgument(fAngle);
}

void seccurved::changecurve(float newAngle, float newRadius, float newDirection)
{
    fAngle = newAngle;
    fRadius = newRadius;
    fDirection = newDirection;
    length = 0.0;
    updateSection();
}

int seccurved::updateSection(int)
{
    length = 0.0;
    int numNodes = 1;
    float fRiddenAngle = 0.0;
    float artificialRoll = 0.0;

    fAngle = getMaxArgument();

    while(lNodes.size() > 1) {
        lNodes.removeAt(1);
        lAngles.removeAt(1);
    }

    int sizediff = lNodes.size() - lAngles.size();
    for(int i = 0; i <= sizediff; ++i) {
        lAngles.append(0.f);
    }
    lAngles[0] = 0.f;
	lNodes[0].updateNorm();

	float diff = lNodes[0].fRollSpeed; // - rollFunc->funcList.at(0)]-startValue;
    if(bOrientation == 1) {
		diff += glm::dot(lNodes[0].vDir, glm::vec3(0.f, 1.f, 0.f))*lNodes[0].getYawChange();
    }
    rollFunc->funcList.at(0)->translateValues(diff);
    rollFunc->translateValues(rollFunc->funcList.at(0));

    mnode* leadOutNode = NULL;
    float myLeadOut = 0.f;

    while(fRiddenAngle < fAngle - std::numeric_limits<float>::epsilon()) {
        float deltaAngle, fTrans;

		mnode* prevNode = &lNodes[numNodes-1];

        deltaAngle = prevNode->fVel / fRadius / F_HZ * 180/F_PI;

		if(fLeadIn > 0.f && (fTrans = (prevNode->fTotalLength - lNodes[0].fTotalLength)/(1.997f/F_HZ*prevNode->fVel/deltaAngle * fLeadIn)) <= 1.f) {
            deltaAngle *= fTrans*fTrans*(3+fTrans*(-2));
        }

        if(leadOutNode == NULL && fRiddenAngle > fAngle-fLeadOut) {
            leadOutNode = prevNode;
            myLeadOut = fAngle - fRiddenAngle;
        }
        if(leadOutNode && fLeadOut > 0.f) {
            if((fTrans = 1.f-(prevNode->fTotalLength - leadOutNode->fTotalLength)/(1.997f/F_HZ*prevNode->fVel/deltaAngle * myLeadOut)) >= 0.f) {
                deltaAngle *= fTrans*fTrans*(3+fTrans*(-2));
            } else {
                break;
            }
        }

        lNodes.append(*prevNode);

		mnode* curNode = &lNodes[numNodes];
		prevNode = &lNodes[numNodes-1]; // in case vector gets copied

        if(curNode->fVel < 0.1f) {
            qWarning("train goes very slowly");
            break;
        }

        fRiddenAngle += deltaAngle;
        lAngles.append(fRiddenAngle);

        curNode->updateNorm();

        float fPureDirection = fDirection - artificialRoll; //- curNode->fRoll;

        curNode->vDir = glm::angleAxis(TO_RAD(deltaAngle), (float)glm::cos(-fPureDirection*F_PI/180) * prevNode->vLat  + (float)glm::sin(-fPureDirection*F_PI/180) * prevNode->vNorm)*prevNode->vDir;
        curNode->vLat = glm::angleAxis(TO_RAD(deltaAngle), (float)glm::cos(-fPureDirection*F_PI/180) * prevNode->vLat  + (float)glm::sin(-fPureDirection*F_PI/180) * prevNode->vNorm)*prevNode->vLat;
        curNode->vDir = glm::normalize(curNode->vDir);
        curNode->vLat = glm::normalize(curNode->vLat);

        curNode->updateNorm();

        curNode->vPos += curNode->vDir*(curNode->fVel/(2.f*F_HZ)) + prevNode->vDir*(curNode->fVel/(2.f*F_HZ)) + (prevNode->vPosHeart(parent->fHeart) - curNode->vPosHeart(parent->fHeart));

        curNode->setRoll(rollFunc->getValue(fRiddenAngle)/F_HZ);
        curNode->fRollSpeed = rollFunc->getValue(fRiddenAngle);
        artificialRoll += rollFunc->getValue(fRiddenAngle)/F_HZ;

        if(bOrientation == EULER) {
            calcDirFromLast(numNodes);
			lNodes[numNodes].setRoll(glm::dot(lNodes[numNodes].vDir, glm::vec3(0.f, -1.f, 0.f))*lNodes[numNodes].fYawFromLast);
			artificialRoll += glm::dot(lNodes[numNodes].vDir, glm::vec3(0.f, -1.f, 0.f))*lNodes[numNodes].fYawFromLast;
			curNode->fRollSpeed += glm::dot(lNodes[numNodes].vDir, glm::vec3(0.f, -1.f, 0.f))*lNodes[numNodes].fYawFromLast*F_HZ;
        }

        curNode->updateNorm();

        if(bSpeed) {
            curNode->fEnergy -= (curNode->fVel*curNode->fVel*curNode->fVel/F_HZ * parent->fResistance);
            curNode->fVel = sqrt(2.f*(curNode->fEnergy-F_G*(curNode->vPosHeart(parent->fHeart*0.9f).y+curNode->fTotalLength*parent->fFriction)));
        } else {
            curNode->fVel = this->fVel;
            curNode->fEnergy = 0.5*fVel*fVel + F_G*(curNode->vPosHeart(parent->fHeart*0.9f).y + curNode->fTotalLength*parent->fFriction);
        }

        //curNode->vPos = glm::vec3(glm::translate(glm::rotate(glm::translate(vCenter), fAngle/numNodes, (float)glm::cos(-fDirection*F_PI/180) * curNode->vLat  + (float)glm::sin(-fDirection*F_PI/180) * prevNode->vNorm), -vCenter)*glm::vec4(prevNode->vPos, 1));

		curNode->updateRoll();

        curNode->fDistFromLast = glm::distance(curNode->vPosHeart(parent->fHeart), prevNode->vPosHeart(parent->fHeart));
        curNode->fTotalLength += curNode->fDistFromLast;
        curNode->fHeartDistFromLast = glm::distance(curNode->vPos, prevNode->vPos);
        curNode->fTotalHeartLength += curNode->fHeartDistFromLast;

        calcDirFromLast(numNodes);

		float temp = cos(fabs(lNodes[numNodes].getPitch())*F_PI/180.f);
        float forceAngle = sqrt(temp*temp*curNode->fYawFromLast*curNode->fYawFromLast + curNode->fPitchFromLast*curNode->fPitchFromLast);//deltaAngle;
        curNode->fAngleFromLast = forceAngle;

        glm::vec3 forceVec;
        if(fabs(curNode->fAngleFromLast) < std::numeric_limits<float>::epsilon()) {
            forceVec = glm::vec3(0.f, 1.f, 0.f);
        } else {
            float normalDAngle = F_PI/180.f*(- curNode->fPitchFromLast * cos(curNode->fRoll*F_PI/180.) - temp*curNode->fYawFromLast*sin(curNode->fRoll*F_PI/180.));
            float lateralDAngle = F_PI/180.f*(curNode->fPitchFromLast * sin(curNode->fRoll*F_PI/180.) - temp*curNode->fYawFromLast*cos(curNode->fRoll*F_PI/180.));

            forceVec = glm::vec3(0.f, 1.f, 0.f) + lateralDAngle*curNode->fVel*F_HZ/F_G * curNode->vLat + normalDAngle*curNode->fHeartDistFromLast*F_HZ*F_HZ/F_G * curNode->vNorm;
        }

        curNode->forceNormal = - glm::dot(forceVec, glm::normalize(curNode->vNorm));
        curNode->forceLateral = - glm::dot(forceVec, glm::normalize(curNode->vLat));

        numNodes++;
    }

    if(fLeadOut > 0.0001f) {
		lNodes.last().fAngleFromLast = 0.f;
		lNodes.last().fPitchFromLast = 0.f;
		lNodes.last().fYawFromLast = 0.f;
    }
	if(lNodes.size()) length = lNodes.last().fTotalLength - lNodes.first().fTotalLength;
    else length = 0;
    return 0;
}


float seccurved::getMaxArgument()
{
    return rollFunc->getMaxArgument();
}

void seccurved::saveSection(iostream& stream)
{
    stream << "CUR";
    writeBytes(&stream, (const char*)&bSpeed, sizeof(bool));

    int namelength = sName.length();
    std::string name = sName.toStdString();

    writeBytes(&stream, (const char*)&namelength, sizeof(int));
    stream << name;

    writeBytes(&stream, (const char*)&fVel, sizeof(float));
    writeBytes(&stream, (const char*)&fAngle, sizeof(float));
    writeBytes(&stream, (const char*)&fRadius, sizeof(float));
    writeBytes(&stream, (const char*)&fDirection, sizeof(float));
    writeBytes(&stream, (const char*)&fLeadIn, sizeof(float));
    writeBytes(&stream, (const char*)&fLeadOut, sizeof(float));
    writeBytes(&stream, (const char*)&bOrientation, sizeof(bool));
    rollFunc->saveFunction(stream);
}

void seccurved::loadSection(iostream& stream)
{
    bSpeed = readBool(&stream);

    int namelength = readInt(&stream);
    sName = QString(readString(&stream, namelength).c_str());

    fVel = readFloat(&stream);
    fAngle = readFloat(&stream);
    fRadius = readFloat(&stream);
    fDirection = readFloat(&stream);
    fLeadIn = readFloat(&stream);
    fLeadOut = readFloat(&stream);
    bOrientation = readBool(&stream);
    rollFunc->loadFunction(stream);
}

void seccurved::legacyLoadSection(iostream& stream)
{
    bSpeed = readBool(&stream);

    int namelength = readInt(&stream);
    sName = QString(readString(&stream, namelength).c_str());


    fVel = readFloat(&stream);
    fAngle = readFloat(&stream);
    fRadius = readFloat(&stream);
    fDirection = readFloat(&stream);
    fLeadIn = readFloat(&stream);
    fLeadOut = readFloat(&stream);
    bOrientation = readBool(&stream);
    rollFunc->legacyLoadFunction(stream);
}

bool seccurved::isInFunction(int index, subfunc* func)
{
    if(func == NULL) return false;
    float angle = lAngles[index];
    if(angle >= func->minArgument && angle <= func->maxArgument) {
        return true;
    }
    return false;
}

bool seccurved::isLockable(func* _func)
{
    Q_UNUSED(_func);
    return false;
}
