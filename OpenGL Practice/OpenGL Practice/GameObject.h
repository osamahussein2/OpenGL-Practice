#pragma once

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glad/glad.h>
#include <glm.hpp>

#include "Texture2D.h"
#include "SpriteRenderer.h"

class GameObject
{
public:

	// Object state
	vec2 position, size, velocity;
	vec3 color;
	float rotation;
	bool isSolid, destroyed;

	// Render state
	Texture2D sprite;

	GameObject();
	GameObject(vec2 pos_, vec2 size_, Texture2D sprite_, vec3 color_ = vec3(1.0f), vec2 velocity_ = vec2(0.0f, 0.0f));

	virtual void DrawSprite(SpriteRenderer& renderer_);
};

#endif