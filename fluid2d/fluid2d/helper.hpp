#ifndef __HELPER_HPP__
#define __HELPER_HPP__


#include <algorithm>
#include <sfml/opengl.hpp>

namespace helper
{


// set a value between lower to upper
template <typename T>
inline T Clip(const T& n, const T& lower, const T& upper)
{
	return std::max(lower, std::min(n, upper));
}


// load a texture for opengl
GLuint LoadTexture(sf::String filename, GLint minFilter, GLint magFilter)
{
	sf::Image img;
	if (!img.loadFromFile(filename))
		return 0;

	// flip the image
	img.flipVertically();

	// create a texture
	GLuint texid = 0;
	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img.getSize().x, img.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.getPixelsPtr());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);

	// clamp to edge
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return texid;
}


}

#endif __HELPER_HPP__