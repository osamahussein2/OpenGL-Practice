#pragma once

#ifndef PARTICLE_GENERATOR_H
#define PARTICLE_GENERATOR_H

#include <glm.hpp>
#include <glad/glad.h>

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "GameObject.h"

struct Particle
{
	vec2 Position, Velocity;
	vec4 Color;

	float Life;

	Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};

class ParticleGenerator
{
public:
    ParticleGenerator(ShaderProgram shader_, Texture2D texture_, unsigned int amount_);

    // update all particles
    void UpdateParticles(float dt, GameObject& object, unsigned int newParticles, vec2 offset = vec2(0.0f, 0.0f));
    
    // render all particles
    void DrawParticles();

private:
    vector<Particle> particles;
    unsigned int amount;

    ShaderProgram shader;
    Texture2D texture;
    unsigned int VAO;

    // initializes buffer and vertex attributes
    void InitParticleGenerator();

    // returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
    unsigned int FirstUnusedParticle();

    // respawns particle
    void RespawnParticle(Particle& particle, GameObject& object, vec2 offset = vec2(0.0f, 0.0f));
};

#endif