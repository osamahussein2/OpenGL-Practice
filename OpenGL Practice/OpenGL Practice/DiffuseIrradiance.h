#include <array>

#include <glad/glad.h>
#include <glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "stb_image.h"

#include "ShaderProgram.h"
#include "Model.h"

class DiffuseIrradiance
{
public:
	~DiffuseIrradiance();

	static DiffuseIrradiance* Instance();

	void InitializeDiffuseIrradiance();
	void RenderDiffuseIrradiance();

private:
	DiffuseIrradiance();

	void RenderSphere();
	void RenderCube();

	static DiffuseIrradiance* objInstance;

	array<ShaderProgram*, 4> irradianceShaders;

	array<vec3, 4> lightPositions, lightColors;

    int nrRows;
    int nrColumns;
    float spacing;

    unsigned int captureFBO, captureRBO, hdrTexture, envCubemap, irradianceMap;

	int width, height, nrComponents;
	float* data;

	mat4 captureProjection, projection, view, model;
	array<mat4, 6> captureViews;

	unsigned int sphereVAO;
	unsigned int indexCount;

	unsigned int cubeVAO;
	unsigned int cubeVBO;
};