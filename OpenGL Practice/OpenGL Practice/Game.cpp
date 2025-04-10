#include "Game.h"

#include <irrKlang.h>
using namespace irrklang;

array<bool, 1024> Game::keys = {};
array<bool, 1024> Game::keysProcessed = {};

bool DetectCollision(GameObject& one, GameObject& two);
Collision DetectCollision(BallObject& one, GameObject& two);
Direction VectorDirection(vec2 target_);

/* Create an irrKlang::ISoundEngine, initialize it with createIrrKlangDevice, and then use the engine to load and play
audio files */

/* 3D audio means that an audio source can have a 3D position that will attenuate its volume based on the camera’s
distance to the audio source, making it feel natural in a 3D world */
ISoundEngine* SoundEngine = createIrrKlangDevice();

Game::Game(unsigned int gameWidth_, unsigned int gameHeight_) : gameState(GAME_MENU), gameWidth(gameWidth_), 
gameHeight(gameHeight_), level(0), lives(3)
{
}

Game::~Game()
{
	delete spriteRenderer, player, ball, Particles, Effects, text;

	SoundEngine->drop();
}

void Game::InitializeGame()
{
	/*glViewport(0, 0, 1280, 960);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/

	// Load shaders
	ResourceManager::LoadShader("SpriteRendererVertexShader.glsl", "SpriteRendererFragmentShader.glsl", nullptr, "sprite");
	ResourceManager::LoadShader("ParticleVertexShader.glsl", "ParticleFragmentShader.glsl", nullptr, "particle");
	ResourceManager::LoadShader("PostprocessingVertexShader.glsl", "PostprocessingFragmentShader.glsl", nullptr, "postprocessing");

	// Configure shaders
	mat4 proj = ortho(0.0f, static_cast<float>(gameWidth), static_cast<float>(gameHeight), 0.0f, -1.0f, 1.0f);

	glUseProgram(ResourceManager::GetShader("sprite").shaderProgram);

	glUniform1i(glGetUniformLocation(ResourceManager::GetShader("sprite").shaderProgram, "image"), 0);
	glUniformMatrix4fv(glGetUniformLocation(ResourceManager::GetShader("sprite").shaderProgram, "projectionMatrix"), 1, GL_FALSE, value_ptr(proj));

	glUseProgram(ResourceManager::GetShader("particle").shaderProgram);

	glUniform1i(glGetUniformLocation(ResourceManager::GetShader("particle").shaderProgram, "sprite"), 0);
	glUniformMatrix4fv(glGetUniformLocation(ResourceManager::GetShader("particle").shaderProgram, "projection"), 1, GL_FALSE, value_ptr(proj));

	// Load textures
	ResourceManager::LoadTexture("Textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("Textures/awesomeface.png", true, "face");
	ResourceManager::LoadTexture("Textures/block.png", false, "block");
	ResourceManager::LoadTexture("Textures/block_solid.png", false, "block_solid");
	ResourceManager::LoadTexture("Textures/paddle.png", true, "paddle");
	ResourceManager::LoadTexture("Textures/particle.png", true, "particle");
	ResourceManager::LoadTexture("Textures/powerup_speed.png", true, "powerup_speed");
	ResourceManager::LoadTexture("Textures/powerup_sticky.png", true, "powerup_sticky");
	ResourceManager::LoadTexture("Textures/powerup_increase.png", true, "powerup_increase");
	ResourceManager::LoadTexture("Textures/powerup_confuse.png", true, "powerup_confuse");
	ResourceManager::LoadTexture("Textures/powerup_chaos.png", true, "powerup_chaos");
	ResourceManager::LoadTexture("Textures/powerup_passthrough.png", true, "powerup_passthrough");

	// Set render-specific controls
	spriteRenderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));

	Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);

	Effects = new Postprocessing(ResourceManager::GetShader("postprocessing"), this->gameWidth, this->gameHeight);

	text = new TextRenderer(gameWidth, gameHeight);
	text->Load("fonts/ocraext.TTF", 24);

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

	// Play audio here and make it loop by passing in true after the file string name
	SoundEngine->play2D("Audio/breakout.mp3", true);
}

void Game::ProcessInput(float dt)
{
	if (gameState == GAME_MENU)
	{
		/* The trick is to, not only record the keys currently pressed, but also store the keys that have been processed
		once, until released again. We then check (before processing) whether the key has not yet been processed, and if
		so, process this key after which we store this key as being processed. Once we want to process the same key again
		without the key having been released, we do not process the key */
		if (keys[GLFW_KEY_ENTER] && !keysProcessed[GLFW_KEY_ENTER])
		{
			gameState = GAME_ACTIVE;
			keysProcessed[GLFW_KEY_ENTER] = true;
		}

		if (keys[GLFW_KEY_W] && !keysProcessed[GLFW_KEY_W])
		{
			/* The modulus operator (%) to make sure the Level variable remains within the acceptable level range
			(between 0 and 3) */
			level = (level + 1) % 4;
			keysProcessed[GLFW_KEY_W] = true;
		}

		if (keys[GLFW_KEY_S] && !keysProcessed[GLFW_KEY_S])
		{
			if (level > 0)
				--level;
			else
				level = 3;
			keysProcessed[GLFW_KEY_S] = true;
		}

	}


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

	if (gameState == GAME_WIN)
	{
		if (keys[GLFW_KEY_ENTER])
		{
			keysProcessed[GLFW_KEY_ENTER] = true;
			Effects->Chaos = false;
			gameState = GAME_MENU;
		}
	}
}

