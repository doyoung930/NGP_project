#pragma once
#include "header.h"

bool collide_sphere(glmvec3 a, glmvec3 b, float coll_dist);
bool collide_box(glmvec3 bb, glmvec3 tb, glmvec3 bb_scale, glmvec3 tb_scale);
glmvec3 CC_CalculateRVector(glmvec3 input, glmvec3 normal);