#include <GL/freeglut.h>
#include <string>

#include "io.h"
#include "core.h"



int main(int argc, char *argv[])
{
        char* res_file="rs.zip";

        for(int i=1; i < argc; i++)
        {
            if (std::string(argv[i]) == "-r" && (i+1)< argc && fileExists(argv[i+1]))
            {
                res_file=argv[i+1];
            }
        }

        glutInit (&argc,argv);
        glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB|GLUT_DEPTH);
        glutInitWindowSize( 800,600);
        int glWin=glutCreateWindow("demo 06: unpacking PNG texture from ZIP");
        init(res_file);
        glutDisplayFunc(display);
        glutReshapeFunc(reshape);
        glutKeyboardFunc(key);
        glutTimerFunc(1000, mytime, 0);
        glutIdleFunc(display);
        glutMainLoop();

        return 0;
}
