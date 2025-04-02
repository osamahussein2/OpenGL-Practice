#include "GameLevel.h"
#include "ResourceManager.h"

void GameLevel::Init(vector<vector<unsigned int>> tileData_, unsigned int levelWidth_, unsigned int levelHeight_)
{
	// Calculate dimensions
	unsigned int height = tileData_.size();
	unsigned int width = tileData_[0].size();
	float unitWidth = levelWidth_ / static_cast<float>(width);
	float unitHeight = levelHeight_ / height;

	// Initialize level tiles based on tileData
	for (unsigned int y = 0; y < height; ++y)
	{
		for (unsigned int x = 0; x < width; ++x)
		{
			// Check block type from level data (2D level array)
			if (tileData_[y][x] == 1) // solid brick
			{
				vec2 pos(unitWidth * x, unitHeight * y);
				vec2 size(unitWidth, unitHeight);

				GameObject obj(pos, size, ResourceManager::GetTexture("block_solid"), vec3(0.8f, 0.8f, 0.7f));

				obj.isSolid = true;
				bricks.push_back(obj);
			}

			else if (tileData_[y][x] > 1)
			{
				vec3 color = vec3(1.0f); // original: white

				if (tileData_[y][x] == 2) color = vec3(0.2f, 0.6f, 1.0f);
				else if (tileData_[y][x] == 3) color = vec3(0.0f, 0.7f, 0.0f);
				else if (tileData_[y][x] == 4) color = vec3(0.8f, 0.8f, 0.4f);
				else if (tileData_[y][x] == 5) color = vec3(1.0f, 0.5f, 0.0f);

				vec2 pos(unitWidth * x, unitHeight * y);
				vec2 size(unitWidth, unitHeight);
				
				bricks.push_back(GameObject(pos, size, ResourceManager::GetTexture("block"), color));
			}
		}
	}
}

void GameLevel::Load(const char* file_, unsigned int levelWidth_, unsigned int levelHeight_)
{
	// Clear the old bricks data
	bricks.clear();

	// Load from file
	unsigned int tileCode;
	GameLevel level;
	string line;
	ifstream fstream(file_);
	vector<vector<unsigned int>> tileData;

	if (fstream)
	{
		while (getline(fstream, line))
		{
			istringstream sstream(line);
			vector<unsigned int> row;

			// Read each word
			while (sstream >> tileCode) row.push_back(tileCode);

			tileData.push_back(row);
		}

		if (tileData.size() > 0)
		{
			Init(tileData, levelWidth_, levelHeight_);
		}
	}
}

void GameLevel::DrawSprite(SpriteRenderer& renderer_)
{
	for (GameObject& tile : this->bricks)
	{
		if (!tile.destroyed) tile.DrawSprite(renderer_);
	}
}

bool GameLevel::IsLevelCompleted()
{
	for (GameObject& tile : this->bricks)
	{
		if (!tile.isSolid && !tile.destroyed) return false;
	}

	return true;
}