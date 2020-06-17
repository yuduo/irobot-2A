#include "sys.h"

float disfloat(float x,float y)
{
	float dis;
	dis = x-y;
	if(dis > 180)
		dis=360 - dis;
	if(dis < -180)
		dis +=360;
	if(dis <0)
		dis= 0-dis;
	return dis;
	//return (x > y)?(x-y):(y-x);	
	
}
uint32_t disxy_32(uint32_t x,uint32_t y)
{
	return (x>y?(x-y):(y-x));
}

