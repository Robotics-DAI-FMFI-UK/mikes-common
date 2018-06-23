#include <stdio.h>

#include "modules/passive/astar.h"

int main() {

	int map[] = {
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
		1,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,
		1,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,0,1,
		1,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,
		1,0,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,
		1,0,0,1,0,1,0,1,0,0,1,0,0,1,0,0,0,1,
		1,0,0,1,0,1,0,1,0,0,1,0,0,1,0,1,1,1,
		1,0,0,1,0,1,0,0,0,0,1,0,0,1,0,0,0,1,
		1,0,0,1,0,1,0,0,0,0,1,0,0,0,0,0,0,1,
		1,0,0,1,0,1,0,0,0,0,1,0,1,1,1,1,0,1,
		1,0,0,1,0,1,0,1,0,0,1,0,0,0,0,0,0,1,
		1,0,0,1,0,1,0,1,0,0,1,0,0,1,0,0,0,1,
		1,0,0,1,0,1,0,1,0,0,0,0,0,1,0,1,1,1,
		1,0,0,1,0,1,0,1,0,0,0,0,0,1,0,1,0,1,
		1,0,0,1,0,1,0,0,0,0,0,0,0,1,0,0,0,1,
		1,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
		
	//for (int i = 0; i < 17; ++i)
	//{
		//for (int j = 0; j < 18; ++j)
		//{
			//if (map[i*18 + j] == 1)
				//printf("o");
			//else
				//printf(" ");
		//}
		//printf("\n");
	//}
		
	astar(map, 18, 17, 1, 1, 16, 13, rectilinear);
	
	printf("path_len = %d\n", path_len);
			
	for (int i = 0; i < path_len; ++i)
	{
		printf("(%d, %d)\n", path[i][0], path[i][1]);
	}
	
	
	return 0;
}
