bin_PROGRAMS = critters
critters_SOURCES = boing.c brain.c breeder.c critter.c critters.c danger.c food.c genome.c scene.c thing.c tree.c window.c

AM_CPPFLAGS = -I$(top_srcdir)/include -DQRT_CONFIG_TREE_KEY_TYPE=float
AM_CFLAGS = -pthread -O3 -msse2 -mfpmath=sse -std=c99 -Wall -pedantic -Werror=implicit -Werror=implicit-function-declaration -Werror=uninitialized -Werror=return-type
AM_LDFLAGS = -lm -lSDL -lpthread
