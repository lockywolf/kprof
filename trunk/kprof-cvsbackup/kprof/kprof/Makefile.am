####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = kprof

## INCLUDES were found outside kdevelop specific part

kprof_SOURCES = ctidyup.cpp parseArguments.cpp kproffile.cpp Log.cpp cprofileinfo.cpp parseprofile_pose.cpp parseprofile_fnccheck.cpp parseprofile.cpp parseprofile_gprof.cpp clientsidemap.cpp vcgCallGraph.cpp dotCallGraph.cpp kprofwidget.cpp cprofileviewitem.cpp call-graph.ui kprof.cpp main.cpp cconfigure.cpp cconfigure.h
kprof_LDADD   =  $(LIB_KFILE) $(LIB_KHTML) $(LIB_KDEUI) $(LIB_KDECORE) $(LIB_QT) $(LIBSOCKET)


EXTRA_DIST = main.cpp kprof.cpp kprof.h kprofdoc.h kprofview.h kprofui.rc kprof.desktop lo32-app-kprof.png lo16-app-kprof.png cprofileviewitem.h call-graph.h call-graph.ui cprofileinfo.h cprofileviewitem.cpp kprofwidget.cpp kprofwidget.h dotCallGraph.cpp dotCallGraph.h vcgCallGraph.cpp vcgCallGraph.h clientsidemap.h clientsidemap.cpp parseprofile_gprof.h parseprofile_gprof.cpp parseprofile.h parseprofile.cpp parseprofile_fnccheck.h parseprofile_fnccheck.cpp parseprofile_pose.h parseprofile_pose.cpp cprofileinfo.cpp Log.cpp Log.h kproffile.cpp kproffile.h parseArguments.cpp parseArguments.h ctidyup.cpp ctidyup.h 

install-data-local:
	$(mkinstalldirs) $(kde_icondir)/locolor/32x32/apps/
	$(INSTALL_DATA) $(srcdir)/lo32-app-kprof.png $(kde_icondir)/locolor/32x32/apps/kprof.png
	$(mkinstalldirs) $(kde_icondir)/locolor/16x16/apps/
	$(INSTALL_DATA) $(srcdir)/lo16-app-kprof.png $(kde_icondir)/locolor/16x16/apps/kprof.png

uninstall-local:
	-rm -f $(kde_icondir)/locolor/32x32/apps/kprof.png
	-rm -f $(kde_icondir)/locolor/16x16/apps/kprof.png

####### kdevelop will overwrite this part!!! (end)############
noinst_HEADERS = kprof.h kprofwidget.h cprofileinfo.h cprofileviewitem.h

INCLUDES=$(all_includes)
kprof_LDFLAGS=$(all_libraries) $(KDE_RPATH) $(LIBQTREEMAP)

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

