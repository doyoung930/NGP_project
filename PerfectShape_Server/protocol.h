#pragma once
//Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_MOUSECLICK = 13;

constexpr char SC_LOGININFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_START = 4;
constexpr char SC_REMOVE_PLAYER = 5;
constexpr char SC_MOVE_PLAYER = 6;
constexpr char SC_ENEMY = 7;
constexpr char SC_ENERMYHIT = 8;
constexpr char SC_BULLET = 9;
constexpr char SC_BULLETHIT = 10;
constexpr char SC_STAGE = 11;
constexpr char SC_PLAYERHIT = 12;


constexpr int PORT_NUM = 9000;
constexpr int BUF_SIZE = 256;
constexpr int NAMESIZE = 20;

#define MAX_BULLET_NUM 60

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

struct CS_MOUSECLICK_PACKET {
	unsigned char size;
	char type;
	short id; // Ŭ���̾�Ʈ ���̵�
	float dx, dy, dz; // �ü� ����(�÷��̾��� �Ѿ� ����)
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
	float x, y, z;
};

struct SC_ENERMYHIT_PACKET {
	unsigned char size;
	char type;
	short hp;
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

	bool IsOut() {
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
};