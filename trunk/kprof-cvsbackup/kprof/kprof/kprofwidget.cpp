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

#include <cstdlib>
#include <cstring>

#include <qhbuttongroup.h>
#include <kfontdialog.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qradiobutton.h>
#include <qvector.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qfile.h>

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
#include <kglobalsettings.h>

#include "config.h"

#include "kprof.h"
#include "kprofwidget.h"
#include "cprofileviewitem.h"
#include "call-graph.h"

#include "dotCallGraph.h"
#include "vcgCallGraph.h"
#include "clientsidemap.h"
#include "parseprofile_gprof.h"
#include "parseprofile_fnccheck.h"
#include "parseprofile_pose.h"

#include "Log.h"

using namespace std;

#ifdef HAVE_LIBQTREEMAP
#include <qtreemap.h>
#include <qtreemapwindow.h>
#include <qlistviewtreemap.h>
#include <qlistviewtreemapwindow.h>
#endif

/*
 * Some globals we need - one of these days I'll have to synthesize this 
 * in a config class or something...
 *
 */
QFont*	KProfWidget::sListFont = NULL;
int		  KProfWidget::sLastFileFormat = FORMAT_GPROF;
bool	  KProfWidget::sDiffMode = false;


KProfWidget::KProfWidget (QWidget *parent, const char *name)
	:	QWidget (parent, name),
		mTabs (NULL),
		mFlat (NULL),
		mHier (NULL),
		mObjs (NULL),
		mCurPage (0),
		mAbbrevTemplates (false)
{
	BEGIN;
	sListFont = new QFont;

	QVBoxLayout *topLayout = new QVBoxLayout (this, 0, 0);

	mTabs = new KTabCtl (this, "tabs");

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

	//create the HTML viewer for the graphical call tree
	mCallTreeHtmlPart = new KHTMLPart(mTabs,  "graph_part");//,mTabs);
	CHECK_PTR(mCallTreeHtmlPart);
	
	//Create the HTML viewer for the method details
	mMethodHtmlPart = new KHTMLPart(mTabs, "method_part");//,mTabs);
	CHECK_PTR(mMethodHtmlPart);

	//KParts::BrowserExtension* ext = mCallTreeHtmlPart->browserExtension();
	connect(mCallTreeHtmlPart->browserExtension(), SIGNAL(openURLRequestDelayed( const KURL &, const KParts::URLArgs &)),
					this, SLOT(openURLRequestDelayed( const KURL &, const KParts::URLArgs & )));

	connect(mMethodHtmlPart->browserExtension(), SIGNAL(openURLRequestDelayed( const KURL &, const KParts::URLArgs &)),
					this, SLOT(openURLRequestDelayed( const KURL &, const KParts::URLArgs & )));

	// add the view to the tab control
 	mTabs->addTab (flatWidget, i18n ("&Flat Profile"));
	mTabs->addTab (mHier, i18n ("&Hierarchical Profile"));
	mTabs->addTab (mObjs, i18n ("O&bject Profile"));
	mTabs->addTab (mCallTreeHtmlPart->view(), i18n("G&raph View"));
	mTabs->addTab (mMethodHtmlPart->view(), i18n("&Method View"));

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

	topLayout->addWidget (mTabs);
	connect (mTabs, SIGNAL (tabSelected (int)), this, SLOT (tabSelected (int)));

	END;
}

KProfWidget::~KProfWidget ()
{
	BEGIN;
	
	if (sListFont != NULL)
	{
		delete sListFont;
		sListFont = 0;
	}

	if (mCallTreeHtmlPart != NULL)
	{
		delete mCallTreeHtmlPart;
		mCallTreeHtmlPart = NULL;
	}
	
	if (mMethodHtmlPart != NULL)
	{	
	delete mMethodHtmlPart;
	mMethodHtmlPart = NULL;
	}

	END;
}

void KProfWidget::tabSelected (int page)
{
	BEGIN;
	
	mCurPage = page;

	END;
}

