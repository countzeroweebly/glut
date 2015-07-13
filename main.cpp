/* Программа рисования вращающегося трехмерного куба
с логотипом Slackware в альфа-канале. Компиляция:
g++ -o ./имя_готовой_программы ./имя_этого_файла.cpp -lGL -lGLU -lglut -lpng
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
float angle=0;
float elapsed_time,current_time;
float last_time=0.0f;
bool flag=false;

GLuint textureId;
GLubyte *textureImage;

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
        glEnable (GL_LIGHT0 ); // включаем источник света
        glEnable (GL_LIGHTING); // включаем освещение

        glEnable(GL_BLEND); // включаем прозрачность
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        int width, height; // ширина и высота текстуры
        bool hasAlpha;     // наличие альфа-канала

        char filename[] = "image3.png"; // наша текстура
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
        glEnable(GL_TEXTURE_2D); // включаем использование текстур
}

void display() {
        flag=true; // мы еще рисуем

        // вычисляем время между отрисовкой двух кадров и по этому
        // и используем это значение величину смещения
        current_time=glutGet(GLUT_ELAPSED_TIME) / 10;
        elapsed_time = current_time - last_time;
        angle = ( angle >= 360.0f ) ? 0.0f : angle+elapsed_time;
        last_time=current_time;

        glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT); // очистка рабочих буферов
        glPushMatrix(); // начинаем матричные преобразования
        glTranslatef(0,0,-1); // смещение
        glRotatef(angle, 1, 1, 0); // вращение

        glClearColor(0.5f,0.5f,0.5f,1.0f);          // We'll Clear To The Color Of The Fog ( Modified )
        // рисуем туман
        glFogi(GL_FOG_MODE, fogMode[fogfilter]);        // Fog Mode
        glFogfv(GL_FOG_COLOR, fogColor);            // Set Fog Color
        glFogf(GL_FOG_DENSITY, 0.35f);              // How Dense Will The Fog Be
        glHint(GL_FOG_HINT, GL_DONT_CARE);          // Fog Hint Value
        glFogf(GL_FOG_START, 3.0f);             // Fog Start Depth
        glFogf(GL_FOG_END, -5.0f);               // Fog End Depth
        glEnable(GL_FOG);                   // Enables GL_FOG

        // рисуем куб с текстурой
        glBegin(GL_QUADS);
        // Front Face
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
        // Back Face
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
        // Top Face
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
        // Bottom Face
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Top Right Of The Texture and Quad
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Top Left Of The Texture and Quad
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
        // Right face
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);  // Bottom Right Of The Texture and Quad
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);  // Top Right Of The Texture and Quad
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);  // Top Left Of The Texture and Quad
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);  // Bottom Left Of The Texture and Quad
        // Left Face
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);  // Bottom Left Of The Texture and Quad
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);  // Bottom Right Of The Texture and Quad
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);  // Top Right Of The Texture and Quad
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);  // Top Left Of The Texture and Quad
        glEnd();
        glPopMatrix(); // завершаем матричные преобразования

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
        std::cerr << "fps= " << fps << std::endl;
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
        int glWin=glutCreateWindow("My program");
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