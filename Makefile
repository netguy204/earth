OBJS=\
	utils.o main.o glew.o SDLMain.o stb_image.o

CFLAGS+=
LDFLAGS+=-framework OpenGL -framework SDL -framework Cocoa

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $<

main: $(OBJS)
	g++ -o $@ $(OBJS) $(LDFLAGS)

clean:
	rm -rf *.o main
