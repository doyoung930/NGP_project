#pragma once
struct CS_MOUSECLICK_PACKET {
	unsigned char size;
	char type;
	short id; // 클라이언트 아이디
	float dx, dy, dz; // 시선 벡터(플레이어의 총알 방향)
};

struct SC_BULLET_PACKET {
	unsigned char size;
	char type;
	short bullet_id;
	float x, y, z;
};

struct SC_STAGE_PACKET {
	unsigned char size;
	char type;
	short state;
};

struct SC_ENEMY_PACKET {
	unsigned char size;
	char type;
	short id;
	float x, y, z;
};

struct SC_BULLETHIT_PACKET {
	unsigned char size;
	char type;
	short bullet_id;
};

struct SC_PLAYERHIT_PACKET {
	unsigned char size;
	char type;
	short id;
};

struct SC_ENERMYHIT_PACKET {
	unsigned char size;
	char type;
	short hp;
	short id;
};