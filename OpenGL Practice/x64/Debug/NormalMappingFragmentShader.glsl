#version 330 core

out vec4 fragColor;

in SHADER_OUT
{
	vec3 fragPos;
	vec2 textureCoords;
	vec3 tangentLightPosition;
	vec3 tangentViewPosition;
	vec3 tangentFragPosition;

} normalMapping_fs;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform vec3 lightPosition;
uniform vec3 viewPosition;

void main()
{
	/* Reverse the process of mapping normals to RGB colors by remapping the sampled normal color from 0 to 1 back to 
	-1 to 1 and then use the sampled normal vectors for the upcoming lighting calculations. */

	// With TBN matrix, update the normal mapping code to include the tangent-to-world space transformation

	// Obtain normal from normal map in range from 0 to 1
    vec3 normal = texture(normalMap, normalMapping_fs.textureCoords).rgb;

    // Transform normal vector to range from -1 to 1 (set the normal to be in tangent space here)
    normal = normalize(normal * 2.0 - 1.0);

	// Get the diffuse color
	vec3 color = texture(diffuseMap, normalMapping_fs.textureCoords).rgb;

	// Ambient
	vec3 ambient = 0.1 * color;

	// Diffuse
	vec3 lightDirection = normalize(normalMapping_fs.tangentLightPosition - normalMapping_fs.tangentFragPosition);
	float diffValue = max(dot(lightDirection, normal), 0.0);

	vec3 diffuse = diffValue * color;

	// Specular
	vec3 viewDirection = normalize(normalMapping_fs.tangentViewPosition - normalMapping_fs.tangentFragPosition);
	vec3 reflectDirection = reflect(-lightDirection, normal);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);

	float specValue = pow(max(dot(normal, halfwayDirection), 0.0), 32.0);

	vec3 specular = vec3(0.2) * specValue;

	fragColor = vec4(ambient + diffuse + specular, 1.0);
}