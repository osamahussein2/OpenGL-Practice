#include "ParticleGenerator.h"

ParticleGenerator::ParticleGenerator(ShaderProgram shader_, Texture2D texture_, unsigned int amount_) : shader(shader_), texture(texture_), amount(amount_)
{
	this->InitParticleGenerator();
}

void ParticleGenerator::UpdateParticles(float dt, GameObject& object, unsigned int newParticles, vec2 offset)
{
    /*  As particles die over time we want to spawn number of particles each frame, but since we don’t want to infinitely keep spawning new particles
    (we’ll quickly run out of memory this way) we only spawn up to a max of number of particles. If were to push all new particles to the end of the
    list we’ll quickly get a list filled with thousands of particles */

    // add new particles 
    for (unsigned int i = 0; i < newParticles; ++i)
    {
        int unusedParticle = this->FirstUnusedParticle();
        this->RespawnParticle(this->particles[unusedParticle], object, offset);
    }

    // update all particles
    for (unsigned int i = 0; i < this->amount; ++i)
    {
        Particle& p = this->particles[i];
        p.Life -= dt; // reduce life

        if (p.Life > 0.0f)
        {	// particle is alive, thus update
            p.Position -= p.Velocity * dt;
            p.Color.a -= dt * 2.5f;
        }
    }
}

void ParticleGenerator::DrawParticles()
{
    /* When rendering the particles, instead of the default destination blend mode of GL_ONE_MINUS_SRC_ALPHA, we use the GL_ONE (additive)
    blend mode that gives the particles a very neat glow effect when stacked onto each other */

    // use additive blending to give it a 'glow' effect
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glUseProgram(shader.shaderProgram);
    for (Particle& particle : this->particles)
    {
        if (particle.Life > 0.0f)
        {
            glUniform2f(glGetUniformLocation(this->shader.shaderProgram, "offset"), particle.Position.x, particle.Position.y);
            glUniform4f(glGetUniformLocation(this->shader.shaderProgram, "color"), particle.Color.x, particle.Color.y, particle.Color.z, particle.Color.w);

            this->texture.Bind();


            glBindVertexArray(this->VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    }
    // don't forget to reset to default blending mode
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleGenerator::InitParticleGenerator()
{
    // set up mesh and attribute properties
    unsigned int VBO;

    array<float, 24> particle_quad = 
    {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &this->VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(this->VAO);

    // fill mesh buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), &particle_quad, GL_STATIC_DRAW);

    // set mesh attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    // create this->amount default particle instances
    for (unsigned int i = 0; i < this->amount; ++i) this->particles.push_back(Particle());
}

unsigned int lastUsedParticle = 0;
unsigned int ParticleGenerator::FirstUnusedParticle()
{
    // search from last used particle, often returns almost instantly
    for (unsigned int i = lastUsedParticle; i < this->amount; ++i) 
    {
        if (particles[i].Life <= 0.0f)
        {
            lastUsedParticle = i;
            return i;
        }
    }

    // otherwise, do a linear search
    for (unsigned int i = 0; i < lastUsedParticle; ++i) 
    {
        if (particles[i].Life <= 0.0f) 
        {
            lastUsedParticle = i;
            return i;
        }
    }

    // override first particle if all others are alive
    lastUsedParticle = 0;
	return 0;
}

void ParticleGenerator::RespawnParticle(Particle& particle, GameObject& object, vec2 offset)
{
    /* Resets the particle’s life to 1.0f, randomly gives it a brightness (via the color vector) starting from 0.5,
    and assigns a (slightly random) position and velocity based on the game object’s data */
    float random = ((rand() % 100) - 50) / 10.0f;
    float rColor = 0.5f + ((rand() % 100) / 100.0f);

    particle.Position = object.position + random + offset;
    particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
    particle.Life = 1.0f;
    particle.Velocity = object.velocity * 0.1f;

}
