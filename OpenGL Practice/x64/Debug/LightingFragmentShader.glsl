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

	// The shininess value defines how much the scatter/radius value of the specular highlight will equal to
	float shininess;
};

struct Light
{
	// The position light vector will determine where the light's brightness be located
	vec3 positionalLight;

	// The ambient light vector will determine the light's ambient intensity
	vec3 ambientLight;

	// The diffuse light vector will determine the light's diffuse intensity
	vec3 diffuseLight;

	// The specular light vector will determine the light's specular intensity
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

void main()
{
	// Ambient lighting
	ambientLight = light.ambientLight * vec3(texture(material.diffuseMap, texCoords));

	// Diffuse lighting

	// Make sure these newly defined vec3s are unit vectors, hence why they should be normalized
	normalizeNormals = normalize(Normal);

	// We know the light direction is equal to the light's position minus the fragment's position
	lightDirection = normalize(light.positionalLight - FragPosition);

	diffuseLight = light.diffuseLight * (max(dot(normalizeNormals, lightDirection), 0.0) * 
	vec3(texture(material.diffuseMap, texCoords)));

	viewDirection = normalize(viewPosition - FragPosition);

	/* We have to negate the light direction by adding a minus sign beside it because the light direction 
	vector is currently looking the other way around, from the fragment position towards the light source. */

	// The reflect function expects a direction vector and a normal vector as its 2 arguments
	reflectionDirection = reflect(-lightDirection, normalizeNormals);

	/* We first calculate the dot produce between the view direction and the reflection direction (but also,
	make sure the the value isn't negative) and then raise it to the power of 32. */

	/* The power of 32 value is the shininess value of the highlight. The higher the shininess value of an object,
	the more it properly reflects the light instead of scattering it all around, meaning that the highlight becomes
	smaller. */

	specularValue = pow(max(dot(viewDirection, reflectionDirection), 0.0), material.shininess);
	specularLight = light.specularLight * specularValue * vec3(texture(material.specularMap, texCoords));

	resultingLight = ambientLight + diffuseLight + specularLight;

	fragColor = vec4(resultingLight, 1.0);
}