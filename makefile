# Based on https://www.cs.colby.edu/maxwell/courses/tutorials/maketutor/

.RECIPEPREFIX = >

CC=emcc

OUTDIR=out

LIBKIRK_INCLUDES := $(wildcard src/ext/libkirk/*.h)
LIBKIRK_SOURCES := $(wildcard src/ext/libkirk/*.c)

COMMON_INCLUDES := $(wildcard src/Common/*.h) $(wildcard src/Common/**/*.h)

CORE_INCLUDES := $(wildcard src/Core/*.h) $(wildcard src/Core/**/*.h)
CORE_SOURCES := $(wildcard src/Core/*.cpp) $(wildcard src/Core/**/*.cpp)

ROOT_INCLUDES := $(wildcard src/*.h)

TESTLIB_EXPORTED_FUNCTIONS := '["_Register_sceChnnlsv"]'
EXPORTED_RUNTIME_FUCTIONS := '["ccall","cwrap"]'

.PHONY: release debug clean

release: CFLAGS=-O3 -g0
release: kirk-engine

debug: CFLAGS=-O0 -g3
debug: kirk-engine

# See https://github.com/emscripten-core/emscripten/blob/main/src/settings.js for details about the various parameters set here

kirk-engine: $(LIBKIRK_INCLUDES) $(LIBKIRK_SOURCES) $(ROOT_INCLUDES) $(COMMON_INCLUDES) $(CORE_INCLUDES) $(CORE_SOURCES)
>$(CC) $(CFLAGS) -o $(OUTDIR)/kirk-engine.js $(LIBKIRK_SOURCES) $(CORE_SOURCES) -Isrc/ -s LLD_REPORT_UNDEFINED -s EXPORTED_FUNCTIONS=$(TESTLIB_EXPORTED_FUNCTIONS) -s EXPORTED_RUNTIME_METHODS=$(EXPORTED_RUNTIME_FUCTIONS) -s ENVIRONMENT='web,webview,node' -s EXPORT_ES6=1 -s MODULARIZE=1 -s USE_ES6_IMPORT_META=0 -s FILESYSTEM=0 -s EXPORT_NAME=createModule

clean:
>rm -f $(OUTDIR)/*