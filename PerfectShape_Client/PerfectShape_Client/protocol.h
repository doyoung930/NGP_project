#pragma once
struct CS_MOUSECLICK_PACKET {
	unsigned char size;
	char type;
	short id; // Ŭ���̾�Ʈ ���̵�
	float dx, dy, dz; // �ü� ����(�÷��̾��� �Ѿ� ����)
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