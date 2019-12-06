CC=g++ -std=c++17
PKGS=sdl2
PKG_C_FLAGS=pkg-config --cflags $(PKGS)
PKG_L_FLAGS=pkg-config --libs $(PKGS)
CFLAGS=`$(PKG_C_FLAGS)` -c
LDFLAGS=`$(PKG_L_FLAGS)` -lSDL2_image
SOURCES=main.cpp conway.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=game

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	@$(CC) -o $@ $(OBJECTS) $(LDFLAGS)
	@echo "LINK	$@"

.cpp.o:
	@$(CC) $(CFLAGS) $< -o $@
	@echo "C++	$<"

clean:
	rm -f $(OBJECTS)
	rm -f $(EXECUTABLE)
