#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>
#include <future>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include "core.h"

ENGINE core;

// настройки тумана
bool   gp;                      // G Pressed? ( New )
GLuint filter;                      // Which Filter To Use
GLuint fogMode[]= { GL_EXP, GL_EXP2, GL_LINEAR };   // Storage For Three Types Of Fog
GLuint fogfilter= 0;                    // Which Fog To Use
GLfloat fogColor[4]= {0.5f, 0.5f, 0.5f, 1.0f};      // Fog Color

int fps=0;
int counter=0;
float angle=0;
float elapsed_time,current_time;
float last_time=0.0f;
bool flag=false;

GLuint textureId;
GLubyte *textureImage;


// вывод текста на холст OpenGL посредством GLUT
void draw_string_bitmap(void *font, const char* string)
{
    while (*string)
        glutBitmapCharacter(font, *string++);
}

void ENGINE::init(const char* rs_filename)
{
    glClearColor(0.0, 0.0, 0.0, 1.0); // цвет очистки буферов - черный
    glEnable(GL_DEPTH_TEST); // включаем сортировку по глубине
    glShadeModel(GL_SMOOTH); // режим закрашивания: интерполируемые значения
    // ставим источник света
    float lightAmb[4] = {0.0, 0.0, 1.0, 1}; // фоновый цвет синий
    float lightDif[4] = {0.8, 0.8, 0.8, 1}; // отраженный цвет красный
    float lightPos[4] = {0.0, 3.0, 3.0, 1};  // положение в пространстве
    glLightfv (GL_LIGHT0, GL_SPECULAR, lightAmb);
    glLightfv (GL_LIGHT0, GL_DIFFUSE, lightDif);
    glLightfv (GL_LIGHT0, GL_POSITION, lightPos);
//        glEnable (GL_LIGHT0 ); // включаем источник света
//        glEnable (GL_LIGHTING); // включаем освещение

    glEnable(GL_BLEND); // включаем прозрачность
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int width, height; // ширина и высота текстуры
    bool hasAlpha;     // наличие альфа-канала

    char filename[] = "texture.png"; // наша текстура
    //char zipname[] = "rs.zip"; // наша текстура
    //bool success = loadPngImage(filename, width, height, hasAlpha, &textureImage);
    int success=loadPngImageFromZip(rs_filename, filename, width, height, hasAlpha, &textureImage);

    if (success != EXIT_SUCCESS) {
            std::cerr << "Невозможно прочитать PNG файл" << std::endl;
            return;
    }
    std::cerr << "Изображение загружено: " << width << " " << height
                            << " наличие  альфа-канала " << hasAlpha << std::endl;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // формат хранения текстуры в памяти
    // добавляем нашу текстуру
    glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3, width,
                    height, 0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, textureImage);
    // параметры фильтрации текстуры
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void ENGINE::draw()
{
    flag=true; // мы еще рисуем

    // вычисляем время между отрисовкой двух кадров и по этому
    // и используем это значение величину смещения
    current_time=glutGet(GLUT_ELAPSED_TIME) / 10;
    elapsed_time = current_time - last_time;
    angle = ( angle >= 360.0f ) ? 0.0f : angle+elapsed_time;
    last_time=current_time;

    // ---- ОТРИСОВКА 3D ------------------
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

    gluPerspective( 60, GLfloat(1.33f), 1, 20 );
    // установка вектора обзора
    // (0,0,3) камера; (0,0,0) центр сцены, верхом будет ось Y
    gluLookAt( 0, 0, 3, 0, 0, 0, 0, 1, 0 );

    glEnable (GL_LIGHT0 ); // включаем источник света
    glEnable (GL_LIGHTING); // включаем освещение
    glEnable(GL_TEXTURE_2D); // включаем использование текстур

    glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT); // очистка рабочих буферов
    glPushMatrix(); // начинаем матричные преобразования
    glTranslatef(0,0,-1); // смещение
    glRotatef(angle, 1, 1, 0); // вращение

    glClear(GL_COLOR_BUFFER_BIT); // очистка рабочих буферов
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3,GL_FLOAT,0,cube);
    glTexCoordPointer(2,GL_FLOAT,0,cubeTex);
    glDrawArrays(GL_QUADS,0,24);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopMatrix(); // завершаем матричные преобразования

    // ------ ОТРИСОВКА 2D ----------------------------------
    glDisable(GL_TEXTURE_2D); // включаем использование текстур
    glDisable (GL_LIGHT0 ); // выключаем источник света
    glDisable (GL_LIGHTING); // выключаем освещение
    glColor3f(1.0f,1.0f,1.0f);
    glLoadIdentity();
    gluOrtho2D(0, 400, 0, 300);
    glRasterPos2f(300, 280);

    std::string s="fps: " +std::to_string(counter);
    const char *cstr = s.c_str();
    draw_string_bitmap(GLUT_BITMAP_HELVETICA_18, cstr);


    glutSwapBuffers(); // выводим на экран
    flag=false;  // все, нарисовали!
    fps++; // плюс один кадр

}

void ENGINE::reshape(int w, int h)
{
    // область видимости в соответствии с новыми координатами
    glViewport( 0, 0, (GLsizei)w, (GLsizei)h);
    // загружаем единичные матрицы в стеки MODELVIEW, PROJECTION
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // установка перспективы: угол обзора 60 градусов, аспект,
    // глубина пространства 20 единиц
    gluPerspective( 60, GLfloat(1.33f), 1, 20 );
    // установка вектора обзора
    // (0,0,3) камера; (0,0,0) центр сцены, верхом будет ось Y
    gluLookAt( 0, 0, 3, 0, 0, 0, 0, 1, 0 );

    xWidth=w;
    xHeight=h;

}

void key(unsigned char key, int  x, int y)
{
        if ( key == 27 || key == 'q' || key == 'Q' ) // ESC or Q(uit)
        {
                glutLeaveMainLoop();
        };

}

void ENGINE::specialInput(int key, int x, int y)
{
    if (key == GLUT_KEY_F12)
    {
        std::future<int> f1 = std::async(&ENGINE::makeScreenshot, this, "screenshot.png", xWidth, xHeight);
        auto res1=f1.get();
    }
}

void mytime(int value)
{
    counter=fps;
    fps=0; // обнуление счетчика
    glutTimerFunc(1000, mytime, 0); // следующий вызов
}

void init(const char* rs_filename)
{
    core.init(rs_filename);
}

void reshape(int w, int h)
{
    core.reshape(w,h);
}

void specialInput(int key, int x, int y)
{
    core.specialInput(key, x, y);
}

void display()
{
    core.draw();
}
