/********************************************************************
        created:        13-6-6    
        filename:     TrackCalculateTool.cpp
        author:        ChenYanNan(yannan.Chen@chukong-inc.com)

        Copyright (c) 2013 Chukong Technologies, Inc.
*********************************************************************/


/*
 In the beginning, Please pardon me for my inadequate English.
 
 This comment was added to provide more information to professors or other personnel
 who might receive this sample of code to evaluate my ability.
 
 This tool module is part of the fish movements system. It calculates every next point to which a fish moves.

 The system uses pertage as movement parameter. This is because the movements are already designed and set by designers, and the fish entities 
 will have no authority to change their moving tracks. Instead they only know the percentage of the moving
 trak they have already moved.

 To improve the game performance, it is crucial to diminish or skip all the comlicated and time-consuming calculations especially power operation in update function. 
 To do this, in the code we do these calculations and save the results in advance at the cost of more memory used. We make all the movement tracks 
 discrete -- dividing them into thousands of 3D points, and store each point's calculating factors. In Update function, the fish will know how much time it has spent,
 and the percentage of the whole track it has moved. By these two values, TrackCalculateTool can find the proper calculating factors to put into the formula.

 The knowledge used in this piece of code can be found here http://www.mvps.org/directx/articles/catmull/. There are other books and websites I learned from. 
 */

#include <map>
#include <vector>
#include "TrackCalculateTool.h"
#include "Utils/SimpleProfile.h"
#include "Actor/FishActor.h"
#include "Track/PathGenerator.h"
#include "Track/TrackManager.h"

#define speedScale 0.6f

const static int s_BezierStep = 4096; // 
const static int s_SplineStep = 4096;
const static int s_PathCount = 1024;

static float s_C0[s_BezierStep] = {0};
static float s_C1[s_BezierStep] = {0};
static float s_C2[s_BezierStep] = {0};
static float s_C3[s_BezierStep] = {0};
static float s_C4[s_BezierStep] = {0};
static float s_C5[s_BezierStep] = {0};

static float s_D0[s_BezierStep] = {0};
static float s_D1[s_BezierStep] = {0};
static float s_D2[s_BezierStep] = {0};
static float s_D3[s_BezierStep] = {0};
static float s_D4[s_BezierStep] = {0};
static float s_D5[s_BezierStep] = {0};


const static int s_CircleStep = 2880;
static float s_CircleSin[s_CircleStep] = {0};
static float s_CircleCos[s_CircleStep] = {0};

static std::vector<const C3DVector3 *> s_SplineCache;
static C3DVector4*   s_splineTable = NULL;

void TrackCalculateTool::initSplineTable2()
{
	s_splineTable = new C3DVector4[s_SplineStep];
    float t;
    for(int step = 0; step < s_SplineStep; step++){
        t = float(step)/s_SplineStep;
        C3DVector4 v4(0.5, 0.5*t, 0.5*t*t, 0.5*t*t*t);
        s_splineTable[step] = v4;

    }
    CCLOG("init ok");
}


void TrackCalculateTool::cleanupAllCache()
{
    std::vector<const C3DVector3 *>::iterator iter = s_SplineCache.begin();
    for(;iter!=s_SplineCache.end();++iter)
    {
        C3DVector3 * cache = (C3DVector3 *)*iter;
        SAFE_DELETE_ARRAY(cache);
    }

	SAFE_DELETE_ARRAY(s_splineTable);
}

C3DVector3 TrackCalculateTool::runSplineLogic2(FishActor * fish, float percentage, MoveDataSpline* spline,bool apllyOnFish)
{
    //PROFILE_START;
    int numSections = spline->m_points.size() - 3;
    spline->m_points[0] = spline->m_initPos;
    int currPt = min((int)(floor(percentage*(float)numSections)),numSections - 1);
    
    float t = percentage * (float) numSections - (float) currPt;
    float fstep = t*(s_SplineStep-1);
    int step = (int)fstep;
    float val = fstep-step-0.5f;
    if( val > 0.0001f){
        step++;
    }
    
    C3DVector3 a = spline->m_points[currPt];
    C3DVector3 b = spline->m_points[currPt + 1];
    C3DVector3 c = spline->m_points[currPt + 2];
    C3DVector3 d = spline->m_points[currPt + 3];
    //CCLOG("u = %f", u);
    C3DVector3 nextPosition = (-a+3*b-3*c+d)*s_splineTable[step].w+(2*a-5*b+4*c-d)*s_splineTable[step].z+(-a+c)*s_splineTable[step].y+b;
    
    //PROFILE_END("compute spline position");
    bool startingTrack = TrackManager::getInstance()->isTideStart(eLogoTideFlag);
    nextPosition.x *= -1*(startingTrack?1:POSITION_SCALE_X);
    nextPosition.y *= startingTrack?1:POSITION_SCALE_Y;
    nextPosition.z *= startingTrack?1:SIZE_SCALE_3D;
    
    C3DVector3 v1 = fish->getPosition3DFish();
    v1.z -= fish->m_depth;
	if (fish->getSprite() && apllyOnFish)
	{
		rotateModel(nextPosition, v1, fish, 8/10.0f, fish->getSprite());
	}
	else
	{
		if(fish->getSprite()){
			C3DVector3 tempOldPosition = nextPosition;
			tempOldPosition.z -= 100.0f;
			rotateModel(nextPosition,tempOldPosition,fish,8/10.0f,fish->getSprite());
		}
	}
    fish->setPosition3DFish(nextPosition); 
    return nextPosition;
}

