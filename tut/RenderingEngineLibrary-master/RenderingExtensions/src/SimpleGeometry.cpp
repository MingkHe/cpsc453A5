#include "SimpleGeometry.h"
#include "glSupport.h"

///////////////////
// Simple geometry
///////////////////

namespace renderlib {

SimpleGeometry::SimpleGeometry(GLenum mode) :bufferSize(0), mode(mode)
{
	initializeVAO();
}

SimpleGeometry::SimpleGeometry(vec3 *positions, size_t elementNum, GLenum mode) : mode(mode)
{
	initializeVAO();

	loadGeometry(positions, elementNum);
}

bool SimpleGeometry::initializeVAO() {
	checkGLErrors("EnterVAO");

	glGenVertexArrays(1, &vao);
	glGenBuffers(COUNT, vbo);

	glBindVertexArray(vao);

	glEnableVertexAttribArray(ATTRIB_LOCATION::POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::POSITION,			//Attribute
		3,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(vec3),		//Stride
		(void*)0			//Offset
	);

	glBindVertexArray(0);

	return !checkGLErrors("SimpleGeometry::initVao");
}

void SimpleGeometry::loadGeometry(vec3 *positions, size_t elementNum)
{
	loadPositions(positions, elementNum);

	bufferSize = elementNum;
}

void SimpleGeometry::loadPositions(vec3 *positions, size_t numPositions, GLenum usage)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(vec3), positions, usage);

	checkGLErrors("SimpleGeometry::loadPositions");
}

void SimpleGeometry::drawGeometry() const {
	bindGeometry();
	glDrawArrays(mode, 0, bufferSize);
}

void SimpleGeometry::bindGeometry() const
{
	glBindVertexArray(vao);
}

//////////////////////
// SimpleTexGeometry
//////////////////////

SimpleTexGeometry::SimpleTexGeometry(GLenum mode, GLenum type) :bufferSize(0), mode(mode), texCoordType(type)
{
	initializeVAO();
}

SimpleTexGeometry::SimpleTexGeometry(vec3 *positions, vec2 *texCoords, size_t elementNum, GLenum mode)
	: mode(mode), texCoordType(GL_FLOAT)
{
	initializeVAO();

	loadGeometry(positions, texCoords, elementNum);
}

SimpleTexGeometry::SimpleTexGeometry(vec3 *positions, ivec2 *texCoords, size_t elementNum, GLenum mode)
	: mode(mode), texCoordType(GL_INT)
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(COUNT, vbo);

	initializeVAO();

	loadGeometry(positions, texCoords, elementNum);
}

bool SimpleTexGeometry::initializeVAO() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(COUNT, vbo);

	glBindVertexArray(vao);

	glEnableVertexAttribArray(ATTRIB_LOCATION::POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::POSITION,	//Attribute
		3,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(vec3),		//Stride
		(void*)0			//Offset
	);

	size_t componentSize = (texCoordType == GL_FLOAT) ? sizeof(vec2) : sizeof(ivec2);
	glEnableVertexAttribArray(ATTRIB_LOCATION::TEX_COORD);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::TEX_COORD,					//Attribute
		2,					//# of components
		texCoordType,			//Type
		GL_FALSE,			//Normalized?
		componentSize,		//Stride
		(void*)0			//Offset
	);

	glBindVertexArray(0);

	return !checkGLErrors("initVao");
}

void SimpleTexGeometry::loadGeometry(vec3 *positions, vec2 *texCoords, size_t elementNum)
{
	if (texCoordType != GL_FLOAT) {
		texCoordType = GL_FLOAT;
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(COUNT, vbo);
		initializeVAO();
	}
	bindGeometry();
	loadPositions(positions, elementNum);
	loadTexCoords(texCoords, elementNum);

	bufferSize = elementNum;
	checkGLErrors("SimpleTexGeometry::loadGeometry");
}

