/*
 * kprofwidget.cpp
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

#include <qlayout.h>
#include <qtextstream.h>
#include <qvector.h>
#include <qlabel.h>

#include <kapp.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcmenumngr.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>
#include <kiconloader.h>

#include "kprofwidget.h"
#include "cprofileviewitem.h"


KProfWidget::KProfWidget (QWidget *parent, const char *name)
	:	QWidget (parent, name),
		mTabs (NULL),
		mFlat (NULL),
		mHier (NULL),
		mObjs (NULL),
		mCurPage (0)
{
	QVBoxLayout *topLayout = new QVBoxLayout (this, 0, 0);

	mTabs = new KTabCtl (this, "tabs");
	topLayout->addWidget (mTabs);

	connect (mTabs, SIGNAL (tabSelected (int)), this, SLOT (tabSelected (int)));

	// create flat profile list
	QWidget *flatWidget = new QWidget (this, "flatWidget");
	QVBoxLayout *flatLayout = new QVBoxLayout (flatWidget, 0, 0);
	QHBoxLayout *flatEditLayout = new QHBoxLayout (flatLayout, 0, 0);
	QLineEdit *flatFilter = new QLineEdit (flatWidget);
	flatEditLayout->addWidget (new QLabel (i18n ("Filter:"), flatWidget));
	flatEditLayout->addItem (new QSpacerItem (10,10));
	flatEditLayout->addWidget (flatFilter);

	mFlat = new KListView (flatWidget, "flatProfile");
	CHECK_PTR (mFlat);
	prepareProfileView (mFlat, false);

	connect (mFlat, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
			 this, SLOT (profileEntryRightClick (QListViewItem*, const QPoint&, int)));
    connect (flatFilter, SIGNAL (textChanged (const QString &)),
			 this, SLOT (flatProfileFilterChanged (const QString &)));

    flatLayout->addWidget (mFlat);

	// create hierarchical profile list
	mHier = new KListView (this, "hierarchicalProfile");
	CHECK_PTR (mHier);
	prepareProfileView (mHier, true);
	connect (mHier, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
			 this, SLOT (profileEntryRightClick (QListViewItem*, const QPoint&, int)));

	// create object profile list
	mObjs = new KListView (this, "objectProfile");
	CHECK_PTR (mObjs);
	prepareProfileView (mObjs, true);
	connect (mObjs, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
			 this, SLOT (profileEntryRightClick (QListViewItem*, const QPoint&, int)));

	// add the view to the tab control
 	mTabs->addTab (flatWidget, i18n("&Flat Profile"));
	mTabs->addTab (mHier, i18n("&Hierarchical Profile"));
	mTabs->addTab (mObjs, i18n("O&bject Profile"));

	loadSettings ();
	applySettings ();
}

KProfWidget::~KProfWidget ()
{
}

void KProfWidget::tabSelected (int page)
{
	mCurPage = page;
}

void KProfWidget::prepareProfileView (KListView *view, bool rootIsDecorated)
{
	KIconLoader *loader = KGlobal::iconLoader ();
	view->addColumn (i18n("Function/Method"), -1);
	view->addColumn (QIconSet (loader->loadIcon ("redo", KIcon::Small)), "", -1);
	view->addColumn (i18n("Count"), -1);
	view->addColumn (i18n("Total (s)"), -1);
	view->addColumn (i18n("%"), -1);
	view->addColumn (i18n("Self (s)"), -1);
	view->addColumn (i18n("Total\nus/call"), -1);
	view->addColumn (i18n("Self\nus/call"), -1);

	view->setAllColumnsShowFocus (true);
	view->setFrameStyle (QFrame::WinPanel + QFrame::Sunken);
	view->setShowSortIndicator (true);
	view->setRootIsDecorated (rootIsDecorated);
}

void KProfWidget::setManualColumnWidths (KListView *view)
{
	for (int i = 0; i < view->columns (); i++)
		view->setColumnWidthMode (i, QListView::Manual);
}

void KProfWidget::openSettingsDialog ()
{
}

void KProfWidget::settingsChanged ()
{
	applySettings ();
	loadSettings ();
}

void KProfWidget::applySettings ()
{
	KConfig &config = *kapp->config ();

	config.setGroup ("KProfiler");
	config.writeEntry ("Width", width ());
	config.writeEntry ("Height", height ());

	// @@@ save columns widhts here

	config.sync ();
	update ();
}

void KProfWidget::loadSettings ()
{
	KConfig &config = *kapp->config ();

	config.setGroup ("KProfiler");

	int w = config.readNumEntry ("Width", width ());
	int h = config.readNumEntry ("Height", height ());
	resize (w,h);

	// @@@ reload columns widths here
}

void KProfWidget::openResultsFile ()
{
	QString filename = KFileDialog::getOpenFileName (NULL, NULL, this, "Select a gprof-generated file");

	if (filename.isEmpty ())
		return;

	mFlat->clear ();
	mHier->clear ();
	mObjs->clear ();
	mProfile.clear ();

	parseProfile (filename);

	QString noFilter;
	fillFlatProfileList (noFilter);
	fillHierProfileList ();
	fillObjsProfileList ();
}

void KProfWidget::parseProfile (QString& filename)
{
	/*
	 * parse a GNU gprof output generated with -b (brief)
	 *
	 */

	QFile f (filename);

	if (!f.open (IO_ReadOnly))
		return;

	QTextStream t (&f);
	QString s;

	// regular expressions we use while parsing
	QRegExp indexRegExp (" *\\[\\d+\\]$");
	QRegExp floatRegExp ("^[0-9]*\\.[0-9]+$");
	QRegExp countRegExp ("^[0-9]+[/\\+]?[0-9]*$");
	QRegExp dashesRegExp ("^\\-+");
	QRegExp	recurCountRegExp (" *<cycle \\d+.*>$");

	// while parsing, we temporarily store all entries of a call graph block
	// in an array
	QVector<SCallGraphEntry> callGraphBlock;
	callGraphBlock.setAutoDelete (true);
	callGraphBlock.resize (32);

	int state = 0;

	while (!t.eof ())
	{
		s = t.readLine ();
		if (s.length() && s[0] == 12) {
			if (state == 3)
				processCallGraphBlock (callGraphBlock);
			state++;			// CTRL_L marks the beginning of a new block
			continue;
   		}
		s = s.simplifyWhiteSpace ();

		// remove <cycle ...> and [n] from the end of the line
		if (state == PROCESS_FLAT_PROFILE || state == PROCESS_CALL_GRAPH)
		{
			int pos = indexRegExp.find (s, 0);
			if (pos != -1)
				s = s.left (pos);
			pos = recurCountRegExp.find (s, 0);
			if (pos != -1)
				s = s.left (pos);
		}

		// split the line in tab-delimited fields
		QStringList fields = QStringList::split (" ", s, false);
		if (fields.isEmpty ())
			continue;

		if (state == PROCESS_FLAT_PROFILE || state == PROCESS_CALL_GRAPH)
		{
			// the split did also split the function name & args. restore them so that they
			// are only one field
			uint join = fields.count () - 1;
			while (join)
			{
				if (fields[join - 1][0].isDigit ())
					break;
				fields[join - 1] += " " + fields[join];
				fields.remove (fields.at (join));
				join--;
			}
   		}

		switch (state)
		{
			/*
			 * look for beginning of flat profile
			 *
			 */
			case SEARCH_FLAT_PROFILE:
				for (unsigned int i = 0; i < fields[0].length (); i++)
				{
					QChar c = s[i];
					if (c != ' ')
					{
	      				if (c.isNumber())
		      				state = 1;
						break;
           			}
              	}
				if (state == 0)
					break;

     			// fall through

			/*
			 * analyze flat profile entry
			 *
			 */
     		case PROCESS_FLAT_PROFILE:
			{
				CProfileInfo *p = new CProfileInfo;
				p->cumPercent		= fields[0].toFloat ();
				p->cumSeconds		= fields[1].toFloat ();
				p->selfSeconds		= fields[2].toFloat ();
				if (fields[3][0].isDigit ())
				{
					p->calls			= fields[3].toLong ();
					p->selfTsPerCall	= fields[4].toFloat ();
					p->totalTsPerCall	= fields[5].toFloat ();
					p->name				= fields[6];
     			}
				else
				{
					// if the profile was generated with -z, uncalled functions
					// have empty "calls", "selfTsPerCall" and "totalTsPerCall" fields
					p->calls			= 0;
					p->selfTsPerCall	= 0;
					p->totalTsPerCall	= 0;
					p->name				= fields[3];
     			}
				p->recursive		= false;
				int j = p->name.find ("::");
				if (j > 0)
					p->object = p->name.left (j);
				j = mProfile.size ();
				mProfile.resize (j + 1);
				mProfile.insert (j, p);
				break;
    		}

   			/*
			 * look for call graph
			 *
			 */
   			case SEARCH_CALL_GRAPH:
				if (fields[0]=="index" && fields[1]=="%")
					state = 3;
				break;

    		/*
			 * analyze call graph entry
			 *
			 */
    		case PROCESS_CALL_GRAPH:
			{
				// if we reach a dashes line, we finalize the previous call graph block
				// by analyzing the block and updating callers, called & recursive
				// information
				if (dashesRegExp.find (fields[0], 0) == 0)
				{
					processCallGraphBlock (callGraphBlock);
					callGraphBlock.resize (0);
					break;
				}

				QString count;
				SCallGraphEntry *e = new SCallGraphEntry;
				uint field = 0;

				e->primary = false;
				e->recursive = false;

				// detect the primary line in the call graph
				if (indexRegExp.find (fields[0], 0) == 0)
				{
					e->primary = true;
					field++;
     			}

				// gather other values (we have to do some guessing to get them right)
				while (field < fields.count ())
				{
					if (countRegExp.find (fields[field], 0) == 0)
						e->recursive = fields[field].find ('+') != -1;
      				else if (floatRegExp.find (fields[field], 0) == -1)
						e->name = fields[field];
      				field++;
				}

				// if we got a call graph block without a primary function name,
				// drop it completely.
				if (e->name == NULL || e->name.length() == 0)
				{
					delete e;
					break;
				}

				if (e->primary == true && count.find ('+') != -1)
					e->recursive = true;

				if (callGraphBlock.count () == callGraphBlock.size ())
					callGraphBlock.resize (callGraphBlock.size () + 32);
				callGraphBlock.insert (callGraphBlock.count (), e);
				break;
    		}
   		}
	}
}

