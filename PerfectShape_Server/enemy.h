#pragma once
class Enemy
{
public:
	int kind;
	int hp;
	float speed;
	float radius;
	float x, y, z;
	bool is_active;
	bool shot;

	Enemy() = default;
	~Enemy() {};
};