void KProfWidget::toggleTemplateAbbrev ()
{
	BEGIN;
	
	mAbbrevTemplates = mAbbrevTemplates ? false : true;

	KToggleAction *action = ((KProfTopLevel *) parent ())->getToggleTemplateAbbrevAction ();
	if (action)
		action->setChecked (mAbbrevTemplates);

	if (mProfile.count ())
	{
		postProcessProfile (false);	// regenerate simplified names
		mFlat->clear ();						// rebuild lists to make sure refresh is done
		mHier->clear ();
		mObjs->clear ();
		
		QListViewItem *obj_toplevel = new QListViewItem(mObjs,"Objects");
		QListViewItem *hier_toplevel = new QListViewItem(mHier,"Hierarchy");
		
		fillFlatProfileList ();
		fillObjsProfileList ();
		fillHierProfileList ();

	}

	END;
}



void KProfWidget::selectListFont ()
{
	BEGIN;
  assert(sListFont);
	int i = KFontDialog::getFont (*sListFont);
	if (i == KFontDialog::Accepted) {
		mFlat->setFont (*sListFont);
		mHier->setFont (*sListFont);
		mObjs->setFont (*sListFont);
	}
	END;
}



void KProfWidget::prepareProfileView (KListView *view, bool rootIsDecorated)
{
	BEGIN;
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
	END;
}

void KProfWidget::customizeColumns (KListView *view, int profiler)
{
	// customize the columns for the profiler we are using
	BEGIN;
	
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

	END;
}

void KProfWidget::settingsChanged ()
{
	BEGIN;
	
	applySettings ();
	loadSettings ();

	END;
}

void KProfWidget::applySettings ()
{
	BEGIN;

  assert(sListFont);

  KConfig &config = *kapp->config ();

	config.setGroup ("KProfiler");
	config.writeEntry ("AbbreviateTemplates", mAbbrevTemplates);
	config.writeEntry ("FontName", 						sListFont->family());
	config.writeEntry ("FontSize", 						sListFont->pointSize());
	config.writeEntry ("LastFileFormat", 			sLastFileFormat);
	update();
	
  END;
}

void KProfWidget::loadSettings ()
{
 	BEGIN;
  assert(sListFont);

  KConfig &config = *kapp->config ();
	config.setGroup ("KProfiler");

	//Load settings for C++ Templates
	mAbbrevTemplates = config.readBoolEntry ("AbbreviateTemplates", true);
	KToggleAction *action = ((KProfTopLevel *) parent ())->getToggleTemplateAbbrevAction ();
	if (action)
		action->setChecked (mAbbrevTemplates);

	
  //Load old Font-Settings
	QString FontName = config.readEntry ("FontName", "");
	int     FontSize = config.readNumEntry ("FontSize", 0);
	if (!FontName.isEmpty() && FontSize > 0)
	{
    DBG1("stored Font was=%s",FontName.ascii());
    sListFont->setFamily(FontName);
    sListFont->setPointSize(FontSize);
	}else{
    DBG1("set Font to menuFont=%s",KGlobalSettings::menuFont().rawName().ascii());
    *sListFont=KGlobalSettings::menuFont();
  }
	mFlat->setFont (*sListFont);
	mHier->setFont (*sListFont);
	mObjs->setFont (*sListFont);

	
  //Load last FileFormat settings
	sLastFileFormat = config.readNumEntry ("LastFileFormat", FORMAT_GPROF);
  DBG1("LastFileFormat was=%d",sLastFileFormat);


	//Finish
	END;
}




void KProfWidget::openRecentFile (const KURL& url)
{
	BEGIN;
	
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

	END;
}




void KProfWidget::compareFile ()
{
	BEGIN;

	// here we do not customize the file open dialog since the compared
	// file should be the same type
	QString f = KFileDialog::getOpenFileName (mCurDir.absPath(), QString::null, this, i18n ("Select a profiling results file to compare..."));
	if (f.isEmpty ()){
		END;
		return;
	}
	openFile (f, sLastFileFormat, true);

	END;
}




