#pragma once

const vec2 powerUpSize(60.0f, 20.0f);
const vec2 powerUpVelocity(0.0f, 150.0f);

#include "GameObject.h"

/* Powerups consist of:

1. Speed - increases the velocity of the ball by 20%

2. Sticky - when the ball collides with the paddle, the ball remains stuck to the paddle unless the spacebar is pressed again.
This allows the player to better position the ball before releasing it

3. Pass-Through - collision resolution is disabled for non-solid blocks, allowing the ball to pass through multiple blocks

4. Pad Size Increase - increases the width of the paddle by 50 pixels

5. Confuse - activates the confuse postprocessing effect for a short period of time, confusing the user

6. Chaos - activates the chaos postprocessing effect for a short period of time, heavily disorienting the user

*/

class PowerUp : public GameObject
{
public:
	string type;
	float duration;
	bool activated;

	PowerUp(string type_, vec3 color_, float duration_, vec2 position_, Texture2D texture_) : 
		GameObject(position_, powerUpSize, texture_, color_, powerUpVelocity), type(type_), duration(duration_), activated()
	{  }
};