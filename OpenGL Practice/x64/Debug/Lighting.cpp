#include "Lighting.h"

Lighting::Lighting()
{
	// Initialize all the light sources to 0
	materialAmbientLighting = vec3(0.0f, 0.0f, 0.0f);
	materialDiffuseLighting = vec3(0.0f, 0.0f, 0.0f);
	materialSpecularLighting = vec3(0.0f, 0.0f, 0.0f);

	shininessOfSpecularHighlight = 0.0f;

	ambientLightingIntensity = vec3(0.0f, 0.0f, 0.0f);
	diffuseLightingIntensity = vec3(0.0f, 0.0f, 0.0f);
	specularLightingIntensity = vec3(0.0f, 0.0f, 0.0f);

	directionalLighting = vec3(0.0f, 0.0f, 0.0f);

	attenuationConstant = 0.0f;
	attenuationLinear = 0.0f;
	attenuationQuadratic = 0.0f;

	cutoffAngle = static_cast<int>(0); // I'm starting to learn about data conversions correctly here
	outerCutoffAngle = static_cast<int>(0);
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

	directionalLighting = vec3(0.0f, 0.0f, 0.0f);

	attenuationConstant = 0.0f;
	attenuationLinear = 0.0f;
	attenuationQuadratic = 0.0f;

	cutoffAngle = static_cast<int>(0);
	outerCutoffAngle = static_cast<int>(0);
}

vec3 Lighting::SetMaterialAmbientLighting(vec3 materialAmbientLighting_)
{
	// Let's set the ambient light source to the numbers we want by passing in a vec3 as an argument
	materialAmbientLighting = materialAmbientLighting_;

	return materialAmbientLighting;
}

vec3 Lighting::SetMaterialDiffuseLighting(vec3 materialDiffuseLighting_)
{
	// Let's set the diffuse light source to the numbers we want by passing in a vec3 as an argument
	materialDiffuseLighting = materialDiffuseLighting_;

	return materialDiffuseLighting;
}

vec3 Lighting::SetMaterialSpecularLighting(vec3 materialSpecularLighting_)
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

vec3 Lighting::SetIntensityAmbientLighting(vec3 ambientLightingIntensity_)
{
	ambientLightingIntensity = ambientLightingIntensity_;

	return ambientLightingIntensity;
}

vec3 Lighting::SetIntensityDiffuseLighting(vec3 diffuseLightingIntensity_)
{
	diffuseLightingIntensity = diffuseLightingIntensity_;

	return diffuseLightingIntensity;
}

vec3 Lighting::SetIntensitySpecularLighting(vec3 specularLightingIntensity_)
{
	specularLightingIntensity = specularLightingIntensity_;

	return specularLightingIntensity;
}

vec3 Lighting::SetDirectionalLighting(vec3 directionalLighting_)
{
	directionalLighting = directionalLighting_;

	return directionalLighting;
}

float Lighting::SetAttenuationConstant(float attenuationConstant_)
{
	attenuationConstant = attenuationConstant_;

	return attenuationConstant;
}

float Lighting::SetAttenuationLinear(float attenuationLinear_)
{
	attenuationLinear = attenuationLinear_;

	return attenuationLinear;
}

float Lighting::SetAttenuationQuadratic(float attenuationQuadratic_)
{
	attenuationQuadratic = attenuationQuadratic_;

	return attenuationQuadratic;
}

float Lighting::SetCutoffAngle(float cutoffAngle_)
{
	cutoffAngle = cutoffAngle_;

	return cutoffAngle;
}

float Lighting::SetOuterCutoffAngle(float outerCutoffAngle_)
{
	outerCutoffAngle = outerCutoffAngle_;

	return outerCutoffAngle;
}