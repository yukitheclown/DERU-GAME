CC=mingw32-gcc

# debugging on, source compiled into exe currently

CFLAGS  = -Wall -Wextra -g -Wl,-subsystem,windows
LFLAGS = -g

# CFLAGS   = -m64 -pipe

SDL_LDFLAGS = $(shell sdl2-config --libs)

GLEW_LDFLAGS = $(shell pkg-config glew --static --libs)

# OPENAL_LDFLAGS = $(shell pkg-config openal freealut --static --libs)

# VORBIS_LDFLAGS = $(shell pkg-config vorbisfile --static --libs)

# LIBS= -static-libgcc $(SDL_LDFLAGS) $(GLEW_LDFLAGS) -lfreetype -lpng -Wl,-Bstatic -lz -lm
FREETYPELIBS = -lfreetype
GLEWLIBS = -lglew32 -lopengl32 -mwindows
SDLLIBS = -lsdl2main -lsdl2 
LIBS = -L. -lthoth -lm -lmingw32 $(GLEWLIBS) $(SDLLIBS) $(FREETYPELIBS)  -lz -lpng

# LIBS = -Wl,-Bdynamic -lmingw32 -lSDL2main -lSDL2 -lopengl32 -lglew32 -Wl,-Bstatic -lpng16 -lz -lm -Llib/freetype/lib/win_cb/ -lfreetype

SOURCES=main.c image_loader.c math.c shaders.c window.c mesh.c \
bounding_box.c octree.c world.c skybox.c hash_table.c object.c deflate.c log.c memory.c


OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main.exe

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) $(LIBS) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

# remember readelf -d main
# then just sudo find / -name 'whaever.so.0'