/*
 * kprofwidget.cpp
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

#include <stdlib.h>

#include <qhbuttongroup.h>
#include <qfontdialog.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include <qvector.h>
#include <qlabel.h>
#include <qwhatsthis.h>

#include <kapp.h>
#include <kaction.h>
#include <kconfig.h>
#include <klocale.h>
#include <kcmenumngr.h>
#include <kglobal.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kfiledialog.h>
#include <kiconloader.h>
#include <krecentdocument.h>
#include <khtml_part.h>
#include <khtmlview.h>

#include "kprof.h"
#include "kprofwidget.h"
#include "cprofileviewitem.h"
#include "call-graph.h"

#include <qtreemap.h>
#include <qtreemapwindow.h>
#include <qlistviewtreemap.h>
#include <qlistviewtreemapwindow.h>

/*
 * Some globals we need - one of these days I'll have to synthesize this 
 * in a config class or something...
 *
 */
QFont*	KProfWidget::sListFont = NULL;
int		KProfWidget::sLastFileFormat = FORMAT_GPROF;
bool	KProfWidget::sDiffMode = false;


KProfWidget::KProfWidget (QWidget *parent, const char *name)
	:	QWidget (parent, name),
		mTabs (NULL),
		mFlat (NULL),
		mHier (NULL),
		mObjs (NULL),
		mCurPage (0),
		mAbbrevTemplates (false)
{
	sListFont = new QFont;

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
	QWhatsThis::add (flatFilter,
		i18n (	"Type text in this field to filter the display "
				"and only show the functions/methods whose name match the text."));
	QWhatsThis::add (mFlat,
		i18n (	"This is the <I>flat view</I>.\n\n"
				"It displays all functions and method 'flat'. Click on a column header "
				"to sort the list on this column (click a second time to reverse the "
				"sort order)."));
	QWhatsThis::add (mHier,
		i18n (	"This is the <I>hierarchical view</I>.\n\n"
				"It displays each function/method like a tree to let you see the other "
				"functions that it calls. For that reason, each function may appear several "
				"times in the list."));
	QWhatsThis::add (mObjs,
		i18n (	"This is the <I>object view</I>.\n\n"
				"It displays C++ methods grouped by object name."));
}

KProfWidget::~KProfWidget ()
{
	delete sListFont;
}

void KProfWidget::tabSelected (int page)
{
	mCurPage = page;
}

void KProfWidget::toggleTemplateAbbrev ()
{
	mAbbrevTemplates = mAbbrevTemplates ? false : true;

	KToggleAction *action = ((KProfTopLevel *) parent ())->getToggleTemplateAbbrevAction ();
	if (action)
		action->setChecked (mAbbrevTemplates);

	if (mProfile.count ())
	{
		postProcessProfile (false);	// regenerate simplified names
		mFlat->clear ();			// rebuild lists to make sure refresh is done
		mHier->clear ();
		mObjs->clear ();
		fillFlatProfileList ();
		fillHierProfileList ();
		fillObjsProfileList ();
	}
}

void KProfWidget::selectListFont ()
{
	bool ok = false;
	QFont f = QFontDialog::getFont (&ok, *sListFont, this, "font selector");
	if (ok) {
		delete sListFont;
		sListFont = new QFont (f);
		mFlat->setFont (*sListFont);
		mHier->setFont (*sListFont);
		mObjs->setFont (*sListFont);
	}
}

void KProfWidget::prepareProfileView (KListView *view, bool rootIsDecorated)
{
	for (int i = view->columns(); i > 0; )
		view->removeColumn (--i);

	view->addColumn (i18n("Function/Method"), -1);
	
	if (sDiffMode)
	{
		// in diff mode, each number column is doubled
		view->addColumn (i18n ("Remarks"));
		view->addColumn (i18n ("Old Count"), -1);
		view->addColumn (i18n ("New Count"), -1);
		view->addColumn (i18n ("Old Total (s)"), -1);
		view->addColumn (i18n ("New Total (s)"), -1);
		view->addColumn (i18n ("Old %"), -1);
		view->addColumn (i18n ("New %"), -1);
		view->addColumn (i18n ("Old Self (s)"), -1);
		view->addColumn (i18n ("New Self (s)"), -1);
		view->addColumn (i18n ("Old Total ms/call"), -1);
		view->addColumn (i18n ("New Total ms/call"), -1);
		
		view->setColumnAlignment (diff_col_status, AlignCenter);
		view->setColumnAlignment (diff_col_count, AlignRight);
		view->setColumnAlignment (diff_col_new_count, AlignRight);
		view->setColumnAlignment (diff_col_total, AlignRight);
		view->setColumnAlignment (diff_col_new_total, AlignRight);
		view->setColumnAlignment (diff_col_totalPercent,	AlignRight);
		view->setColumnAlignment (diff_col_new_totalPercent,	AlignRight);
		view->setColumnAlignment (diff_col_self, AlignRight);
		view->setColumnAlignment (diff_col_new_self, AlignRight);
		view->setColumnAlignment (diff_col_totalMsPerCall, AlignRight);
		view->setColumnAlignment (diff_col_new_totalMsPerCall, AlignRight);
	}
	else
	{
		view->addColumn (i18n ("Count"), -1);
		view->addColumn (i18n ("Total (s)"), -1);
		view->addColumn (i18n ("%"), -1);
		view->addColumn (i18n ("Self (s)"), -1);
		view->addColumn (i18n ("Total ms/call"), -1);
		
		view->setColumnAlignment (col_count, AlignRight);
		view->setColumnAlignment (col_total, AlignRight);
		view->setColumnAlignment (col_totalPercent,	AlignRight);
		view->setColumnAlignment (col_self, AlignRight);
		view->setColumnAlignment (col_totalMsPerCall, AlignRight);
	}

	view->setAllColumnsShowFocus (true);
	view->setFrameStyle (QFrame::WinPanel + QFrame::Sunken);
	view->setShowSortIndicator (true);
	view->setRootIsDecorated (rootIsDecorated);
	view->setItemMargin (2);
}

