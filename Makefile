
ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
endif
endif

TARGET_NAME := heightmap

ifeq ($(platform), unix)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
   GL_LIB := -lGL
   INCFLAGS += -I.
else ifeq ($(platform), osx)
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
   GL_LIB := -framework OpenGL
   INCFLAGS += -I.
else
   TARGET := $(TARGET_NAME)_libretro.dll
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=link.T -Wl,--no-undefined
   GL_LIB := -lopengl32
   INCFLAGS = -I. -Iinclude/win32
endif

CXXFLAGS += $(INCFLAGS)

ifeq ($(DEBUG), 1)
   CXXFLAGS += -O0 -g -DGL_DEBUG
   CFLAGS += -O0 -g
else
   CXXFLAGS += -O3
   CFLAGS += -O3
endif

CXXFLAGS += -std=gnu++11 -Wall -pedantic $(fpic) -DHAVE_ZIP_DEFLATE $(shell pkg-config assimp --cflags)
CFLAGS += -std=gnu99 -Wall -pedantic $(fpic) -DHAVE_ZIP_DEFLATE

SOURCES := $(wildcard *.cpp) $(wildcard */*.cpp)
CSOURCES := $(wildcard *.c) $(wildcard */*.c)
OBJECTS := $(SOURCES:.cpp=.o) $(CSOURCES:.c=.o)
LIBS += $(GL_LIB) $(shell pkg-config assimp --libs)

all: $(TARGET)

HEADERS := $(wildcard *.hpp) $(wildcard *.h) $(wildcard */*.hpp) $(wildcard */*.h)

$(TARGET): $(OBJECTS)
	$(CXX) $(fpic) $(SHARED) $(INCLUDES) -o $@ $(OBJECTS) $(LIBS) -lm -lz

%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean

