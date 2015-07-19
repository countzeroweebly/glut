#include <GL/gl.h>
#include <GL/glu.h>

bool loadPngImage(char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);
int loadPngImageFromZip(const char* zip_name, const char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);
int loadTextFromZip(const char* name_zipped_file, const char *filename, std::stringstream &ssBuf);
bool fileExists(const char* filename);
