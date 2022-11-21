#include "Player.h"


Player::Player()
{
}

Player::Player(SOCKET& socket, short id): _c_socket(socket), _id(id)
{
	x = 0;
	y = 0;
	z = 0;
	dx = 0;
	dy = 0;
	dz = 0;
	_in_use = false;
}
Player::~Player()
{

}