void KProfWidget::openResultsFile ()
{
	BEGIN;
	
	// customize the Open File dialog: we add
	// a few widgets at the end which allow the user
	// to give us a hint at which profiler the results
	// file comes from (GNU gprof, Function Check, Palm OS Emulator)
	KFileDialog fd (mCurDir.absPath(), QString::null, this, NULL, i18n ("Select a profiling results file"));

	QWidget *w = fd.mainWidget ();
	QLayout *layout = w->layout ();

	QHBoxLayout *hl = new QHBoxLayout (layout);
	hl->add (new QLabel (i18n ("Text File Format:"), w));
	hl->addSpacing (10);

	QButtonGroup *bg = new QHButtonGroup (w);
	bg->setRadioButtonExclusive (true);
	QRadioButton *fmtGPROF = 		new QRadioButton (i18n ("GNU gprof  "), bg);
	QRadioButton *fmtFNCCHECK = new QRadioButton (i18n ("Function Check  "), bg);
	QRadioButton *fmtPOSE = 		new QRadioButton (i18n ("Palm OS Emulator"), bg);

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

		//print a debug message
  	switch(sLastFileFormat){
		case	FORMAT_GPROF:			RUN("Suppose Fileformat is \"GNU gprof\"");				break;
		case	FORMAT_FNCCHECK:	RUN("Suppose Fileformat is \"Function Check\"");	break;
		case	FORMAT_POSE:			RUN("Suppose Fileformat is \"PalmOS Emulator\"");	break;
		}

   	//open the file
		openFile (filename, sLastFileFormat, false);
	}

	END;
}