void SimpleTexGeometry::loadGeometry(vec3 *positions, ivec2 *texCoords, size_t elementNum)
{
	if (texCoordType != GL_INT) {
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(COUNT, vbo);
		texCoordType = GL_INT;
		initializeVAO();
	}
	bindGeometry();
	loadPositions(positions, elementNum);
	loadTexCoords(texCoords, elementNum);

	bufferSize = elementNum;
	checkGLErrors("SimpleTexGeometry::loadGeometry");
}

void SimpleTexGeometry::loadPositions(vec3 *positions, size_t numPositions, GLenum usage)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(vec3), positions, usage);
}

void SimpleTexGeometry::loadTexCoords(vec2 *texCoords, size_t numTexCoords, GLenum usage)
{
	if (texCoordType != GL_FLOAT) {
		texCoordType = GL_FLOAT;
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(COUNT, vbo);
		initializeVAO();
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, numTexCoords * sizeof(vec2), texCoords, usage);
}

void SimpleTexGeometry::loadTexCoords(ivec2 *texCoords, size_t numTexCoords, GLenum usage)
{
	if (texCoordType != GL_INT) {
		texCoordType = GL_INT;
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(COUNT, vbo);
		initializeVAO();
	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, numTexCoords * sizeof(ivec2), texCoords, usage);
}

void SimpleTexGeometry::bindGeometry() const
{
	glBindVertexArray(vao);
}

void SimpleTexGeometry::drawGeometry() const {
	bindGeometry();
	glDrawArrays(mode, 0, bufferSize);
}


//////////////////////
// SimpleTexGeometryI
//////////////////////

SimpleTexGeometryI::SimpleTexGeometryI(GLenum mode) :bufferSize(0), mode(mode)
{
	initializeVAO();
}

SimpleTexGeometryI::SimpleTexGeometryI(vec3 *positions, ivec2 *texCoords, size_t elementNum, GLenum mode)
	: mode(mode)
{
	initializeVAO();

	loadGeometry(positions, texCoords, elementNum);
}

bool SimpleTexGeometryI::initializeVAO() {
	glGenVertexArrays(1, &vao);
	glGenBuffers(COUNT, vbo);

	glBindVertexArray(vao);

	glEnableVertexAttribArray(ATTRIB_LOCATION::POSITION);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::POSITION,	//Attribute
		3,					//# of components
		GL_FLOAT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(vec3),		//Stride
		(void*)0			//Offset
	);

	glEnableVertexAttribArray(ATTRIB_LOCATION::TEX_COORD);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD]);
	glVertexAttribPointer(
		ATTRIB_LOCATION::TEX_COORD,					//Attribute
		2,					//# of components
		GL_INT,			//Type
		GL_FALSE,			//Normalized?
		sizeof(GLint) * 2,		//Stride
		(void*)0			//Offset
	);

	glBindVertexArray(0);

	return !checkGLErrors("initVao");
}

void SimpleTexGeometryI::loadGeometry(vec3 *positions, ivec2 *texCoords, size_t elementNum)
{
	bindGeometry();
	loadPositions(positions, elementNum);
	loadTexCoords(texCoords, elementNum);

	bufferSize = elementNum;
}

void SimpleTexGeometryI::loadPositions(vec3 *positions, size_t numPositions, GLenum usage)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[POSITION]);
	glBufferData(GL_ARRAY_BUFFER, numPositions * sizeof(vec3), positions, usage);
}

void SimpleTexGeometryI::loadTexCoords(ivec2 *texCoords, size_t numTexCoords, GLenum usage)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[TEXCOORD]);
	glBufferData(GL_ARRAY_BUFFER, numTexCoords * sizeof(vec2), texCoords, usage);
}

void SimpleTexGeometryI::bindGeometry() const
{
	glBindVertexArray(vao);
}

void SimpleTexGeometryI::drawGeometry() const {
	bindGeometry();
	glDrawArrays(mode, 0, bufferSize);
}
}