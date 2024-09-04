#include "Lighting.h"

Lighting::Lighting()
{
	// Initialize all the light sources to 0
	materialAmbientLighting = glm::vec3(0.0f, 0.0f, 0.0f);
	materialDiffuseLighting = glm::vec3(0.0f, 0.0f, 0.0f);
	materialSpecularLighting = glm::vec3(0.0f, 0.0f, 0.0f);

	shininessOfSpecularHighlight = 0.0f;

	ambientLightingIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
	diffuseLightingIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
	specularLightingIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
}

Lighting::~Lighting()
{
	// Set all the light sources to 0 when we call the lighting deconstructor
	materialAmbientLighting = glm::vec3(0.0f, 0.0f, 0.0f);
	materialDiffuseLighting = glm::vec3(0.0f, 0.0f, 0.0f);
	materialDiffuseLighting = glm::vec3(0.0f, 0.0f, 0.0f);

	shininessOfSpecularHighlight = 0.0f;

	ambientLightingIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
	diffuseLightingIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
	specularLightingIntensity = glm::vec3(0.0f, 0.0f, 0.0f);
}

glm::vec3 Lighting::SetMaterialAmbientLighting(glm::vec3 materialAmbientLighting_)
{
	// Let's set the ambient light source to the numbers we want by passing in a vec3 as an argument
	materialAmbientLighting = materialAmbientLighting_;

	return materialAmbientLighting;
}

glm::vec3 Lighting::SetMaterialDiffuseLighting(glm::vec3 materialDiffuseLighting_)
{
	// Let's set the diffuse light source to the numbers we want by passing in a vec3 as an argument
	materialDiffuseLighting = materialDiffuseLighting_;

	return materialDiffuseLighting;
}

glm::vec3 Lighting::SetMaterialSpecularLighting(glm::vec3 materialSpecularLighting_)
{
	// Let's set the specular light source to the numbers we want by passing in a vec3 as an argument
	materialSpecularLighting = materialSpecularLighting_;

	return materialSpecularLighting;
}

float Lighting::SetShininessLighting(float shininessLighting_)
{
	// Let's set the shininess light source to the value we want by passing in a float as an argument
	shininessOfSpecularHighlight = shininessLighting_;

	return shininessOfSpecularHighlight;
}

glm::vec3 Lighting::SetIntensityAmbientLighting(glm::vec3 ambientLightingIntensity_)
{
	ambientLightingIntensity = ambientLightingIntensity_;

	return ambientLightingIntensity;
}

glm::vec3 Lighting::SetIntensityDiffuseLighting(glm::vec3 diffuseLightingIntensity_)
{
	diffuseLightingIntensity = diffuseLightingIntensity_;

	return diffuseLightingIntensity;
}

glm::vec3 Lighting::SetIntensitySpecularLighting(glm::vec3 specularLightingIntensity_)
{
	specularLightingIntensity = specularLightingIntensity_;

	return specularLightingIntensity;
}