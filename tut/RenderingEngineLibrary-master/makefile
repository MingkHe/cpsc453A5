CC=g++


CFLAGS=-c -std=c++11 -O3 -Wall -g
LINKFLAGS=-O3

#debug = true
ifdef debug
	CFLAGS +=-g
	LINKFLAGS += -flto
endif

INCDIR= -I./include

LIBDIR=-L/usr/X11R6 -L/usr/local/lib

LIBS=

OS_NAME:=$(shell uname -s)

ifeq ($(OS_NAME),Darwin)
	LIBS += `pkg-config --static --libs glfw3 gl`
	LIBS += \
	 -lglfw3 \
	 -lGLEW \
	 -framework Cocoa \
	 -framework OpenGL \
	 -framework IOKit \
	-framework CoreVideo
endif
ifeq ($(OS_NAME),Linux)
	LIBS += `pkg-config --static --libs glfw3 gl`
endif

CORE_DIR=./RenderingLibrary
EXTENSION_DIR=./RenderingExtensions
ENGINE_DIR=./RenderingEngine

CORE_SOURCES=$(wildcard $(CORE_DIR)/src/*cpp) $(wildcard $(CORE_DIR)/src/*c)
EXTENSION_SOURCES=$(wildcard $(EXTENSION_DIR)/src/*cpp)
ENGINE_SOURCES=$(wildcard $(ENGINE_DIR)/*cpp)

CORE_HEADERS=-I$(CORE_DIR)/headers
EXTENSION_HEADERS=-I$(EXTENSION_DIR)/headers $(CORE_HEADERS)
ENGINE_HEADERS=-I$(ENGINE_DIR) $(EXTENSION_HEADERS)

CORE_OBJS=$(addprefix $(CORE_DIR)/obj/,$(notdir $(CORE_SOURCES:.cpp=.o)))
EXTENSION_OBJS=$(addprefix $(EXTENSION_DIR)/obj/,$(notdir $(EXTENSION_SOURCES:.cpp=.o))) 
ENGINE_OBJS=$(addprefix $(ENGINE_DIR)/obj/,$(notdir $(ENGINE_SOURCES:.cpp=.o)))

.PHONY: default
default:  buildDirectories $(ENGINE_DIR)/renderingengine.out
	echo "Build completed"
ifeq ($(OS_NAME),Darwin)
$(ENGINE_DIR)/renderingengine.out: $(CORE_OBJS) $(EXTENSION_OBJS) $(ENGINE_OBJS)
	$(CC) $(LINK_FLAGS) $(ENGINE_OBJS) $(CORE_OBJS) $(EXTENSION_OBJS) -o $@ -L. $(LIBS)
endif
ifeq ($(OS_NAME),Linux)
$(ENGINE_DIR)/renderingengine.out: librenderingcore.a librenderingextensions.a $(ENGINE_OBJS)
	$(CC) $(LINK_FLAGS) $(ENGINE_OBJS) -o $@ -L. -lrenderingextensions -lrenderingcore $(LIBS)
endif

all: librenderingcore.a librenderingextensions.a $(ENGINE_DIR)/renderingengine.out 

$(CORE_DIR)/obj/%.o: $(CORE_DIR)/src/%.cpp 
	$(CC) -c $(CFLAGS) $(CORE_HEADERS) $(INCDIR) $< -o $@

$(EXTENSION_DIR)/obj/%.o: $(EXTENSION_DIR)/src/%.cpp
	$(CC) -c $(CFLAGS) $(EXTENSION_HEADERS) $(INCDIR) $< -o $@

$(ENGINE_DIR)/obj/%.o: $(ENGINE_DIR)/%.cpp
	$(CC) -c $(CFLAGS) $(ENGINE_HEADERS) $(INCDIR) $< -o $@

librenderingcore.a: $(CORE_OBJS)
	ar rvs $@ $(CORE_OBJS)

librenderingextensions.a: $(EXTENSION_OBJS)
	ar rvs $@ $(EXTENSION_OBJS)

.PHONY: buildDirectories
buildDirectories:
	mkdir -p $(CORE_DIR)/obj $(EXTENSION_DIR)/obj $(ENGINE_DIR)/obj

.PHONY: clean
clean:
	rm *.a $(ENGINE_DIR)/*.out $(CORE_DIR)/obj/*.o $(EXTENSION_DIR)/obj/*.o $(ENGINE_DIR)/obj/*.o
