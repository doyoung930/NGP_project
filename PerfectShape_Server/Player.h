#pragma once
#include "header.h"
class Player
{
public:
	Player(SOCKET& socket, int id);
	~Player();
public:
	SOCKET	_c_socket;
	int		_id;
};

