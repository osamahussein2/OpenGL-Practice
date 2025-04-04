#include "Game.h"

array<bool, 1024> Game::keys = {};

bool DetectCollision(GameObject& one, GameObject& two);
Collision DetectCollision(BallObject& one, GameObject& two);
Direction VectorDirection(vec2 target_);

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
			/* If the paddle�s x value would be less than 0 it would�ve moved outside the left edge, so move the paddle to 
			the left if the paddle�s x value is higher than the left edge�s x position */
			if (player->position.x >= 0.0f) player->position.x -= velocity;
			if (ball->stuck) ball->position.x -= velocity;
		}

		if (keys[GLFW_KEY_D])
		{
			/* Do the same for when the paddle breaches the right edge, but compare the right edge�s position with the right
			edge of the paddle (subtract the paddle�s width from the right edge�s x position) */
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

	if (ball->position.y >= gameHeight) // did ball reach bottom edge?
	{
		ResetLevel();
		ResetPlayer();
	}
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
	/* Check for collisions between the ball object and each brick of the level. If we detect a collision, we set the brick�s Destroyed property to true,
	which instantly stops the level from rendering this brick */
	for (GameObject& box : levels[level].bricks)
	{
		if (!box.destroyed)
		{
			Collision collision = DetectCollision(*ball, box);

			if (get<0>(collision)) // if collision is true
			{
				// destroy block if not solid
				if (!box.isSolid) box.destroyed = true;

				// collision resolution
				Direction dir = get<1>(collision);
				vec2 diff_vector = get<2>(collision);
				if (dir == LEFT || dir == RIGHT) // horizontal collision
				{
					ball->velocity.x = -ball->velocity.x; // reverse

					// relocate
					float penetration = ball->radius - abs(diff_vector.x);
					if (dir == LEFT)
						ball->position.x += penetration; // move right
					else
						ball->position.x -= penetration; // move left;
				}
				else // vertical collision
				{
					ball->velocity.y = -ball->velocity.y; // reverse
					// relocate
					float penetration = ball->radius -
						abs(diff_vector.y);
					if (dir == UP)
						ball->position.y -= penetration; // move up
					else
						ball->position.y += penetration; // move down
				}
			}
		}

		/* After checking collisions between the ball and each brick, check if the ball collided with the player paddle. If true (and the ball is not stuck to the paddle),
		we calculate the percentage of how far the ball�s center is moved from the paddle�s center compared to the half-extent of the paddle. The horizontal velocity of 
		the ball is then updated based on the distance it hit the paddle from its center. In addition to updating the horizontal velocity, we also have to reverse the 
		y velocity */
		Collision result = DetectCollision(*ball, *player);
		if (!ball->stuck && get<0>(result))
		{
			// check where it hit the board, and change velocity
			float centerBoard = player->position.x + player->size.x / 2.0f;
			float distance = (ball->position.x + ball->radius) - centerBoard;
			float percentage = distance / (player->size.x / 2.0f);

			// Move the ball accordingly
			float strength = 2.0f;
			vec2 oldVelocity = ball->velocity;
			ball->velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;

			/* This issue is called the sticky paddle issue. This happens, because the player paddle moves with a high velocity towards the ball with the ball�s center
			ending up inside the player paddle. Since we did not account for the case where the ball�s center is inside an AABB, the game tries to continuously react to
			all the collisions. Once it finally breaks free, it will have reversed its y velocity so much that it�s unsure whether to go up or down after breaking free */
			//ball->velocity.y = -ball->velocity.y;

			/* Fix this behavior by introducing a small hack made possible by the fact that the we can always assume we have a collision at the top of the paddle. Instead
			of reversing the y velocity, we simply always return a positive y direction so whenever it does get stuck, it will immediately break free */
			ball->velocity.y = -1.0f * abs(ball->velocity.y);
			ball->velocity = normalize(ball->velocity) * length(oldVelocity);
		}

	}
}

void Game::ResetLevel()
{
	if (level == 0) levels[0].Load("levels/one.lvl", gameWidth, gameHeight / 2);
	else if (level == 1) levels[1].Load("levels/two.lvl", gameWidth, gameHeight / 2);
	else if (level == 2) levels[2].Load("levels/three.lvl", gameWidth, gameHeight / 2);
	else if (level == 3) levels[3].Load("levels/four.lvl", gameWidth, gameHeight / 2);
}

void Game::ResetPlayer()
{
	// reset player/ball stats
	player->size = PLAYER_SIZE;
	player->position = vec2(gameWidth / 2.0f - PLAYER_SIZE.x / 2.0f, gameHeight - PLAYER_SIZE.y);
	ball->Reset(player->position + vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

bool DetectCollision(GameObject& one, GameObject& two)
{
	/* Check if the right side of the first object is greater than the left side of the second object and if the second object�s right side is greater than the first
	object�s left side; similarly for the vertical axis (AABB - AABB) */

	// collision x-axis?
	bool collisionX = one.position.x + one.size.x >= two.position.x && two.position.x + two.size.x >= one.position.x;

	// collision y-axis?
	bool collisionY = one.position.y + one.size.y >= two.position.y && two.position.y + two.size.y >= one.position.y;

	// collision only if on both axes
	return collisionX && collisionY;
}

// Create an overloaded function for CheckCollision that specifically deals with the case between a BallObject and a GameObject
Collision DetectCollision(BallObject& one, GameObject& two)
{
	/* First, get the difference vector between the ball�s center C and the AABB�s center B to obtain D. Then, clamp vector D to the AABB�s half-extents w and h
	and add it to B. The half-extents of a rectangle are the distances between the rectangle�s center and its edges: its size divided by two. This returns a
	position vector that is always located somewhere at the edge of the AABB (unless the circle�s center is inside the AABB). This clamped vector P is then the
	closest point from the AABB to the circle. And finally, calculate a new difference vector D that is the difference between the circle�s center C and the vector P */

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

	// Return the direction and difference vector
	if (length(difference) <= one.radius) return make_tuple(true, VectorDirection(difference), difference);
	else return make_tuple(false, UP, vec2(0.0f, 0.0f));
}

Direction VectorDirection(vec2 target_)
{
	array<vec2, 4> compass = 
	{
		vec2(0.0f, 1.0f), // up
		vec2(1.0f, 0.0f), // right
		vec2(0.0f, -1.0f), // down
		vec2(-1.0f, 0.0f) // left
	};

	float max = 0.0f;
	unsigned int best_match = -1; // set it to an undefined number like -1 in this case

	for (unsigned int i = 0; i < 4; i++)
	{
		/* Compare target to each of the direction vectors in the compass array. The compass vector target is closest to in angle, is the direction returned
		to the function caller */
		float dot_product = dot(normalize(target_), compass[i]);

		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}

