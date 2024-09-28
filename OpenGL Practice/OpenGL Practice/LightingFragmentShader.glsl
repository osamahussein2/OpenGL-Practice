#version 330 core

float ambientLightBrightness;
vec3 ambientLight;
vec3 resultingLight;

vec3 normalizeNormals;
vec3 lightDirection;
vec3 diffuseLight;

float specularBrightness;
vec3 viewDirection;
vec3 reflectionDirection;
float specularValue;
vec3 specularLight;

float distanceToLightSource;
float attenuationValue;

float thetaAngle;
float epsilon;
float spotlightIntensity;

struct Material
{
	// The ambient material vector defines what color the surface reflects under ambient lighting (in most cases, it's the same color as the surface color)
	//vec3 ambientLight;

	// The diffuse material vector defines the color of the surface under diffuse lighting
	//vec3 diffuseLight;

	// The specular material vector defines the color of the specular highlight on the surface
	//vec3 specularLight;

	// The diffuse map sampler2D defines texture image of the diffuse material
	sampler2D diffuseMap;

	// The specular map sampler2D defines texture image of the specular material
	sampler2D specularMap;

	/* Create an emission map texture that stores emission values per fragment. Emission values are colors an object
	may emit as if it contains a light source itself.  */
	sampler2D emissionMap;

	// The shininess value defines how much the scatter/radius value of the specular highlight will equal to
	float shininess;
};

struct Light
{
	// The position light vector will determine where the light's brightness be located
	vec3 positionalLight;

	// Instead of calculating the light's direction vector, we can just use the directional light vector directly here
	vec3 directionalLight;

	float cutoffAngle;
	float outerCutoffAngle; // In other words, this is the angle of the spotlight's outer cone 

	// The ambient light vector will determine the light's ambient intensity
	vec3 ambientLight;

	// The diffuse light vector will determine the light's diffuse intensity
	vec3 diffuseLight;

	// The specular light vector will determine the light's specular intensity
	vec3 specularLight;
};

// Attenuation means reducing the light intensity over the distance that the light ray is travelling
struct Attenuation
{
	/* The constant value is usually equal to 1 so that the denominator never gets smaller than 1. If it does get
	smaller than 1, then the light intensity will be boosted even if the distance is far away. */
	float constant;

	/* The linear value is used to multiply with the distance value that will help reduce light intensity as
	the distance is farther away in a linear way. */
	float linear;

	/* The quadratic value is used to multiply with the quadrant of the distance and sets a quadratic decrease of
	light intensity. */
	float quadratic;
};

struct DirectionalLight
{
	vec3 lightDirection;

	vec3 ambientLight;
	vec3 diffuseLight;
	vec3 specularLight;
};

struct PointLight
{
	vec3 lightPosition;

	float constant;
	float linear;
	float quadratic;

	vec3 ambientLight;
	vec3 diffuseLight;
	vec3 specularLight;
};

struct SpotLight
{
	vec3 lightPosition;
	vec3 directionalLight;

	vec3 ambientLight;
	vec3 diffuseLight;
	vec3 specularLight;
};

out vec4 fragColor;

in vec2 texCoords;
in vec3 FragPosition;
in vec3 Normal;

uniform vec3 objectColor;
uniform vec3 lightColor;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

uniform Material material;
uniform Light light;
uniform Attenuation attenuation;

uniform DirectionalLight directionalLight;

#ifndef NUMBER_OF_POINT_LIGHTS
#define NUMBER_OF_POINT_LIGHTS 4

uniform PointLight pointLights[NUMBER_OF_POINT_LIGHTS];

uniform SpotLight spotLight;

uniform sampler2D TextureDiffuse1;
uniform sampler2D TextureDiffuse2;
uniform sampler2D TextureDiffuse3;

uniform sampler2D TextureSpecular1;
uniform sampler2D TextureSpecular2;

vec3 CalculateDirectionalLighting(DirectionalLight directionalLight, vec3 normal, vec3 viewDirection)
{
	lightDirection = normalize(-directionalLight.lightDirection);

	/* We have to negate the light direction by adding a minus sign beside it because the light direction 
	vector is currently looking the other way around, from the fragment position towards the light source. */

	// The reflect function expects a direction vector and a normal vector as its 2 arguments
	reflectionDirection = reflect(-lightDirection, normal);

	ambientLight = directionalLight.ambientLight * vec3(texture(material.diffuseMap, texCoords));

	diffuseLight = directionalLight.diffuseLight * max(dot(normal, lightDirection), 0.0) * 
	vec3(texture(material.diffuseMap, texCoords));

	specularLight = directionalLight.specularLight * pow(max(dot(viewDirection, reflectionDirection), 0.0),
	material.shininess) * vec3(texture(material.specularMap, texCoords));

	return (ambientLight + diffuseLight + specularLight);
}

