#SUBDIRS = pics

INCLUDES= $(all_includes)

bin_PROGRAMS = kprof

kprof_SOURCES = main.cpp kprof.cpp kprofwidget.cpp cprofileviewitem.cpp
kprof_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kprof_LDADD  = $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)

noinst_HEADERS = kprof.h kprofwidget.h cprofileinfo.h cprofileviewitem.h

METASOURCES = kprof.moc kprofwidget.moc

desktopdir = $(kde_appsdir)/Development
desktop_DATA = kprof.desktop

messages:
	$(XGETTEXT) *.cpp -o $(podir)/kprof.pot
