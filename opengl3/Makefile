
CPP = g++
OBJ = main.o
LIBS = sdl gl glew
BIN = opengl3
CXXFLAGS = `pkg-config --cflags --libs $(LIBS)` --std=c++0x -Wall -g -pipe

.PHONY: all clean

all: $(BIN)

clean:
	rm $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) -Dmain=SDL_main $(CXXFLAGS)

%.o: %.cpp %.hpp
	$(CPP) -c $(CXXFLAGS) $< -o $@


