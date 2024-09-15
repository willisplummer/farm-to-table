COMPILER = clang

SOURCE_LIBS = -Ilib/

DEBUG = --debug

CFLAGS = -std=c99 \
				 -Wall \
				 -Wextra \
				 -Wno-missing-braces \
				 -Wunused-result \
				 -Wno-unused-parameter \
				 -D_DEFAULT_SOURCE

MAC_OPT = -Llib/ \
					-framework CoreVideo \
					-framework IOKit \
					-framework Cocoa \
					-framework GLUT \
					-framework OpenGL lib/libraylib.a \

MAC_OUT = -o "bin/build_mac"

CFILES = src/*.c

build_mac:
	$(COMPILER) $(CFILES) $(SOURCE_LIBS) $(MAC_OUT) $(MAC_OPT) ${CFLAGS}
