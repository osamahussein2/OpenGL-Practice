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

	vec2 ballPos = playerPos + vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
	ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
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
			if (ball->stuck) ball->position.x -= velocity;
		}

		if (keys[GLFW_KEY_D])
		{
			/* Do the same for when the paddle breaches the right edge, but compare the right edge’s position with the right
			edge of the paddle (subtract the paddle’s width from the right edge’s x position) */
			if (player->position.x <= gameWidth - player->size.x) player->position.x += velocity;
			if (ball->stuck) ball->position.x += velocity;
		}

		if (keys[GLFW_KEY_SPACE])
		{
			ball->stuck = false;
		}
	}

}

void Game::UpdateGame(float dt)
{
	// Update objects during runtime
	ball->Move(dt, gameWidth);

	// Check for collisions between the ball and the bricks
	CheckCollisions();
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

		ball->DrawSprite(*spriteRenderer);
	}

}

void Game::CheckCollisions()
{
	/* Check for collisions between the ball object and each brick of the level. If we detect a collision, we set the brick’s Destroyed property to true,
	which instantly stops the level from rendering this brick */
	for (GameObject& box : levels[level].bricks)
	{
		if (!box.destroyed)
		{
			if (DetectCollision(*ball, box))
			{
				if (!box.isSolid) box.destroyed = true;
			}
		}
	}
}

bool Game::DetectCollision(GameObject& one, GameObject& two)
{
	/* Check if the right side of the first object is greater than the left side of the second object and if the second object’s right side is greater than the first 
	object’s left side; similarly for the vertical axis (AABB - AABB) */

	// collision x-axis?
	bool collisionX = one.position.x + one.size.x >= two.position.x && two.position.x + two.size.x >= one.position.x;

	// collision y-axis?
	bool collisionY = one.position.y + one.size.y >= two.position.y && two.position.y + two.size.y >= one.position.y;

	// collision only if on both axes
	return collisionX && collisionY;
}

// Create an overloaded function for CheckCollision that specifically deals with the case between a BallObject and a GameObject
bool Game::DetectCollision(BallObject& one, GameObject& two)
{
	/* First, get the difference vector between the ball’s center C and the AABB’s center B to obtain D. Then, clamp vector D to the AABB’s half-extents w and h
	and add it to B. The half-extents of a rectangle are the distances between the rectangle’s center and its edges: its size divided by two. This returns a
	position vector that is always located somewhere at the edge of the AABB (unless the circle’s center is inside the AABB). This clamped vector P is then the
	closest point from the AABB to the circle. And finally, calculate a new difference vector D that is the difference between the circle’s center C and the vector P */

	// get center point circle first (AABB - Circle)
	vec2 center(one.position + one.radius);

	// calculate AABB info (center, half-extents)
	vec2 aabb_half_extents(two.size.x / 2.0f, two.size.y / 2.0f);
	vec2 aabb_center(two.position.x + aabb_half_extents.x, two.position.y + aabb_half_extents.y);

	// get difference vector between both centers
	vec2 difference = center - aabb_center;
	vec2 clamped = clamp(difference, -aabb_half_extents, aabb_half_extents);

	// add clamped value to AABB_center and get the value closest to circle
	vec2 closest = aabb_center + clamped;

	// vector between center circle and closest point AABB
	difference = closest - center;

	return length(difference) < one.radius;
}
