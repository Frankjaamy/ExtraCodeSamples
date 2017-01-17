/********************************************************************
	created:	2013/06/21
	filename: 	DepthHelper.h
	author:		Yannan  Chen & reed Hong
	
	purpose:	鱼的深度分配  the arrangement of depths of fishes
*********************************************************************/

#ifndef __FishingJoy3_DepthHelper_h__
#define __FishingJoy3_DepthHelper_h__

#include "Constants/GameConfig.h"
class DepthHelper
{
public:
	DepthHelper();
	~DepthHelper();

	static DepthHelper* sharedDepthHelper();
    // to return a depth to posit a fish entity
	float getDepth();
	// to specific a level of depth to place a fish entity
	float getDepthByIndex(int index);
    // once a fish is gone or is killed, then its level of depth shall be recycled.
	void recyleDepth(float depth);
private:
	static const int BITS_LEN = MAX_WAVE_IN_SCREEN/8;
	unsigned char m_bits[BITS_LEN];
	float m_grid;
	int m_lastIndex;
};
#endif // __FishingJoy3_DepthHelper_h__
