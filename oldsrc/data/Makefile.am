## Process this file with automake to produce Makefile.in

THEMES = archon

configfile = xarchon.default
configfile_in = $(configfile).in

install-data-hook:
	sed "s/DEFAULT_TOOLKIT/@DEFAULT_TOOLKIT@/" $(configfile_in) > $(DESTDIR)$(pkgdatadir)/$(configfile)
	@for theme in $(THEMES); do \
	   echo "--> Installing theme '$$theme'"; \
	   THEME_DIRS=`find $$theme -type d`; \
	   for p in $$THEME_DIRS; do \
	      echo "mkdir $(pkgdatadir)/$$p"; \
	      mkdir $(pkgdatadir)/$$p; \
	   done; \
	   THEME_FILES=`find $$theme -type f`; \
	   for p in $$THEME_FILES; do \
	      echo " $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(pkgdatadir)/$$p"; \
	      $(INSTALL_DATA) $(srcdir)/$$p $(DESTDIR)$(pkgdatadir)/$$p; \
	   done; \
	   THEME_LINKS=`find $$theme -type l`; \
	   for p in $$THEME_LINKS; do \
	      q=`ls -l $$p | cut -d '>' -f 2`; \
	      echo " ln -sf $$q $(DESTDIR)$(pkgdatadir)/$$p"; \
	      ln -sf $$q $(DESTDIR)$(pkgdatadir)/$$p; \
	   done; \
	done

pkgdata_DATA = \
   statistics \
   icon.xpm logo.xpm \
   xarchon-palette

EXTRA_DIST = $(pkgdata_DATA) $(configfile_in) $(THEMES)
