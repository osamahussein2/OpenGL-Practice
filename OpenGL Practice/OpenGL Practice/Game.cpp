#include "Game.h"

array<bool, 1024> Game::keys = {};

Game::Game(unsigned int gameWidth_, unsigned int gameHeight_) : gameState(GAME_ACTIVE), gameWidth(gameWidth_), gameHeight(gameHeight_)
{
}

Game::~Game()
{
	
}

void Game::InitializeGame()
{
	glViewport(0, 0, 1280, 960);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Game::ProcessInput(float dt)
{

}

void Game::UpdateGame(float dt)
{

}

void Game::RenderGame()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}