void KProfWidget::openFile (const QString &filename, int format, bool compare)
{
	BEGIN;

	
	bool isExec = false;

	if (filename.isEmpty ()){
		END;
		return;
	}
	
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
				END;
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
			//return;
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

		CParseProfile* profile = 0;

		if (format == FORMAT_GPROF)
			profile = new CParseProfile_gprof(t, mProfile);
		else
			profile = new CParseProfile_fnccheck(t, mProfile);

		if (!profile->valid())
		{
			KMessageBox::error(this, i18n("This fnccheck file is not in the correct format"));
			END;
			return;
		}
	}
	else
	{
		// if user tried to open gmon.out, have him select the executable instead, then recurse to use
		// the executable file
		if (finfo.fileName() == "gmon.out")
		{
			KMessageBox::error (this, i18n ("File 'gmon.out' is the result of the execution of an application\nwith gprof(1) profiling turned on.\nYou can not open it as such: either open the application itself\nor open a text results file generated with 'gprof -b application-name'"),
								i18n ("Opening gmon.out not allowed"));
			END;
			return;
		}
		else if (finfo.fileName() == "fnccheck.out")
		{
			KMessageBox::error (this, i18n ("File 'fnccheck.out' is the result of the execution of an application\nwith Function Check profiling turned on.\nYou can not open it as such: either open the application itself\nor open a text results file generated with 'fncdump +calls application-name'"),
								i18n ("Opening fnccheck.out not allowed"));
			END;
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
		if (!f.open (IO_ReadOnly)){
			END;
			return;
		}

		QTextStream t (&f);
		if (format == FORMAT_GPROF)
			CParseProfile_gprof (t, mProfile);
		else if (format == FORMAT_FNCCHECK)
			CParseProfile_fnccheck (t, mProfile);
		else
			CParseProfile_pose (t, mProfile);
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

	prepareHtmlPart(mCallTreeHtmlPart);

	//For the time being dump all the method html
	//files to the tmp directory.
	for (unsigned int i = 0; i < mProfile.size (); i++)
	{
		(mProfile[i])->dumpHtml();
 	}


	QListViewItem *obj_toplevel = new QListViewItem(mObjs,"Objects");
	QListViewItem *hier_toplevel = new QListViewItem(mHier,"Hierarchy");

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

	END;

}



void KProfWidget::gprofStdout (KProcess *, char *buffer, int buflen)
{
	BEGIN;
	
	char* newbuf = new char[buflen];
	strncpy(newbuf, buffer, buflen);
	newbuf[buflen] = '\0';

	mGProfStdout += QString::fromLocal8Bit (newbuf, buflen);

	delete [] newbuf;

	END;
}



void KProfWidget::gprofStderr (KProcess *, char *buffer, int buflen)
{
	BEGIN;
	mGProfStderr += QString::fromLocal8Bit (buffer, buflen);
	END;
}

//Handle the standard output from the GraphViz command
void KProfWidget::graphVizStdout (KProcess*, char* buffer, int buflen)
{
	BEGIN;
	mGraphVizStdout += QString::fromLocal8Bit(buffer, buflen);

	END;
}

//Handle the standard errors from the GraphViz command
void KProfWidget::graphVizStderr (KProcess*, char* buffer, int buflen)
{
	BEGIN;
	mGraphVizStderr += QString::fromLocal8Bit(buffer, buflen);
	END;
}

//Handle the standard output from the GraphViz command
void KProfWidget::graphVizDispStdout (KProcess*, char* buffer, int buflen)
{
	BEGIN;
	mGraphVizDispStdout += QString::fromLocal8Bit(buffer, buflen);
	END;
}

//Handle the standard errors from the GraphViz command
void KProfWidget::graphVizDispStderr (KProcess*, char* buffer, int buflen)
{
	BEGIN;
	mGraphVizDispStderr += QString::fromLocal8Bit(buffer, buflen);
	END;
}


void KProfWidget::postProcessProfile (bool compare)
{
	// once we have read a profile information file, we can post-process
	// the data. First, we need to create the list of classes that were
	// found.
	// After that, we check every function/method to see if it
	// has multiple signatures. We mark entries with multiple signatures
	// so that we can display the arguments only when needed
	BEGIN;
	
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

		//construct the HTML name
		QString htmlName = mProfile[i]->object;
		htmlName.replace(QRegExp("<"), "[");
		htmlName.replace(QRegExp(">"),"]");
		mProfile[i]->htmlName = htmlName;


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
	if (compare == false){
		END;
		return;
	}
	
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
	END;
}


void KProfWidget::fillFlatProfileList ()
{
	BEGIN;
	bool filter = mFlatFilter.isEmpty()==false && mFlatFilter.length() > 0;
	for (unsigned int i = 0; i < mProfile.size (); i++)
	{
		if (filter && !mProfile[i]->name.contains (mFlatFilter))
			continue;
		new CProfileViewItem (mFlat, mProfile[i]);
  	}
	mFlat->setColumnWidthMode (0, QListView::Manual);
	update ();
	END;
}


void KProfWidget::fillHierProfileList ()
{
	BEGIN;

	for (unsigned int i = 0; i < mProfile.size(); i++)
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

  END;
	
}//fillHierProfileList()

void KProfWidget::fillHierarchy (
		CProfileViewItem *item,
		CProfileInfo *parent,
		QArray<CProfileInfo *> &addedEntries,
		int &count)
{
	BEGIN;
	for (uint i = 0; i < parent->called.count (); i++)
	{
		// skip items already added to avoid recursion
		if (addedEntries.find (parent->called[i]) != -1)
			continue;

		addedEntries[count++] = parent->called[i];
		CProfileViewItem *newItem = new CProfileViewItem (item, parent->called[i]);
		fillHierarchy (newItem, parent->called[i], addedEntries, count);
	}
	END;
}

void KProfWidget::fillObjsProfileList ()
{
	BEGIN;

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
	END;
}

void KProfWidget::profileEntryRightClick (QListViewItem *listItem, const QPoint &p, int)
{
	BEGIN;

	if (!listItem){
		END;
		return;
	}

	CProfileViewItem *item = (CProfileViewItem *) listItem;
	CProfileInfo *info = item->getProfile();
	if (info == NULL){
		END;
		return;				// in objs profile, happens on class name lines
	}

	KPopupMenu pop (mTabs, 0);

	QArray<CProfileInfo *> itemProf;
	itemProf.resize (info->callers.count () + info->called.count ());
	uint n = 0;

	// if there are no callers nor called functions, return
	if (itemProf.size() == 0){
		END;
		return;
	}

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

	END;
}

void KProfWidget::selectionChanged (QListViewItem *item)
{
	BEGIN;


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

	END;
}

void KProfWidget::selectProfileItem (CProfileInfo *info)
{
	BEGIN;

	// synchronize the three views by selecting the
	// same item in all lists
	selectItemInView (mFlat, info, false);
	selectItemInView (mHier, info, false);
	selectItemInView (mObjs, info, true);

	END;
}

void KProfWidget::selectItemInView (QListView *view, CProfileInfo *info, bool examineSubs)
{
	BEGIN;

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
			END;
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
	END;
}

void KProfWidget::flatProfileFilterChanged (const QString &filter)
{
	BEGIN;
	
	mFlat->clear ();
	mFlatFilter = filter;
	fillFlatProfileList ();

	END;
}

void KProfWidget::doPrint ()
{
	BEGIN;

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

	END;
}

void KProfWidget::generateCallGraph ()
{
	BEGIN;
	
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
				END;
				return;
			}
		}

	if (dialog.mSaveFile->isChecked() )
	{

			QString dotfile = KFileDialog::getSaveFileName (
				QString::null,
				dialog.mGraphViz->isChecked () ? i18n("*.dot|GraphViz files") : i18n("*.vcg|VCG files"),
				this,
				i18n ("Save call-graph as..."));
	
			if (dotfile.isEmpty ()){
				END;
				return;
			}

			QFile file (dotfile);
			if (!file.open (IO_WriteOnly | IO_Truncate | IO_Translate))
			{
				KMessageBox::error (this, i18n ("File could not be opened for writing."), i18n ("File Error"));
				END;
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
						END;
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
				DotCallGraph dotCallGraph(file, currentSelectionOnly, false, mProfile);
			else
				VCGCallGraph vcgCallGraph(file, currentSelectionOnly, mProfile);

			file.close ();
		}
		else
		{
			//Generate a temporary file
			QFile file;

			if (dialog.mGraphViz->isChecked ())
			{
				file.setName(".graphViz_temp");
			}
			else
			{
            	file.setName(".vcg_temp");
			}

			file.open(IO_ReadWrite);
			if (!file.exists())
			{
				KMessageBox::error (this, i18n ("Internal Error"), i18n ("Could not open a temporary file for writing."));
				END;
				return;
			}

			// graph generation
			if (dialog.mGraphViz->isChecked ())
			{
				DotCallGraph dotCallGraph(file, currentSelectionOnly, true, mProfile);
			}
			else
			{
				VCGCallGraph vcgCallGraph(file, currentSelectionOnly, mProfile);
			}

			file.close ();

			KProcess graphApplication;
			KProcess displayApplication;

			if (dialog.mGraphViz->isChecked ())
			{
				graphApplication << "dot" << file.name() << "-Timap" ;
				displayApplication << "dot" << file.name() << "-Tgif" << "-o" << ".graphViz.gif";

				connect (&graphApplication, SIGNAL (receivedStdout (KProcess*, char*, int)), this, SLOT (graphVizStdout (KProcess*, char*, int)));
				connect (&graphApplication, SIGNAL (receivedStderr (KProcess*, char*, int)), this, SLOT (graphVizStderr (KProcess*, char*, int)));

				graphApplication.start(KProcess::Block, KProcess::AllOutput);
				displayApplication.start();

				QFile mapFile;
				QTextStream t (&mGraphVizStdout, IO_ReadOnly);
				mapFile.setName("./.kprof.html");
				mapFile.open(IO_ReadWrite);
				 ClientSideMap(t, mapFile);
			}
			else
			{
				graphApplication << "xvcg" << file.name() ;
				graphApplication.start();
			}
		
			if (!graphApplication.normalExit () || graphApplication.exitStatus ())
			{
				QString text = i18n ("Could not display the data.\n");
				if (graphApplication.normalExit () && graphApplication.exitStatus ())
				{
					QString s;
					s.sprintf (i18n ("Error %d was returned.\n"), graphApplication.exitStatus ());
					text += s;
				}
				KMessageBox::error (this, text, i18n ("xvcg exited with error(s)"));
				END;
				return;
			}
		}
	}
	END;
}

