#pragma once

typedef union _Vec2
    {
    float f[ 2 ];
    struct
        {
        float           x;
        float           y;
        } v;
    } Vec2;

typedef union _Vec3
    {
    float f[ 3 ];
    struct
        {
        float           x;
        float           y;
        float           z;
        } v;
    } Vec3;

typedef union _Vec4
    {
    float f[ 4 ];
    struct
        {
        float           x;
        float           y;
        float           z;
        float           w;
        } v;
    } Vec4;

typedef Vec4 Quaternion;