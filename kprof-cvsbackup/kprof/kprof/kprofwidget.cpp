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
#include <qradiobutton.h>
#include <qvector.h>
#include <qlabel.h>
#include <qwhatsthis.h>

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
#include "call-graph.h"

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
	connect (mFlat, SIGNAL (selectionChanged (QListViewItem *)),
			 this, SLOT (selectionChanged (QListViewItem *)));

    flatLayout->addWidget (mFlat);

	// create hierarchical profile list
	mHier = new KListView (this, "hierarchicalProfile");
	CHECK_PTR (mHier);
	prepareProfileView (mHier, true);
	connect (mHier, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
			 this, SLOT (profileEntryRightClick (QListViewItem*, const QPoint&, int)));
	connect (mHier, SIGNAL (selectionChanged (QListViewItem *)),
			 this, SLOT (selectionChanged (QListViewItem *)));

	// create object profile list
	mObjs = new KListView (this, "objectProfile");
	CHECK_PTR (mObjs);
	prepareProfileView (mObjs, true);
	connect (mObjs, SIGNAL (rightButtonPressed (QListViewItem*, const QPoint&, int)),
			 this, SLOT (profileEntryRightClick (QListViewItem*, const QPoint&, int)));
	connect (mObjs, SIGNAL (selectionChanged (QListViewItem *)),
			 this, SLOT (selectionChanged (QListViewItem *)));

	// add the view to the tab control
 	mTabs->addTab (flatWidget, i18n ("&Flat Profile"));
	mTabs->addTab (mHier, i18n ("&Hierarchical Profile"));
	mTabs->addTab (mObjs, i18n ("O&bject Profile"));

	// add some help on items
	QWhatsThis::add (flatFilter, i18n ("Type text in this field to filter the display "
		"and only show the functions/methods whose name match the text."));
	QWhatsThis::add (mFlat, i18n ("This is the <I>flat view</I>.\n\n"
		"It displays all functions and method 'flat'. Click on a column header "
		"to sort the list on this column (click a second time to reverse the "
		"sort order)."));
	QWhatsThis::add (mHier, i18n ("This is the <I>hierarchical view</I>.\n\n"
		"It displays each function/method like a tree to let you see the other "
		"functions that it calls. For that reason, each function may appear several "
		"times in the list."));
	QWhatsThis::add (mObjs, i18n ("This is the <I>object view</I>.\n\n"
		"It displays C++ methods grouped by object name."));

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

	// TODO: save columns widhts here

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

	// TODO: reload columns widths here
}

void KProfWidget::openRecentFile (const KURL& url)
{
	QString filename = url.path ();
	openFile (filename);
}

void KProfWidget::openResultsFile ()
{
	QString filename = KFileDialog::getOpenFileName (NULL, NULL, this, "Select a gprof-generated file");
	openFile (filename);
}

void KProfWidget::openFile (const QString &filename)
{
	if (filename.isEmpty ())
		return;

	// if the file is an executable file, generate the profiling information
	// directly from gprof and the gmon.out file
	QFileInfo finfo (filename);
	if (finfo.isExecutable ())
	{
		// prepare the "gmon.out" filename
		QString gmonfilename = finfo.dirPath() + "/gmon.out";
		QFileInfo gmonfinfo (gmonfilename);
		if (!gmonfinfo.exists ())
		{
			QString text;
			text.sprintf (i18n ("Can't find the gprof output file '%s'"), gmonfilename.latin1());
			KMessageBox::error (this, text, i18n ("File not found"));
			return;
		}

		// exec "gprof -b filename"
		KProcess gprof;
		gprof << "gprof" << "-b" << filename << gmonfilename;

		mGProfStdout = "";
		mGProfStderr = "";
		connect (&gprof, SIGNAL (receivedStdout (KProcess*, char*, int)), this, SLOT (gprofStdout (KProcess*, char*, int)));
		connect (&gprof, SIGNAL (receivedStderr (KProcess*, char*, int)), this, SLOT (gprofStderr (KProcess*, char*, int)));

		gprof.start (KProcess::Block, KProcess::AllOutput);

		if (!gprof.normalExit () || gprof.exitStatus ())
		{
			QString text = i18n ("gprof(1) could not generate the profile data.\n");
			if (gprof.normalExit () && gprof.exitStatus ())
			{
				QString s;
				s.sprintf (i18n ("Error %d was returned.\n"), gprof.exitStatus ());
				text += s;
			}
			if (!mGProfStderr.isEmpty ())
			{
				text += i18n ("It returned the following error message(s):\n");
				text += mGProfStderr;
			}
			else if (!mGProfStdout.isEmpty ())
			{
				text += i18n ("Following output was displayed:\n");
				text += mGProfStdout;
			}
			KMessageBox::error (this, text, i18n ("gprof exited with error(s)"));
			return;
		}

		mFlat->clear ();
		mHier->clear ();
		mObjs->clear ();
		mProfile.clear ();

		// parse profile data
		QTextStream t (&mGProfStdout, IO_ReadOnly);
		parseProfile (t);
	}
	else
	{
		// if user tried to open gmon.out, have him select the executable instead, then recurse to use
		// the executable file
		if (finfo.fileName() == QString ("gmon.out"))
		{
			KMessageBox::error (this, i18n ("File 'gmon.out' is the result of the execution of an application\nwith profiling turned on.\nYou can not open it as such: either open the application itself\nor open a text results file generated with 'gprof -b application-name'"),
								i18n ("Opening gmon.out not allowed"));
			return;
		}

		mFlat->clear ();
		mHier->clear ();
		mObjs->clear ();
		mProfile.clear ();

		// parse profile data
		QFile f (filename);
		if (!f.open (IO_ReadOnly))
			return;

		QTextStream t (&f);
		parseProfile (t);
	}

	// fill lists
	QString noFilter;
	fillFlatProfileList (noFilter);
	fillHierProfileList ();
	fillObjsProfileList ();

	// make sure we add the recent file
	KURL url;
	url.setProtocol ("file");
	url.setFileName (filename);
	emit addRecentFile (url);
}

