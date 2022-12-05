#pragma once
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
constexpr int BUF_SIZE = 1024;
constexpr int NAMESIZE = 20;

#define MAX_BULLET_NUM 60
#define MAX_ENEMY_NUM 20

struct PlayerInfo {
	int id;
	int hp;
	float x, y, z;
	float dx, dy, dz;

	PlayerInfo() : hp{ 3 }, id{ 0 }, x{ 0.f }, y{ 0.f }, z{ 0.f } {
		dx = 0;
		dy = 0;
		dz = 0;
	};
};

struct CS_LOGIN_PACKET {
	char size;
	char type;
	char name[NAMESIZE];
};

struct CS_DIRECTION_PACKET {
	char	size;
	char	type;
	short	id;
	float	dx;
	float	dz;
	float	degree;
};

struct CS_KEYBOARD_PACKET {
	char	size;
	char	type;
	short	id;
	int		direction;
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
	float dx, dy, dz;
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

struct SC_REMOVE_PLAYER_PACKET{
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
	bool is_active;
	bool is_team;

	Bullet() 
	{
		x = 0; y = 0; z = 0;
		is_active = false;
		is_team = false;
	};
};

struct Enemy {
	int hp;
	int kind;
	float x, y, z;
	bool pop[3];
	float px, py, pz; // ��ƼŬ ���� ��ġ
	bool is_active;
};
