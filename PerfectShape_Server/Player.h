#pragma once
#include "header.h"
class Player
{
public:
	Player();
	Player(SOCKET& socket, short id);
	~Player();
public:
	SOCKET	_c_socket;
	short	_id;
	float x, y, z;
	float dx, dy, dz;
	bool	_in_use;
};

