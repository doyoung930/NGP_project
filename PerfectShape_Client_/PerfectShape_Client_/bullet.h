#pragma once

struct Bullet
{
    float x, y, z;
    bool is_active;
    bool is_team;

    Bullet()
    {
        x = 0; y = 0; z = 0;
        is_active = false;
        is_team = false;
    };
};