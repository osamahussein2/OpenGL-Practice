#pragma once

#ifndef GAME_H
#define GAME_H

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include "ResourceManager.h"
#include "SpriteRenderer.h"
#include "GameLevel.h"

// Initial size of the player paddle
const vec2 PLAYER_SIZE(100.0f, 20.0f);

// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);

using namespace std;

// Represents the current state of the game
enum GameState
{
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

class Game
{
public:

	// Set up the constructor and deconstructor
	Game(unsigned int gameWidth_, unsigned int gameHeight_);
	~Game();

	// Initialize the game state
	void InitializeGame();

	// Game loop
	void ProcessInput(float dt);
	void UpdateGame(float dt);
	void RenderGame();

	// Create a game state enumeration object
	GameState gameState;

	static array<bool, 1024> keys;
	unsigned int gameWidth, gameHeight;

	SpriteRenderer* spriteRenderer;

	std::vector<GameLevel> levels;
	unsigned int level;

	GameObject* player;
};

#endif GAME_H