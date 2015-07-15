#include <iostream>

#include <png.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

bool loadPngImage(char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);
