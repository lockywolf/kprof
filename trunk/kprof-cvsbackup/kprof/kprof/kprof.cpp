/*
 * kprof.cpp
 *
 * $Id$
 *
 * Copyright (c) 2000-2001 Florent Pillet <fpillet@users.sourceforge.net>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 2.0 (KDE 2.0) libraries or later,
 * available at no cost at http://www.kde.org/
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

	// load the recent files list
	KConfig *config = kapp->config ();
	KRecentFilesAction *recent = (KRecentFilesAction *) actionCollection()->action (KStdAction::stdName (KStdAction::OpenRecent));
	recent->loadEntries (config);

	connect (mProf, SIGNAL (addRecentFile(const KURL&)), this, SLOT(addRecentFile(const KURL&)));

	loadSettings ();
	applySettings ();
}

void KProfTopLevel::loadSettings ()
{
	KConfig &config = *kapp->config ();
	config.setGroup ("KProfiler");
	int w = config.readNumEntry ("Width", width ());
	int h = config.readNumEntry ("Height", height ());
	resize (w,h);

	mProf->loadSettings ();
}

void KProfTopLevel::applySettings ()
{
	KConfig &config = *kapp->config ();
	config.setGroup ("KProfiler");
	config.writeEntry ("Width", width ());
	config.writeEntry ("Height", height ());
	mProf->applySettings ();
	config.sync ();
}

void KProfTopLevel::setupActions ()
{
	KStdAction::open (mProf, SLOT(openResultsFile()), actionCollection());
	KStdAction::openRecent (mProf, SLOT(openRecentFile(const KURL&)), actionCollection());
	KStdAction::print (mProf, SLOT(doPrint()), actionCollection());
	KStdAction::quit (this, SLOT(close()), actionCollection ());

	KStdAction::showToolbar (this, SLOT(toggleToolBar()), actionCollection());

	mToggleTemplateAbbrev = new KToggleAction (i18n ("Abbreviate C++ &Templates"), 0, mProf, SLOT (toggleTemplateAbbrev ()), actionCollection(), "toggle_template_abbreviations");
	mSelectFont = new KAction (i18n ("Select Font..."), 0, mProf, SLOT (selectListFont ()), actionCollection(), "select_list_font");
	mGenCallGraphAction = new KAction (i18n ("&Generate Call Graph..."), 0, mProf, SLOT (generateCallGraph ()), actionCollection(), "generate_call_graph");
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
	KConfig *config = kapp->config ();
	KRecentFilesAction *recent = (KRecentFilesAction *) actionCollection()->action (KStdAction::stdName (KStdAction::OpenRecent));
	recent->saveEntries (config);
	applySettings ();
	return true;
}

void KProfTopLevel::addRecentFile (const KURL& url)
{
	// this slot is called by kprofwidget when a file has been opened.
	// we store it in the recent files and also change the window title
	KRecentFilesAction *recent = (KRecentFilesAction *) actionCollection()->action (KStdAction::stdName (KStdAction::OpenRecent));
	recent->addURL (url);

	setCaption (url.fileName ());
}

#include "kprof.moc"
