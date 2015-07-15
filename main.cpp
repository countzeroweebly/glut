#include <GL/freeglut.h>
#include "core.h"

int main(int argc, char** argv) 
{
        glutInit (&argc,argv); // инициализация glut
        glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
        glutInitWindowSize( 800,600);
        int glWin=glutCreateWindow("OpenGL 1.x Demo Cube");
        init(); // подготовка к рисованию
        glutDisplayFunc(display); // рисование
        glutReshapeFunc(reshape); // действия при изменении окна
        glutKeyboardFunc(key);   // обработка нажатий клавиш
        glutTimerFunc(1000, mytime, 0);
        glutIdleFunc(display); // анимация
        glutMainLoop(); // главный цикл glut

        return 0;
}
