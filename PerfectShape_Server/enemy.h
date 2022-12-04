#pragma once

class Enemy
{
public:
	int kind;
	int hp;
	float speed;
	float radius;
	float x, y, z;
	float dx, dz;
	bool is_active;
	bool shot;

	Enemy() = default;
	~Enemy() {};
	void update();

};

