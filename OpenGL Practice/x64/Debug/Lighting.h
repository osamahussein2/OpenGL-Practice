#pragma once

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

class Lighting
{
public:
	Lighting();
	~Lighting();

	glm::vec3 SetMaterialAmbientLighting(glm::vec3 ambientLighting_);
	glm::vec3 SetMaterialDiffuseLighting(glm::vec3 diffuseLighting_);
	glm::vec3 SetMaterialSpecularLighting(glm::vec3 specularLighting_);
	float SetShininessLighting(float shininessLighting_);
	glm::vec3 SetIntensityAmbientLighting(glm::vec3 ambientLightingIntensity_);
	glm::vec3 SetIntensityDiffuseLighting(glm::vec3 diffuseLightingIntensity_);
	glm::vec3 SetIntensitySpecularLighting(glm::vec3 specularLightingIntensity_);
private:
	glm::vec3 materialAmbientLighting, ambientLightingIntensity;
	glm::vec3 materialDiffuseLighting, diffuseLightingIntensity;
	glm::vec3 materialSpecularLighting, specularLightingIntensity;
	float shininessOfSpecularHighlight;
};