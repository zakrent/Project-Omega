#include "common.h"
#include <stdlib.h>
#include <time.h>

static b32 randInit = false;
r32 randf(r32 min, r32 max){
	if(!randInit){
		srand(time(NULL));
		randInit = true;
	}
	return (r32)rand()/(r32)(RAND_MAX)*(max-min)+min;
}
