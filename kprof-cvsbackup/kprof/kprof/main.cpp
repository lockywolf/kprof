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
#include <kconfig.h>
#include <klocale.h>
#include <kstdaccel.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "kprof.h"

static const char *description = I18N_NOOP("KDE profiling result examination utility");
static const char *version = "v1.0.0";

int main(int argc, char **argv)
{
	KAboutData aboutData(
			"kprof", I18N_NOOP("KProf"),
			version, description, KAboutData::License_GPL,
			"(c) 2000, Florent Pillet");
			aboutData.addAuthor("Florent Pillet",0, "florent.pillet@wanadoo.fr");

	KCmdLineArgs::init (argc, argv, &aboutData);

	KApplication app;

	if( app.isRestored() ) //SessionManagement
	{
		for( int n=1; KMainWindow::canBeRestored(n); n++ )
		{
			KProfTopLevel *ktl = new KProfTopLevel();
			CHECK_PTR(ktl);

			app.setMainWidget(ktl);
			ktl->restore(n);
		}
	}
	else
	{
		KProfTopLevel *ktl = new KProfTopLevel();
		CHECK_PTR(ktl);

		ktl->show();
	}

	return app.exec();
}
