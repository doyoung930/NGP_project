#include "enemy.h"

void Enemy::update()
{
	if (kind < 4) {
		x += speed * dx;
		z += speed * dz;
	}

	else if (kind == 4) {

	}
}