float ShakeTime = 0.0f;

void Game::UpdateGame(float dt)
{
	// Update objects during runtime
	ball->Move(dt, gameWidth);

	// Check for collisions between the ball and the bricks
	CheckCollisions();

	if (ball->position.y >= gameHeight) // did ball reach bottom edge?
	{
		--lives;

		// did the player lose all his lives? : Game over
		if (lives == 0)
		{
			ResetLevel();
			gameState = GAME_MENU;
		}

		ResetPlayer();
	}

	// Only update the particles once the ball isn't stuck on the paddle
	if (!ball->stuck)
	{
		if (Particles == nullptr)
		{
			Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), 
				ResourceManager::GetTexture("particle"), 500);
		}

		else
		{
			// update particles
			Particles->UpdateParticles(dt, *ball, 2, vec2(ball->radius / 2.0f));
		}
	}

	// update PowerUps
	UpdatePowerUps(dt);

	if (ShakeTime > 0.0f)
	{
		ShakeTime -= dt;

		if (ShakeTime <= 0.0f) Effects->Shake = false;
	}

	if (gameState == GAME_ACTIVE && levels[level].IsLevelCompleted())
	{
		ResetLevel();
		ResetPlayer();
		Effects->Chaos = true;
		gameState = GAME_WIN;
	}

}

void Game::RenderGame()
{
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

	//spriteRenderer->DrawSprite(ResourceManager::GetTexture("face"), vec2(200.0f, 200.0f), vec2(300.0f, 400.0f), 45.0f, vec3(0.0f, 1.0f, 0.0f));

	if (gameState == GAME_ACTIVE || gameState == GAME_MENU || gameState == GAME_WIN)
	{
		Effects->BeginRender();

		// Draw background
		spriteRenderer->DrawSprite(ResourceManager::GetTexture("background"), vec2(0.0f, 0.0f), 
			vec2(gameWidth, gameHeight), 0.0f);

		// Draw level
		levels[level].DrawSprite(*spriteRenderer);

		// Draw the player
		player->DrawSprite(*spriteRenderer);

		// Only draw the particles once the ball isn't stuck on the paddle
		if (!ball->stuck && Particles != nullptr)
		{
			// draw particles
			Particles->DrawParticles();
		}

		ball->DrawSprite(*spriteRenderer);

		Effects->EndRender();
		Effects->RenderPostprocessing(glfwGetTime());

		// Render all the power ups in the game only if they're not destroyed yet
		for (PowerUp& powerUp : PowerUps)
			if (!powerUp.destroyed) powerUp.DrawSprite(*spriteRenderer);

		stringstream ss;
		ss << lives;

		text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);
	}

	if (gameState == GAME_MENU)
	{
		text->RenderText("Press ENTER to start", 250.0f, gameHeight / 2.0f, 1.0f);
		text->RenderText("Press W or S to select level", 245.0f, gameHeight / 2.0f + 20.0f, 0.75f);
	}

	if (gameState == GAME_WIN)
	{
		text->RenderText("You WON!!!", 320.0, gameHeight / 2 - 20.0, 1.0, vec3(0.0, 1.0, 0.0));
		text->RenderText("Press ENTER to retry or ESC to quit", 130.0, gameHeight / 2, 1.0, vec3(1.0, 1.0, 0.0));
	}

}

