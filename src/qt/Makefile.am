## Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_srcdir)/src @X_CFLAGS@ @QT_INCLUDES@

noinst_LIBRARIES = libqt.a

libqt_a_SOURCES = \
   qt-toolkit.cpp       qt-toolkit.h            \
   qmymenubar.cpp       qmymenubar.h            \
   qmyprogressbar.cpp   qmyprogressbar.h        \
   qkeypushbutton.cpp   qkeypushbutton.h        \
   qtiface.cpp          qtiface.h               \
   qthumaniface.cpp     qthumaniface.h          \
   qtcomputeriface.cpp  qtcomputeriface.h       \
   qtnetworkiface.cpp   qtnetworkiface.h        \
   qtplayerwidget.cpp   qtplayerwidget.h        \
   qtplayersdialog.cpp  qtplayersdialog.h       \
   qtguiwidget.cpp      qtguiwidget.h

qkeypushbutton.cpp:     qkeypushbutton.moc
qtiface.cpp:            qtiface.moc
qthumaniface.cpp:       qthumaniface.moc
qtplayersdialog.cpp:    qtplayersdialog.moc
qtguiwidget.cpp:        qtguiwidget.moc

.h.moc:
	@MOC@ $< -o $@

CLEANFILES = *.moc
