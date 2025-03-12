#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

//in vec3 Normal;uniform sampler2D tex;void main()
{
	//FragColor.rgb = Normal;
	//FragColor.a = 1.0f;

	FragColor = texture(tex, TexCoords);
}