void Game::ActivatePowerUp(PowerUp& powerUp)
{
	if (powerUp.type == "speed")
	{
		ball->velocity *= 1.2;
	}

	else if (powerUp.type == "sticky")
	{
		ball->sticky = true;
		player->color = vec3(1.0f, 0.5f, 1.0f);
	}
	else if (powerUp.type == "pass-through")
	{
		ball->passThrough = true;
		ball->color = vec3(1.0f, 0.5f, 0.5f);
	}

	else if (powerUp.type == "pad-size-increase")
	{
		player->size.x += 50;
	}

	else if (powerUp.type == "confuse")
	{
		if (!Effects->Chaos) Effects->Confuse = true; // only if chaos isn’t already active
	}

	else if (powerUp.type == "chaos")
	{
		if (!Effects->Confuse) Effects->Chaos = true;
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
			Collision collision = DetectCollision(*ball, box);

			if (get<0>(collision)) // if collision is true
			{
				// destroy block if not solid
				if (!box.isSolid)
				{
					box.destroyed = true;
					SpawnPowerUps(box);

					SoundEngine->play2D("Audio/bleep.mp3", false);
				}

				// if block is solid, enable shake effect
				else
				{
					ShakeTime = 0.05f; // Reset the shake time duration to a specific value over 0
					Effects->Shake = true;

					SoundEngine->play2D("Audio/solid.wav", false);
				}

				// collision resolution
				Direction dir = get<1>(collision);
				vec2 diff_vector = get<2>(collision);

				if (!(ball->passThrough && !box.isSolid))
				{
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
		}

		/* After checking collisions between the ball and each brick, check if the ball collided with the player paddle. If true (and the ball is not stuck to the paddle),
		we calculate the percentage of how far the ball’s center is moved from the paddle’s center compared to the half-extent of the paddle. The horizontal velocity of 
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

			/* This issue is called the sticky paddle issue. This happens, because the player paddle moves with a high velocity towards the ball with the ball’s center
			ending up inside the player paddle. Since we did not account for the case where the ball’s center is inside an AABB, the game tries to continuously react to
			all the collisions. Once it finally breaks free, it will have reversed its y velocity so much that it’s unsure whether to go up or down after breaking free */
			//ball->velocity.y = -ball->velocity.y;

			/* Fix this behavior by introducing a small hack made possible by the fact that the we can always assume we have a collision at the top of the paddle. Instead
			of reversing the y velocity, we simply always return a positive y direction so whenever it does get stuck, it will immediately break free */
			ball->velocity.y = -1.0f * abs(ball->velocity.y);
			ball->velocity = normalize(ball->velocity) * length(oldVelocity);

			ball->stuck = ball->sticky;

			SoundEngine->play2D("Audio/bleep.wav", false);
		}

	}

	for (PowerUp& powerUp : PowerUps)
	{
		if (!powerUp.destroyed)
		{
			// If the power up's y position exceeds the window's height, destroy the power up
			if (powerUp.position.y >= gameHeight) powerUp.destroyed = true;

			if (DetectCollision(*player, powerUp))
			{
				// collided with player, now activate powerup
				ActivatePowerUp(powerUp);

				powerUp.destroyed = true;
				powerUp.activated = true;

				SoundEngine->play2D("Audio/powerup.wav", false);
			}
		}
	}
}

void Game::ResetLevel()
{
	if (level == 0) levels[0].Load("levels/one.lvl", gameWidth, gameHeight / 2);
	else if (level == 1) levels[1].Load("levels/two.lvl", gameWidth, gameHeight / 2);
	else if (level == 2) levels[2].Load("levels/three.lvl", gameWidth, gameHeight / 2);
	else if (level == 3) levels[3].Load("levels/four.lvl", gameWidth, gameHeight / 2);

	lives = 3;
}

void Game::ResetPlayer()
{
	// reset player/ball stats
	player->size = PLAYER_SIZE;
	player->position = vec2(gameWidth / 2.0f - PLAYER_SIZE.x / 2.0f, gameHeight - PLAYER_SIZE.y);
	ball->Reset(player->position + vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);

	// also disable all active powerups
	Effects->Chaos = Effects->Confuse = false;
	ball->passThrough = ball->sticky = false;
	player->color = vec3(1.0f);
	ball->color = vec3(1.0f);

	Particles = nullptr;
}

