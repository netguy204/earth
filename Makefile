OBJS=\
	utils.o stb_image.o shaders.o gl_headers.o matrix.o

CFLAGS+=-DBUILD_SDL

LDFLAGS+=

PLATFORM:=$(shell uname)
ifeq ($(PLATFORM), Darwin)
	OBJS+=SDLMain.o glew.o
	LDFLAGS+=-framework OpenGL -framework SDL -framework Cocoa
else
	LDFLAGS+=-lGL -lm -lutil `sdl-config --libs` -ldl -lGLEW
	CFLAGS+=`sdl-config --cflags`
endif

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<

main: main.o $(OBJS)
	g++ -o $@ main.o $(OBJS) $(LDFLAGS)

test_quat: test_quat.o $(OBJS)
	g++ -o $@ test_quat.o $(OBJS) $(LDFLAGS)

clean:
	rm -rf *.o main
