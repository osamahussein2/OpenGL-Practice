#pragma once

#include "GameObject.h"

class GameLevel
{
public:
	vector<GameObject> bricks;

	GameLevel() {}

	void Load(const char* file_, unsigned int levelWidth_, unsigned int levelHeight_);
	void DrawSprite(SpriteRenderer& renderer_);
	bool IsLevelCompleted();

private:
	void Init(vector<vector<unsigned int>> tileData_, unsigned int levelWidth_, unsigned int levelHeight_);
};