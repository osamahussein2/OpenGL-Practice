#version 330 core
out vec4 fragColor;

// Was called AdvancedGLSLFragmentShader.glsl but now it's going to be RedFragmentShader

//uniform bool isItRed, isItGreen, isItBlue, isItYellow;

//uniform sampler2D frontTexture;
//uniform sampler2D backTexture;

/* Then we also need to declare an input interface block in the fragment shader. The block name (VS_OUT) should be the 
same in the fragment shader, but the instance name (vs_out as used in the vertex shader) can be anything we like - 
avoiding confusing names like vs_out for a fragment struct containing input values */

/* As long as both interface block names are equal, their corresponding input and output is matched together. This is 
another useful feature that helps organize code and is useful when crossing between certain shader stages like the geometry 
shader */ 

/*in VS_OUT
{
	vec2 textureCoordinate;
} fs_in;*/

//uniform sampler2D textureImage;


/* By writing to gl_FragDepth you should take this performance penalty into consideration. From OpenGL 4.2 however, we 
can still sort of mediate between both sides by redeclaring the gl_FragDepth variable at the top of the fragment shader 
with a depth condition */

//layout (depth_<condition>) out float gl_FragDepth;

void main()
{
	// Make the fragment red
	fragColor = vec4(1.0, 0.0, 0.0, 1.0);

	//fragColor = texture(textureImage, fs_in.textureCoordinate);
	
	/* Fragment shader has 2 input variables: gl_FragCoord and gl_FrontFacing
	

	gl_FragCoord - gl_FragCoord z component vector is equal to the depth value of that particular fragment. 
	The gl_FragCoord’s x and y component are the window or screen-space coordinates of the fragment, originating from 
	the bottom-left of the window.

	*/

	/*if (gl_FragCoord.x < 400)
	{
		fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}

	else
	{
		fragColor = vec4(0.0, 1.0, 0.0, 1.0);
	}*/

	/* gl_FrontFacing tells us if the current fragment is part of a front-facing or a back-facing face.
	Output different colors for all back faces. The gl_FrontFacing variable is a boolean that is true if the fragment
	is part of a front face and false otherwise. This will create a cube with a different texture inside the fragment than
	the outside of the fragment. */

	// If face culling is enabled, there won't be any faces inside the container and using gl_FrontFacing would be pointless

	/*if (gl_FrontFacing) // If gl_FrontFacing returns true
	{
		fragColor = texture(frontTexture, textureCoords);
	}

	else
	{
		fragColor = texture(backTexture, textureCoords);
	}*/

	/* gl_FragCoord is an input variable that allows us to read screen-space coordinates and get the depth value of the 
	current fragment, but it is a read-only variable. We can’t influence the screen-space coordinates of the fragment, 
	but it is possible to set the depth value of the fragment */

	/* gl_FragDepth is an output variable that we can use to manually set the depth value of the fragment within the 
	shader. To set the depth value in the shader we write any value between 0.0 and 1.0 to the output variable. If the 
	shader does not write anything to gl_FragDepth, the variable will automatically take its value from gl_FragCoord.z */

	// The fragment will be set to a depth value of 0
	//gl_FragDepth = 0.0;

	/* Setting the depth value manually has a major disadvantage because OpenGL disables early depth testing as soon as
	we write to gl_FragDepth in the fragment shader. It is disabled, because OpenGL cannot know what depth value the 
	fragment will have before we run the fragment shader, since the fragment shader may actually change this value */

	/* Depth condition can take the values: any, greater, less and unchanged.

	any - The default value. Early depth testing is disabled and you lose most performance.

	greater - You can only make the depth value larger compared to gl_FragCoord.z.

	less - You can only make the depth value smaller compared to gl_FragCoord.z.

	unchanged - If you write to gl_FragDepth, you will write exactly gl_FragCoord.z. */

	/* By specifying greater or less as the depth condition, OpenGL can make the assumption that you’ll only write depth 
	values larger or smaller than the fragment’s depth value. This way OpenGL is still able to do early depth testing when 
	the depth buffer value is part of the other direction of gl_FragCoord.z */
	 
	 /* An example of where we increase the depth value in the fragment shader, but still want to preserve some of the 
	 early depth testing */

	 // fragColor = vec4(1.0);
	 // gl_FragDepth = glFragCoord.z + 0.1; // This is only available for OpenGL version 4.2 or higher
}