#include "BallObject.h"

BallObject::BallObject() : GameObject(), radius(12.5), stuck(true)
{
}

BallObject::BallObject(vec2 position_, float radius_, vec2 velocity_, Texture2D sprite_) : GameObject(position_, vec2(radius_ * 2.0f, radius_ * 2.0f), sprite_, vec3(1.0f), 
	velocity_), radius(radius_), stuck(true)
{

}

vec2 BallObject::Move(float dt, unsigned int windowWidth_)
{
	if (!stuck)
	{
		// Move the ball
		position += velocity * dt;

		// Check if outside window bounds and if so, reverse velocity and restore at current position
		if (position.x <= 0.0f)
		{
			velocity.x = -velocity.x;
			position.x = 0.0f;
		}

		else if (position.x + size.x >= windowWidth_)
		{
			velocity.x = -velocity.x;
			position.x = windowWidth_ - size.x;
		}

		if (position.y <= 0.0f)
		{
			velocity.y = -velocity.y;
			position.y = 0.0f;
		}
	}

	return position;
}

void BallObject::Reset(vec2 position_, vec2 velocity_)
{
	// this is a pointer to the instance of the class itself
	this->position = position_;
	this->velocity = velocity_;
	this->stuck = true;
}
