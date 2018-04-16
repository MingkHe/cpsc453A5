// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Co-Authors:
//			Jeremy Hart, University of Calgary
//			John Hall, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "texture.h"
#include "Camera.h"

using namespace std;
using namespace glm;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

#define PI_F 3.14159265359f

int startFlag=0;
int scenesFlag=0;
int speed=1;

float scaler =5.0;

struct planet
{
	planet* parent;
	vector<planet*> children;
	int texture;
	float distanceToParent;
	float axialTilt;
	float inclination;
	float rotationPeriod;
	float OrbitalSpeed;
	mat4 transform;
	vec3 origin = vec3(0,0,0);
	float radius;
	//image | texture
};

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

bool lbPushed = false;

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

// load, compile, and link shaders, returning true if successful
GLuint InitializeShaders()
{
	// load shader source from files
	string vertexSource = LoadSource("shaders/vertex.glsl");
	string fragmentSource = LoadSource("shaders/fragment.glsl");
	if (vertexSource.empty() || fragmentSource.empty()) return false;

	// compile shader source into shader objects
	GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	// link shader program
	GLuint program = LinkProgram(vertex, fragment);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// check for OpenGL errors and return false if error occurred
	return program;
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct Geometry
{
	// OpenGL names for array buffer objects, vertex array object
	GLuint  vertexBuffer;
	GLuint  textureBuffer;
	GLuint  colourBuffer;
	GLuint  vertexArray;
	GLsizei elementCount;

	// initialize object names to zero (OpenGL reserved value)
	Geometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
	{}
};

bool InitializeVAO(Geometry *geometry){

	const GLuint VERTEX_INDEX = 0;
	const GLuint TEXT_INDEX = 1;

	//Generate Vertex Buffer Objects
	// create an array buffer object for storing our vertices
	glGenBuffers(1, &geometry->vertexBuffer);
	
	glGenBuffers(1, &geometry->textureBuffer);

	//Set up Vertex Array Object
	// create a vertex array object encapsulating all our vertex attributes
	glGenVertexArrays(1, &geometry->vertexArray);
	glBindVertexArray(geometry->vertexArray);

	// associate the position array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glVertexAttribPointer(
		VERTEX_INDEX,		//Attribute index 
		3, 					//# of components
		GL_FLOAT, 			//Type of component
		GL_FALSE, 			//Should be normalized?
		sizeof(vec3),		//Stride - can use 0 if tightly packed
		0);					//Offset to first element
	glEnableVertexAttribArray(VERTEX_INDEX);
	
	glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
	glVertexAttribPointer(
		TEXT_INDEX,		//Attribute index 
		2, 					//# of components
		GL_FLOAT, 			//Type of component
		GL_FALSE, 			//Should be normalized?
		sizeof(vec2), 		//Stride - can use 0 if tightly packed
		0);					//Offset to first element
	glEnableVertexAttribArray(TEXT_INDEX);

	// unbind our buffers, resetting to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return !CheckGLErrors();
}

