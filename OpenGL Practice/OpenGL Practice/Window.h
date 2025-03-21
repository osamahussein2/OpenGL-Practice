#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <glfw3.h>
#include <iostream>
#include "VertexShaderLoader.h"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Color.h"
#include "Lighting.h"
#include "Model.h"
#include "Blending.h"
#include "FaceCulling.h"
#include "FrameBuffer.h"
#include "Skybox.h"
#include "AdvancedData.h"
#include "GeometryShader.h"
#include "Instancing.h"
#include "AntiAliasing.h"
#include "AdvancedLighting.h"
#include "GammaCorrection.h"
#include "ShadowMapping.h"
#include "PointShadows.h"
#include "NormalMapping.h"
#include "ParallaxMapping.h"
#include "HDR.h"
#include "Bloom.h"
#include "DeferredShading.h"
#include "SSAO.h"
#include "PBRLighting.h"
#include "DiffuseIrradiance.h"
#include "SpecularIBL.h"
#include "Debugging.h"
#include "TextRendering.h"
#include "Game.h"
#include "ResourceManager.h"

class Blending;

class Window
{
public:
	// Initialize the window's variables to be NULL first before we do anything with it
	Window();

	// Initialize OpenGL window here
	void InitializeOpenGLwindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);

	void WindowStillRunning();

	// Get the keyboard input whenever we want to close the window
	void ProcessInput(GLFWwindow* window);

	//void IncludeFrameBufferMethods();
	//void UseSkybox();
	//void AdvancedGLSL();
	//void UseGeometryShader();
	//void UseInstancingClass();
	//void UseAntiAliasing();
	//void RenderAdvancedLighting();
	//void InitializeGammaCorrection();
	//void InitializeShadowMapping();
	//void CallPointShadows();
	//void CallDiffuseIrradianceViewport();
	//void CallSpecularIBLViewport();

private:
	// Make this function static to use it inside the glfwSetFramebufferSizeCallback function

	/* If I don't make this static, it'll give an error that it cannot convert this function for the Window class
	to a parameter type of GLFWframebuffersizefun */
	static void FrameBufferSizeCallback(GLFWwindow* window, int width, int height);

	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);

	// positionX and positionY represent as current mouse positions, along the x and y axes respectively
	static void MouseCallback(GLFWwindow* window, double positionX, double positionY);

	static void MouseScrollCallback(GLFWwindow* window, double offsetX, double offsetY);

	std::array<VertexShaderLoader*, 12> vertexShaderLoader;
	std::array<FragmentShaderLoader*, 15> fragmentShaderLoader;
	//ShaderProgram* shaderProgram;
	GLFWwindow* openGLwindow;
	Camera* camera;

	Color* color;
	Lighting* lighting;

	float cameraMoveSpeed;

	float deltaTime, lastFrame, currentFrame;

	static float lastPositionX, lastPositionY;

	CameraMovement cameraMovement;

	Model* model;

	Blending* blendTexture;
	FaceCulling* faceCulling;
	FrameBuffer* framebuffer;
	Skybox* skybox;
	AdvancedData* advancedData;
	GeometryShader* geometryShader;

	ShaderProgram* geometryShaderProgram;

	Instancing* instancing;

	AntiAliasing* antiAliasing;

	Game breakout;
};

#endif