#pragma once
#include "header.h"
class Player
{
public:
	Player();
	Player(SOCKET& socket, short id);
	~Player();
	void update();
public:
	SOCKET	_c_socket;
	short	_id;
	float	x, y, z;
	float	dx, dy, dz;
	float	view_degree;
	float	speed;
	int		FB_Dir;
	int		LR_Dir;
	bool	_in_use;
};

