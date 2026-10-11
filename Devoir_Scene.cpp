//-----------------------------------------------------------------------------
// DEVOIR - Introduction to Modern OpenGL
// Scene de nuit : place avec lampadaires, tonneaux, caisses, robot, lapin, quilles.
//
//  - Plusieurs modeles 3D (OBJ) affiches plusieurs fois
//  - Trois types de lumieres : directionnelle (soleil/lune), ponctuelles (3 lampadaires
//    colores qui pulsent), spot (lampe torche attachee a la camera)
//  - Animations : robot et lapin qui tournent, lumieres qui pulsent, intensite du soleil
//  - Camera animee automatiquement (visite virtuelle) autour de la scene
//
// Touches :
//   C      : bascule camera automatique <-> camera libre (W A S D Z X + souris)
//   F      : allume / eteint la lampe torche (spot)
//   F1     : mode fil de fer
//   Echap  : quitter
//-----------------------------------------------------------------------------
#include <iostream>
#include <sstream>
#include <string>
#include <cmath>
#define GLEW_STATIC
#include "GL/glew.h"	// Important - this header must come before glfw3 header
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Camera.h"
#include "Mesh.h"


// Global Variables
const char* APP_TITLE = "Devoir OpenGL - Visite de la scene";
int gWindowWidth = 1024;
int gWindowHeight = 768;
GLFWwindow* gWindow = NULL;
bool gWireframe = false;
bool gFlashlightOn = true;
bool gAutoCamera = true;			// true = visite automatique, false = camera libre
glm::vec4 gClearColor(0.02f, 0.02f, 0.05f, 1.0f);	// ciel de nuit

FPSCamera fpsCamera(glm::vec3(0.0f, 3.5f, 10.0f));	// camera libre
const double ZOOM_SENSITIVITY = -3.0;
const float MOVE_SPEED = 5.0; // units per second
const float MOUSE_SENSITIVITY = 0.1f;

// Un objet de la scene : quel mesh, quelle texture, ou, quelle taille, vitesse de rotation
struct SceneObject
{
	int meshId;
	int texId;
	glm::vec3 pos;
	glm::vec3 scale;
	float spin;			// degres par seconde autour de l'axe Y (0 = fixe)
};

// Identifiants des modeles charges
enum { M_BARREL = 0, M_CRATE, M_ROBOT, M_FLOOR, M_PIN, M_BUNNY, M_LAMP, M_TREE, NUM_MODELS };


// Function prototypes
void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode);
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height);
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY);
void update(double elapsedTime);
void showFPS(GLFWwindow* window);
bool initOpenGL();

