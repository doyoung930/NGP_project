#include "bullet.h"

Bullet::Bullet()
{
	x = 0; y = 0; z = 0;
	dx = 0; dy = 0; dz = 0;
	is_active = false;
	is_team = false;
};

void Bullet::update() {
	x += 0.3f * dx;
	y += 0.3f * dy;
	z += 0.3f * dz;
}

bool Bullet::isOut()
{
	if (x > 5.0f || x < -5.0f) {
		return true;
	}
	if (y > 10.0f || y < -1.0f) {
		return true;
	}
	if (z > 5.0f || z < -5.0f) {
		return true;
	}
	return false;
}

int Bullet::ColisionCheckEnemy(Enemy enemy[])
{
	for (int i = 0; i < MAX_ENEMY_NUM; ++i) {
		if (is_team && enemy[i].is_active) {
			if (!is_active) continue;

			float cx = enemy[i].x - x;
			float cy = enemy[i].y - y;
			float cz = enemy[i].z - z;
			float dist = sqrt(cx * cx + cy * cy + cz * cz);
			float weight = 0.03f;
			if (enemy[i].kind == 4) weight = 0.04;

			if (enemy[i].radius > dist + weight) {
				is_active = false;
				enemy[i].hp -= 1;
				if (enemy[i].hp <= 0) {
					enemy[i].is_active = false;
					return i;
				}
				enemy[i].speed = 0.035 / float(enemy[i].hp);
				enemy[i].radius = 0.5f * 0.25f * (float)enemy[i].hp * 1.3f;
				enemy[i].y = 0.25f * (float)enemy[i].hp * 0.5f - 0.5f;

				//부딪힌 적의 아이디를 반환
				return i;
			}
		}
	}
	return -1;
}

bool Bullet::ColisionCheckClient()
{
	return false;
}