/*
 * kprof.h
 *
 * $Id$
 *
 * Copyright (c) 2000 Florent Pillet <florent.pillet@wanadoo.fr>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 * Requires the K Desktop Environment 3.0 (KDE 3.0) libraries, available
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

#ifndef __KPROF_H__
#define __KPROF_H__

#include <kmainwindow.h>

class KToggleAction;
class KAction;
class KProfWidget;

class KProfTopLevel : public KMainWindow
{
	Q_OBJECT

protected:
	KProfWidget*	mProf;
	KToggleAction*	mToggleTemplateAbbrev;
	KAction*	mSelectFont;
	KAction*	mGenCallGraphAction;
	KAction*	mCompareFile;
	KAction*	mRunApplication;
	KAction*	mDisplayTreeMapAction;
	KAction*	mConfigure;

public:
	KProfTopLevel (QWidget *parent = 0, const char *name = NULL);
	~KProfTopLevel ();

	inline KToggleAction* getToggleTemplateAbbrevAction ()
	{
		return mToggleTemplateAbbrev;
	}

protected slots:
	virtual bool queryExit ();
	void toggleToolBar ();
	void addRecentFile (const KURL& url);

private:
	void setupActions ();
	void loadSettings ();
	void applySettings ();
};

#endif
