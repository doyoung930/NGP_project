#pragma once
#include "header.h"

class Player
{
public:
	Player();
	Player(SOCKET& socket, short id);
	~Player();
	void update();
	void undo();
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
	short	hp;
	float	_hx, _hy, _hz;
	bool	_is_hit;
	int		_hit_cnt;
	float	_hit_speed;
	bool	_is_unbeatable;
	chrono::system_clock::time_point	_unbeatable_time;
};

