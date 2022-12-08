#pragma once
#include "header.h"

class Map
{
public:
	glmvec3 s;
	glmvec3 floor_t;
	glmvec3 roof_t;

	glmvec3 pillar_s;
	glmvec3 pillar_t[4][10];
	glmvec3 pillar_normal[4];

	glmvec3 rooms_s;
	glmvec3 rooms_t[4];

	int random_door;
	int open[4];
	bool wave;
	int clear_num;
public:
	Map();
	~Map();
	void OpenDoor(int num);
};