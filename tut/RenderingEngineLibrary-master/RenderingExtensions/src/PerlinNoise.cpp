#include "PerlinNoise.h"
#include "TextureMat.h"

using namespace glm;

namespace renderlib {

enum {
	VP_MATRIX_LOCATION = TextureMat::COUNT,
	M_MATRIX_LOCATION,
	COUNT
};

static vector<pair<GLenum, string>> shaders{
	{ GL_VERTEX_SHADER, "shaders/implicitLines.vert" },
	{ GL_FRAGMENT_SHADER, "shaders/implicitLines.frag" }
};

PerlinNoiseShader2D::PerlinNoiseShader2D(map<GLenum, string> defines) {
	createProgram(defines);
	calculateUniformLocations();
}

bool PerlinNoiseShader2D::createProgram(map<GLenum, string> defines) {
	programID = createGLProgram(shaders, defines);

	return programID != 0;
}

void PerlinNoiseShader2D::calculateUniformLocations() {
	glUseProgram(programID);

	//Material uniforms
	uniformLocations.resize(COUNT);
	uniformLocations[TextureMat::TEXTURE_LOCATION] =
		glGetUniformLocation(programID, "colorTexture");

	//Other uniforms
	uniformLocations[VP_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"view_projection_matrix");
	uniformLocations[M_MATRIX_LOCATION] = glGetUniformLocation(programID,
		"model_matrix");
}

void PerlinNoiseShader2D::loadUniforms(const mat4& vp_matrix, const mat4& m_matrix) {
	glUniformMatrix4fv(uniformLocations[VP_MATRIX_LOCATION], 1, false, &vp_matrix[0][0]);
	glUniformMatrix4fv(uniformLocations[M_MATRIX_LOCATION], 1, false, &m_matrix[0][0]);
}

void PerlinNoiseShader2D::draw(const Camera &cam, const Drawable &obj) {
	glUseProgram(programID);
	loadUniforms(cam.getProjectionMatrix()*cam.getCameraMatrix(), obj.getTransform());
	obj.loadUniforms(TextureMat::id, &uniformLocations[0]);

	obj.getGeometry().drawGeometry();
	glUseProgram(0);
}

}