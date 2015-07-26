#include <png.h>
#include <zip.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

class DATA {
protected:
    float cube[72]={
            // Front face
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            // Back face
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            // Top face
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
            // Bottom face
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            // Right face
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            // Left face
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f};

     // текстурные координаты куба
     float cubeTex[48]={
     // Front face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Back Face
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            // Top Face
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            // Bottom Face
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f,
            // Right face
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            0.0f, 0.0f,
            // Left Face
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
    };


};

class ENGINE : private DATA {
private:

    int xWidth;
    int xHeight;

    int makeScreenshot(const char* filename, int width, int height);
    int loadPngImageFromZip(const char* name_zipped_file, const char *filename, int &outWidth,
                            int &outHeight, bool &outHasAlpha, GLubyte **outData);
    bool loadPngImage(char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);
    int loadTextFromZip(const char* name_zipped_file, const char *filename, std::stringstream &ssBuf);

public:
    void init(const char*);
    void draw();
    void reshape(int, int);
    void specialInput(int, int, int);
};


void mytime(int value);
void key(unsigned char key, int  x, int y);
void specialInput(int key, int x, int y);
void reshape(int w, int h);
void display();
void init(const char* rs_filename);

