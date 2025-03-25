#include "Game.h"

array<bool, 1024> Game::keys = {};

Game::Game(unsigned int gameWidth_, unsigned int gameHeight_) : gameState(GAME_ACTIVE), gameWidth(gameWidth_), gameHeight(gameHeight_)
{
}

Game::~Game()
{
	delete spriteRenderer;
}

void Game::InitializeGame()
{
	glViewport(0, 0, 1280, 960);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Load shaders
	ResourceManager::LoadShader("SpriteRendererVertexShader.glsl", "SpriteRendererFragmentShader.glsl", nullptr, "sprite");

	// Configure shaders
	mat4 proj = ortho(0.0f, static_cast<float>(gameWidth), static_cast<float>(gameHeight), 0.0f, -1.0f, 1.0f);

	glUseProgram(ResourceManager::GetShader("sprite").shaderProgram);

	glUniform1i(glGetUniformLocation(ResourceManager::GetShader("sprite").shaderProgram, "image"), 0);
	glUniformMatrix4fv(glGetUniformLocation(ResourceManager::GetShader("sprite").shaderProgram, "projectionMatrix"), 1, GL_FALSE, value_ptr(proj));

	// Set render-specific controls
	spriteRenderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

	// Load textures
	ResourceManager::LoadTexture("Textures/awesomeface.png", true, "face");
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

	spriteRenderer->DrawSprite(ResourceManager::GetTexture("face"), vec2(200.0f, 200.0f), vec2(300.0f, 400.0f), 45.0f, vec3(0.0f, 1.0f, 0.0f));
}