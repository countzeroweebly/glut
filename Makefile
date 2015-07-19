CC=g++
CFLAGS=--std=c++11 -Wall -g -I .
OBJ=main.o io.o core.o
TARGET=myGL
.PHONY: all clean

%.o:	%.cpp
	$(CC) -c -o $@ $< $(CFLAGS)
	
all:	$(OBJ)
	$(CC)  -o $(TARGET) -lGL -lGLU -lglut -lpng -lzip $^ $(CFLAGS)

clean:
	@rm -v $(OBJ) $(TARGET)
