#pragma once

#ifndef GAME_H
#define GAME_H

#include <array>

#include <glad/glad.h>
#include <glfw3.h>

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
};

#endif GAME_H