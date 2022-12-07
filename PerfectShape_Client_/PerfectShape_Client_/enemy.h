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
	bool pop[3];
	float px, py, pz; // 파티클 원점 위치


	Enemy() = default;
	~Enemy() {};
	void update();

};