// create buffers and fill with geometry data, returning true if successful
bool LoadGeometry(Geometry *geometry, vec3 *vertices, vec2* texCoord, int elementCount)
{
	geometry->elementCount = elementCount;

	// create an array buffer object for storing our vertices
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3)*geometry->elementCount, vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*geometry->elementCount, texCoord, GL_STATIC_DRAW);
	//Unbind buffer to reset to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(Geometry *geometry)
{
	// unbind and destroy our vertex array object and associated buffers
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &geometry->vertexArray);
	glDeleteBuffers(1, &geometry->vertexBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(Geometry *geometry, GLuint program, vec3 color, Camera* camera, mat4 perspectiveMatrix,mat4 transistion, GLenum rendermode)
{

	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glUseProgram(program);

	int vp [4];
	glGetIntegerv(GL_VIEWPORT, vp);
	int width = vp[2];
	int height = vp[3];


	//Bind uniforms
	GLint uniformLocation = glGetUniformLocation(program, "Colour");
	glUniform3f(uniformLocation, color.r, color.g, color.b); 

	mat4 modelViewProjection = perspectiveMatrix*camera->viewMatrix();
	uniformLocation = glGetUniformLocation(program, "modelViewProjection");
	glUniformMatrix4fv(uniformLocation, 1, false, glm::value_ptr(modelViewProjection));
	
	uniformLocation = glGetUniformLocation(program, "transistion");
	glUniformMatrix4fv(uniformLocation, 1, false, glm::value_ptr(transistion));

	glBindVertexArray(geometry->vertexArray);
	glDrawArrays(rendermode, 0, geometry->elementCount);

	// for testing purpose
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// reset state to default (no shader or geometry bound)
	glBindVertexArray(0);
	glUseProgram(0);

	// check for an report any OpenGL errors
	CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
		
	if (key == GLFW_KEY_G && action == GLFW_PRESS)
		startFlag = (startFlag+1)%2;
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
		scenesFlag=1;
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
		scenesFlag=2;
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
		scenesFlag=3;
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
		scenesFlag=4;
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
		scenesFlag=5;
	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
		scenesFlag=6;
	if (key == GLFW_KEY_7 && action == GLFW_PRESS)
		scenesFlag=7;
	if (key == GLFW_KEY_8 && action == GLFW_PRESS)
		scenesFlag=8;
	if (key == GLFW_KEY_9 && action == GLFW_PRESS)
		scenesFlag=9;
	if (key == GLFW_KEY_0 && action == GLFW_PRESS)
		scenesFlag=0;
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		speed+=1;
		if(speed>=5){
			speed=5;
		}
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS){
		speed-=1;
		if(speed<=1){
			speed=1;
		}
	}
}

// --------------------------------------------------------------------------
// Geometry method
vec3 s(float theta, float phi, float radius){
	return vec3(radius*sin(theta)*cos(phi),radius*cos(theta),radius*sin(theta)*sin(phi));
}

void triangulateSphere(vec3 origin, float radius, vector<vec3>* data, vector<vec2>* texCoord){
	float phiNum = 1000;
	float phiStep = 2*PI_F/(phiNum-1);
	float thetaNum = 1000;
	float thetaStep = PI_F/(thetaNum-1);

	for(float theta=0; theta<PI_F; theta += thetaStep){
		for(float phi=0; phi<2*PI_F; phi += phiStep){
			data->push_back(origin+s(theta,phi,radius));
			texCoord->push_back(vec2(1-phi/(2*PI_F),1-theta/PI_F));
			
			data->push_back(origin+s(theta,phi+phiStep,radius));
			texCoord->push_back(vec2(1-(phi+phiStep)/(2*PI_F),1-theta/PI_F));
			
			data->push_back(origin+s(theta+thetaStep,phi,radius));
			texCoord->push_back(vec2(1-phi/(2*PI_F),1-(theta+thetaStep)/PI_F));
			
			data->push_back(origin+s(theta,phi+phiStep,radius));
			texCoord->push_back(vec2(1-(phi+phiStep)/(2*PI_F),1-theta/PI_F));
						
			data->push_back(origin+s(theta+thetaStep,phi,radius));
			texCoord->push_back(vec2(1-phi/(2*PI_F),1-(theta+thetaStep)/PI_F));
						
			data->push_back(origin+s(theta+thetaStep,phi+phiStep,radius));
			texCoord->push_back(vec2(1-(phi+phiStep)/(2*PI_F),1-(theta+thetaStep)/PI_F));
		}
	}

}

mat4 calculateTransform(float distanceToParent, float inclination, float phi){
	//vec3 pos = vec3(distanceToParent*sin(inclination),distanceToParent*cos(inclination),0);
	vec3 pos = vec3(distanceToParent*sin(phi),0,distanceToParent*cos(phi));
	vec3 z = vec3(0,0,1);
	mat4 result;
	mat4 rotationMatrix = glm::rotate(mat4(1.f), inclination, z);
	mat4 translateMatrix = glm::translate(mat4(1.f),pos);
	result = translateMatrix*rotationMatrix;
	return result;
}

