/********************************************************************
        created:        13-6-6
        filename:       TrackCalculateTool.h
        author:         ChenYanNan(yannan.Chen@chukong-inc.com, no longer used)

        purpose:

        Copyright (c) 2013 Chukong Technologies, Inc.
*********************************************************************/
/*
 In the beginning, Please pardon me for my inadequate English.
 
 This comment was added to provide more information to professors or other personnel
 who might receive this sample of code to evaluate my ability.
 
 This is the tool for generating actual moving paths of fishes in real game time. The data passed by path editor includes reference points, which will be processed by this tool to generate actual paths of different types.
 
 This at first did not work as expected. I made too much calculations that it seriously affected gaming experiences --- the game became slow when too many fishes appeared on screen, moving along a B-spline curve or a Bezier curve. Then I made a change. I did all the necessary calculations ahead and then used several vector arrays to storage these important results.  Consequently, in update functions where every fish's next position is calculated, the process becomes faster as all the important parameters used in the formula are all calculated before. This change affected the accuracy, however, since it made the linear data discrete and I used a parameter called percentage to determine the results on which index of the array should be used, which means to have to convert a float result to an integer(the index). But things went well beause I built a large-enough vector.
 */

#ifndef __FishingJoy3__TrackCalculateTool_H__
#define __FishingJoy3__TrackCalculateTool_H__
#include "cocos2d.h"
#include "cocos3d.h"

#include "Actor/FishActor.h"
#define TOTAL_SIMPSON_STEP 10000
using namespace cocos2d;
class FishActor;
class SurroundFish;
class TrackCalculateTool
{
public:
    
    static void initBezier5Table();
    static void initCircleTable();
	static void initSplineTable2();
    static void cleanupAllCache();
    
    static C3DVector3 runStraightNormalLogic(FishActor * fish,float percentage,bool apllyOnFish = true);
    static C3DVector3 runBezierLogic(FishActor * fish,float percentage,bool apllyOnFish = true);
    static C3DVector3 runCircleLogic(FishActor * fish ,float percentage,bool apllyOnFish = true);
    static C3DVector3 runSplineLogic2(FishActor * fish,float percentage, MoveDataSpline* spline,bool apllyOnFish = true);
    static C3DVector3 bezier5(C3DVector3 p0, C3DVector3 p1, C3DVector3 p2, C3DVector3 p3, C3DVector3 p4, C3DVector3 p5, float t);
    static cocos3d::C3DVector3 * pathControlGenerate(cocos3d::C3DVector3 vecs[] );
    
	static void rotateModel(C3DVector3 oldPostion,C3DVector3 nextPosition,FishActor * pTarget,float factor, C3DSprite *fishSprite);
private:


};

#endif // __FishingJoy3__TrackCalculateTool_H__
