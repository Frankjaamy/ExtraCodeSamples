﻿#include "DepthHelper.h"
#include "Utils/Random.h"
#include "Constants/GameConfig.h"
#include "cocos2d.h"

/*
 In the beginning, Please pardon me for my inadequate English.
 
 This comment was added to provide more information to professors or other personnel
 who might receive this sample of code to evaluate my ability.
 
 This is a very important class called DepthHelper in FishingJoy 3. In our game, all the fishes that spawned must be arranged a certain depth level. The fish can move at all directions around this level of depth. 

 This is because we do not want the fishes to collide --- that will destroy the paths generated by PathEditor ---, and so that we give every fish spawned a different level of depth.
 
 I finished this work in a easy way before the leading programmer checked my codes. Then he pointed out that my codes were not efficient and fast enough. He gave me some hints and I rewrote the codes.
 
 Through this piece of code ,I hope to show you my understadning of computer memory and bit manipulation, and the ability and mind of analyzing pratical issues and optimizing solutions. 
 
 */

DepthHelper::DepthHelper()
{
	memset(m_bits, 0, sizeof(unsigned char)*BITS_LEN);
	m_grid = MAX_DEPTH_FILED*1.0f/MAX_WAVE_IN_SCREEN;
}

DepthHelper::~DepthHelper()
{

}

DepthHelper* DepthHelper::sharedDepthHelper()
{
	static DepthHelper s_instance;
	return &s_instance;
}

/*
	The whole idea is simple. Since we want all the fishes spawned to be placed in a certain level of depth, we use an array of 0s and 1s
	to represent all the levels. 

	The original design is an int array m_bits, and for every int in m_bits, 1 means this level is currently occupied, and 0 means available. So DepthHelper will
	search the array of m_bits, finding the first position where there is a 0, and return the index of that position -- this index can be regarded as the level
	of depth available for the next spawning fish.

	But the code above can be optimized. We need not use an int array. Instead we can use a unsigned char array. Each char is a 8-bit memory, and for every bit, 1 and 0
	can mean different things like the int array does. Furthermore, instead of int assignment, we can use bit bit manipulations to check if a position is available and
	set it to 0 or 1 by AND-IR-INVERT operations.
*/

float DepthHelper::getDepth()
{
	int index = -1;
	// designate a random level 
	m_lastIndex = Random::sharedRandom()->randInt(0, MAX_WAVE_IN_SCREEN);
	for(int k = 0; k < BITS_LEN*8; k++){
		// find out which unsigned char m_lastIndex is in
		int i = m_lastIndex/8;
		// find out which bit exactly will be checked of the char above
		int j = m_lastIndex%8;
		//AND operation -- to find if this bit is 0 or 1
		unsigned char flag = m_bits[i] & (1<<j);
		if(flag == 0){
			//this bit is 0, it can be used. So use OR to set this bit to 1 where another bits remain their own value. 
			m_bits[i] |= (1<<j);
			index = m_lastIndex;
			break;
		}
		// if this bit is not 0, then we have to choose a new level to be examined.
		m_lastIndex++;
		m_lastIndex%=(BITS_LEN*8);
	}
	///CCAssert(index>=0, "can't find empty depth");
	if(index < 0)
    {
        CCLOG("Find Empty Depth!");
        index = 0;
    }
    return m_grid*(index+0.5f);
}
/*
	To get and return a level of depth regardless of its current status.
*/
float  DepthHelper::getDepthByIndex(int index)
{
	int i = index/8;
	int j = index%8;
	m_bits[i] |= (1<<j);

	return m_grid*(index+0.5f);
}
/*
	When a fish disappear or killed, DepthHelper knows its depth and gets its level of depth.
	Then we can use bit manipulations to reset the array of levels.
*/
void DepthHelper::recyleDepth(float depth)
{
	int index = (int)((depth/m_grid)-0.5);
	if(index >= MAX_WAVE_IN_SCREEN){
		return ;
	}
	int i = index/8;
	int j = index%8;
	m_bits[i] &= (~(1<<j));
}