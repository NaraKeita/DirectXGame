#pragma once
#include "math/Vector3.h"

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

Transform cameraTransform{
    {1.0f, 1.0f, 1.0f },
    {0.0f, 0.0f, 0.0f },
    {0.0f, 0.0f, -5.0f}
};