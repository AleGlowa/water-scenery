#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <time.h>
#include <cstdlib>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;

Core::Shader_Loader shaderLoader;

obj::Model goldenFishModel;		// Model zlotej ryby(glowny bohater)
obj::Model bubbleModels[5];		// Modele baniek
obj::Model bottomModel;			// Model podloza

glm::vec3 cameraPos = glm::vec3(0, 0, 5);
glm::vec3 cameraDir; // Wektor "do przodu" kamery
glm::vec3 cameraSide; // Wektor "w bok" kamery
float cameraAngle = 0;
float bubble_speed = 0.9f;		// Predkosc unoszenia sie baniek
float rand_x[5];		// 5 losowych wspolrzednych x dla 5 baniek
float rand_z[5];		// 5 losowych wspolrzednych z dla 5 baniek

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(-1.0f, -5.0f, -1.0f));		// Polozenie zrodla swiatla

glm::quat rotation = glm::quat(1, 0, 0, 0);

GLuint textureBubble;		// Tekstura banki
GLuint textureGoldenFish;	// Tekstura zlotej ryby
GLuint textureBottom;		// Tekstura dna

void keyboard(unsigned char key, int x, int y)
{
	
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
}

glm::mat4 createCameraMatrix()
{
	cameraDir = glm::vec3(cosf(cameraAngle - glm::radians(90.0f)), 0.0f, sinf(cameraAngle - glm::radians(90.0f)));
	glm::vec3 up = glm::vec3(0, 1, 0);
	cameraSide = glm::cross(cameraDir, up);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}
void renderScene()
{
	// Aktualizacja macierzy widoku i rzutowania
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.66f, 0.71f, 0.96f, 1.0f);		// Kolor tla(blekit)

	// Macierz poczatkowej transformacji dla zlotej ryby
	glm::mat4 goldenFishInitialTransformation = glm::translate(glm::vec3(0, 0, 5)) * glm::rotate(glm::radians(-90.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f)); //* glm::rotate(glm::radians(180.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.25f));
	
	// Macierze babelkow i ich aktualizacja
	glm::mat4 bubbleInitialTransformation[5];
	glm::mat4 bubbleModelMatrix[5];
	
	for (size_t i = 0; i < 5; ++i) {
		bubbleInitialTransformation[i] = glm::translate(glm::vec3(rand_x[i], 0, rand_z[i])) * glm::scale(glm::vec3(0.25f));
		bubbleModelMatrix[i] = glm::translate(glm::vec3(0, bubble_speed * (float)clock() / CLOCKS_PER_SEC, 0)) * bubbleInitialTransformation[i];
	}

	// Macierz aktualizacji transformacji dla zotej ryby
	glm::mat4 goldenFishModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f) * glm::rotate(-cameraAngle, glm::vec3(0,1,0)) * goldenFishInitialTransformation;

	// Macierz transformacji powierzchni
	glm::mat4 bottomModelMatrix = glm::translate(glm::vec3(0, -2, 0)) * glm::scale(glm::vec3(0.5f));
	
	// Nadaj kolor zlotej rybie(zloty)
	drawObjectColor(&goldenFishModel, goldenFishInitialTransformation, glm::vec3(0.85f, 0.93f, 0.18f));
	//drawObjectTexture(&goldenFishModel, goldenFishModelMatrix, textureGoldenFish);
	
	// Przypisz teksture bankom
	for (size_t i = 0; i < 5; ++i)
		drawObjectTexture(&bubbleModels[i], bubbleModelMatrix[i], textureBubble);

	// Nadaj kolor powierzchni(brazowy)
	//drawObjectTexture(&bottomModel, bottomModelMatrix, textureBottom);
	drawObjectColor(&bottomModel, bottomModelMatrix, glm::vec3(0.98f, 0.84f, 0.60f));
	

	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	goldenFishModel = obj::loadModelFromFile("models/ultra_golden_fish.obj");		// Wczytaj model zlotej ryby
	bottomModel = obj::loadModelFromFile("models/bottom.obj");				// Wczytaj model powierzchni
	textureBubble = Core::LoadTexture("textures/bubble2.png");				// Wczytaj teksture banki
	textureGoldenFish = Core::LoadTexture("textures/ultra_golden_fish_scales.png");// Wczytaj teksture zlotej ryby
	textureBottom = Core::LoadTexture("textures/bottom_sand.png");				// Wczytaj teksture dna

	// Losuj pozycje na osi x i z dla baniek
	for (size_t i = 0; i < 5; ++i) {
		rand_x[i] = -2.5f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.5f + 2.5f)));
		rand_z[i] = -1.0f + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (1.0f + 1.0f)));
	}

	// Wczytaj modele baniek
	for (size_t i = 0; i < 5; ++i)
		bubbleModels[i] = obj::loadModelFromFile("models/bubble.obj");


}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(600, 600);
	glutCreateWindow("Przygody zlotej rybki");		// Tytul okienka
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
