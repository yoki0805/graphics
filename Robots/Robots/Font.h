#pragma once
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <GL/freeglut.h>

class Font
{
protected:
	bool init;
	char buffer[128];
	GLuint fontOffset;

	void makeRasterFont();
	void spitLetters(char* str);

public:
	void fixFonts(float x, float y, char* str);
	void puts(int x, int y, char* str);
	void printf(int x, int y, char* format, ...);

	Font() : init(false) {}
	// ~Font() { glDeleteLists(fontOffset, 128); }
};