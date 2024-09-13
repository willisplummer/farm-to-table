COMPILER = clang

SOURCE_LIBS = -Ilib/

FLAGS = --debug \
				-Werror \
				-Weverything \
				-Wno-declaration-after-statement \
				-Wno-unused-parameter \
				-Wno-missing-prototypes \
				-Wno-covered-switch-default \
				-Wno-padded \
				-Wno-cast-align \
				-Wno-cast-qual \
				-Wno-disabled-macro-expansion \
				-Wno-double-promotion \
				-Wno-implicit-int-conversion \
				-Wno-missing-prototypes \
				-Wno-sign-conversion \
				-Wno-unused-macros \
				-Wno-tautological-unsigned-enum-zero-compare \
				-Wno-float-equal \
				-Wno-poison-system-directories

MAC_OPT = -Llib/ \
					-framework CoreVideo \
					-framework IOKit \
					-framework Cocoa \
					-framework GLUT \
					-framework OpenGL lib/libraylib.a \

MAC_OUT = -o "bin/build_mac"

CFILES = src/*.c

build_mac:
	$(COMPILER) $(CFILES) $(SOURCE_LIBS) $(MAC_OUT) $(MAC_OPT) ${FLAGS}
