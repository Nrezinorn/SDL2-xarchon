## Process this file with automake to produce Makefile.in

SUBDIRS = @GTK_SUBDIR@ @QT_SUBDIR@

INCLUDES = @X_CFLAGS@ @ESD_CFLAGS@

SOUND_DRIVER_C = sample-driver-@SOUND_DRIVER@.c

COMMON_sources = \
   actors.c             actors.h                \
   audio.c              audio.h                 \
   board.c              board.h                 \
   canvas.c             canvas.h                \
   computer.c           computer.h              \
   field.c              field.h                 \
   human.c              human.h                 \
   iface.c              iface.h                 \
   list.c               list.h                  \
   network.c            network.h               \
   sprite.c             sprite.h                \
   theme.c              theme.h                 \
   toolkit.c            toolkit.h               \
   $(SOUND_DRIVER_C)    sample-driver.h         \
   Xarchon_Problem.cpp  Xarchon_Problem.hpp     \
   Xarchon_Interface.cpp Xarchon_Interface.hpp  \
   Xarchon_Game.cpp     Xarchon_Game.hpp        \
   Xarchon_Genetic.cpp  Xarchon_Genetic.hpp

bin_PROGRAMS = xarchon

# noinst_PROGRAMS = genetics

xarchon_SOURCES = $(COMMON_sources) main.c main.h

xarchon_DEPENDENCIES = @TOOLKIT_LIBS@

# genetics_SOURCES = $(COMMON_sources) genetics_main.c main.h

xarchon_LDADD = @X_LIBS@ @TOOLKIT_LIBS@ @TOOLKIT_LDADDS@ @ESD_LIBS@

# genetics_LDADD = @X_LIBS@ @ESD_LIBS@

man_MANS = xarchon.6

EXTRA_DIST = box xarchon.6

CLEANFILES = genetics
