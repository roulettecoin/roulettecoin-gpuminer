
if WANT_JANSSON
JANSSON_INCLUDES= -I$(top_srcdir)/compat/jansson
else
JANSSON_INCLUDES=
endif

SPH_INCLUDES    = -I$(top_srcdir)/sphlib

EXTRA_DIST	= example-cfg.json nomacro.pl

SUBDIRS		= compat sphlib

INCLUDES	= $(PTHREAD_FLAGS) -fno-strict-aliasing $(JANSSON_INCLUDES) $(SPH_INCLUDES)

bin_PROGRAMS	= minerd

dist_man_MANS	= minerd.1

minerd_SOURCES	= elist.h miner.h compat.h \
		  cpu-miner.c util.c \
		  sha2.c roulette.c \
		  roulette.h

minerd_LDFLAGS	= $(PTHREAD_FLAGS)
minerd_LDADD	= @LIBCURL@ @JANSSON_LIBS@ @SPH_LIBS@ @PTHREAD_LIBS@ @WS2_LIBS@
minerd_CPPFLAGS = @LIBCURL_CPPFLAGS@

