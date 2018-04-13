#include "TextureCreation.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace std;

namespace renderlib {

Texture createTexture2D(string filename, TextureManager *manager) {
	int width, height, comp;
	unsigned char *image = stbi_load(filename.c_str(), 
		&width, &height, &comp, 0);

	//Calculate alignment
	int alignment = 4 - (width * comp) % 4;
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	if ((image == nullptr) || (comp > 4)) {
		cout << "Texture invalid - " << filename.c_str() << endl;
		return Texture();
	}

	const GLenum formats[4] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };

	TexInfo info(GL_TEXTURE_2D, { width, height }, 0, 
		formats[comp-1], formats[comp-1], GL_UNSIGNED_BYTE);
		
	Texture tex = createTexture2D(info, manager, image);
	stbi_image_free(image);
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);		//Return to default

	return tex;
}

Texture createTexture2D(TexInfo info, unsigned char *data) {
	return createTexture2D(info, nullptr, data);
}

Texture createTexture2D(int width, int height, TextureManager *manager) {
	return createTexture2D(TexInfo(GL_TEXTURE_2D, { width, height }, 0,
		GL_RGBA, GL_RGBA8, GL_FLOAT), manager);
}

Texture createTexture2D(TexInfo info, TextureManager *manager, 
	unsigned char *data) {
	GLuint texID;
	glGenTextures(1, &texID);
	glActiveTexture(NO_ACTIVE_TEXTURE);	//Bind to avoid disturbing active units
	glBindTexture(GL_TEXTURE_2D, texID);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, info.level, info.internalFormat, info.dimensions[0],
		info.dimensions[1], 0, info.format, GL_UNSIGNED_BYTE, data);

	return Texture(texID, info, manager);
}

Texture createDepthTexture(int width, int height, TextureManager *manager) {
	return createTexture2D(TexInfo(GL_TEXTURE_2D, { width, height }, 0,
		GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT), manager);
}

Texture createDepthTextureMulti(int width, int height, TextureManager *manager, size_t num_samples) {
	return createTexture2DMulti(TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { width, height }, 0, GL_NONE, GL_DEPTH_COMPONENT32F, GL_NONE), manager, num_samples);
}

Framebuffer createNewFramebuffer(int width, int height) {
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	
	return Framebuffer(width, height, fbo);
}

Texture createTexture2DMulti(int width, int height, TextureManager *manager, size_t num_samples) {
	return createTexture2DMulti(TexInfo(GL_TEXTURE_2D_MULTISAMPLE, { width, height }, 0, GL_NONE, GL_RGBA8, GL_NONE), manager, num_samples);
}

Texture createTexture2DMulti(TexInfo info, TextureManager *manager, size_t num_samples) {
	GLuint texID;
	glGenTextures(1, &texID);
	glActiveTexture(NO_ACTIVE_TEXTURE);	//Bind to avoid disturbing active units
	glBindTexture(info.target, texID);
	glTexParameteri(info.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(info.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(info.target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(info.target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2DMultisample(info.target, num_samples, info.internalFormat, info.dimensions[0], info.dimensions[1], GL_FALSE);

	return Texture(texID, info, manager);
}

}