void KProfWidget::gprofStdout (KProcess *, char *buffer, int buflen)
{
	mGProfStdout += QString::fromLocal8Bit (buffer, buflen);
}

void KProfWidget::gprofStderr (KProcess *, char *buffer, int buflen)
{
	mGProfStderr += QString::fromLocal8Bit (buffer, buflen);
}

void KProfWidget::parseProfile (QTextStream& t)
{
	/*
	 * parse a GNU gprof output generated with -b (brief)
	 *
	 */

	// regular expressions we use while parsing
	QRegExp indexRegExp (" *\\[\\d+\\] *$");
	QRegExp floatRegExp ("^[0-9]*\\.[0-9]+$");
	QRegExp countRegExp ("^[0-9]+[/\\+]?[0-9]*$");
	QRegExp dashesRegExp ("^\\-+");
	QRegExp	recurCountRegExp (" *<cycle \\d+.*> *$");

	// while parsing, we temporarily store all entries of a call graph block
	// in an array
	QVector<SCallGraphEntry> callGraphBlock;
	callGraphBlock.setAutoDelete (true);
	callGraphBlock.resize (32);

	int state = 0;
	long line = 0;
	QString s;

	t.setEncoding (QTextStream::Latin1);
	while (!t.eof ())
	{
		line++;
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

		if (fields.count() > 1 && (state == PROCESS_FLAT_PROFILE || state == PROCESS_CALL_GRAPH))
		{
			// the split did also split the function name & args. restore them so that they
			// are only one field
			uint join;
			for (join = 0; join < fields.count (); join++)
			{
				QChar c = fields[join][0];
				if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
					break;
			}
			while (join < (fields.count () - 1))
			{
				fields[join] += " " + fields[join + 1];
				fields.remove (fields.at (join + 1));
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
				p->ind				= mProfile.count ();
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

				e->line = line;
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
			fprintf (stderr, "%s: missing flat profile entry for '%s' (line %ld)\n",
					"kprof"/*kapp->name.latin1()*/,
					data[i]->name.latin1(),
					data[i]->line);
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
	mFlat->setColumnWidthMode (0, QListView::Manual);
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
	mHier->setColumnWidthMode (0, QListView::Manual);
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
	
	mObjs->setColumnWidthMode (0, QListView::Manual);
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

void KProfWidget::selectionChanged (QListViewItem *item)
{
	QListView *view = item->listView();
	CProfileInfo *info = ((CProfileViewItem *)item)->getProfile ();
	if (view != mFlat)
	{
		disconnect (mFlat, SIGNAL (selectionChanged (QListViewItem *)),
			 this, SLOT (selectionChanged (QListViewItem *)));
		selectItemInView (mFlat, info, false);
		connect (mFlat, SIGNAL (selectionChanged (QListViewItem *)),
				 this, SLOT (selectionChanged (QListViewItem *)));
	}
	if (view != mHier)
	{
		disconnect (mHier, SIGNAL (selectionChanged (QListViewItem *)),
			 this, SLOT (selectionChanged (QListViewItem *)));
		selectItemInView (mHier, info, false);
		connect (mHier, SIGNAL (selectionChanged (QListViewItem *)),
				 this, SLOT (selectionChanged (QListViewItem *)));
	}
	if (view != mObjs)
	{
		disconnect (mObjs, SIGNAL (selectionChanged (QListViewItem *)),
			 this, SLOT (selectionChanged (QListViewItem *)));
		selectItemInView (mObjs, info, true);
		connect (mObjs, SIGNAL (selectionChanged (QListViewItem *)),
				 this, SLOT (selectionChanged (QListViewItem *)));
	}
}

void KProfWidget::selectProfileItem (CProfileInfo *info)
{
	// synchronize the three views by selecting the
	// same item in all lists
	selectItemInView (mFlat, info, false);
	selectItemInView (mHier, info, false);
	selectItemInView (mObjs, info, true);
}

void KProfWidget::selectItemInView (QListView *view, CProfileInfo *info, bool examineSubs)
{
	QListViewItem *item = view->firstChild ();
	while (item)
	{
		if (((CProfileViewItem *)item)->getProfile () == info)
		{
			view->clearSelection ();

			for (QListViewItem *parent = item->parent(); parent; parent=parent->parent())
				parent->setOpen(true);

			view->ensureItemVisible (item);
			view->setSelected (item, true);
			return;
		}

		if (examineSubs && item->firstChild ())
			item = item->firstChild ();
		else if (item->nextSibling ())
			item = item->nextSibling ();
		else if (examineSubs)
		{
			item = item->parent();
			if (item)
				item = item->nextSibling ();
		}
		else
			break;
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

void KProfWidget::generateCallGraph ()
{
	// Display the call-graph format selection dialog and generate a
	// call graph
	CCallGraph dialog (this, "Call-Graph Format", true);
	if (dialog.exec ())
	{
		bool currentSelectionOnly = dialog.mSelectedFunction->isChecked ();
		if (dialog.mGraphViz->isChecked ())
			generateDotCallGraph (currentSelectionOnly);
		else
			generateVCGCallGraph (currentSelectionOnly);
	}
}

void KProfWidget::generateVCGCallGraph (bool currentSelectionOnly)
{
	// TODO
}

void KProfWidget::generateDotCallGraph (bool currentSelectionOnly)
{
	// generate a call-graph to a .dot file in a format compatible with
	// GraphViz, a free graph generator from ATT (http://www.research.att.com/sw/tools/graphviz/)

	QString dotfile = KFileDialog::getSaveFileName (QString::null, i18n("*.dot|GraphViz DOT files"), this,
													i18n ("Save call-graph file as..."));

	if (dotfile.isEmpty ())
		return;

	QFile file (dotfile);
	if (!file.open (IO_WriteOnly | IO_Truncate | IO_Translate))
	{
		KMessageBox::error (this, i18n ("File could not be opened for writing."), i18n ("File Error"));
		return;
	}

	QByteArray graph;
	QTextOStream stream (graph);

	stream << "Digraph \"call-graph\" {\n";

	// first create all the nodes
	// TODO: take templates into account in classReg
	QRegExp classReg ("::[^\\(:]*");			// regexp to collect class names

	for (uint i = 0; i < mProfile.count (); i++)
	{
		QString className, methodName, args;

		// extract class name if any
		int classOff = classReg.find (mProfile[i]->name, 0);
		if (classOff > 0)
		{
			className = mProfile[i]->name.left (classOff) + "\\n";
			classOff += 2;						// skip ::
		}
		else
			classOff = 0;

		// extract method name and args
		int methOff = mProfile[i]->name.find ('(', classOff);
		if (methOff > 0)
		{
			methodName = mProfile[i]->name.mid (classOff, methOff - classOff) + "\\n";
//			args = mProfile[i]->name.mid (methOff);
		}
		else
			methodName = mProfile[i]->name.mid (classOff);

		stream << i << " [label=\"" << className << methodName << args << "\"";
		if (mProfile[i]->callers.count()==0 || mProfile[i]->called.count()==0)
			stream << ", shape=box";
		stream << "];\n";
	}

	for (uint i = 0; i < mProfile.count (); i++)
	{
		if (mProfile[i]->recursive)
			stream << i << " -> " << i << " [style=dotted];\n";

		if (mProfile[i]->called.count ())
		{
			stream << i << " -> {";
			for (uint j = 0; j < mProfile[i]->called.count (); j++)
			{
				if (j)
					stream << "; ";
				stream << mProfile[i]->called[j]->ind;
			}
			stream << "};\n";
		}
	}
	stream << "}\n";

	file.writeBlock (graph);
	file.close ();
}

#include "kprofwidget.moc"