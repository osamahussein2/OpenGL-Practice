#include "Game.h"

array<bool, 1024> Game::keys = {};

Game::Game(unsigned int gameWidth_, unsigned int gameHeight_) : gameState(GAME_ACTIVE), gameWidth(gameWidth_), gameHeight(gameHeight_)
{
}

Game::~Game()
{
	delete spriteRenderer, player;
}

void Game::InitializeGame()
{
	/*glViewport(0, 0, 1280, 960);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

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
	ResourceManager::LoadTexture("Textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("Textures/awesomeface.png", true, "face");
	ResourceManager::LoadTexture("Textures/block.png", false, "block");
	ResourceManager::LoadTexture("Textures/block_solid.png", false, "block_solid");
	ResourceManager::LoadTexture("textures/paddle.png", true, "paddle");

	// Load levels
	GameLevel one; one.Load("Levels/one.lvl", gameWidth, gameHeight / 2);
	GameLevel two; two.Load("Levels/two.lvl", gameWidth, gameHeight / 2);
	GameLevel three; three.Load("Levels/three.lvl", gameWidth, gameHeight / 2);
	GameLevel four; four.Load("Levels/four.lvl", gameWidth, gameHeight / 2);

	levels.push_back(one);
	levels.push_back(two);
	levels.push_back(three);
	levels.push_back(four);

	level = 0;

	vec2 playerPos = vec2(gameWidth / 2.0f - PLAYER_SIZE.x / 2.0f, gameHeight - PLAYER_SIZE.y);

	player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
}

void Game::ProcessInput(float dt)
{
	if (gameState == GAME_ACTIVE)
	{
		float velocity = PLAYER_VELOCITY * dt;

		// Move the paddle (player board)
		if (keys[GLFW_KEY_A])
		{
			/* If the paddle’s x value would be less than 0 it would’ve moved outside the left edge, so move the paddle to 
			the left if the paddle’s x value is higher than the left edge’s x position */
			if (player->position.x >= 0.0f) player->position.x -= velocity;
		}

		if (keys[GLFW_KEY_D])
		{
			/* Do the same for when the paddle breaches the right edge, but compare the right edge’s position with the right
			edge of the paddle (subtract the paddle’s width from the right edge’s x position) */
			if (player->position.x <= gameWidth - player->size.x) player->position.x += velocity;
		}
	}

}

void Game::UpdateGame(float dt)
{

}

void Game::RenderGame()
{
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

	//spriteRenderer->DrawSprite(ResourceManager::GetTexture("face"), vec2(200.0f, 200.0f), vec2(300.0f, 400.0f), 45.0f, vec3(0.0f, 1.0f, 0.0f));

	if (gameState == GAME_ACTIVE)
	{
		// Draw background
		spriteRenderer->DrawSprite(ResourceManager::GetTexture("background"), vec2(0.0f, 0.0f), 
			vec2(gameWidth, gameHeight), 0.0f);

		// Draw level
		levels[level].DrawSprite(*spriteRenderer);

		// Draw the player
		player->DrawSprite(*spriteRenderer);
	}

}