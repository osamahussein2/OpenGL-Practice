#pragma once

#include "GameObject.h"

class BallObject : public GameObject
{
public:
	// Ball state
	float radius;
	bool stuck;
	bool sticky, passThrough;

	BallObject();
	BallObject(vec2 position_, float radius_, vec2 velocity_, Texture2D sprite_);

	vec2 Move(float dt, unsigned int windowWidth_);
	void Reset(vec2 position_, vec2 velocity_);
};