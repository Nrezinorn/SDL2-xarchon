CXX = g++
CXXFLAGS = -w -Wall -g -DDEBUG
LDFLAGS :=
PROG :=
CMD_CLEAN :=

PROG_EXE = SDLArchon

ifeq ($(OS),Windows_NT)
PROG = ${PROG_EXE}.exe
else
# I use .bin on non windows systems
PROG = ${PROG_EXE}.bin
endif

all:  $(PROG)

# https://gist.github.com/sighingnow/deee806603ec9274fd47
ifeq ($(OS),Windows_NT)
#@echo "Building for Windows"

# Set Extra Flags here, I use SDL2 mostly, so here are my MSYS2 windows presets
#CXXFLAGS += -fpermissive -IC:\\msys64\\mingw64\\include -LC:\\msys64\\mingw64\\lib -Dmain=SDL_main
#LDFLAGS += -lmingw32 -mwindows -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
CMD_CLEAN = del /f /s $(obj)
else
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
#@echo "Building for Linux"
CXXFLAGS += -fpermissive
# More SDL2 Related flags
#LDFLAGS += `/usr/bin/pkg-config --libs SDL2_mixer SDL2_image SDL2_ttf`
CMD_CLEAN = rm -f $(obj)
endif
ifeq ($(UNAME_S),Darwin)
#@echo "Building for Mac"
CXXFLAGS += -F/Library/Frameworks
LDFLAGS= -framework cocoa
# SDL2 Frameworks used on mac 1
# LDFLAGS= -framework SDL2 -framework SDL2_ttf -framework SDL2_mixer -framework SDL2_image -framework cocoa
CMD_CLEAN = rm -f $(obj)
endif
endif

#give up on object files in a folder of their own for now
OBJDIR=obj

src = $(wildcard src/*.cpp)
obj = $(src:.cpp=.o)
dep = $(obj:.o=.d)

#http://nuclear.mutantstargoat.com/articles/make/  
# buy this guy a beer or soda

$(PROG): $(obj)
#@echo "${CXXFLAGS} ${LDFLAGS}"
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	@${CMD_CLEAN}