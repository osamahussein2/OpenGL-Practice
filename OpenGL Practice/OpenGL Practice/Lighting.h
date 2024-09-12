#pragma once

// Include the GLM header files (OpenGL Mathematics Library)
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

// For the first time ever, I'm learning how to use namespaces of certain C++ libraries properly
using namespace glm;

class Lighting
{
public:
	Lighting();
	~Lighting();

	vec3 SetMaterialAmbientLighting(vec3 ambientLighting_);
	vec3 SetMaterialDiffuseLighting(vec3 diffuseLighting_);
	vec3 SetMaterialSpecularLighting(vec3 specularLighting_);
	float SetShininessLighting(float shininessLighting_);
	vec3 SetIntensityAmbientLighting(vec3 ambientLightingIntensity_);
	vec3 SetIntensityDiffuseLighting(vec3 diffuseLightingIntensity_);
	vec3 SetIntensitySpecularLighting(vec3 specularLightingIntensity_);
	vec3 SetDirectionalLighting(vec3 directionalLighting_);
	float SetAttenuationConstant(float attenuationConstant_);
	float SetAttenuationLinear(float attenuationLinear_);
	float SetAttenuationQuadratic(float attenuationQuadratic_);
	float SetCutoffAngle(float cutoffAngle_);
	float SetOuterCutoffAngle(float outerCutoffAngle_);
private:
	vec3 materialAmbientLighting, ambientLightingIntensity;
	vec3 materialDiffuseLighting, diffuseLightingIntensity;
	vec3 materialSpecularLighting, specularLightingIntensity;
	float shininessOfSpecularHighlight;
	vec3 directionalLighting;
	float attenuationConstant, attenuationLinear, attenuationQuadratic;
	float cutoffAngle, outerCutoffAngle;
};