void TrackCalculateTool::initBezier5Table()
{
    for (int step = 0; step < s_BezierStep; ++step)
    {
        float t = (float)step/(float)s_BezierStep;
        s_C0[step] = (1-t)*(1-t)*(1-t)*(1-t)*(1-t); // * P0
        s_C1[step] = 5* (1-t)*(1-t) *(1-t)*(1-t) * t; // * P1
        s_C2[step] = 10 * (1-t) *(1-t)*(1-t)* t*t; // * P2
        s_C3[step] = 10 *(1-t)*(1-t)*t*t*t; // * P3;
        s_C4[step] = 5*(1-t)*t*t*t*t;
        s_C5[step] = t*t*t*t*t;
        
        float it = 1-t;
        s_D0[step] = -5*it*it*it*it; // * P0
        s_D1[step] = 5*it*it*it*(1-5*t); // * P1
        s_D2[step] = 10*t*it*it*(2-5*t); // * P2
        s_D3[step] = 10*t*t*it*(3-5*t); // * P3;
        s_D4[step] = 5*t*t*t*(4-5*t);
        s_D5[step] = 5*t*t*t*t;
    }
}
void TrackCalculateTool::initCircleTable()
{
	for(int step = 0;step< s_CircleStep;++step)
	{
		float t = float(step)/8.0f;
		s_CircleSin[step] = sin(t/180*M_PI) ;
		s_CircleCos[step] = cos(t/180*M_PI);
	}
}

C3DVector3 TrackCalculateTool::bezier5(C3DVector3 p0, C3DVector3 p1, C3DVector3 p2, C3DVector3 p3, C3DVector3 p4, C3DVector3 p5, float t)
{
    float fstep = t*(s_BezierStep-1);
    int step = (int)fstep;
    float val = fstep-step-0.5f;
    if( val > 0.0001f){
        step++;
    }
 
    float x = s_C0[step]*p0.x + s_C1[step]*p1.x + s_C2[step]*p2.x + s_C3[step]*p3.x + s_C4[step]*p4.x + s_C5[step]*p5.x;
    float y = s_C0[step]*p0.y + s_C1[step]*p1.y + s_C2[step]*p2.y + s_C3[step]*p3.y + s_C4[step]*p4.y + s_C5[step]*p5.y;
    float z = s_C0[step]*p0.z + s_C1[step]*p1.z + s_C2[step]*p2.z + s_C3[step]*p3.z + s_C4[step]*p4.z + s_C5[step]*p5.z;
    return C3DVector3(x,y,z);

}

C3DVector3 TrackCalculateTool::runStraightNormalLogic(FishActor * fish,float percentage,bool apllyOnFish)
{
    int curMoveIndex = fish->getPathGenerator()->m_curMoveIndex;
    MoveDataStraight * straight = (MoveDataStraight *)fish->getPathGenerator()->m_pathData->m_moveData[curMoveIndex];
    C3DVector3 nextPostion = straight->m_initPos + (straight->m_endPosition-straight->m_initPos)*percentage;
    nextPostion.x *= -1;
    C3DVector3 v1 = fish->getPosition3DFish();
    v1.z -= fish->m_depth;
	if (apllyOnFish)
	{
		rotateModel(nextPostion, v1, fish, 8/10.0f,fish->getSprite());
		fish->setPosition3DFish(nextPostion);
	}
    return nextPostion;
}


C3DVector3 TrackCalculateTool::runBezierLogic(FishActor * fish,float percentage,bool apllyOnFish)
{
    int curMoveIndex = fish->getPathGenerator()->m_curMoveIndex;
    MoveDataBezier * bezier = (MoveDataBezier *)fish->getPathGenerator()->m_pathData->m_moveData[curMoveIndex];
    
    C3DVector3 nextPosition = bezier5(bezier->m_initPos, bezier->m_endPosition, bezier->m_controlPoint, bezier->m_controlPoint2, bezier->m_controlPoint3, bezier->m_controlPoint4, percentage);
    
    C3DVector3 v1 = fish->getPosition3DFish();
    v1.z -= fish->m_depth;
    nextPosition.x *= -1;
	if (apllyOnFish)
	{
		rotateModel(nextPosition, v1, fish, 8/10.0f,fish->getSprite());
		fish->setPosition3DFish(nextPosition);
	}
    return nextPosition;
}


C3DVector3 * TrackCalculateTool::pathControlGenerate(cocos3d::C3DVector3 *vecs)
{
    return NULL;
}

