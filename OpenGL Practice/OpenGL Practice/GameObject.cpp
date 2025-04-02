#include "GameObject.h"

GameObject::GameObject() : position(vec2(0.0f)), size(vec2(1.0f)), velocity(vec2(0.0f)), color(vec3(1.0f)), rotation(0.0f),
sprite(), isSolid(false), destroyed(false)
{

}

GameObject::GameObject(vec2 pos_, vec2 size_, Texture2D sprite_, vec3 color_, vec2 velocity_)
{
	position = pos_;
	size = size_;
	sprite = sprite_;
	color = color_;
	velocity = velocity_;

	rotation = 0.0f;
	isSolid = false;
	destroyed = false;
}

void GameObject::DrawSprite(SpriteRenderer& renderer_)
{
	renderer_.DrawSprite(this->sprite, this->position, this->size, this->rotation, this->color);
}