vec3 CalculatePointLighting(PointLight pointLight, vec3 normal,  vec3 fragPosition, vec3 viewDirection)
{
	lightDirection = normalize(pointLight.lightPosition - fragPosition);

	/* We have to negate the light direction by adding a minus sign beside it because the light direction 
	vector is currently looking the other way around, from the fragment position towards the light source. */

	// The reflect function expects a direction vector and a normal vector as its 2 arguments
	reflectionDirection = reflect(-lightDirection, normal);

	// Attenuation equation
	distanceToLightSource = length(pointLight.lightPosition - FragPosition);

	// I can do either (distanceToLightSource * distanceToLightSource) or pow(distanceToLightSource, 2)
	attenuationValue = 1.0 / (pointLight.constant + pointLight.linear * distanceToLightSource + 
	pointLight.quadratic * (pow(distanceToLightSource, 2)));

	ambientLight = pointLight.ambientLight * vec3(texture(material.diffuseMap, texCoords));

	diffuseLight = pointLight.diffuseLight * max(dot(normal, lightDirection), 0.0) * 
	vec3(texture(material.diffuseMap, texCoords));

	/* We first calculate the dot produce between the view direction and the reflection direction (but also,
	make sure the the value isn't negative) and then raise it to the power of 32. */

	/* The power of 32 value is the shininess value of the highlight. The higher the shininess value of an object,
	the more it properly reflects the light instead of scattering it all around, meaning that the highlight becomes
	smaller. */

	specularLight = pointLight.specularLight * pow(max(dot(viewDirection, reflectionDirection), 0.0),
	material.shininess) * vec3(texture(material.specularMap, texCoords));

	ambientLight *= attenuationValue;
	diffuseLight *= attenuationValue;
	specularLight *= attenuationValue;

	return (ambientLight + diffuseLight + specularLight);
}

vec3 CalculateSpotLight(SpotLight spotLight, vec3 normal,  vec3 fragPosition, vec3 viewDirection)
{
	// We know the light direction is equal to the light's position minus the fragment's position
	lightDirection = normalize(spotLight.lightPosition - fragPosition);

	// Check if the light is inside the spotlight cone
	thetaAngle = dot(lightDirection, normalize(-spotLight.directionalLight));

	// Epsilon is the difference between the inner cone (in angle) and the outer cone (also, in angle)
	epsilon = light.cutoffAngle - light.outerCutoffAngle;

	// Attenuation equation
	distanceToLightSource = length(spotLight.lightPosition - FragPosition);

	// I can do either (distanceToLightSource * distanceToLightSource) or pow(distanceToLightSource, 2)
	attenuationValue = 1.0 / (attenuation.constant + attenuation.linear * distanceToLightSource + 
	attenuation.quadratic * (pow(distanceToLightSource, 2)));

	// The clamp function ensures that the number can't go below the minimum value and can't go above the maximum value
	spotlightIntensity = clamp((thetaAngle - light.cutoffAngle) / epsilon, 0.0, 1.0);

	ambientLight = spotLight.ambientLight * vec3(texture(material.diffuseMap, texCoords));

	diffuseLight = spotLight.diffuseLight * max(dot(normal, lightDirection), 0.0) * 
	vec3(texture(material.diffuseMap, texCoords));

	specularLight = spotLight.specularLight * pow(max(dot(viewDirection, reflectionDirection), 0.0),
	material.shininess) * vec3(texture(material.specularMap, texCoords));

	ambientLight *= attenuationValue * spotlightIntensity;
	diffuseLight *= attenuationValue * spotlightIntensity;
	specularLight *= attenuationValue * spotlightIntensity;

	return (diffuseLight + specularLight);
}

void main()
{
	normalizeNormals = normalize(Normal);

	viewDirection = normalize(viewPosition - FragPosition);

	// Add directional lighting to the scene
	resultingLight = CalculateDirectionalLighting(directionalLight, normalizeNormals, viewDirection);

	// Add point lights to the scene (for/while loop is mandatory here for iterating through the array elements)
	for(int i = 0; i < NUMBER_OF_POINT_LIGHTS; i++)
	{
		resultingLight += CalculatePointLighting(pointLights[i], normalizeNormals, FragPosition, viewDirection);
	}

	// Add spotlights to the scene
	resultingLight += CalculateSpotLight(spotLight, normalizeNormals, FragPosition, viewDirection);

	fragColor = vec4(resultingLight, 1.0);
}

#endif