void KProfWidget::customizeColumns (KListView *view, int profiler)
{
	// customize the columns for the profiler we are using
	switch (profiler)
	{
		case FORMAT_GPROF:
			if (sDiffMode)
			{
				view->addColumn (i18n("Old Self ms/call"), -1);
				view->addColumn (i18n("New Self ms/call"), -1);
				view->setColumnAlignment (diff_col_selfMsPerCall, AlignRight);
				view->setColumnAlignment (diff_col_new_selfMsPerCall, AlignRight);
			}
			else
			{
				view->addColumn (i18n("Self ms/call"), -1);
				view->setColumnAlignment (col_selfMsPerCall, AlignRight);
			}
			break;

		case FORMAT_FNCCHECK:
			if (sDiffMode)
			{
				view->addColumn (i18n("Old Min. ms/call"), -1);
				view->addColumn (i18n("New Min. ms/call"), -1);
				view->addColumn (i18n("Old Max. ms/call"), -1);
				view->addColumn (i18n("New Max. ms/call"), -1);
				view->setColumnAlignment (diff_col_minMsPerCall, AlignRight);
				view->setColumnAlignment (diff_col_new_minMsPerCall, AlignRight);
				view->setColumnAlignment (diff_col_maxMsPerCall, AlignRight);
				view->setColumnAlignment (diff_col_new_maxMsPerCall, AlignRight);
			}
			else
			{
				view->addColumn (i18n("Min. ms/call"), -1);
				view->addColumn (i18n("Max. ms/call"), -1);
				view->setColumnAlignment (col_minMsPerCall, AlignRight);
				view->setColumnAlignment (col_maxMsPerCall, AlignRight);
			}
			break;

		case FORMAT_POSE:
			if (sDiffMode)
			{
				view->addColumn (i18n("Old Self Cycles"), -1);
				view->addColumn (i18n("New Self Cycles"), -1);
				view->addColumn (i18n("Old Total Cycles"), -1);
				view->addColumn (i18n("New Total Cycles"), -1);
				view->setColumnAlignment (diff_col_selfCycles, AlignRight);
				view->setColumnAlignment (diff_col_new_selfCycles, AlignRight);
				view->setColumnAlignment (diff_col_cumCycles, AlignRight);
				view->setColumnAlignment (diff_col_new_cumCycles, AlignRight);
			}
			else
			{
				view->addColumn (i18n("Self Cycles"), -1);
				view->addColumn (i18n("Total Cycles"), -1);
				view->setColumnAlignment (col_selfCycles, AlignRight);
				view->setColumnAlignment (col_cumCycles, AlignRight);
			}
			break;
	}
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
	config.writeEntry ("AbbreviateTemplates", mAbbrevTemplates);
	config.writeEntry ("Font", sListFont->rawName ());
	config.writeEntry ("LastFileFormat", sLastFileFormat);
	update ();
}

void KProfWidget::loadSettings ()
{
	KConfig &config = *kapp->config ();
	config.setGroup ("KProfiler");

	mAbbrevTemplates = config.readBoolEntry ("AbbreviateTemplates", true);
	KToggleAction *action = ((KProfTopLevel *) parent ())->getToggleTemplateAbbrevAction ();
	if (action)
		action->setChecked (mAbbrevTemplates);

	QString s = config.readEntry ("Font", sListFont->rawName ());
	if (s)
	{
		sListFont->setRawName (s);
		mFlat->setFont (*sListFont);
		mHier->setFont (*sListFont);
		mObjs->setFont (*sListFont);
	}

	sLastFileFormat = config.readNumEntry ("LastFileFormat", FORMAT_GPROF);
}

void KProfWidget::openRecentFile (const KURL& url)
{
	QString filename = url.path ();
	QString protocol = url.protocol ();
	if (protocol == "file-gprof")
		openFile (filename, FORMAT_GPROF, false);
	else if (protocol == "file-fnccheck")
		openFile (filename, FORMAT_FNCCHECK, false);
	else if (protocol == "file-pose")
		openFile (filename, FORMAT_POSE, false);
	else
		openFile (filename, -1);
}

void KProfWidget::compareFile ()
{
	// here we do not customize the file open dialog since the compared
	// file should be the same type
	QString f = KFileDialog::getOpenFileName (mCurDir.absPath(), QString::null, this, i18n ("Select a profiling results file to compare..."));
	if (f.isEmpty ())
		return;
	openFile (f, sLastFileFormat, true);
}

void KProfWidget::openResultsFile ()
{
	// customize the Open File dialog: we add
	// a few widgets at the end which allow the user
	// to give us a hint at which profiler the results
	// file comes from (GNU gprof, Function Check, Palm OS Emulator)
	KFileDialog fd (mCurDir.absPath(), QString::null, this, NULL, i18n ("Select a profiling results file"));

	QWidget *w = fd.getMainWidget ();
	QLayout *layout = w->layout ();

	QHBoxLayout *hl = new QHBoxLayout (layout);
	hl->add (new QLabel (i18n ("Text File Format:"), w));
	hl->addSpacing (10);

	QButtonGroup *bg = new QHButtonGroup (w);
	bg->setRadioButtonExclusive (true);
	QRadioButton *fmtGPROF = new QRadioButton (i18n ("GNU gprof  "), bg);
	QRadioButton *fmtFNCCHECK = new QRadioButton (i18n ("Function Check  "), bg);
	QRadioButton *fmtPOSE = new QRadioButton (i18n ("Palm OS Emulator"), bg);

	// reset format button to last used format
	if (sLastFileFormat == FORMAT_GPROF && !fmtGPROF->isOn ())
		fmtGPROF->toggle ();
	else if (sLastFileFormat == FORMAT_FNCCHECK && !fmtFNCCHECK->isOn ())
		fmtFNCCHECK->toggle ();
	else if (!fmtPOSE->isOn ())
		fmtPOSE->toggle ();

	hl->add (bg);
	hl->addStretch ();
	
    fd.exec();

    QString filename = fd.selectedFile();
    if (!filename.isEmpty())
    {
		sLastFileFormat =	fmtGPROF->isChecked () ?		FORMAT_GPROF :
	                        fmtFNCCHECK->isChecked () ?		FORMAT_FNCCHECK :
    	                   									FORMAT_POSE;
		openFile (filename, sLastFileFormat, false);
	}
}

