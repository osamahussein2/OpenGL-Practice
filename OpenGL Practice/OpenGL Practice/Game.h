#pragma once

#ifndef GAME_H
#define GAME_H

#include <array>

#include <glad/glad.h>
#include <glfw3.h>
#include <tuple>

#include "ResourceManager.h"
#include "SpriteRenderer.h"
#include "GameLevel.h"
#include "BallObject.h"

// Initial size of the player paddle
const vec2 PLAYER_SIZE(100.0f, 20.0f);

// Initial velocity of the player paddle
const float PLAYER_VELOCITY(500.0f);

// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);

// Radius of the ball object
const float BALL_RADIUS = 12.5f;

using namespace std;

// Represents the current state of the game
enum GameState
{
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

enum Direction {
	UP,
	RIGHT,
	DOWN,
	LEFT
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

	void CheckCollisions();

	void ResetLevel();
	void ResetPlayer();

	// Create a game state enumeration object
	GameState gameState;

	static array<bool, 1024> keys;
	unsigned int gameWidth, gameHeight;

	SpriteRenderer* spriteRenderer;

	vector<GameLevel> levels;
	unsigned int level;

	GameObject* player;
	BallObject* ball;
};

/* To calculate the required values for collision resolution we need a bit more information from the collision function(s) than just a true or false.
We’re now going to return a tuple of information that tells us if a collision occurred, what direction it occurred, and the difference vector R. You can
find the tuple container in the <tuple> header */
typedef tuple<bool, Direction, vec2> Collision;

#endif GAME_H