//-----------------------------------------------------------------------------
// Main Application Entry Point
//-----------------------------------------------------------------------------
int main()
{
	if (!initOpenGL())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return -1;
	}

	ShaderProgram lightingShader;
	lightingShader.loadShaders("shaders/lighting_dir_point_spot.vert", "shaders/lighting_dir_point_spot.frag");

	// Chargement des modeles et des textures (un seul chargement par modele)
	Mesh mesh[NUM_MODELS];
	Texture2D texture[NUM_MODELS];

	mesh[M_BARREL].loadOBJ("models/barrel.obj");
	mesh[M_CRATE].loadOBJ("models/woodcrate.obj");
	mesh[M_ROBOT].loadOBJ("models/robot.obj");
	mesh[M_FLOOR].loadOBJ("models/floor.obj");
	mesh[M_PIN].loadOBJ("models/bowling_pin.obj");
	mesh[M_BUNNY].loadOBJ("models/bunny.obj");
	mesh[M_LAMP].loadOBJ("models/lampPost.obj");
	mesh[M_TREE].loadOBJ("models/tree.obj");	// modele telecharge : "Tree" par Kenney (poly.pizza, CC0)

	texture[M_BARREL].loadTexture("textures/barrel_diffuse.png", true);
	texture[M_CRATE].loadTexture("textures/woodcrate_diffuse.jpg", true);
	texture[M_ROBOT].loadTexture("textures/robot_diffuse.jpg", true);
	texture[M_FLOOR].loadTexture("textures/tile_floor.jpg", true);
	texture[M_PIN].loadTexture("textures/AMF.tga", true);
	texture[M_BUNNY].loadTexture("textures/bunny_diffuse.jpg", true);
	texture[M_LAMP].loadTexture("textures/lamp_post_diffuse.png", true);
	texture[M_TREE].loadTexture("textures/tree_green.ppm", true);

	// La scene : chaque ligne = un objet affiche
	const glm::vec3 one(1.0f, 1.0f, 1.0f);
	const glm::vec3 pinScale(0.1f, 0.1f, 0.1f);
	SceneObject scene[] = {
		// arbres (modele telecharge sur internet)
		{ M_TREE,   M_TREE,   glm::vec3(-2.5f, 0.0f, -5.5f), glm::vec3(4.0f, 4.0f, 4.0f), 0.0f },
		{ M_TREE,   M_TREE,   glm::vec3( 2.5f, 0.0f, -5.5f), glm::vec3(4.0f, 4.0f, 4.0f), 0.0f },
		{ M_TREE,   M_TREE,   glm::vec3(-3.0f, 0.0f,  5.0f), glm::vec3(4.0f, 4.0f, 4.0f), 0.0f },
		{ M_TREE,   M_TREE,   glm::vec3( 3.0f, 0.0f,  5.5f), glm::vec3(4.0f, 4.0f, 4.0f), 0.0f },
		// sol
		{ M_FLOOR,  M_FLOOR,  glm::vec3( 0.0f, 0.0f,  0.0f), glm::vec3(10.0f, 1.0f, 10.0f), 0.0f },
		// trois lampadaires (les lumieres ponctuelles sont placees dessus)
		{ M_LAMP,   M_LAMP,   glm::vec3(-5.5f, 0.0f,  0.0f), one, 0.0f },
		{ M_LAMP,   M_LAMP,   glm::vec3( 0.0f, 0.0f,  0.0f), one, 0.0f },
		{ M_LAMP,   M_LAMP,   glm::vec3( 5.5f, 0.0f,  0.0f), one, 0.0f },
		// tonneaux
		{ M_BARREL, M_BARREL, glm::vec3(-3.5f, 0.0f,  0.0f), one, 0.0f },
		{ M_BARREL, M_BARREL, glm::vec3(-4.2f, 0.0f,  3.0f), one, 0.0f },
		{ M_BARREL, M_BARREL, glm::vec3( 4.5f, 0.0f, -3.5f), one, 0.0f },
		// caisses
		{ M_CRATE,  M_CRATE,  glm::vec3( 3.5f, 0.0f,  0.0f), one, 0.0f },
		{ M_CRATE,  M_CRATE,  glm::vec3( 5.0f, 0.0f,  3.0f), one, 0.0f },
		{ M_CRATE,  M_CRATE,  glm::vec3(-5.0f, 0.0f, -3.5f), one, 0.0f },
		// robot qui tourne
		{ M_ROBOT,  M_ROBOT,  glm::vec3( 0.0f, 0.0f, -2.0f), one, 30.0f },
		// quilles
		{ M_PIN,    M_PIN,    glm::vec3( 0.0f, 0.0f,  2.0f), pinScale, 0.0f },
		{ M_PIN,    M_PIN,    glm::vec3( 0.6f, 0.0f,  2.5f), pinScale, 0.0f },
		{ M_PIN,    M_PIN,    glm::vec3(-0.6f, 0.0f,  2.5f), pinScale, 0.0f },
		{ M_PIN,    M_PIN,    glm::vec3( 0.0f, 0.0f,  3.0f), pinScale, 0.0f },
		// lapin qui tourne
		{ M_BUNNY,  M_BUNNY,  glm::vec3(-2.0f, 0.0f,  2.0f), glm::vec3(0.7f, 0.7f, 0.7f), 45.0f }
	};
	const int numObjects = sizeof(scene) / sizeof(scene[0]);

	// Positions des 3 lumieres ponctuelles (en haut des lampadaires)
	glm::vec3 pointLightPos[3] = {
		glm::vec3(-5.0f, 3.8f, 0.0f),
		glm::vec3( 0.5f, 3.8f, 0.0f),
		glm::vec3( 5.0f, 3.8f, 0.0f)
	};

	// Couleurs de base des 3 lumieres ponctuelles : vert, rouge, bleu
	glm::vec3 pointLightColor[3] = {
		glm::vec3(0.0f, 1.0f, 0.1f),
		glm::vec3(1.0f, 0.1f, 0.0f),
		glm::vec3(0.0f, 0.1f, 1.0f)
	};

	double lastTime = glfwGetTime();

	// Rendering loop
	while (!glfwWindowShouldClose(gWindow))
	{
		showFPS(gWindow);

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - lastTime;
		float t = (float)currentTime;		// temps ecoule en secondes, sert aux animations

		// Poll for and process events
		glfwPollEvents();

		// ---- Camera ----
		glm::vec3 camPos, camLook;
		glm::mat4 view(1.0);
		if (gAutoCamera)
		{
			// Visite automatique : la camera tourne autour de la scene,
			// s'approche et s'eloigne (rayon 7 a 11) et monte et descend (hauteur 1.5 a 4.5)
			float angle = 0.25f * t;
			float radius = 9.0f + 2.0f * sinf(0.3f * t);
			camPos = glm::vec3(radius * sinf(angle), 3.0f + 1.5f * sinf(0.5f * t), radius * cosf(angle));
			glm::vec3 target(0.0f, 1.5f, 0.0f);
			view = glm::lookAt(camPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
			camLook = glm::normalize(target - camPos);
		}
		else
		{
			update(deltaTime);				// camera libre : clavier + souris
			view = fpsCamera.getViewMatrix();
			camPos = fpsCamera.getPosition();
			camLook = fpsCamera.getLook();
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 model(1.0), projection(1.0);

		// Create the projection matrix
		projection = glm::perspective(glm::radians(fpsCamera.getFOV()), (float)gWindowWidth / (float)gWindowHeight, 0.1f, 200.0f);

		// Must be called BEFORE setting uniforms because setting uniforms is done
		// on the currently active shader program.
		lightingShader.use();
		lightingShader.setUniform("model", glm::mat4(1.0));
		lightingShader.setUniform("view", view);
		lightingShader.setUniform("projection", projection);
		lightingShader.setUniform("viewPos", camPos);

		// Lumiere directionnelle (lune) : son intensite varie lentement entre 0.1 et 0.35
		float sunI = 0.1f + 0.25f * (0.5f + 0.5f * sinf(0.3f * t));
		lightingShader.setUniform("sunLight.direction", glm::vec3(0.0f, -0.9f, -0.17f));
		lightingShader.setUniform("sunLight.ambient", glm::vec3(0.1f, 0.1f, 0.12f));
		lightingShader.setUniform("sunLight.diffuse", glm::vec3(sunI, sunI, sunI * 1.2f));
		lightingShader.setUniform("sunLight.specular", glm::vec3(0.1f, 0.1f, 0.1f));

		// Lumieres ponctuelles : chaque lampadaire pulse avec un decalage different
		for (int i = 0; i < 3; i++)
		{
			float pulse = 0.65f + 0.35f * sinf(2.0f * t + i * 2.1f);	// entre 0.3 et 1.0
			std::string name = "pointLights[" + std::to_string(i) + "].";

			lightingShader.setUniform((name + "ambient").c_str(), glm::vec3(0.2f, 0.2f, 0.2f));
			lightingShader.setUniform((name + "diffuse").c_str(), pointLightColor[i] * pulse);
			lightingShader.setUniform((name + "specular").c_str(), glm::vec3(1.0f, 1.0f, 1.0f));
			lightingShader.setUniform((name + "position").c_str(), pointLightPos[i]);
			lightingShader.setUniform((name + "constant").c_str(), 1.0f);
			lightingShader.setUniform((name + "linear").c_str(), 0.22f);
			lightingShader.setUniform((name + "exponent").c_str(), 0.20f);
		}

		// Spot : lampe torche attachee a la camera
		glm::vec3 spotlightPos = camPos;
		spotlightPos.y -= 0.5f;		// un peu plus bas que la camera

		lightingShader.setUniform("spotLight.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
		lightingShader.setUniform("spotLight.diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		lightingShader.setUniform("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		lightingShader.setUniform("spotLight.position", spotlightPos);
		lightingShader.setUniform("spotLight.direction", camLook);
		lightingShader.setUniform("spotLight.cosInnerCone", glm::cos(glm::radians(15.0f)));
		lightingShader.setUniform("spotLight.cosOuterCone", glm::cos(glm::radians(20.0f)));
		lightingShader.setUniform("spotLight.constant", 1.0f);
		lightingShader.setUniform("spotLight.linear", 0.07f);
		lightingShader.setUniform("spotLight.exponent", 0.017f);
		lightingShader.setUniform("spotLight.on", gFlashlightOn);

		// Rendu de tous les objets de la scene
		for (int i = 0; i < numObjects; i++)
		{
			const SceneObject& obj = scene[i];

			// translation * rotation (animation) * mise a l'echelle
			model = glm::translate(glm::mat4(1.0), obj.pos)
				  * glm::rotate(glm::mat4(1.0), glm::radians(obj.spin * t), glm::vec3(0.0f, 1.0f, 0.0f))
				  * glm::scale(glm::mat4(1.0), obj.scale);
			lightingShader.setUniform("model", model);

			// Set material properties
			lightingShader.setUniform("material.ambient", glm::vec3(0.1f, 0.1f, 0.1f));
			lightingShader.setUniformSampler("material.diffuseMap", 0);
			lightingShader.setUniform("material.specular", glm::vec3(0.8f, 0.8f, 0.8f));
			lightingShader.setUniform("material.shininess", 32.0f);

			texture[obj.texId].bind(0);		// set the texture before drawing.
			mesh[obj.meshId].draw();		// Render the OBJ mesh
			texture[obj.texId].unbind(0);
		}

		// Swap front and back buffers
		glfwSwapBuffers(gWindow);

		lastTime = currentTime;
	}

	glfwTerminate();

	return 0;
}

//-----------------------------------------------------------------------------
// Initialize GLFW and OpenGL
//-----------------------------------------------------------------------------
bool initOpenGL()
{
	// Intialize GLFW
	// GLFW is configured.  Must be called before calling any GLFW functions
	if (!glfwInit())
	{
		// An error occured
		std::cerr << "GLFW initialization failed" << std::endl;
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);	// forward compatible with newer versions of OpenGL as they become available but not backward compatible (it will not run on devices that do not support OpenGL 3.3


	// Create an OpenGL 3.3 core, forward compatible context window
	gWindow = glfwCreateWindow(gWindowWidth, gWindowHeight, APP_TITLE, NULL, NULL);
	if (gWindow == NULL)
	{
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	// Make the window's context the current one
	glfwMakeContextCurrent(gWindow);

	// Initialize GLEW
	// (sous Wayland, glewInit renvoie GLEW_ERROR_NO_GLX_DISPLAY alors que les fonctions
	//  OpenGL sont bien chargees : on ignore cette erreur precise)
	glewExperimental = GL_TRUE;
	GLenum glewErr = glewInit();
	if (glewErr != GLEW_OK && glewErr != GLEW_ERROR_NO_GLX_DISPLAY)
	{
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return false;
	}

	// Set the required callback functions
	glfwSetKeyCallback(gWindow, glfw_onKey);
	glfwSetFramebufferSizeCallback(gWindow, glfw_onFramebufferSize);
	glfwSetScrollCallback(gWindow, glfw_onMouseScroll);

	// En mode visite automatique, le curseur reste visible et libre.
	// Il est capture seulement en camera libre (touche C).
	glfwSetInputMode(gWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glClearColor(gClearColor.r, gClearColor.g, gClearColor.b, gClearColor.a);

    // Define the viewport dimensions
    int w, h;
    glfwGetFramebufferSize( gWindow, &w, &h); // For retina display
    glViewport(0, 0, w, h);

    glEnable(GL_DEPTH_TEST);

	return true;
}

//-----------------------------------------------------------------------------
// Is called whenever a key is pressed/released via GLFW
//-----------------------------------------------------------------------------
void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
	{
		gWireframe = !gWireframe;
		if (gWireframe)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		// toggle the flashlight
		gFlashlightOn = !gFlashlightOn;
	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		// bascule visite automatique <-> camera libre
		gAutoCamera = !gAutoCamera;
		if (gAutoCamera)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(window, gWindowWidth / 2.0, gWindowHeight / 2.0);
		}
	}
}

//-----------------------------------------------------------------------------
// Is called when the window is resized
//-----------------------------------------------------------------------------
void glfw_onFramebufferSize(GLFWwindow* window, int width, int height)
{
	gWindowWidth = width;
	gWindowHeight = height;

    // Define the viewport dimensions
    int w, h;
    glfwGetFramebufferSize( gWindow, &w, &h); // For retina display
    glViewport(0, 0, w, h);
}

//-----------------------------------------------------------------------------
// Called by GLFW when the mouse wheel is rotated
//-----------------------------------------------------------------------------
void glfw_onMouseScroll(GLFWwindow* window, double deltaX, double deltaY)
{
	double fov = fpsCamera.getFOV() + deltaY * ZOOM_SENSITIVITY;

	fov = glm::clamp(fov, 1.0, 120.0);

	fpsCamera.setFOV((float)fov);
}

//-----------------------------------------------------------------------------
// Camera libre : mise a jour a chaque image (clavier + souris)
//-----------------------------------------------------------------------------
void update(double elapsedTime)
{
	// Camera orientation
	double mouseX, mouseY;

	// Get the current mouse cursor position delta
	glfwGetCursorPos(gWindow, &mouseX, &mouseY);

	// Rotate the camera the difference in mouse distance from the center screen.  Multiply this delta by a speed scaler
	fpsCamera.rotate((float)(gWindowWidth / 2.0 - mouseX) * MOUSE_SENSITIVITY, (float)(gWindowHeight / 2.0 - mouseY) * MOUSE_SENSITIVITY);

	// Clamp mouse cursor to center of screen
	glfwSetCursorPos(gWindow, gWindowWidth / 2.0, gWindowHeight / 2.0);

	// Camera FPS movement

	// Forward/backward
	if (glfwGetKey(gWindow, GLFW_KEY_W) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getLook());
	else if (glfwGetKey(gWindow, GLFW_KEY_S) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getLook());

	// Strafe left/right
	if (glfwGetKey(gWindow, GLFW_KEY_A) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -fpsCamera.getRight());
	else if (glfwGetKey(gWindow, GLFW_KEY_D) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * fpsCamera.getRight());

	// Up/down
	if (glfwGetKey(gWindow, GLFW_KEY_Z) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * glm::vec3(0.0f, 1.0f, 0.0f));
	else if (glfwGetKey(gWindow, GLFW_KEY_X) == GLFW_PRESS)
		fpsCamera.move(MOVE_SPEED * (float)elapsedTime * -glm::vec3(0.0f, 1.0f, 0.0f));
}

//-----------------------------------------------------------------------------
// Code computes the average frames per second, and also the average time it takes
// to render one frame.  These stats are appended to the window caption bar.
//-----------------------------------------------------------------------------
void showFPS(GLFWwindow* window)
{
	static double previousSeconds = 0.0;
	static int frameCount = 0;
	double elapsedSeconds;
	double currentSeconds = glfwGetTime(); // returns number of seconds since GLFW started, as double float

	elapsedSeconds = currentSeconds - previousSeconds;

	// Limit text updates to 4 times per second
	if (elapsedSeconds > 0.25)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		double msPerFrame = 1000.0 / fps;

		// The C++ way of setting the window title
		std::ostringstream outs;
		outs.precision(3);	// decimal places
		outs << std::fixed
			<< APP_TITLE << "    "
			<< "FPS: " << fps << "    "
			<< "Frame Time: " << msPerFrame << " (ms)";
		glfwSetWindowTitle(window, outs.str().c_str());

		// Reset for next average.
		frameCount = 0;
	}

	frameCount++;
}