void KProfWidget::markForOutput (CProfileInfo *p)
{
	BEGIN;
	
	// if true, we already passed this item; avoid entering a recursive loop
	if (p->output){
		END;
		return;
	}

	p->output = true;
	for (uint i = 0; i < p->called.count(); i++)
		markForOutput (p->called[i]);

	END;
}





QString KProfWidget::getClassName (const QString &name)
{
	BEGIN;
	// extract the class name from a complete method prototype
	int args = name.find ('(');
	if (args != -1)
	{
		// remove extra spaces before '(' (should not happen more than once..)
		while (--args>0 && (name[args]==' ' || name[args]=='\t'))
			;
		if (args <= 0){
			END;
			return QString ("");
		}

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

			if (args <= 0){
				END;
				return QString ("");
			}
		}

		while (args > 0)
		{
			if (name[args] == '>')
			{
				// end of another template: this is definitely
				// not a class name
				END;
				return "";
			}
			if (name[args]==':' && name[args-1]==':')
			{
				args--;
				break;
			}
			args--;
		}

		if (args <= 0){
			END;
			return QString ("");
		}

		// TODO: remove return type by analyzing the class name token
		END;
		return name.left (args);
	}
	END;
	return QString ("");
}

QString KProfWidget::removeTemplates (const QString &name)
{
	BEGIN;
	if (mAbbrevTemplates == false){
		END;
		return name;
	}

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
	END;
	return s;
}

