#ifndef UTIL_H
#define UTIL_H

#include <glad/glad.h>
#include <iostream>

unsigned int createTexture(int width, int height, int internalFormat, int format, int type);
unsigned int createFBO(unsigned int texture);

#endif