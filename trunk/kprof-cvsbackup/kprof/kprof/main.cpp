/*
 * main.cpp
 *
 * $Id$
 *
 * Copyright (c) 2000 Florent Pillet <florent.pillet@wanadoo.fr>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 2.0 (KDE 2.0) libraries, available
 * at no cost at http://www.kde.org/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <kapp.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include "../config.h"
#include "kprof.h"
#include "Log.h"
#include "kprofwidget.h"
#include <qstring.h>

static const char *description = I18N_NOOP("Execution profile results analysis utility");

static KCmdLineOptions options[] =
{
	{ "f ", I18N_NOOP("file to open"), 0 },
	{ "p ", I18N_NOOP("profiler used - one of <gprof, fnccheck, pose>"),0},
  { 0, 0, 0 }
};


int main(int argc, char **argv)
{
	KAboutData aboutData(
			PACKAGE, I18N_NOOP("KProf"),
			VERSION, description, KAboutData::License_GPL,
			"(c) 2000-2002, Florent Pillet & Colin Desmond",
			NULL,
			"http://kprof.sourceforge.net/",
			"fpillet,cdesmond@users.sourceforge.net");

	KCmdLineArgs::init (argc, argv, &aboutData);
	KCmdLineArgs::addCmdLineOptions( options );

	aboutData.addAuthor("Florent Pillet",0, "florent.pillet@wanadoo.fr ");
	aboutData.addAuthor("Colin Desmond", 0, "colin.desmond@btopenworld.com");

	KApplication app;

	Log::Init("TRC",80000);
	RUN("create the application");

	if( app.isRestored() ) //SessionManagement
	{
		RUN("restore the top level widget");
		RESTORE(KProfTopLevel);
	}
	else
	{
		RUN("create the top level widget");
		KProfTopLevel *ktl = new KProfTopLevel(0,"KProf main");
		CHECK_PTR(ktl);
		RUN("show the top level widget");
		ktl->show();

	}

	RUN("start the application");
	int i = app.exec();
	RUN("finish the application");

	return i;
}

