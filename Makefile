CC		= g++ -std=c++11
C		= cpp

CFLAGS		= -g

ifeq ("$(shell uname)", "Darwin")
  LDFLAGS     = -framework Foundation -framework GLUT -framework OpenGL -lOpenImageIO -lm
else
  ifeq ("$(shell uname)", "Linux")
    LDFLAGS   = -L /usr/lib64/ -lglut -lGL -lGLU -lOpenImageIO -lm
  endif
endif

PROJECT		= image_processing

${PROJECT}:	${PROJECT}.o Image.o
	${CC} ${CFLAGS} -o ${PROJECT} ${PROJECT}.o Image.o ${LDFLAGS}

Image.o: Image.${C}
	${CC} ${CFLAGS} -c Image.${C}

${PROJECT}.o:	${PROJECT}.${C}
	${CC} ${CFLAGS} -c ${PROJECT}.${C}

clean:
	rm -f core.* *.o *~ ${PROJECT}
