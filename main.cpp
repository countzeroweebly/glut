/* Программа рисования вращающегося трехмерного куба
с логотипом Slackware в альфа-канале.
complie: g++ -o cube cube.cpp -lGL -lGLU -lglut -lpng -std=gnu++11
*/

#include <iostream>
#include <string>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>
#include <png.h>


// именованные константы убранные из libpng-1.4
#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#define png_voidp_NULL NULL


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

 // вершинные координаты куба
 float cube[]={
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
 float cubeTex[]={
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
        0.0f, 1.0f};

// вывод текста на холст OpenGL посредством GLUT
void draw_string_bitmap(void *font, const char* string) {
        while (*string)
                glutBitmapCharacter(font, *string++);

}

bool loadPngImage(char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);

void init() {
        glClearColor(0.0, 0.0, 0.0, 1.0); // цвет очистки буферов - черный
        glEnable(GL_DEPTH_TEST); // включаем сортировку по глубине
        glShadeModel(GL_SMOOTH); // режим закрашивания: интерполируемые значения
        // ставим источник света
        float lightAmb[4] = {0.0, 0.0, 1.0, 1}; // фоновый цвет синий
        float lightDif[4] = {1.0, 0.0, 0.0, 1}; // отраженный цвет красный
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

        char filename[] = "cube_face.png"; // наша текстура
        bool success = loadPngImage(filename, width, height, hasAlpha, &textureImage);

        if (!success) {
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

void display() {
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

void reshape(int w, int h)
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
}

void key(unsigned char key, int  x, int y) {
        // если была нажата клавиша ESC, Q, q, то завершаем программу
        if ( key == 27 || key == 'q' || key == 'Q' ) {
                glutLeaveMainLoop();
        }
}

void mytime(int value) {
        // выводим количество кадров которые успели нарисовать за 1 секунду
//        std::cerr << "fps= " << fps << std::endl;
 counter=fps;
        fps=0; // обнуление счетчика
        glutTimerFunc(1000, mytime, 0); // следующий вызов
}

int main(int argc, char** argv) {
        glutInit (&argc,argv); // инициализация glut
        // будем использовать двойную буферизацию, цветовое пространство RGB,
        // сортировку по глубине aka Z-буфер
        glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
        // размер окна пусть будет 800 на 600
        glutInitWindowSize( 800,600);
        // создаем окно, на выходе получаем идентификатор созданного окна
        int glWin=glutCreateWindow("Slackware Cube");
        init(); // подготовка к рисованию
        glutDisplayFunc(display); // рисование
        glutReshapeFunc(reshape); // действия при изменении окна
        glutKeyboardFunc(key);   // обработка нажатий клавиш
        // запускаем таймер м периодичностью 1 раз в секунду
        glutTimerFunc(1000, mytime, 0);
        glutIdleFunc(display); // анимация
        glutMainLoop(); // главный цикл glut

        return 0;
}

// загрузка png файла
bool loadPngImage(char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData) {
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    FILE *fp;

    if ((fp = fopen(name, "rb")) == NULL)
        return false;

    /* Create and initialize the png_struct with the desired error handler
     * functions.  If you want to use the default stderr and longjump method,
     * you can supply NULL for the last three parameters.  We also supply the
     * the compiler header file version, so that we know if the application
     * was compiled with a compatible version of the library.  REQUIRED
     */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
            NULL, NULL, NULL);

    if (png_ptr == NULL) {
        fclose(fp);
        return false;
    }

    // Allocate/initialize the memory for image information.  REQUIRED.
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return false;
    }

    /* Set error handling if you are using the setjmp/longjmp method
     * (this is the normal method of doing things with libpng).
     * REQUIRED unless you  set up your own error handlers in
     * the png_create_read_struct() earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(fp);
        /* If we get here, we had a problem reading the file */
        return false;
    }

    /* Set up the output control if you are using standard C streams */
    png_init_io(png_ptr, fp);

    /* If we have already read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);

    /*
     * If you have enough memory to read in the entire image at once, and
     * you need to specify only transforms that can be controlled
     * with one of the PNG_TRANSFORM_* bits (this presently excludes
     * dithering, filling, setting background, and doing gamma
     * adjustment), then you can read the entire image (including pixels)
     * into the info structure with this call
     *
     * PNG_TRANSFORM_STRIP_16 |
     * PNG_TRANSFORM_PACKING  forces 8 bit
     * PNG_TRANSFORM_EXPAND forces to
     *  expand a palette into RGB
     */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, png_voidp_NULL);

    outWidth = png_get_image_width(png_ptr,info_ptr); // ширина изображение
    outHeight = png_get_image_height(png_ptr,info_ptr); // высота изображения

   // проверка на наличие альфа-канала
    switch (png_get_color_type(png_ptr,info_ptr)) {
        case PNG_COLOR_TYPE_RGBA:
            outHasAlpha = true;
            break;
        case PNG_COLOR_TYPE_RGB:
            outHasAlpha = false;
            break;
        default:
            std::cout << "Color type " << png_get_color_type(png_ptr,info_ptr) << " not supported" << std::endl;
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return false;
    }
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    *outData = (unsigned char*) malloc(row_bytes * outHeight);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (int i = 0; i < outHeight; i++) {
        // note that png is ordered top to bottom, but OpenGL expect it bottom to top
        // so the order or swapped
        memcpy(*outData+(row_bytes * (outHeight-1-i)), row_pointers[i], row_bytes);
    }

    /* Clean up after the read, and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    /* Close the file */
    fclose(fp);

    /* да, мы сделали это */
    return true;
}