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

KProfTopLevel::KProfTopLevel (QWidget *parent, const char *name)
	:	KMainWindow (parent, name)
{
	mProf = new KProfWidget (this,"kprof");
	CHECK_PTR(mProf);

	setupActions ();
	createGUI ("kprofui.rc");
	setCentralWidget (mProf);
}

void KProfTopLevel::setupActions ()
{
	KStdAction::open (mProf, SLOT(openResultsFile()), actionCollection());
	KStdAction::print (mProf, SLOT(doPrint()), actionCollection());
	KStdAction::quit (this, SLOT(close()), actionCollection ());

//	KStdAction::showToolbar (this, SLOT(toggleToolBar()), actionCollection());
}

void KProfTopLevel::toggleToolBar ()
{
	if (toolBar()->isVisible ())
		toolBar()->hide ();
	else
		toolBar()->show ();
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