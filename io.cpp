#include <iostream>
#include <sstream>

#include <png.h>
#include <zip.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include <cstdlib>
#include <sys/stat.h> // check that file exists
#include "io.h"

// dropped from libpng 1.4
#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL
#define png_voidp_NULL NULL

void png_zip_read(png_structp png, png_bytep data, png_size_t size);

bool fileExists(const char* filename)
{
    struct stat buf;
    return (stat(filename, &buf) != -1) ? true : false;
};



int loadPngImageFromZip(const char* name_zipped_file, const char *filename, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData)
{
    struct zip_file* zipped_file;
    struct zip* z;
    int error;

    z = zip_open(name_zipped_file, 0, &error);
    if (z == NULL)
    {
        std::cerr << "Error: can't open file: " << name_zipped_file << std::endl << "error code: " << error << std::endl;
        return EXIT_FAILURE;
    };

    int idx=zip_name_locate(z,filename,0);
    if (idx < 0)
    {
        std::cerr << "ERROR: file: " << filename << " not found" << std::endl;
        zip_close(z);
        return EXIT_FAILURE;
    } else
    {
        zipped_file=zip_fopen_index(z,idx, 0);
        if (zipped_file != NULL)
        {

            //std::cout << "open file: " << filename << std::endl;

            ///***//-> VALIDATE <-///***/
            int sigLength = 8;
            int pngCheck=0;
            png_byte header[sigLength];

            zip_fread(zipped_file, header, sigLength);

            pngCheck = png_sig_cmp(header, 0, sigLength);
            if (pngCheck != 0)
            {
                std::cerr << "ERROR: file: " << filename << " is not a valid png file!"
                    << std::endl <<  "error code:" << pngCheck <<std::endl;
                zip_fclose(zipped_file);
                zip_close(z);
                return EXIT_FAILURE;
             } //else
                //std::cerr << "was found  png file: " << filename  << std::endl;

            ///***//-> CREATE STRUCTURS <-//***/
            png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if (png_ptr == NULL)
            {
                std::cerr << "ERROR: file: " << filename << " png_create_read_struct failed!" << std::endl;
                zip_fclose(zipped_file);
                zip_close(z);
                return EXIT_FAILURE;
            }

            png_infop info_ptr = png_create_info_struct(png_ptr);
            if (info_ptr == NULL)
            {
                std::cerr << "ERROR: file: " << filename << " png_create_info_struct failed!" << std::endl;
                png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
                zip_fclose(zipped_file);
                zip_close(z);
                return EXIT_FAILURE;
            }
            png_infop end_info = png_create_info_struct(png_ptr);
            if (end_info == NULL)
            {
                std::cerr << "ERROR: file: " << filename << " png_create_info_struct failed!" << std::endl;
                png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
                zip_fclose(zipped_file);
                zip_close(z);
                return EXIT_FAILURE;
            }

            ///***/-> SET ERRORS HANDLED  <-///***/
            if (setjmp(png_jmpbuf(png_ptr)))
            {
                std::cerr << "file: " << filename << " Error during init_io!" << std::endl;
                png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
                zip_fclose(zipped_file);
                zip_close(z);
                return EXIT_FAILURE;
            }

            /// png_init_io(png_ptr, fp)
            png_set_read_fn(png_ptr, zipped_file, png_zip_read);
            // set offset
            png_set_sig_bytes(png_ptr, sigLength);

            png_read_png(png_ptr, info_ptr,
                         PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, png_voidp_NULL);

            outWidth = png_get_image_width(png_ptr,info_ptr);
            outHeight = png_get_image_height(png_ptr,info_ptr);

            // check Alpha channel
            switch (png_get_color_type(png_ptr,info_ptr))
            {
                case PNG_COLOR_TYPE_RGBA:
                    outHasAlpha = true;
                    break;
                case PNG_COLOR_TYPE_RGB:
                    outHasAlpha = false;
                    break;
                default:
                    std::cout << "Color type " << png_get_color_type(png_ptr,info_ptr) << " not supported" << std::endl;
                    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
                    zip_fclose(zipped_file);
                    zip_close(z);
                    return EXIT_FAILURE;
            }

            unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
            *outData = (unsigned char*) malloc(row_bytes * outHeight);

            png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

            for (int i = 0; i < outHeight; i++) {
                // note that png is ordered top to bottom, but OpenGL expect it bottom to top
                // so the order or swapped
                memcpy(*outData+(row_bytes * (outHeight-1-i)), row_pointers[i], row_bytes);
            }


            //std::cout << "width: " << outWidth << " hieght: " << outHeight << " alpha: " << outHasAlpha <<  std::endl;

            png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
            zip_fclose(zipped_file);

        } else
        {
            std::cerr << "ERROR: can't open file' " << filename << std::endl;
            zip_close(z);
            return EXIT_FAILURE;
        }
    }

    zip_close(z);
    return EXIT_SUCCESS;
}

void png_zip_read(png_structp png, png_bytep data, png_size_t size)
{
    zip_file* zfp = (zip_file*)(png_get_io_ptr(png));
    zip_fread(zfp, data, size);
}

// загрузка png файла
bool loadPngImage(char *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData) {
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    FILE *fp;

    if ((fp = fopen(name, "rb")) == NULL)
        return false;


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


    if (setjmp(png_jmpbuf(png_ptr))) {

        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(fp);

        return false;
    }


    png_init_io(png_ptr, fp);

    png_set_sig_bytes(png_ptr, sig_read);

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


    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    fclose(fp);


    return true;
}

int loadTextFromZip(const char* name_zipped_file, const char *filename, std::stringstream &ssBuf)
{
    struct zip_file* zipped_file;
    struct zip* z;
    int error;

    z = zip_open(name_zipped_file, 0, &error);
        if (z == NULL)
        {
        std::cerr << "Error: can't open file: " << name_zipped_file << std::endl << "error code: " << error << std::endl;
        return EXIT_FAILURE;
        };

    int idx=zip_name_locate(z,filename,0);
    if (idx < 0)
    {
        std::cerr << "ERROR: file: " << filename << " not found" << std::endl;
        zip_close(z);
        return EXIT_FAILURE;
    } else
    {
        zipped_file=zip_fopen_index(z,idx, 0);
        if (zipped_file != NULL)
        {
            int r; char* bf;
            while ((r =zip_fread(zipped_file, bf, sizeof(bf))) > 0 )
            {
                ssBuf << bf;
            }
            zip_fclose(zipped_file);
        } else
        {
            std::cerr << "ERROR: can't open file' " << filename << std::endl;
            zip_close(z);
            return EXIT_FAILURE;
        }
    }

    zip_close(z);
    return EXIT_SUCCESS;
}
