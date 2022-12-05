#pragma once
#include "enemy.h"
//Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_MOUSECLICK = 13;
constexpr char CS_DIRECTION = 15;
constexpr char CS_KEYBOARD = 16;

constexpr char SC_LOGININFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_START = 4;
constexpr char SC_REMOVE_PLAYER = 5;
constexpr char SC_MOVE_PLAYER = 6;
constexpr char SC_ENEMY = 7;
constexpr char SC_ENEMYHIT = 8;
constexpr char SC_BULLET = 9;
constexpr char SC_BULLETHIT = 10;
constexpr char SC_STAGE = 11;
constexpr char SC_PLAYERHIT = 12;
constexpr char SC_GEN_ENEMY = 14;

constexpr int PORT_NUM = 9000;
constexpr int BUF_SIZE = 256;
constexpr int NAMESIZE = 20;

#define MAX_BULLET_NUM 60
#define MAX_ENEMY_NUM 20

struct CS_LOGIN_PACKET {
	char size;
	char type;
	char name[NAMESIZE];
};

struct CS_MOVE_PACKET {
	char	size;
	char	type;
	short	id;
	float	x;
	float	z;
};

struct CS_KEYBOARD_PACKET {
	char	size;
	char	type;
	short	id;
	int		direction;
};

struct CS_MOUSECLICK_PACKET {
	unsigned char size;
	char type;
	short id; // 클라이언트 아이디
	float dx, dy, dz; // 시선 벡터(플레이어의 총알 방향)
};

struct CS_DIRECTION_PACKET {
	char	size;
	char	type;
	short	id;
	float	dx;
	float	dz;
	float	degree;
};

struct SC_LOGININFO_PACKET {
	char	size;
	char	type;
	short	id;
};

struct SC_ADD_PLAYER_PACKET {
	char	size;
	char	type;
	short	id;
};

struct SC_START_PACKET {
	char	size;
	char	type;
};

struct SC_REMOVE_PLAYER_PACKET
{
	char	size;
	char	type;
	short	id;
};

struct SC_MOVE_PLAYER_PACKET {
	char	size;
	char	type;
	short	id;
	float	x, z;
};

struct SC_PLAYERHIT_PACKET {
	unsigned char size;
	char type;
	short id;
};

struct SC_ENEMY_PACKET {
	unsigned char size;
	char type;
	short id;
	float x, z;
};

struct SC_GEN_ENEMY_PACKET {
	unsigned char size;
	char type;
	short id;
	int hp;
	int kind;
	float x, y, z;
};

struct SC_ENEMYHIT_PACKET {
	unsigned char size;
	char type;
	short id;
};

struct SC_BULLET_PACKET {
	unsigned char size;
	char type;
	short bullet_id;
	float x, y, z;
};

struct SC_BULLETHIT_PACKET {
	unsigned char size;
	char type;
	short bullet_id;
};

struct SC_STAGE_PACKET {
	unsigned char size;
	char type;
	short state;
};

struct Bullet
{
	float x, y, z;
	float dx, dy, dz;
	bool is_active;
	bool is_team;

	Bullet()
	{
		x = 0; y = 0; z = 0;
		dx = 0; dy = 0; dz = 0;
		is_active = false;
		is_team = false;
	};

	void update() {
		x += 0.3f * dx;
		y += 0.3f * dy;
		z += 0.3f * dz;
	}

	bool isOut()
	{
		if (x > 5.0f || x < -5.0f) {
			return true;
		}
		if (y > 10.0f || y < -1.0f) {
			return true;
		}
		if (z > 5.0f || z < -5.0f) {
			return true;
		}
		return false;
	}

	int ColisionCheckEnemy(Enemy enemy[]) 
	{	
		for (int i = 0; i < MAX_ENEMY_NUM; ++i) {
			if (is_team && enemy[i].is_active) {
				if (!is_active) continue;

				float cx = enemy[i].x - x;
				float cy = enemy[i].y - y;
				float cz = enemy[i].z - z;
				float dist = sqrt(cx * cx + cy * cy + cz * cz);
				float weight = 0.02f;
				if (enemy[i].kind == 4) weight = 0.04;

				if (enemy[i].radius > dist + weight) {
					is_active = false;
					enemy[i].hp -= 1;
					if (!enemy[i].hp) {
						enemy[i].is_active = false;
					}
					//부딪힌 적의 아이디를 반환
					return i;
				}
			}
		}
		return -1;
	}

	bool ColisionCheckClient()
	{
	}
};