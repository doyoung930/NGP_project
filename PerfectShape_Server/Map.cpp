#include "Map.h"

Map::Map()
{
	s = { 10.0f,0.1f,10.0f };
	floor_t = { 0.0f,-0.5f,0.0f }; // 중간 방
	roof_t = { 0.0f,5.0f,0.0f }; // 중간 방

	pillar_s = { 1.0f,6.0f,1.0f };
	float dy = (pillar_s.y - 0.5f) * 0.5f - 0.5f;

	pillar_normal[0] = { 1.0f,0.0f,0.0f };
	pillar_normal[1] = { -1.0f,0.0f,0.0f };
	pillar_normal[2] = { 0.0f,0.0f,1.0f };
	pillar_normal[3] = { 0.0f,0.0f,-1.0f };
	for (int i = 0; i < 10; i++)
	{
		pillar_t[0][i] = { -s.x * 0.5f,dy,-s.z * 0.5f + 1.0f * (float)i };
		pillar_t[1][i] = { s.x * 0.5f,dy,-s.z * 0.5f + 1.0f * (float)i };

		pillar_t[2][i] = { -s.x * 0.5f + 1.0f * (float)i,dy,-s.z * 0.5f };
		pillar_t[3][i] = { -s.x * 0.5f + 1.0f * (float)i,dy,s.z * 0.5f };
	}

	for (int i{}; i < 4; ++i)
	{
		open[i] = 0;
	}

	rooms_s = { 10.0f,5.5f,10.0f };
	rooms_t[0] = { -s.x * 1.0f,dy - 0.05f,0.0f }; // 왼
	rooms_t[1] = { s.x * 1.0f,dy - 0.05f,0.0f }; // 오
	rooms_t[2] = { 0.0f,dy - 0.05f,-s.z * 1.0f }; // 위
	rooms_t[3] = { 0.0f,dy - 0.05f,s.z * 1.0f }; // 아래

	wave = true;

	random_door = -1;
	clear_num = 0;
}

Map::~Map()
{

}

void Map::OpenDoor(int num) {

}