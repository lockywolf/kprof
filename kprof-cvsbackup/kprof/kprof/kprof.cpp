/*
 * kprof.cpp
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

#include <qiconset.h>

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kaction.h>
#include <kstdaction.h>
#include <kiconloader.h>

#include "kprof.h"
#include "kprofwidget.h"

KProfTopLevel::KProfTopLevel (QWidget *, const char *name)
	:	KTMainWindow (name)
{
	mProf = new KProfWidget (this,"kprof");
	CHECK_PTR(mProf);

	QPopupMenu *file = new QPopupMenu;
	CHECK_PTR(file);

	KIconLoader *loader = KGlobal::iconLoader ();
	file->insertItem (	QIconSet (loader->loadIcon ("open", KIcon::Small)),
						i18n( "&Open..." ), mProf, SLOT(openResultsFile()),
						KStdAccel::key(KStdAccel::Open));
	file->insertSeparator();
	file->insertItem (	QIconSet (loader->loadIcon ("fileprint", KIcon::Small)),
						i18n( "&Print..."), mProf, SLOT(doPrint()),
						KStdAccel::key(KStdAccel::Print));
	file->insertSeparator();
	file->insertItem (	QIconSet (loader->loadIcon ("exit", KIcon::Small)),
						i18n( "&Quit" ), this, SLOT(close()),
						KStdAccel::key(KStdAccel::Quit) );

//	QPopupMenu *option = new QPopupMenu;
//	CHECK_PTR(option);

//	option->insertItem(i18n("&Configure %1...").arg(kapp->caption()), mProf, SLOT(openSettingsDialog()));

	QPopupMenu *help = helpMenu(i18n("KProf\n\n(C) 2000\nFlorent Pillet (florent.pillet@wanadoo.fr)"));

	menuBar()->insertItem (i18n("&File"), file );
//	menuBar()->insertItem (i18n("&Options"), option );
	menuBar()->insertSeparator ();
	menuBar()->insertItem (i18n("&Help"), help );

	setView (mProf);
	resize (mProf->width(), mProf->height() + menuBar()->height());
}

KProfTopLevel::~KProfTopLevel ()
{
}

bool KProfTopLevel::queryExit( void )
{
	mProf->applySettings();
	return true;
}

#include "kprof.moc"