mat4 calculateAnimationTransform(float time,float factor){

	vec3 y = vec3(0,1,0);

	return glm::rotate(mat4(1.f), factor*time, y);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	scaler=scaler-yoffset*0.2;
	if(scaler<0.6)
		scaler=0.6;
}

// --------------------------------------------------------------------------
// Geometry method

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	int width = 512, height = 512;
	window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	// set keyboard callback function and make our context current (active)
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwMakeContextCurrent(window);

	//Intialize GLAD
	if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}

	// query and print out information about our OpenGL environment
	QueryGLVersion();

	// call function to load and compile shader programs
	GLuint program = InitializeShaders();
	if (program == 0) {
		cout << "Program could not initialize shaders, TERMINATING" << endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	
	planet sun;
	sun.radius = 8.0f;
	sun.texture = 0;
	
	
	planet mercury;
	mercury.radius = 0.2f;
	mercury.texture = 5;
	mercury.axialTilt = 0;
	mercury.inclination = 7;
	mercury.distanceToParent = 13;
	
	planet venus;
	venus.radius = 0.3f;
	venus.texture = 6;
	venus.axialTilt = 177.3;
	venus.inclination = 3.39;
	venus.distanceToParent = 16;
	
	planet earth;
	earth.radius = 0.5f;
	earth.texture = 1;
	earth.axialTilt = 23.44;
	earth.inclination = 0;
	earth.distanceToParent = 19;
	
	planet mars;
	mars.radius = 0.46f;
	mars.texture = 7;
	mars.axialTilt = 25.19;
	mars.inclination = 1.85;
	mars.distanceToParent = 21;
	
	planet jupiter;
	jupiter.radius = 2.0f;
	jupiter.texture = 8;
	jupiter.axialTilt = 3.12;
	jupiter.inclination = 1.31;
	jupiter.distanceToParent = 25;
	
	planet saturn;
	saturn.radius = 1.0f;
	saturn.texture = 9;
	saturn.axialTilt = 3.12;
	saturn.inclination = 2.48;
	saturn.distanceToParent = 30;

	planet uranus;
	uranus.radius = 1.35f;
	uranus.texture = 10;
	uranus.axialTilt = 97.86;
	uranus.inclination = 0.76;
	uranus.distanceToParent = 35;
	
	planet neptune;
	neptune.radius = 1.45f;
	neptune.texture = 11;
	neptune.axialTilt = 29.58;
	neptune.inclination = 1.77;
	neptune.distanceToParent = 39;
	
	planet moon;
	moon.parent = &earth;
	moon.radius = 0.1f;
	moon.texture = 0.7;
	moon.axialTilt = 6.68;
	moon.inclination = 18.29;
	moon.distanceToParent = 2;
		
	vector<vec2> texCoords;

	//testing sphere
	vector<vec3> testSphere;
	triangulateSphere(vec3(0,0,0),0.5f,&testSphere, &texCoords);
	
	
	vector<vec3> sunSphere;
	vector<vec3> earthSphere;
	vector<vec3> moonSphere;
	vector<vec3> star;
	
	vector<vec3> mercurySphere;
	vector<vec3> venusSphere;
	vector<vec3> marsSphere;
	vector<vec3> jupiterSphere;
	vector<vec3> saturnSphere;
	vector<vec3> uranusSphere;
	vector<vec3> neptuneSphere;
	
	vector<vec3> saturnRing;
	vector<vec2> ringTexCoords;
	Geometry saturnRingGeometry;
	
	float step = (1.0f/100.0f)*(2*PI_F);
	for(float t=0;t<2*PI_F;t+=step){
		saturnRing.push_back(vec3(2*sin(t),0,2*cos(t)));
		saturnRing.push_back(vec3(2*sin(t+step),0,2*cos(t+step)));
		saturnRing.push_back(vec3(0,0,0));
		ringTexCoords.push_back(vec2(0.8,t/(2*PI_F)));
		ringTexCoords.push_back(vec2(0.8,t+step/(2*PI_F)));
		ringTexCoords.push_back(vec2(0,0.5));
	}
	if (!InitializeVAO(&saturnRingGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&saturnRingGeometry, saturnRing.data(), ringTexCoords.data(), saturnRing.size()))
		cout << "Failed to load geometry" << endl;

	
	triangulateSphere(sun.origin,sun.radius,&sunSphere, &texCoords);
	triangulateSphere(earth.origin,earth.radius,&earthSphere, &texCoords);
	triangulateSphere(moon.origin,moon.radius,&moonSphere, &texCoords);
	triangulateSphere(vec3(0,0,0),500,&star, &texCoords);
	
	triangulateSphere(mercury.origin,mercury.radius,&mercurySphere, &texCoords);
	triangulateSphere(venus.origin,venus.radius,&venusSphere, &texCoords);
	triangulateSphere(mars.origin,mars.radius,&marsSphere, &texCoords);
	triangulateSphere(jupiter.origin,jupiter.radius,&jupiterSphere, &texCoords);
	triangulateSphere(saturn.origin,saturn.radius,&saturnSphere, &texCoords);
	triangulateSphere(uranus.origin,uranus.radius,&uranusSphere, &texCoords);
	triangulateSphere(neptune.origin,neptune.radius,&neptuneSphere, &texCoords);
	

	mat4 perspectiveMatrix = glm::perspective(PI_F*0.4f, float(width)/float(height), 0.1f, 1000.f);	//Fill in with Perspective Matrix

	Geometry geometry;
	Geometry testGeometry;
	
	Geometry sunGeometry;
	Geometry earthGeometry;
	Geometry moonGeometry;
	Geometry starGeometry;
	
	Geometry mercuryGeometry;
	Geometry venusGeometry;
	Geometry marsGeometry;
	Geometry jupiterGeometry;
	Geometry saturnGeometry;
	Geometry uranusGeometry;
	Geometry neptuneGeometry;


	// call function to create and fill buffers with geometry data
		
	if (!InitializeVAO(&testGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&testGeometry, testSphere.data(), texCoords.data(), testSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&sunGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&sunGeometry, sunSphere.data(), texCoords.data(), sunSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&earthGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&earthGeometry, earthSphere.data(), texCoords.data(), earthSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&moonGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&moonGeometry, moonSphere.data(), texCoords.data(), moonSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&starGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&starGeometry, star.data(), texCoords.data(), star.size()))
		cout << "Failed to load geometry" << endl;

		
	if (!InitializeVAO(&mercuryGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&mercuryGeometry, mercurySphere.data(), texCoords.data(), mercurySphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&venusGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&venusGeometry, venusSphere.data(), texCoords.data(), venusSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&marsGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&marsGeometry, marsSphere.data(), texCoords.data(), marsSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&jupiterGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&jupiterGeometry, jupiterSphere.data(), texCoords.data(), jupiterSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&saturnGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&saturnGeometry, saturnSphere.data(), texCoords.data(), saturnSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&uranusGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&uranusGeometry, uranusSphere.data(), texCoords.data(), uranusSphere.size()))
		cout << "Failed to load geometry" << endl;
		
	if (!InitializeVAO(&neptuneGeometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&neptuneGeometry, neptuneSphere.data(), texCoords.data(), neptuneSphere.size()))
		cout << "Failed to load geometry" << endl;

	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	
	MyTexture image = MyTexture();
	vector<MyTexture> images;// 0-sun|1-earth day|2-earth night|3-moon|4-star|5 |6 |7 |8 |
	InitializeTexture(&image, "8k_sun.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_earth_daymap.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_earth_nightmap.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_moon.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_stars_milky_way.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_mercury.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_venus_surface.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_mars.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_jupiter.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_saturn.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "2k_uranus.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "2k_neptune.jpg", GL_TEXTURE_2D);
	images.push_back(image);
	InitializeTexture(&image, "8k_saturn_ring_alpha.png", GL_TEXTURE_2D);
	images.push_back(image);
	// run an event-triggered main loop
	
	glUseProgram(program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,images[1].textureID);
	
	GLint loc = glGetUniformLocation(program,"imageTexture");
	glUniform1i(loc,0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,images[2].textureID);
	
	GLint loc2 = glGetUniformLocation(program, "earthNight");
	glUniform1i(loc2,1);
	
	glActiveTexture(GL_TEXTURE0);
	float timeElapsed = 0.f;
	GLint timeLocation = glGetUniformLocation(program, "time");	

	Camera cam;

	vec2 lastCursorPos;

	float cursorSensitivity = PI_F/200.f;	//PI/hundred pixels
	float movementSpeed = 0.01f;

	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{
		////////////////////////
		//Camera interaction
		////////////////////////
		//Translation
		vec3 movement(0.f);

		//if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			//movement.z += 1.f;
		//if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			//movement.z -= 1.f;
		//if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			//movement.x += 1.f;
		//if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			//movement.x -= 1.f;
		
		//cam.move(movement*movementSpeed);
		glUseProgram(program);
		glUniform1f(timeLocation, timeElapsed);


		
		

		//cout<<movement.x*movementSpeed<<endl;
		
		
		//cout<<"Cx: "<<cam.pos.x<<"Cy: "<<cam.pos.y<<"Cz: "<<cam.pos.z<<endl;
	
		///////////
		//Drawing
		//////////

		// clear screen to a dark grey colour
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// call function to draw our scene
		//RenderScene(&geometry, program, vec3(1, 0, 0), &cam, perspectiveMatrix, GL_TRIANGLES);
		//RenderScene(&frustumGeometry, program, vec3(0, 1, 0), &cam, perspectiveMatrix, GL_LINE_STRIP);
		mat4 transistionMatrix_sun = calculateTransform(0,0,0);
		mat4 transistionMatrix_earth = calculateTransform(earth.distanceToParent,-(earth.axialTilt/360.0f)*2*PI_F,timeElapsed*0.5f);
		mat4 transistionMatrix_moon = calculateTransform(moon.parent->distanceToParent,-(moon.inclination/360.0f)*2*PI_F,timeElapsed*0.5f)*calculateTransform(moon.distanceToParent,-(moon.axialTilt/360.0f)*2*PI_F,2.0f*timeElapsed);
		
		mat4 transistionMatrix_mercury = calculateTransform(0,-(mercury.inclination/360.0f)*2*PI_F,0)*calculateTransform(mercury.distanceToParent,-(mercury.axialTilt/360.0f)*2*PI_F,timeElapsed*1.5f);
		mat4 transistionMatrix_venus = calculateTransform(0,-(venus.inclination/360.0f)*2*PI_F,0)*calculateTransform(venus.distanceToParent,-(venus.axialTilt/360.0f)*2*PI_F,timeElapsed*1.2f);
		mat4 transistionMatrix_mars = calculateTransform(0,-(mars.inclination/360.0f)*2*PI_F,0)*calculateTransform(mars.distanceToParent,-(mars.axialTilt/360.0f)*2*PI_F,timeElapsed*0.6);
		mat4 transistionMatrix_jupiter = calculateTransform(0,-(jupiter.inclination/360.0f)*2*PI_F,0)*calculateTransform(jupiter.distanceToParent,-(jupiter.axialTilt/360.0f)*2*PI_F,timeElapsed*0.16f);
		mat4 transistionMatrix_saturn = calculateTransform(0,-(saturn.inclination/360.0f)*2*PI_F,0)*calculateTransform(saturn.distanceToParent,-(saturn.axialTilt/360.0f)*2*PI_F,timeElapsed*0.4f);
		mat4 transistionMatrix_uranus = calculateTransform(0,-(uranus.inclination/360.0f)*2*PI_F,0)*calculateTransform(uranus.distanceToParent,-(uranus.axialTilt/360.0f)*2*PI_F,timeElapsed*0.2);
		mat4 transistionMatrix_neptune = calculateTransform(0,-(neptune.inclination/360.0f)*2*PI_F,0)*calculateTransform(neptune.distanceToParent,-(neptune.axialTilt/360.0f)*2*PI_F,timeElapsed*0.25);
		
		mat4 animationMatrixSun = calculateAnimationTransform(timeElapsed,1.0f);
		mat4 animationMatrixEarth = calculateAnimationTransform(timeElapsed,2.0f);
		mat4 animationMatrixMoon = calculateAnimationTransform(timeElapsed,3.0f);
		
		mat4 animationMatrixMercury = calculateAnimationTransform(timeElapsed,4.0f);
		mat4 animationMatrixVenus = calculateAnimationTransform(timeElapsed,3.0f);
		mat4 animationMatrixMars = calculateAnimationTransform(timeElapsed,2.0f);
		mat4 animationMatrixJupiter = calculateAnimationTransform(timeElapsed,0.5f);
		mat4 animationMatrixSaturn = calculateAnimationTransform(timeElapsed,0.7f);
		mat4 animationMatrixUranus = calculateAnimationTransform(timeElapsed,0.65f);
		mat4 animationMatrixNeptune = calculateAnimationTransform(timeElapsed,0.63f);

		
		GLint cameraPos_loc = glGetUniformLocation(program, "cameraPos");
		GLint shading_loc = glGetUniformLocation(program, "shading");
		GLint earth_loc = glGetUniformLocation(program, "earth");
		
		if(scenesFlag==0){
			cam.lookat = vec3(0,0,0);
			cam.radius = sun.radius*2;
		}else if(scenesFlag==1){
			cam.lookat = vec3(transistionMatrix_mercury*animationMatrixMercury*vec4(0,0,0,1));
			cam.radius = mercury.radius*2;
		}else if(scenesFlag==2){
			cam.lookat = vec3(transistionMatrix_venus*animationMatrixVenus*vec4(0,0,0,1));
			cam.radius = venus.radius*2;
		}else if(scenesFlag==3){
			cam.lookat = vec3(transistionMatrix_earth*animationMatrixEarth*vec4(0,0,0,1));
			cam.radius = earth.radius*2;
		}else if(scenesFlag==4){
			cam.lookat = vec3(transistionMatrix_mars*animationMatrixMars*vec4(0,0,0,1));
			cam.radius = mars.radius*2;
		}else if(scenesFlag==5){
			cam.lookat = vec3(transistionMatrix_jupiter*animationMatrixJupiter*vec4(0,0,0,1));
			cam.radius = jupiter.radius*2;
		}else if(scenesFlag==6){
			cam.lookat = vec3(transistionMatrix_saturn*animationMatrixSaturn*vec4(0,0,0,1));
			cam.radius = saturn.radius*2;
		}else if(scenesFlag==7){
			cam.lookat = vec3(transistionMatrix_uranus*animationMatrixUranus*vec4(0,0,0,1));
			cam.radius = uranus.radius*2;
		}else if(scenesFlag==8){
			cam.lookat = vec3(transistionMatrix_neptune*animationMatrixNeptune*vec4(0,0,0,1));
			cam.radius = neptune.radius*2;
		}else if(scenesFlag==9){
			cam.lookat = vec3(transistionMatrix_moon*animationMatrixMoon*vec4(0,0,0,1));
			cam.radius = moon.radius*2;
		}
		

		
		//Rotation
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		vec2 cursorPos (xpos, ypos);
		vec2 cursorChange = cursorPos - lastCursorPos;
		

		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
			//cam.rotateHorizontal(-cursorChange.x*cursorSensitivity);
			//cam.rotateVertical(-cursorChange.y*cursorSensitivity);
			//cam.move2(-cursorChange.y*cursorSensitivity,-cursorChange.x*cursorSensitivity);
			cam.move3(-cursorChange.y*cursorSensitivity,-cursorChange.x*cursorSensitivity);
		}
		
		if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
			movement.z += 1.f;
		}
		if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
			movement.z -= 1.f;
		}
		if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
			movement.x += 1.f;
		}
		if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
			movement.x -= 1.f;
		}
		cam.scale(scaler);
		cam.move3(-movement.z*movementSpeed,-movement.x*movementSpeed);

		lastCursorPos = cursorPos;
		
		glUniform3f(cameraPos_loc,cam.pos.x,cam.pos.y,cam.pos.z);
		glUniform1i(earth_loc,false);
		
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[0].textureID);
		
		glUniform1i(shading_loc,false);
				
		RenderScene(&sunGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_sun*animationMatrixSun, GL_TRIANGLES);
		
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[1].textureID);
		
		glUseProgram(program);
		glUniform1i(earth_loc,true);
		glUniform1i(shading_loc,true);
		
		RenderScene(&earthGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_earth*animationMatrixEarth, GL_TRIANGLES);
		
		glUseProgram(program);
		glUniform1i(earth_loc,false);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[3].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&moonGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_moon*animationMatrixMoon, GL_TRIANGLES);
		
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[5].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&mercuryGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_mercury*animationMatrixMercury, GL_TRIANGLES);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[6].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&venusGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_venus*animationMatrixVenus, GL_TRIANGLES);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[6].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&venusGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_venus*animationMatrixVenus, GL_TRIANGLES);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[7].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&marsGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_mars*animationMatrixMars, GL_TRIANGLES);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[8].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&jupiterGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_jupiter*animationMatrixJupiter, GL_TRIANGLES);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[9].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&saturnGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_saturn*animationMatrixSaturn, GL_TRIANGLES);
		
		glBindTexture(GL_TEXTURE_2D,images[12].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&saturnRingGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_saturn*animationMatrixSaturn, GL_TRIANGLES);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[10].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&uranusGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_uranus*animationMatrixUranus, GL_TRIANGLES);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[11].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,true);
		
		RenderScene(&neptuneGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_neptune*animationMatrixNeptune, GL_TRIANGLES);
		//-----------------------------------------------------------
		glBindTexture(GL_TEXTURE_2D,images[4].textureID);
		
		glUseProgram(program);
		glUniform1i(shading_loc,false);
		
		RenderScene(&starGeometry, program, vec3(0.6, 0.7, 0.8), &cam, perspectiveMatrix, transistionMatrix_sun, GL_TRIANGLES);
		
		if(startFlag==0){
			timeElapsed += 0.01f*speed;
		}
		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	// clean up allocated resources before exit
	DestroyGeometry(&geometry);
	glUseProgram(0);
	glDeleteProgram(program);
	glfwDestroyWindow(window);
	glfwTerminate();

	cout << "Goodbye!" << endl;
	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "OpenGL ERROR:  ";
		switch (flag) {
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM" << endl; break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE" << endl; break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION" << endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY" << endl; break;
		default:
			cout << "[unknown error code]" << endl;
		}
		error = true;
	}
	return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
	string source;

	ifstream input(filename.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	}
	else {
		cout << "ERROR: Could not load shader source from file "
			<< filename << endl;
	}

	return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
	// allocate shader object name
	GLuint shaderObject = glCreateShader(shaderType);

	// try compiling the source as a shader of the given type
	const GLchar *source_ptr = source.c_str();
	glShaderSource(shaderObject, 1, &source_ptr, 0);
	glCompileShader(shaderObject);

	// retrieve compile status
	GLint status;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
		cout << "ERROR compiling shader:" << endl << endl;
		cout << source << endl;
		cout << info << endl;
	}

	return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	// allocate program object name
	GLuint programObject = glCreateProgram();

	// attach provided shader objects to this program
	if (vertexShader)   glAttachShader(programObject, vertexShader);
	if (fragmentShader) glAttachShader(programObject, fragmentShader);

	// try linking the program with given attachments
	glLinkProgram(programObject);

	// retrieve link status
	GLint status;
	glGetProgramiv(programObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
		cout << "ERROR linking shader program:" << endl;
		cout << info << endl;
	}

	return programObject;
}
