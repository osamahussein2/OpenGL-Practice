#version 330 core

out vec4 FragColor;
in vec2 TexCoords;
in vec3 worldPosition;
in vec3 Normal;

// Uniform light objects
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 cameraPosition;

// Material parameters (PBR Lighting Part 1)
//uniform vec3 albedo;
//uniform float metallic;
//uniform float roughness;
//uniform float ambientOcclusion;

// IBL
uniform samplerCube irradianceMap;

// Applies to only Specular IBL
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// PBR Lighting Part 2
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	/* Calculate the ratio between specular and diffuse reflection, or how much the surface reflects light versus how much 
	it refracts light */

	/* The Fresnel-Schlick approximation expects a F0 parameter which is known as the surface reflection at zero incidence 
	or how much the surface reflects if looking directly at the surface */

	return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness*roughness;
	float a2 = a*a;
	float NdotH = max(dot(N, H), 0.0);
	float NdotH2 = NdotH*NdotH;
	
	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	//float r = (roughness + 1.0);
	//float k = (r*r) / 8.0;

	// Specular IBL
	float r = roughness;
	float k = (r*r) / 2.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

void main()
{
	/* The albedo textures that come from artists are generally authored in sRGB space which is why we first convert them to 
	linear space before using albedo in our lighting calculations. Based on the system artists use to generate ambient 
	occlusion maps you may also have to convert these from sRGB to linear space as well. Metallic and roughness maps are 
	almost always authored in linear space */

	// PBR Lighting Part 2
	vec3 albedo = pow(texture(albedoMap, TexCoords).rgb, vec3(2.2));
    float metallic = texture(metallicMap, TexCoords).r;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ambientOcclusion = texture(aoMap, TexCoords).r;

	vec3 N = normalize(Normal);
	vec3 V = normalize(cameraPosition - worldPosition);
	vec3 R = reflect(-V, N); 

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);
	
	// reflectance equation
	vec3 Lo = vec3(0.0);

	for(int i = 0; i < 4; ++i)
	{
		// Calculate per-light radiance
		vec3 L = normalize(lightPositions[i] - worldPosition);
		vec3 H = normalize(V + L);

		float distance = length(lightPositions[i] - worldPosition);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;
		
		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
		vec3 specular = numerator / denominator;

		vec3 kS = F;
		vec3 kD = vec3(1.0) - kS;
		kD *= 1.0 - metallic;
		
		// add to outgoing radiance Lo
		float NdotL = max(dot(N, L), 0.0);
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;
	}

	// Add an (improvised) ambient term to the direct lighting result Lo and that'll be the final lit fragment color
	//vec3 ambient = vec3(0.03) * albedo * ambientOcclusion;

	// ambient lighting (we now use IBL as the ambient term)
    vec3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	
	
	/* Given the irradiance map that holds all of the scene’s indirect diffuse light, retrieving the irradiance influencing 
	the fragment is as simple as a single texture sample given the surface normal */

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    //vec3 ambient = (kD * diffuse) * ambientOcclusion;

	/* Sample both the pre-filter map and the BRDF LUT and combine them together as per the split-sum approximation to get 
	the IBL specular part */
    const float MAX_REFLECTION_LOD = 4.0;

	/* Generate the lookup texture by treating the horizontal texture coordinate (ranged between 0.0 and 1.0) of a plane as 
	the BRDF’s input n · ωi, and its vertical texture coordinate as the input roughness value. With this BRDF integration map 
	and the pre-filtered environment map we can combine both to get the result of the specular integral */

    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ambientOcclusion;

	vec3 color = ambient + Lo;
	
	/* Tone map the HDR color using the Reinhard operator, preserving the high dynamic range of a possibly highly varying 
	irradiance, after which we gamma correct the color */

	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));
	
	FragColor = vec4(color, 1.0);
}