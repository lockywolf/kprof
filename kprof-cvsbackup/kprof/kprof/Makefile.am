#SUBDIRS = pics

INCLUDES= $(all_includes)

# the program to be generated
bin_PROGRAMS = kprof

# metasources
kprof_METASOURCES = \
	kprof.moc \
	kprofwidget.moc

# sources 
kprof_SOURCES = \
	main.cpp \
	kprof.cpp \
	kprofwidget.cpp \
	cprofileviewitem.cpp \
	$(kprof_METASOURCES)

# library search path
kprof_LDFLAGS = $(all_libraries) $(KDE_RPATH)

# libraries to link against
kprof_LDADD  = $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE)

# files not to install
noinst_HEADERS = \
	kprof.h \
	kprofwidget.h \
	cprofileinfo.h \
	cprofileviewitem.h

# menu to put kprof in & desktop file
desktopdir = $(kde_appsdir)/Development
desktop_DATA = kprof.desktop

# make messages.po
messages:
	$(XGETTEXT) *.cpp -o $(podir)/kprof.pot

# moc file dependencies
kprof.cpp: kprof.moc

kprof.moc: kprof.h

kprofwidget.moc: kprofwidget.h

kprofwidget.cpp: kprofwidget.moc

# rule to make moc files
%.moc: %.h
	$(MOC) $< -o $@
