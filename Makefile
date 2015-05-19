CFLAGS = -std=c99 -pedantic -pedantic-errors -Wall -g3 -O2 -D_ANSI_SOURCE_
CFLAGS += -fno-common \
			-Wall \
			-Wdeclaration-after-statement \
			-Wextra \
			-Wformat=2 \
			-Winit-self \
			-Winline \
			-Wpacked \
			-Wp,-D_FORTIFY_SOURCE=2 \
			-Wpointer-arith \
			-Wlarger-than-65500 \
			-Wmissing-declarations \
			-Wmissing-format-attribute \
			-Wmissing-noreturn \
			-Wmissing-prototypes \
			-Wnested-externs \
			-Wold-style-definition \
			-Wredundant-decls \
			-Wsign-compare \
			-Wstrict-aliasing=2 \
			-Wstrict-prototypes \
			-Wundef \
			-Wunreachable-code \
			-Wunsafe-loop-optimizations \
			-Wunused-but-set-variable \
			-Wwrite-strings

CFLAGS += $(shell pkg-config --cflags gstreamer-1.0)
LDLIBS += $(shell pkg-config --libs gstreamer-1.0)

gst-perspective-transform: gst-perspective-transform.o

clean:
		rm -f *~ *.o gst-perspective-transform

test: gst-perspective-transform
		G_DEBUG=gc-friendly G_SLICE=always-malloc \
		valgrind --leak-check=full --show-reachable=yes ./gst-perspective-transform