Name: kprof
Summary: KProf -- Profiling results viewer
Version: 1.3
Release: 1
Copyright: GPL
Group: X11/KDE/Development
Source: http://download.sourceforge.net/kprof/kprof-1.3.tar.gz
Packager: Florent Pillet <fpillet@users.sourceforge.net>
BuildRoot: /tmp/kprof-1.3
Prefix: /opt/kde2

%description
A visual tool for developers that displays the execution profiling output generated by code profiling tools.

%prep
rm -rf $RPM_BUILD_ROOT
%setup -n kprof-1.3

%build
./configure
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%{prefix}/bin/kprof
%{prefix}/share/applnk/Development/kprof.desktop
%{prefix}/share/apps/kprof/
%{prefix}/share/doc/HTML/en/kprof/
%{prefix}/share/icons/medium/locolor/apps/kprof.png
%{prefix}/share/icons/small/locolor/apps/kprof.png