/** If the QTreeMapView library is present, display the call tree in this format */
void KProfWidget::displayTreeMapView()
{
#ifdef HAVE_LIBQTREEMAP
	BEGIN;
	// create options for the treemap
	mTreemapOptions = new QTreeMapOptions ();
	mTreemapOptions->calc_nodesize = CALCNODE_ALWAYS;
	
	// open up the treemap widget
	mObjTreemap = new QListViewTreeMapWindow (KProfWidget::col_function, KProfWidget::col_totalPercent);
	mObjTreemap->makeWidgets ();
	mObjTreemap->makeColumnMenu (mObjs);
	mObjTreemap->getArea()->setOptions (mTreemapOptions);
	mObjTreemap->getArea()->setTreeMap ((Object *)mObjs->firstChild ());
	mObjTreemap->setWindowTitle (i18n ("KProf Object Profile"));

	mHierTreemap = new QListViewTreeMapWindow (KProfWidget::col_function, KProfWidget::col_totalPercent);
	mHierTreemap->makeWidgets ();
	mHierTreemap->makeColumnMenu (mHier);
	mHierTreemap->getArea()->setOptions (mTreemapOptions);
	mHierTreemap->getArea()->setTreeMap ((Object *)mHier->firstChild());
	mHierTreemap->setWindowTitle (i18n ("KProf Hierarchy Profile"));
	END;
#endif
}

//This method takes the call graph and turns it into a client side image map
//in the html viewer
void KProfWidget::prepareHtmlPart(KHTMLPart* part)
{
	BEGIN;
	//Generate a temporary file
	QFile file;

	file.setName("/tmp/graphViz_temp");
	if (file.exists())
	{
		file.remove();
	}

	file.open(IO_ReadWrite);

	DotCallGraph dotCallGraph(file, true, true, mProfile);
	file.close ();

	KProcess graphApplication;
	KProcess displayApplication;

	graphApplication << "dot" << file.name() << "-Timap" ;
	displayApplication << "dot" << file.name() << "-Tjpg" << "-o" << "/tmp/graphViz.jpg";

	connect (&graphApplication, SIGNAL (receivedStdout (KProcess*, char*, int)), this, SLOT (graphVizStdout (KProcess*, char*, int)));
	connect (&graphApplication, SIGNAL (receivedStderr (KProcess*, char*, int)), this, SLOT (graphVizStderr (KProcess*, char*, int)));

	connect (&displayApplication, SIGNAL (receivedStdout (KProcess*, char*, int)), this, SLOT (graphVizDispStdout (KProcess*, char*, int)));
	connect (&displayApplication, SIGNAL (receivedStderr (KProcess*, char*, int)), this, SLOT (graphVizDispStderr (KProcess*, char*, int)));

	graphApplication.start(KProcess::Block, KProcess::AllOutput);
	displayApplication.start(KProcess::Block, KProcess::AllOutput);

	file.close();

	QFile mapFile;
	QTextStream t (&mGraphVizStdout, IO_ReadOnly);
	mapFile.setName("/tmp/kprof.html");

	if(mapFile.exists())
	{
		mapFile.remove();
	}

	mapFile.open(IO_ReadWrite);
	 ClientSideMap(t, mapFile);

	mapFile.close();
	part->openURL(KURL("file:///tmp/kprof.html"));
	END;
}



//Captures the URL clicked signal from the call tree HTML widget and
//opens the required URL in the method widget.
void KProfWidget::openURLRequestDelayed( const KURL &url, const KParts::URLArgs &args)
{
	BEGIN;
	mMethodHtmlPart->openURL(url);
	END;
}

#include "kprofwidget.moc"
