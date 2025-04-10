#version 330 core

out vec4 fragColor;

in vec3 worldPosition;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);

/* Take the fragment’s sample direction as interpolated from the cube’s local position and then use this direction vector 
and some trigonometry magic (spherical to cartesian) to sample the equirectangular map as if it’s a cubemap itself. 
Then, store that result onto the cube-face’s fragment.  */

// Also applies to Specular IBL as well

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));

    uv *= invAtan;
    uv += 0.5;

    return uv;
}

void main()
{		
    vec2 uv = SampleSphericalMap(normalize(worldPosition));
    vec3 color = texture(equirectangularMap, uv).rgb;
    
    fragColor = vec4(color, 1.0);
}