## Makefile.am for kprof

# this has all of the subdirectories that make will recurse into.  if
# there are none, comment this out
SUBDIRS = . doc

# the program to be generated
bin_PROGRAMS = kprof

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# library search path
kprof_LDFLAGS = $(KDE_RPATH) $(all_libraries)

# libraries to link against
kprof_LDADD  = -lqtreemap $(LIB_KFILE) $(LIB_KHTML)

# sources 
kprof_SOURCES = main.cpp kprof.cpp kprofwidget.cpp cprofileviewitem.cpp call-graph.ui

# headers (do not install)
noinst_HEADERS = kprof.h kprofwidget.h cprofileinfo.h cprofileviewitem.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

# make messages.po
messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kprof.pot

# the program icon
KDE_ICON = kprof

# this is where the kdelnk file will go 
kdelnkdir   = $(kde_appsdir)/Development
kdelnk_DATA = kprof.desktop

# this is where the XML-GUI resource file goes
rcdir = $(kde_datadir)/kprof
rc_DATA = kprofui.rc