bool ShouldSpawn(unsigned int chance)
{
	// Should spawn function will randomize the chance that it could spawn based on percentage (probability)
	unsigned int random = rand() % chance;

	// Don't ever exceed the amount of chances it could spawn if the number is above a 100
	if (chance >= 100)
	{
		chance = 100;
	}

	return random == 0;
}

bool isOtherPowerUpActive(vector<PowerUp>& powerUps, string type)
{
	/* Whenever one of these powerups gets deactivated, we don’t want to disable its effects yet since another powerup of 
	the same type may still be active. For this reason we use the IsOtherPowerUpActive function to check if there is still
	another powerup active of the same type. Only if this function returns false we deactivate the powerup. This way, the 
	powerup’s duration of a given type is extended to the duration of its last activated powerup */
	for (const PowerUp& powerUp : powerUps)
	{
		if (powerUp.activated)
			if (powerUp.type == type)
				return true;
	}
	return false;
}

void Game::SpawnPowerUps(GameObject& block)
{
	// 1 in 75 chance
	if (ShouldSpawn(75))
		PowerUps.push_back(PowerUp("speed", vec3(0.5f, 0.5f, 1.0f), 0.0f, block.position, 
			ResourceManager::GetTexture("powerup_speed")));

	if (ShouldSpawn(75))
		PowerUps.push_back(PowerUp("sticky", vec3(1.0f, 0.5f, 1.0f), 20.0f, block.position, 
			ResourceManager::GetTexture("powerup_sticky")));

	if (ShouldSpawn(75))
		PowerUps.push_back(PowerUp("pass-through", vec3(0.5f, 1.0f, 0.5f), 10.0f, block.position, 
			ResourceManager::GetTexture("powerup_passthrough")));

	if (ShouldSpawn(75))
		PowerUps.push_back(PowerUp("pad-size-increase", vec3(1.0f, 0.6f, 0.4), 0.0f, block.position, 
			ResourceManager::GetTexture("powerup_increase")));

	// negative powerups should spawn more often
	if (ShouldSpawn(15))
		PowerUps.push_back(PowerUp("confuse", glm::vec3(1.0f, 0.3f, 0.3f), 15.0f, block.position, 
			ResourceManager::GetTexture("powerup_confuse")));

	if (ShouldSpawn(15))
		PowerUps.push_back(PowerUp("chaos", glm::vec3(0.9f, 0.25f, 0.25f), 15.0f, block.position, 
			ResourceManager::GetTexture("powerup_chaos")));
}

void Game::UpdatePowerUps(float dt)
{
	for (PowerUp& powerUp : PowerUps)
	{
		powerUp.position += powerUp.velocity * dt;
		if (powerUp.activated)
		{
			powerUp.duration -= dt;
			if (powerUp.duration <= 0.0f)
			{
				// remove powerup from list (will later be removed)
				powerUp.activated = false;

				// deactivate effects
				if (powerUp.type == "sticky")
				{
					if (!isOtherPowerUpActive(PowerUps, "sticky"))
					{ // reset if no other PowerUp of sticky is active
						ball->sticky = false;
						player->color = vec3(1.0f);
					}
				}
				else if (powerUp.type == "pass-through")
				{
					if (!isOtherPowerUpActive(PowerUps, "pass-through"))
					{ 
						// reset if no other PowerUp of pass-through is active
						ball->passThrough = false;
						ball->color = vec3(1.0f);
					}
				}

				else if (powerUp.type == "confuse")
				{
					if (!isOtherPowerUpActive(PowerUps, "confuse"))
					{ // reset if no other PowerUp of confuse is active
						Effects->Confuse = false;
					}
				}

				else if (powerUp.type == "chaos")
				{
					if (!isOtherPowerUpActive(PowerUps, "chaos"))
					{ // reset if no other PowerUp of chaos is active
						Effects->Chaos = false;
					}
				}
			}
		}
	}

	/* The remove_if function moves all elements for which the lambda predicate is true to the end of the container object 
	and returns an iterator to the start of this removed elements range. The container’s erase function then takes this 
	iterator and the vector’s end iterator to remove all the elements between these two iterators */
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(), [](const PowerUp& powerUp)
		{
			return powerUp.destroyed && !powerUp.activated; }),
			this->PowerUps.end());
}

bool DetectCollision(GameObject& one, GameObject& two)
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
Collision DetectCollision(BallObject& one, GameObject& two)
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