void KProfWidget::processCallGraphBlock (const QVector<SCallGraphEntry> &data)
{
	// process a call graph block. The list of entries is stored in the
	// order they are encountered, which is very important because the
	// order matters (see the gprof manual)

	// first, locate the primary entry
	CProfileInfo *primary = NULL;
	uint i;
	for (i = 0; i < data.count(); i++)
	{
		if (data[i]->primary)
		{
			primary = locateProfileEntry (data[i]->name);
			break;
   		}
   	}

	// make sure primary is not NULL !
	if (primary == NULL)
	{
		// @@@ should perform better error reporting here
		if (i != data.count ())
			fprintf (stderr, "%s: missing flat profile entry for %s\n",
					"kprof"/*kapp->name.latin1()*/,
					data[i]->name.latin1());
  		return;
	}

	// store callers, called funcs and info about primary entry
	bool beforePrimary = true;
	for (i = 0; i < data.count (); i++)
	{
		CProfileInfo *p = locateProfileEntry (data[i]->name);
	 	if (data[i]->primary)
		{
			if (data[i]->recursive)
				p->recursive = true;
			beforePrimary = false;
			continue;
   		}
		if (p)
		{
			if (beforePrimary)
			{
				primary->callers.resize (primary->callers.count () + 1);
				primary->callers [primary->callers.count () - 1] = p;
    		}
			else
			{
				primary->called.resize (primary->called.count () + 1);
				primary->called [primary->called.count () - 1] = p;
    		}
      	}
	}
}

