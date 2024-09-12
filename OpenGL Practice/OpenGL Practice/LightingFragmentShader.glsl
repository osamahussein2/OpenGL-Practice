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

void main()
{
	// Attenuation equation
	distanceToLightSource = length(light.positionalLight - FragPosition);

	// I can do either (distanceToLightSource * distanceToLightSource) or pow(distanceToLightSource, 2)
	attenuationValue = 1.0 / (attenuation.constant + attenuation.linear * distanceToLightSource + 
	attenuation.quadratic * (pow(distanceToLightSource, 2)));

	// We know the light direction is equal to the light's position minus the fragment's position
	lightDirection = normalize(light.positionalLight - FragPosition);

	// Check if the light is inside the spotlight cone
	thetaAngle = dot(lightDirection, normalize(-light.directionalLight));

	// Epsilon is the difference between the inner cone (in angle) and the outer cone (also, in angle)
	epsilon = light.cutoffAngle - light.outerCutoffAngle;

	// The clamp function ensures that the number can't go below the minimum value and can't go above the maximum value
	spotlightIntensity = clamp((thetaAngle - light.cutoffAngle) / epsilon, 0.0, 1.0);

	// Ambient lighting
	ambientLight = light.ambientLight * vec3(texture(material.diffuseMap, texCoords));

	// Diffuse lighting

	// Make sure these newly defined vec3s are unit vectors, hence why they should be normalized
	normalizeNormals = normalize(Normal);

	// We have to negate the directional light vector first (switching its direction)
	//lightDirection = normalize(-light.directionalLight);

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

	// The ambient lighting will be unaffected by the intensity of the spot light because we want to have some light
	diffuseLight *= spotlightIntensity;
	specularLight *= spotlightIntensity;

	ambientLight *= attenuationValue;
	diffuseLight *= attenuationValue;
	specularLight *= attenuationValue;

	resultingLight = ambientLight + diffuseLight + specularLight;

	fragColor = vec4(resultingLight, 1.0);
}