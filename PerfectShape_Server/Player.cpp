#include "Player.h"

Player::Player()
{

}

Player::Player(SOCKET& socket, short id) : _c_socket(socket), _id(id)
{
	x = 0;
	y = 0;
	z = 0;
	dx = 0;
	dy = 0;
	dz = 0;
	FB_Dir = 0;
	LR_Dir = 0;
	speed = 0.0525f;
	view_degree = 0;
	_in_use = false;
	hp = 3;
	_hx = 0;
	_hy = 0;
	_hz = 0;
	_is_hit = false;
	_hit_cnt = 0;
	_hit_speed = 0.2f;
	_is_unbeatable = false;
	room = -1;
}
Player::~Player()
{

}

void Player::update() {
	if (FB_Dir == 1)
	{
		x += speed * dx;
		z += speed * dz;
	}
	else if (FB_Dir == -1)
	{
		x -= speed * dx;
		z -= speed * dz;
	}
	if (LR_Dir == -1)
	{
		x -= speed * cos(3.141592 * (view_degree + 90.0f) / 180.0f);
		z -= speed * sin(3.141592 * (view_degree + 90.0f) / 180.0f);
	}
	else if (LR_Dir == 1)
	{
		x += speed * cos(3.141592 * (view_degree + 90.0f) / 180.0f);
		z += speed * sin(3.141592 * (view_degree + 90.0f) / 180.0f);
	}
}

void Player::undo()
{
	if (FB_Dir == 1)
	{
		x -= speed * dx;
		z -= speed * dz;
	}
	else if (FB_Dir == -1)
	{
		x += speed * dx;
		z += speed * dz;
	}
	if (LR_Dir == -1)
	{
		x += speed * cos(3.141592 * (view_degree + 90.0f) / 180.0f);
		z += speed * sin(3.141592 * (view_degree + 90.0f) / 180.0f);
	}
	else if (LR_Dir == 1)
	{
		x -= speed * cos(3.141592 * (view_degree + 90.0f) / 180.0f);
		z -= speed * sin(3.141592 * (view_degree + 90.0f) / 180.0f);
	}
}