void KProfWidget::openFile (const QString &filename, int format, bool compare)
{
	bool isExec = false;

	if (filename.isEmpty ())
		return;
	
	// if the file is an executable file, generate the profiling information
	// directly from gprof and the gmon.out file
	QFileInfo finfo (filename);
	if (finfo.isExecutable ())
	{
		isExec = true;

		// prepare the "gmon.out" filename
		QString outfile = finfo.dirPath() + "/gmon.out";
		QFileInfo gmonfinfo (outfile);
		if (!gmonfinfo.exists ())
		{
			outfile = finfo.dirPath () + "/fnccheck.out";
			QFileInfo fnccheckinfo (outfile);
			if (!fnccheckinfo.exists ())
			{
				KMessageBox::error (this, i18n ("Can't find any profiling output file\n(gmon.out or fnccheck.out)"), i18n ("File not found"));
				return;
			}
			else
				format = FORMAT_FNCCHECK;
		}
		else
			format = FORMAT_GPROF;

		KProcess profile_generator;
		if (format == FORMAT_GPROF)
		{
			// GNU gprof analysis of gmon.out file
			// exec "gprof -b filename gmon.out"
			profile_generator << "gprof" << "-b" << filename << outfile;
		}
		else
		{
			// Function Check analysis of fnccheck.out file
			// exec "fncdump +calls -no-decoration filename"
			profile_generator << "fncdump" << "+calls" << "-no-decoration" << "-sfile" << outfile << filename;
		}

		mGProfStdout = "";
		mGProfStderr = "";
		connect (&profile_generator, SIGNAL (receivedStdout (KProcess*, char*, int)), this, SLOT (gprofStdout (KProcess*, char*, int)));
		connect (&profile_generator, SIGNAL (receivedStderr (KProcess*, char*, int)), this, SLOT (gprofStderr (KProcess*, char*, int)));

		profile_generator.start (KProcess::Block, KProcess::AllOutput);

		if (!profile_generator.normalExit () || profile_generator.exitStatus ())
		{
			QString text = i18n ("Could not generate the profile data.\n");
			if (profile_generator.normalExit () && profile_generator.exitStatus ())
			{
				QString s;
				s.sprintf (i18n ("Error %d was returned.\n"), profile_generator.exitStatus ());
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


		// if we are going to compare results, save the previous results and
		// remove any previously deleted entry
		sDiffMode = compare;
		if (compare)
		{
			mPreviousProfile = mProfile;
			for (uint i=mPreviousProfile.count(); i > 0; )
			{
				if (mPreviousProfile[--i]->deleted)
					mPreviousProfile.remove (i);
				mPreviousProfile[i]->previous = NULL;
			}
		}
		mProfile.clear ();

		// parse profile data
		QTextStream t (&mGProfStdout, IO_ReadOnly);

		if (format == FORMAT_GPROF)
			parseProfile_gprof (t);
		else
			parseProfile_fnccheck (t);
	}
	else
	{
		// if user tried to open gmon.out, have him select the executable instead, then recurse to use
		// the executable file
		if (finfo.fileName() == "gmon.out")
		{
			KMessageBox::error (this, i18n ("File 'gmon.out' is the result of the execution of an application\nwith gprof(1) profiling turned on.\nYou can not open it as such: either open the application itself\nor open a text results file generated with 'gprof -b application-name'"),
								i18n ("Opening gmon.out not allowed"));
			return;
		}
		else if (finfo.fileName() == "fnccheck.out")
		{
			KMessageBox::error (this, i18n ("File 'fnccheck.out' is the result of the execution of an application\nwith Function Check profiling turned on.\nYou can not open it as such: either open the application itself\nor open a text results file generated with 'fncdump +calls application-name'"),
								i18n ("Opening fnccheck.out not allowed"));
			return;
		}

		mFlat->clear ();
		mHier->clear ();
		mObjs->clear ();

		// if we are going to compare results, save the previous results and
		// remove any previously deleted entry
		sDiffMode = compare;
		if (compare)
		{
			mPreviousProfile = mProfile;
			for (uint i=mPreviousProfile.count(); i > 0; )
			{
				if (mPreviousProfile[--i]->deleted)
					mPreviousProfile.remove (i);
				mPreviousProfile[i]->previous = NULL;
			}
		}
		mProfile.clear ();

		// parse profile data
		QFile f (filename);
		if (!f.open (IO_ReadOnly))
			return;

		QTextStream t (&f);
		if (format == FORMAT_GPROF)
			parseProfile_gprof (t);
		else if (format == FORMAT_FNCCHECK)
			parseProfile_fnccheck (t);
		else
			parseProfile_pose (t);
	}

	// post-process the parsed data
	postProcessProfile (compare);

	// customize the on-screen columns
	prepareProfileView (mFlat, false);
	customizeColumns (mFlat, sLastFileFormat);
	prepareProfileView (mHier, true);
	customizeColumns (mHier, sLastFileFormat);
	prepareProfileView (mObjs, true);
	customizeColumns (mObjs, sLastFileFormat);

	QListViewItem *obj_toplevel=new QListViewItem(mObjs,"Objects");
	QListViewItem *hier_toplevel=new QListViewItem(mHier,"Hierarchy");

	// fill lists
	fillFlatProfileList ();
	fillHierProfileList ();
	fillObjsProfileList ();

	// make sure we add the recent file (this also changes the window title)
	KURL url (filename);
	url.setProtocol (isExec ? "file" : sLastFileFormat==FORMAT_GPROF ? "file-gprof" : sLastFileFormat==FORMAT_FNCCHECK ? "file-fnccheck" : "file-pose");
	emit addRecentFile (url);

	// update the current directory
	mCurDir = finfo.dir ();

	// create options for the treemap

	treemap_options=new QTreeMapOptions();
	//treemap_options->path_separator=QString("::");
	treemap_options->calc_nodesize=CALCNODE_ALWAYS;
	// open up the treemap widget

	obj_treemap= new QListViewTreeMapWindow(KProfWidget::col_function,KProfWidget::col_totalPercent);
	//obj_treemap= new QListViewTreeMapWindow(KProfWidget::col_function,KProfWidget::col_count);
	obj_treemap->makeWidgets();
	obj_treemap->makeColumnMenu(mObjs);
	obj_treemap->getArea()->setOptions(treemap_options);
	obj_treemap->getArea()->setTreeMap((Object *)mObjs->firstChild());
	obj_treemap->setWindowTitle("KProf Object Profile");

	hier_treemap= new QListViewTreeMapWindow(KProfWidget::col_function,KProfWidget::col_totalPercent);
	hier_treemap->makeWidgets();
	hier_treemap->makeColumnMenu(mHier);
	hier_treemap->getArea()->setOptions(treemap_options);
	hier_treemap->getArea()->setTreeMap((Object *)mHier->firstChild());
	hier_treemap->setWindowTitle("KProf Hierarchy Profile");


}

void KProfWidget::gprofStdout (KProcess *, char *buffer, int buflen)
{
	mGProfStdout += QString::fromLocal8Bit (buffer, buflen);
}

void KProfWidget::gprofStderr (KProcess *, char *buffer, int buflen)
{
	mGProfStderr += QString::fromLocal8Bit (buffer, buflen);
}

void KProfWidget::parseProfile_pose (QTextStream& t)
{
	/*
	 * parse a profile results file generated by the PalmOS Emulator
	 *
	 */

	int line = 0;
	int numEntries = 0;
	int cgCount = 0;
	QString s;

	// because of the way POSE results are shown, we have to keep a dictionnary
	// of indexes -> CProfileInfo*, and a list of call maps index -> parent index
	QAsciiDict<CProfileInfo> functions (257);
	SPoseCallGraph* callGraph = (SPoseCallGraph *) malloc (256 * sizeof (SPoseCallGraph));
	
	// mapping between indexes and profile ptrs. because of the number of indexes
	// one typically encounters in a profile results file, use of a dictionnary
	// leads to very slow parsing. This is why I use the array below...
	int indexes = 8192; 
	CProfileInfo **indexToProfile = (CProfileInfo **) malloc (indexes * sizeof (CProfileInfo *));
	for (int i=0; i < indexes; i++)
		indexToProfile[i] = NULL;

	t.setEncoding (QTextStream::Latin1);
	while (!t.eof ())
	{
		if (++line == 0)
			continue;			// skip first line, it only contains field descriptor
		
		s = t.readLine ();
		if (s.length() == 0)
			continue;

		// split the line fields
		QStringList fields;
		fields = QStringList::split ("\t", s, false);
		if (fields.isEmpty ())
			continue;
		for (uint i=0; i < fields.count(); i++)
			fields[i] = fields[i].stripWhiteSpace();

		if (fields.count() != 15)
		{
			line--;
			continue;
		}

		// gather the index of this entry
		int ind = fields[0].toInt();
		if (ind > 512000) {
			// uh ? this is probably a parsing problem!
			line--;
			continue;
		}

		// first look if we have a dictionnary entry for this function
		bool created = false;
		CProfileInfo *p = functions.find (fields[3].latin1());
		if (p == NULL)
		{
			// nope: create a new one
			p = new CProfileInfo;
			functions.insert (fields[3].latin1(), p);
			p->ind  = numEntries;
			p->name = fields[3];
			created = true;
		}

		// add entry to the indexes dictionary
		if (ind >= indexes)
		{
			int n = ((ind / 8192) + 1) * 8192;
			indexToProfile = (CProfileInfo **) realloc (indexToProfile, n * sizeof (CProfileInfo *));
			// @@@ TODO: test and report memory error here
			for (int i = indexes; i < n; i++)
				indexToProfile[i] = NULL;
			indexes = n;
		}
		indexToProfile[ind] = p;

		// add caller to the callers list
		if (cgCount && (cgCount & 0xff)==0)
			callGraph = (SPoseCallGraph *) realloc (callGraph, (cgCount + 256) * sizeof (SPoseCallGraph));

		callGraph[cgCount].index = ind;
		callGraph[cgCount++].parent = fields[1].toInt ();

		p->cumPercent		+= fields[10].toFloat ();
		p->cumSeconds		+= fields[9].toFloat () / 1000.0;		// value given in milliseconds
		p->selfSeconds		+= fields[6].toFloat () / 1000.0;		// value given in milliseconds
		p->calls			+= fields[4].toLong ();
		p->custom.pose.selfCycles+= fields[5].toLong ();
		p->custom.pose.cumCycles += fields[8].toLong ();

		// @@@ TODO: check and fix this
		float v = fields[11].toFloat ();
		if (v > p->totalMsPerCall)
			p->totalMsPerCall = v;
		p->totalMsPerCall	+= fields[5].toFloat ();

		// p->simplifiedName will be updated in postProcessProfile()
		p->recursive		= false;
		p->object			= getClassName (p->name);
		p->multipleSignatures = false;								// will be updated in postProcessProfile()

		if (created)
		{
			int argsoff = p->name.find ('(');
			if (argsoff != -1)
			{
				p->method = p->name.mid (p->object.length(), argsoff - p->object.length());
				p->arguments  = p->name.right (p->name.length() - argsoff);
			}
			else
				p->method = p->name.right (p->name.length() - p->object.length());

			if (p->method.startsWith ("::"))
				p->method.remove (0,2);

			mProfile.resize (numEntries + 1);
			mProfile.insert (numEntries++, p);
		}
	}

	// post-process call-graph data
	for (int i=0; i < cgCount; i++)
	{
		int father = callGraph[i].parent;
		int child  = callGraph[i].index;
		if (father != -1)
		{
			CProfileInfo *pFather = indexToProfile [father];
			CProfileInfo *pChild  = indexToProfile [child];

			// these errors should not happen in a well-formed profile result,
			// but who knows...
			if (pFather == NULL) {
				fprintf (stderr, "kprof: pFather==NULL: No profile entry for index %d!\n", father);
				continue;
			}
			if (pChild == NULL) {
				fprintf (stderr, "kprof: pChild==NULL: No profile entry for index %d!\n", child);
				continue;
			}

			if (pFather == pChild)
				pFather->recursive = true;
			else
			{
				if (pFather->called.count()==0 || pFather->called.find (pChild) == -1)
				{
					int n = pFather->called.count ();
					pFather->called.resize (n + 1);
					pFather->called [n] = pChild;
				}
				if (pChild->callers.count()==0 || pChild->callers.find (pFather) == -1)
				{
					int n = pChild->callers.count ();
					pChild->callers.resize (n + 1);
					pChild->callers [n] = pFather;
				}
			}
		}
	}

	free (indexToProfile);
	free (callGraph);
}

bool KProfWidget::parseProfile_fnccheck (QTextStream& t)
{
	/*
	 * parse a profile results generated with FUNCTION CHECK
	 *
	 */

	// while parsing, we temporarily store all entries of a call graph block
	// in an array
	QVector<SCallGraphEntry> callGraphBlock;
	callGraphBlock.setAutoDelete (true);
	callGraphBlock.resize (32);

	int state = ANALYZING;
	long line = 0;
	bool processingCallers=false;	// used during call-graph processing
	int curFunctionIndex = -1;		// current function index while processing call-graph
	QString s;

	t.setEncoding (QTextStream::Latin1);
	while (!t.eof ())
	{
		line++;
		s = t.readLine ();
		s = s.simplifyWhiteSpace ();
		if (s.length() == 0)
			continue;
		if (s[0] == '<')			// marks the end of the current section
		{
			state = ANALYZING;
			continue;
		}

		// check whether the file is valid or not
		if (line==1 && s[0]!='>')
		{
			QMessageBox::critical (this, i18n("Invalid File"),i18n("This file does not appear to be\na valid Function Check output file.\nIt should have been generated using the following command line:\nfncdump +calls -no-decoration {application_name}"));
			return false;
		}

		// look for markers to change the state
		QStringList fields;
		if (s.startsWith (">cycles"))
			state = PROCESS_CYCLES;
		else if (s.startsWith (">profile"))
			state = PROCESS_FLAT_PROFILE;
		else if (s.startsWith (">minmax"))
			state = PROCESS_MIN_MAX_TIME;
		else if (s.startsWith (">callgraph"))
			state = PROCESS_CALL_GRAPH;
		else if (state != ANALYZING)
		{
			uint pos=0,i;
			for (i=0; i<s.length(); i++)
			{
				if (s[i]==' ')
				{
					if (i==pos)
						fields += "";
					else
						fields += s.mid (pos, i-pos);
					pos = i+1;
				}
				else if (pos==i && s[i]=='"')
				{
					while (s[++i] != '"')
						;
					pos++;
					fields += s.mid (pos, i-pos);
					pos = ++i + 1;
				}
			}
			if (i > pos)
				fields += s.mid (pos,i-pos);
			if (fields.isEmpty ())
				continue;
			for (uint i=0; i < fields.count(); i++)
				fields[i] = fields[i].stripWhiteSpace();
		}

		switch (state)
		{
			/*
			 * analyze cycles detected during execution
			 *
			 */
			case PROCESS_CYCLES:
				// @@@ TODO
				break;

			/*
			 * analyze flat profile entry
			 *
			 */
     		case PROCESS_FLAT_PROFILE:
			{
				if (fields.count() != 7)
					break;
				if (!fields[0].length())
					break;
				if (!fields[0][0].isDigit ())
					break;

				CProfileInfo *p = new CProfileInfo;
				p->ind				= mProfile.count ();
				p->cumPercent		= fields[3].toFloat ();
				p->cumSeconds		= fields[2].toFloat ();
				p->selfSeconds		= fields[0].toFloat ();
				p->calls			= fields[4].toLong ();
				p->totalMsPerCall	= fields[5].toFloat () * 1000.0;
				p->name				= fields[6];

				// p->simplifiedName will be updated in postProcessProfile()
				p->recursive		= false;
				p->object			= getClassName (p->name);
				p->multipleSignatures = false;				// will be updated in postProcessProfile()

				int argsoff = p->name.find ('(');
				if (argsoff != -1)
				{
					p->method = p->name.mid (p->object.length(), argsoff - p->object.length());
					p->arguments  = p->name.right (p->name.length() - argsoff);
				}
				else
					p->method = p->name.right (p->name.length() - p->object.length());

				if (p->method.startsWith ("::"))
					p->method.remove (0,2);

				int j = mProfile.size ();
				mProfile.resize (j + 1);
				mProfile.insert (j, p);
				break;
			}

			/*
			 * analyze MIN/MAX time per function
			 *
			 */
			case PROCESS_MIN_MAX_TIME:
			{
				// we assume that the flat profile and the min/max time profile
				// are displayed using the same order
				if (fields.count() != 4)
					break;
				if (!fields[0].length())
					break;
				if (!fields[0][0].isDigit ())
					break;

				curFunctionIndex = fields[0].toInt ();
				if (curFunctionIndex < 0 || (uint)curFunctionIndex >= mProfile.count())
				{
					fprintf (stderr, "kprof: missing flat profile entry for [%d] (line %ld)\n", curFunctionIndex, line);
					break;
				}

				CProfileInfo *p = mProfile[curFunctionIndex];
				p->custom.fnccheck.minMsPerCall = fields[1].toFloat() * 1000.0;
				p->custom.fnccheck.maxMsPerCall = fields[2].toFloat() * 1000.0;
				break;
			}

    		/*
			 * analyze call graph entry
			 *
			 */
    		case PROCESS_CALL_GRAPH:
			{
				if (s[0]=='"')
				{
					// found another call-graph entry
					processingCallers = fields.count() == 3;
					curFunctionIndex = fields[1].toInt ();
					break;
				}
				if (curFunctionIndex < 0 || (uint)curFunctionIndex >= mProfile.count())
					break;

				if (fields[0]=="*")
				{
					// "this function does not call anyone we know"
					curFunctionIndex = -1;
					break;
				}

				// process current call-graph entry
				CProfileInfo *p = mProfile[curFunctionIndex];
				for (uint i=0; i < fields.count(); i++)
				{
					int referred = fields[i].toInt ();
					if (referred < 0 || (uint)referred >= mProfile.count())
					{
						fprintf (stderr, "kprof: missing flat profile entry for [%d] (line %ld)\n", referred, line);
						break;
					}

					if (referred == curFunctionIndex)
						p->recursive = true;
					else
					{
						CProfileInfo *pi = mProfile[referred];
						if (processingCallers)
						{
							// "called by"
							if (p->callers.count()==0 || p->callers.find(pi)==-1)
							{
								p->callers.resize (p->callers.count () + 1);
								p->callers [p->callers.count () - 1] = pi;
							}
						}
						else
						{
							// "calls"
							if (p->called.count()==0 || p->called.find(pi)==-1)
							{
								p->called.resize (p->called.count () + 1);
								p->called [p->called.count () - 1] = pi;
							}
						}
					}
				}
				curFunctionIndex = -1;
    		}
   		}
	}
	return true;
}

void KProfWidget::parseProfile_gprof (QTextStream& t)
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

	int state = SEARCH_FLAT_PROFILE;
	long line = 0;
	QString s;

	t.setEncoding (QTextStream::Latin1);
	while (!t.eof ())
	{
		line++;
		s = t.readLine ();
		if (s.length() && s[0] == 12)
		{
			if (state == PROCESS_CALL_GRAPH)
			{
				processCallGraphBlock (callGraphBlock);
				break;
			}
			if (state == PROCESS_FLAT_PROFILE)
				state = SEARCH_CALL_GRAPH;
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
				if (fields[0]=="time" && fields[1]=="seconds" && fields[2]=="seconds")
					state = PROCESS_FLAT_PROFILE;
				break;

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
					p->custom.gprof.selfMsPerCall	= fields[4].toFloat ();
					p->totalMsPerCall	= fields[5].toFloat ();
					p->name				= fields[6];
     			}
				else
				{
					// if the profile was generated with -z, uncalled functions
					// have empty "calls", "selfTsPerCall" and "totalTsPerCall" fields
					p->calls			= 0;
					p->custom.gprof.selfMsPerCall = 0;
					p->totalMsPerCall	= 0;
					p->name				= fields[3];
     			}
				// p->simplifiedName will be updated in postProcessProfile()
				p->recursive		= false;
				p->object			= getClassName (p->name);
				p->multipleSignatures = false;				// will be updated in postProcessProfile()

				int argsoff = p->name.find ('(');
				if (argsoff != -1)
				{
					p->method = p->name.mid (p->object.length(), argsoff - p->object.length());
					p->arguments  = p->name.right (p->name.length() - argsoff);
				}
				else
					p->method = p->name.right (p->name.length() - p->object.length());

				if (p->method.startsWith ("::"))
					p->method.remove (0,2);

				int j = mProfile.size ();
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
					state = PROCESS_CALL_GRAPH;
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
			fprintf (stderr, "kprof: missing flat profile entry for '%s' (line %ld)\n", data[i]->name.latin1(), data[i]->line);
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
				if (primary->callers.count()==0 || primary->callers.find(p)==-1)
				{
					primary->callers.resize (primary->callers.count () + 1);
					primary->callers [primary->callers.count () - 1] = p;
				}
    		}
			else
			{
				if (primary->called.count()==0 || primary->called.find(p)==-1)
				{
					primary->called.resize (primary->called.count () + 1);
					primary->called [primary->called.count () - 1] = p;
				}
    		}
      	}
	}
}

