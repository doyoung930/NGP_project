#include "physics.h"

bool collide_sphere(glmvec3 a, glmvec3 b, float coll_dist)
{
	glmvec3 c;
	c.x = a.x - b.x;
	c.y = a.y - b.y;
	c.z = a.z - b.z;

	float dist = sqrt(c.x * c.x + c.y * c.y + c.z * c.z);

	if (coll_dist > dist)
	{
		return true;
	}

	return false;
}

bool collide_box(glmvec3 bb, glmvec3 tb, glmvec3 bb_scale, glmvec3 tb_scale)
{
	float size = 0.5;

	glmvec3 bb_min = { bb.x - size * bb_scale.x,bb.y - size * bb_scale.y,bb.z - size * bb_scale.z };
	glmvec3 bb_max = { bb.x + size * bb_scale.x,bb.y + size * bb_scale.y,bb.z + size * bb_scale.z };

	glmvec3 tb_min = { tb.x - size * tb_scale.x ,tb.y - size * tb_scale.y,tb.z - size * tb_scale.z };
	glmvec3 tb_max = { tb.x + size * tb_scale.x,tb.y + size * tb_scale.y,tb.z + size * tb_scale.z };

	if (bb_min.x <= tb_max.x &&
		bb_max.x >= tb_min.x &&
		bb_min.y <= tb_max.y &&
		bb_max.y >= tb_min.y &&
		bb_min.z <= tb_max.z &&
		bb_max.z >= tb_min.z)
	{
		return true;
	}

	return false;
}

glmvec3 CC_CalculateRVector(glmvec3 input, glmvec3 normal)
{
	float dotvec{ normal.x * input.x + normal.y * input.y + normal.z * input.z };
	glmvec3 ret{ input.x - 2.0f * dotvec * normal.x, input.y - 2.0f * dotvec * normal.y, input.z - 2.0f * dotvec * normal.z };
	return ret;
}