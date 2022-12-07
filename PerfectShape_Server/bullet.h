#pragma once
#include <cmath>
#include "protocol.h"
#include "enemy.h"

struct Bullet
{
	float x, y, z;
	float dx{}, dy{}, dz{};
	bool is_active;
	bool is_team;
	int enemyID;
	float speed;

	Bullet();
	void update();
	bool isOut();
	int ColisionCheckEnemy(Enemy enemy[]);
	bool ColisionCheckClient();
};

