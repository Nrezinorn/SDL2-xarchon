## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/src @X_CFLAGS@ @GTK_CFLAGS@

noinst_LIBRARIES = libgtk.a

# Glade related rules
# We're having $(GLADE_sources) all depend on xarchon.glade

gladefile = xarchon.glade

GLADE_sources =                    \
   gtk-callbacks.h                 \
   gtk-interface.c gtk-interface.h \
   gtk-support.c gtk-support.h

$(GLADE_sources): $(gladefile)
	-rm -rf $(GLADE_sources)
	@GLADE@ -w $<

# libgtk.a

libgtk_a_SOURCES = \
   gtk-toolkit.c        gtk-toolkit.h           \
                        gtk-toolkit-priv.h      \
   gtk-main-window.c    gtk-main-window.h       \
   gtk-iface.c          gtk-iface.h             \
   gtk-computer.c	                        \
   gtk-human.c                                  \
   gtk-network.c	             		\
   gtk-theme.c                                  \
   gtk-help.c                                   \
   $(GLADE_sources)

$(libgtk_a_OBJECTS): gtk-callbacks.h

# Housekeeping

EXTRA_DIST = $(gladefile)

CONFIG_CLEAN_FILES = gtk-callbacks.* gtk-interface.* gtk-support.*

CLEANFILES = $(gladefile).bak 