C3DVector3 TrackCalculateTool::runCircleLogic(FishActor * fish ,float percentage,bool apllyOnFish)
{
    int curMoveIndex = fish->getPathGenerator()->m_curMoveIndex;
    MoveDataCircle * circle = (MoveDataCircle *)fish->getPathGenerator()->m_pathData->m_moveData[curMoveIndex];
    
    C3DVector3 center = circle->m_centerPosition;
    C3DVector3 start =  circle->m_initPos;
    C3DVector3 nDirection = circle->m_nDirection;
    nDirection=nDirection.normalize();
    float radius = circle->m_radius;
    float beginAngle= circle->m_beginAngle;
    float factor;
    if(circle->m_clockWise)
    {
        factor =-1.0f;
    }
    else
    {
        factor = 1.0f;
    }
    float usingAngle = percentage * M_PI*2 *circle->m_totalAngle/360.0f;
    float cos = cosf (beginAngle+factor*usingAngle);
    float sin = sinf (beginAngle+factor*usingAngle);
    //nDirection.z *= -1;
    C3DVector3 nextPostion;
    nextPostion.x = center.x + radius*nDirection.y/sqrt(nDirection.x*nDirection.x + nDirection.y*nDirection.y)*cos+radius*nDirection.x*nDirection.z/(sqrt(nDirection.x*nDirection.x+nDirection.y*nDirection.y)*sqrt(nDirection.x*nDirection.x+nDirection.y*nDirection.y+nDirection.z*nDirection.z))*sin;
    nextPostion.y = center.y - radius*nDirection.x/sqrt(nDirection.x*nDirection.x + nDirection.y*nDirection.y)*cos+radius*nDirection.y*nDirection.z/(sqrt(nDirection.x*nDirection.x+nDirection.y*nDirection.y)*sqrt(nDirection.x*nDirection.x+nDirection.y*nDirection.y+nDirection.z*nDirection.z))*sin;
    nextPostion.z = center.z - radius*sqrt(nDirection.x*nDirection.x + nDirection.y*nDirection.y)/sqrt(nDirection.x*nDirection.x + nDirection.y*nDirection.y+nDirection.z*nDirection.z)*sin;
   
    nextPostion.x *= -1;
    //nextPostion.z *= (POSITION_SCALE_X+POSITION_SCALE_Y)/2;
    
    if(usingAngle <= 0.0f)
    {
        circle->m_initPos = nextPostion;
    }
    
    C3DVector3 v1 = fish->getPosition3DFish();
    v1.z -= fish->m_depth;
	if (apllyOnFish)
	{
		 rotateModel(nextPostion, v1, fish, 8/10.0f,fish->getSprite());
		fish->setPosition3DFish(nextPostion);
	}
    return nextPostion;
}

void TrackCalculateTool::rotateModel(cocos3d::C3DVector3 curPostion, cocos3d::C3DVector3 lastPosition,FishActor * pTarget,float factor,C3DSprite* fishSprite)
{
    C3DVector3 xAxis = curPostion-lastPosition;
	if (pTarget)
	{
		if (xAxis.x >= 0 )
		{
			pTarget->m_bFaceRight = true;	
		}
		else
		{
			pTarget->m_bFaceRight = false;	
		}
	}
	xAxis.normalize();
	C3DVector3 up(0.0f, 1.0f, 0.0f);
    //C3DVector3 up = -m_pTarget->getRightVectorWorld();
	float rotateAngle = C3DVector3::angle(xAxis, up);
	static float s_tmp = 0.01f;
	if(-s_tmp <rotateAngle && rotateAngle < s_tmp){
		//return ;
        up = C3DVector3(1,0, 0);
	}
    
	C3DVector3 zAxis;
	C3DVector3::cross( xAxis, up,&zAxis);
    
	C3DVector3 yAxis;
	C3DVector3::cross( zAxis, xAxis, &yAxis);
    
	C3DMatrix martrix = fishSprite->getMatrix();
    C3DQuaternion old_quat;
	martrix.getRotation(&old_quat);
	martrix.m[0] =  xAxis.x;
	martrix.m[1] =  xAxis.y;
	martrix.m[2] =  xAxis.z;
	martrix.m[4] =  yAxis.x;
	martrix.m[5] =  yAxis.y;
	martrix.m[6] =  yAxis.z;
	martrix.m[8] =  zAxis.x;
	martrix.m[9] =  zAxis.y;
	martrix.m[10] = zAxis.z;
	fishSprite->setRotation(martrix);
    
    C3DQuaternion new_quat;
	martrix.getRotation(&new_quat);
    
    float cosOmega = new_quat.x*old_quat.x + new_quat.y*old_quat.y+new_quat.z*old_quat.z+new_quat.w*old_quat.w;
    if(cosOmega <0.0f)
    {
        new_quat.x = -new_quat.x;
        new_quat.y = -new_quat.y;
        new_quat.z = -new_quat.z;
        new_quat.w = -new_quat.w;
        cosOmega = -cosOmega;
    }
    C3DQuaternion quat;
    if(cosOmega > 0.95f)
    {
        //C3DQuaternion::lerp(old_quat, new_quat, s_t, &quat);
        C3DQuaternion::slerp(old_quat, new_quat, factor, &quat);
    }
    else
    {
        C3DQuaternion::slerp(old_quat, new_quat, factor, &quat);
    }
	
	fishSprite->setRotation(quat);
}
