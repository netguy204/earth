OBJS=\
	utils.o main.o stb_image.o

CFLAGS+=
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

main: $(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -rf *.o main