CProfileInfo *KProfWidget::locateProfileEntry (const QString& name)
{
	// find an entry in our profile table based on function name
	for (uint j = 0; j < mProfile.count (); j++)
	{
		if (mProfile[j]->name == name)
			return mProfile[j];
	}

	return NULL;
}

void KProfWidget::fillFlatProfileList (const QString& filter)
{
	for (unsigned int i = 0; i < mProfile.size (); i++)
	{
		if (filter.length () && !mProfile[i]->name.contains (filter))
			continue;
		new CProfileViewItem (mFlat, mProfile[i]);
  	}
	setManualColumnWidths (mFlat);
	update ();
}


void KProfWidget::fillHierProfileList ()
{
	for (unsigned int i = 0; i < mProfile.size (); i++)
	{
		CProfileViewItem *item = new CProfileViewItem (mHier, mProfile[i]);

		QArray<CProfileInfo *> addedEntries (mProfile.size ());

		CProfileInfo *p = NULL;
		addedEntries.fill (p);

		int count = 1;
		addedEntries [0] = mProfile[i];

		fillHierarchy (item, mProfile[i], addedEntries, count);
  	}
	setManualColumnWidths (mFlat);
	update ();
}

void KProfWidget::fillHierarchy (
		CProfileViewItem *item,
		CProfileInfo *parent,
		QArray<CProfileInfo *> &addedEntries,
		int &count)
{
	for (uint i = 0; i < parent->called.count (); i++)
	{
		// skip items already added to avoid recursion
		if (addedEntries.find (parent->called[i]) != -1)
			continue;

		addedEntries[count++] = parent->called[i];
		CProfileViewItem *newItem = new CProfileViewItem (item, parent->called[i]);
		fillHierarchy (newItem, parent->called[i], addedEntries, count);
	}
}

