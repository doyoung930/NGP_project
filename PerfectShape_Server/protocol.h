#pragma once
//Packet ID
constexpr char CS_MOUSECLICK = 13;
constexpr char CS_DIRECTION = 15;
constexpr char CS_KEYBOARD = 16;
constexpr char CS_PLAYER_DEAD = 20;

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
constexpr char SC_HITEND = 17;
constexpr char SC_DEAD = 18;
constexpr char SC_PILLAR = 19;

constexpr int PORT_NUM = 9000;
constexpr int BUF_SIZE = 1024;
constexpr int NAMESIZE = 20;

constexpr int MAX_USER = 3;

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
	int	id;
	float	x;
	float	z;
};

struct CS_KEYBOARD_PACKET {
	char	size;
	char	type;
	int	id;
	int		direction;
};

struct CS_MOUSECLICK_PACKET {
	unsigned char size;
	char type;
	int id; // Ŭ���̾�Ʈ ���̵�
	float dx, dy, dz; // �ü� ����(�÷��̾��� �Ѿ� ����)
};

struct CS_DIRECTION_PACKET {
	char	size;
	char	type;
	int	id;
	float	dx;
	float	dz;
	float	degree;
};

struct CS_DEAD_PACKET {
	char	size;
	char	type;
	bool	state;
};

struct SC_LOGININFO_PACKET {
	char	size;
	char	type;
	int	id;
};

struct SC_ADD_PLAYER_PACKET {
	char	size;
	char	type;
	int	id;
};

struct SC_START_PACKET {
	char	size;
	char	type;
};

struct SC_REMOVE_PLAYER_PACKET
{
	char	size;
	char	type;
	int	id;
};

struct SC_MOVE_PLAYER_PACKET {
	char	size;
	char	type;
	int	id;
	float	x, z;
};

struct SC_PLAYERHIT_PACKET {
	unsigned char size;
	char type;
	int id;
};

struct SC_ENEMY_PACKET {
	unsigned char size;
	char type;
	int id;
	float x, z;
};

struct SC_GEN_ENEMY_PACKET {
	unsigned char size;
	char type;
	int id;
	int hp;
	int kind;
	float x, y, z;
};

struct SC_ENEMYHIT_PACKET {
	unsigned char size;
	char type;
	int id;
};

struct SC_BULLET_PACKET {
	unsigned char size;
	char type;
	int bullet_id;
	float x, y, z;
};

struct SC_BULLETHIT_PACKET {
	unsigned char size;
	char type;
	int bullet_id;
};

struct SC_STAGE_PACKET {
	unsigned char size;
	char type;
	int state;
};

struct SC_HITEND_PACKET {
	unsigned char size;
	char type;
	int id;
};


struct SC_PLAYER_DEAD_PACKET {
	unsigned char size;
	char type;
	int id;
};

struct SC_PILLAR_PACKET {
	unsigned char size;
	char type;
	int id;
	float y;
};

struct SC_HP_PACKET {
	unsigned char size;
	char type;
	int hp;
};