void KProfWidget::postProcessProfile (bool compare)
{
	// once we have read a profile information file, we can post-process
	// the data. First, we need to create the list of classes that were
	// found.
	// After that, we check every function/method to see if it
	// has multiple signatures. We mark entries with multiple signatures
	// so that we can display the arguments only when needed
	mClasses.resize (0);
	uint i, j;
	for (i = 0; i < mProfile.count (); i++)
	{
		// fill the class list
		if (!mProfile[i]->object.isEmpty ())
		{
			uint k;
			for (k = 0; k < mClasses.count (); k++)
			{
				if (mClasses[k]->compare (mProfile[i]->object) == 0)
					break;
     		}
			if (k == mClasses.count ())
			{
				mClasses.resize (mClasses.count () + 1);
				mClasses.insert (mClasses.count (), &mProfile[i]->object);
    		}
		}

		// check for multiple signatures
		for (j = i + 1; j < mProfile.count(); j++)
		{
			if (mProfile[i]->multipleSignatures)
				continue;

			if (mProfile[j]->multipleSignatures==false &&
				mProfile[j]->object==mProfile[i]->object &&
				mProfile[j]->method==mProfile[i]->method)
			{
				mProfile[i]->multipleSignatures = true;
				mProfile[j]->multipleSignatures = true;
			}
		}

		// construct the function/method's simplified name
		if (mProfile[i]->multipleSignatures)
			mProfile[i]->simplifiedName	= removeTemplates (mProfile[i]->name);
		else if (mProfile[i]->object.isEmpty ())
			mProfile[i]->simplifiedName = removeTemplates (mProfile[i]->method);
		else
			mProfile[i]->simplifiedName = removeTemplates (mProfile[i]->object) + "::" + removeTemplates (mProfile[i]->method);
  	}

	// profile results comparison: link new entry with previous entry, add deleted entries
	// to the list (marking them "deleted"). To mark entries that we have already seen,
	// set their "output" flag to true. This is temporary, just for the duration of
	// the code below.
	if (compare == false)
		return;
	
	for (i = 0; i < mPreviousProfile.count(); i++)
		mPreviousProfile[i]->output = false;		// reset all "output" flags
	
	for (i = 0; i < mProfile.count(); i++)
	{
		for (j = 0; j < mPreviousProfile.count(); j++)
		{
			if (mPreviousProfile[j]->output == false && mProfile[i]->name == mPreviousProfile[j]->name)
			{
				mProfile[i]->previous = mPreviousProfile[j];
				mPreviousProfile[j]->output = true;
				break;
			}
		}
	}

	for (j = mPreviousProfile.count(); j > 0;)
	{
		if (mPreviousProfile[--j]->output == false)
		{
			// this item was deleted, add it to the new list and mark it 'deleted'
			mPreviousProfile[j]->deleted = true;
			mProfile.resize (mProfile.size() + 1);
			mProfile.insert (mProfile.size() - 1, mPreviousProfile[j]);
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

void KProfWidget::fillFlatProfileList ()
{
	bool filter = mFlatFilter.isEmpty()==false && mFlatFilter.length() > 0;
	for (unsigned int i = 0; i < mProfile.size (); i++)
	{
		if (filter && !mProfile[i]->name.contains (mFlatFilter))
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
		CProfileViewItem *item = new CProfileViewItem (mHier->firstChild(), mProfile[i]);

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
	// create all toplevel elements and their descendants
	for (uint i = 0; i < mClasses.count (); i++)
	{
		CProfileViewItem *parent = new CProfileViewItem (mObjs->firstChild(), NULL);
		for (uint j = 0; j < mProfile.count (); j++)
		{
			if (mProfile[j]->object == *mClasses[i])
				new CProfileViewItem (parent, mProfile[j]);
		}
	}
	
	mObjs->setColumnWidthMode (0, QListView::Manual);
}

void KProfWidget::profileEntryRightClick (QListViewItem *listItem, const QPoint &p, int)
{
	if (!listItem)
		return;

	CProfileViewItem *item = (CProfileViewItem *) listItem;
	CProfileInfo *info = item->getProfile();
	if (info == NULL)
		return;				// in objs profile, happens on class name lines

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
	mFlatFilter = filter;
	fillFlatProfileList ();
}

void KProfWidget::doPrint ()
{
	KListView *view =	mCurPage == 0 ? mFlat :
						mCurPage == 1 ? mHier :
										mObjs;

	QString s;
	s = "<HTML><HEAD><META http-equiv=\"content-type\" content=\"text/html\" charset=\"iso-8859-1\"></HEAD>";
	s += "<BODY bgcolor=\"#FFFFFF\">";
	s += "<TABLE border=\"0\" cellspacing=\"2\" cellpadding=\"1\">";

	QListViewItem *item = view->firstChild ();
	int cols = view->columns();

	s += "<BR><BR><THEAD><TR>";		// two BRs to alleviate for margin problems
	for (int i=0; i<cols; i++)
		if (i==0)
			s += "<TH align=\"left\"><B>" + view->columnText(i) + "</B></TH>";
		else
			s += "<TH align=\"right\"><B>" + view->columnText(i) + "</B></TH>";
	s += "</TR></THEAD><TBODY>";

	while (item) {
		CProfileViewItem *pitem = (CProfileViewItem *) item;
		QString sitem = "<TR valign=\"top\">";
		for (int i=0; i<cols; i++)
			if (i==0)
				sitem += "<TD align=\"left\">" + pitem->text(i) + "</TD>";
			else
				sitem += "<TD align=\"right\">" + pitem->text(i) + "</TD>";
		sitem += "</TR>";
		s += sitem;
		item = item->nextSibling();
	}
	s += "</TBODY></TABLE></BODY></HTML>";

	KHTMLPart *part = new KHTMLPart ();
	part->begin();
	part->write(s);
	part->end();
	part->view()->print();
	delete part;
}

void KProfWidget::generateCallGraph ()
{
	// Display the call-graph format selection dialog and generate a
	// call graph
	CCallGraph dialog (this, "Call-Graph Format", true);
	if (dialog.exec ())
	{
		bool currentSelectionOnly = dialog.mSelectedFunction->isChecked ();

		QListViewItem *selectedItem = NULL;
		if (currentSelectionOnly)
		{
			selectedItem = (mCurPage == 0 ? mFlat->selectedItem() :
							mCurPage == 1 ? mHier->selectedItem() :
							mObjs->selectedItem());
			if (selectedItem == NULL)
			{
				KMessageBox::sorry (this, i18n ("To export the current selection's call-graph,\nyou must select an item in the profile view."), i18n ("Selection Empty"));
				return;
			}
		}

		QString dotfile = KFileDialog::getSaveFileName (
			QString::null,
			dialog.mGraphViz->isChecked () ? i18n("*.dot|GraphViz files") : i18n("*.vcg|VCG files"),
			this,
			i18n ("Save call-graph as..."));
		if (dotfile.isEmpty ())
			return;

		QFile file (dotfile);
		if (!file.open (IO_WriteOnly | IO_Truncate | IO_Translate))
		{
			KMessageBox::error (this, i18n ("File could not be opened for writing."), i18n ("File Error"));
			return;
		}

		if (currentSelectionOnly)
		{
			for (uint i=0; i < mProfile.count(); i++)
				mProfile[i]->output = false;

			CProfileInfo *info = ((CProfileViewItem *) selectedItem)->getProfile ();
			if (info == NULL)
			{
				// probably a parent item in the object profile view;
				// in this case, mark all objects of the same class for output
				QListViewItem *childItem = selectedItem->firstChild ();
				if (childItem)
					info = ((CProfileViewItem *) childItem)->getProfile ();

				if (info == NULL)
				{
					KMessageBox::error (this, i18n ("Internal Error"), i18n ("Could not find any function or class to export."));
					return;
				}

				QString className = info->object;
				for (uint i = 0; i < mProfile.count(); i++)
				{
					if (mProfile[i]->output==false && mProfile[i]->object==info->object)
						markForOutput (mProfile[i]);
				}
			}
			else
				markForOutput (info);
		}

		// graph generation
		if (dialog.mGraphViz->isChecked ())
			generateDotCallGraph (file, currentSelectionOnly);
		else
			generateVCGCallGraph (file, currentSelectionOnly);

		file.close ();
	}
}

void KProfWidget::markForOutput (CProfileInfo *p)
{
	// if true, we already passed this item; avoid entering a recursive loop
	if (p->output)
		return;

	p->output = true;
	for (uint i = 0; i < p->called.count(); i++)
		markForOutput (p->called[i]);
}

void KProfWidget::generateVCGCallGraph (QFile& file, bool currentSelectionOnly)
{
	// generate a call-graph to a .vcg file in a format compatible with
	// VCG, a free graph generator available from http://www.cs.uni-sb.de/RW/users/sander/html/gsvcg1.html
	QByteArray graph;
	QTextOStream stream (graph);

	stream << "graph: {\n";
	stream << "spreadlevel: 1\n";
	stream << "treefactor: 0.5\n";
	stream << "splines: yes\n";
	stream << "node_alignment: bottom\n";
	stream << "orientation: left_to_right\n";

	// first create all the nodes
	for (uint i = 0; i < mProfile.count (); i++)
	{
		if (currentSelectionOnly && mProfile[i]->output==false)
			continue;

		QString className = mProfile[i]->object;
		if (className.length ())
			className += "\\n";

		stream << "node: {title:\"" << i << "\" label:\"" << className << mProfile[i]->method;

		if (mProfile[i]->multipleSignatures)
			stream << "\\n" << mProfile[i]->arguments;
		stream << "\"";

		if (!(mProfile[i]->callers.count()==0 || mProfile[i]->called.count()==0))
			stream << " shape: ellipse";

		stream << "}\n";
	}

	// then output the nodes relationships (edges)
	for (uint i = 0; i < mProfile.count (); i++)
	{
		if (currentSelectionOnly && mProfile[i]->output==false)
			continue;

		if (mProfile[i]->recursive)
			stream << "edge:{sourcename:\"" << i << "\" targetname:\"" << i << "\" thickness:3}\n";

		if (mProfile[i]->called.count ())
		{
			for (uint j = 0; j < mProfile[i]->called.count (); j++)
				stream << "edge:{sourcename:\"" << i << "\" targetname:\"" << mProfile[i]->called[j]->ind << "\"}\n";
		}
	}
	stream << "}\n";

	file.writeBlock (graph);
}

void KProfWidget::generateDotCallGraph (QFile& file, bool currentSelectionOnly)
{
	// generate a call-graph to a .dot file in a format compatible with
	// GraphViz, a free graph generator from ATT (http://www.research.att.com/sw/tools/graphviz/)
	QByteArray graph;
	QTextOStream stream (graph);

	stream << "Digraph \"kprof-call-graph\" {\nratio=fill\n";

	// first create all the nodes
	for (uint i = 0; i < mProfile.count (); i++)
	{
		if (currentSelectionOnly && mProfile[i]->output==false)
			continue;

		QString className = mProfile[i]->object;
		if (className.length ())
			className += "\\n";

		stream << i << " [label=\"" << className << mProfile[i]->method;
		if (mProfile[i]->multipleSignatures)
			stream << "\\n" << mProfile[i]->arguments;
		stream << "\"";
		if (mProfile[i]->callers.count()==0 || mProfile[i]->called.count()==0)
			stream << ", shape=box";
		stream << "];\n";
	}

	// then output the nodes relationships
	for (uint i = 0; i < mProfile.count (); i++)
	{
		if (currentSelectionOnly && mProfile[i]->output==false)
			continue;

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
}

QString KProfWidget::getClassName (const QString &name)
{
	// extract the class name from a complete method prototype
	int args = name.find ('(');
	if (args != -1)
	{
		// remove extra spaces before '(' (should not happen more than once..)
		while (--args>0 && (name[args]==' ' || name[args]=='\t'))
			;
		if (args <= 0)
			return QString ("");

		// if there is a function template as member, make sure we
		// properly skip the template
		if (name[args] == '>')
		{
			int depth = 1;
			while (--args > 0 && depth)
			{
				if (name[args] == '>')
					depth++;
				else if (name[args] == '<')
					depth--;
			}

			// remove extra spaces before '(' (should not happen more than once..)
			while (--args>0 && (name[args]==' ' || name[args]=='\t'))
				;

			if (args <= 0)
				return QString ("");
		}

		while (args > 0)
		{
			if (name[args] == '>')
			{
				// end of another template: this is definitely
				// not a class name
				return "";
			}
			if (name[args]==':' && name[args-1]==':')
			{
				args--;
				break;
			}
			args--;
		}

		if (args <= 0)
			return QString ("");

		// TODO: remove return type by analyzing the class name token
		return name.left (args);
	}
	return QString ("");
}

QString KProfWidget::removeTemplates (const QString &name)
{
	if (mAbbrevTemplates == false)
		return name;

	// remove the templates from inside a name, leaving only
	// the <...> and return the converted name
	QString s (name);
	int tmpl = -1;
	int depth = 0;
	for (uint i=0; i < s.length(); i++)
	{
		if (s[i]=='<')
		{
			if (depth++ == 0)
				tmpl = i;
		}
		else if (s[i] == '>')
		{
			if (depth == 0)
				continue;
			if (--depth == 0)
			{
				s = s.replace (tmpl+1, i-tmpl-1, "...");
				i = tmpl + 4;
				tmpl = -1;
			}
		}
	}
	return s;
}

#include "kprofwidget.moc"