void KProfWidget::fillObjsProfileList ()
{
	// first collect all class names
	QRegExp reg ("::[^\\(:]*\\(");
	QVector<QString> classes;

	for (uint i = 0; i < mProfile.count (); i++)
	{
		int j = reg.find (mProfile[i]->name, 0);
		if (j > 0)
		{
			QString *cl = new QString (mProfile[i]->name.left (j));
			uint k;
			for (k = 0; k < classes.count (); k++)
			{
				if (classes[k]->compare (*cl) == 0)
					break;
     		}
			if (k == classes.count ())
			{
				classes.resize (classes.count () + 1);
				classes.insert (classes.count (), cl);
    		}
		}
  	}

	// create all toplevel elements and their descendants
	for (uint i = 0; i < classes.count (); i++)
	{
		CProfileViewItem *item = new CProfileViewItem (mObjs, NULL);
		fillObjsHierarchy (item, classes[i]);
	}
	
	setManualColumnWidths (mFlat);
}

void KProfWidget::fillObjsHierarchy (CProfileViewItem *parent, QString *className)
{
	for (uint i = 0; i < mProfile.count (); i++)
	{
		if (mProfile[i]->name.startsWith (*className))
			new CProfileViewItem (parent, mProfile[i]);
	}
}

void KProfWidget::profileEntryRightClick (QListViewItem *listItem, const QPoint &p, int)
{
	if (!listItem)
		return;

	CProfileViewItem *item = (CProfileViewItem *) listItem;
	CProfileInfo *info = item->getProfile();

	KPopupMenu pop (mTabs, 0);

	QArray<CProfileInfo *> itemProf;
	itemProf.resize (info->callers.count () + info->called.count ());
	uint n = 0;

	// if there are no callers nor called functions, return
	if (itemProf.size() == 0)
		return;

	// add callers to the pop-up menu
	if (info->callers.count ())
	{
		pop.insertTitle (i18n ("Called By:"));
		for (uint i = 0; i < info->callers.count (); i++)
		{
			CProfileInfo *p = info->callers[i];
			pop.insertItem (p->name, n);
			itemProf[n++] = p;
   		}
   	}

	// add called functions to the popup menu
	if (info->called.count ())
	{
		pop.insertTitle (i18n ("Calls:"));
		for (uint i = 0; i < info->called.count (); i++)
		{
			CProfileInfo *p = info->called[i];
			pop.insertItem (p->name, n);
			itemProf[n++] = p;
   		}
	}

	int sel = pop.exec (p);

	if (sel != -1)
		selectProfileItem (itemProf[sel]);
}


void KProfWidget::selectProfileItem (CProfileInfo *info)
{
	selectItemInView (mFlat, info);
	selectItemInView (mHier, info);
}

void KProfWidget::selectItemInView (QListView *view, CProfileInfo *info)
{
	QListViewItem *item = view->firstChild ();
	while (item)
	{
		if (((CProfileViewItem *)item)->getProfile () == info)
		{
			view->clearSelection ();
			view->setSelected (item, true);
			view->ensureItemVisible (item);
			return;
		}
		item = item->itemBelow ();
	}
}

void KProfWidget::flatProfileFilterChanged (const QString &filter)
{
	mFlat->clear ();
	fillFlatProfileList (filter);
}

void KProfWidget::doPrint ()
{
	KListView *view =	mCurPage == 0 ? mFlat :
						mCurPage == 1 ? mHier :
										mObjs;
#ifndef QT_NO_PRINTER
	if (mPrinter.setup(view))
	{
        QPainter paint (&mPrinter);
        // @@@ TODO: finish printing code
    }
#endif
}

#include "kprofwidget.moc"
