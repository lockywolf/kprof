/*
 * cprofileviewitem.cpp
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
#include <qstring.h>
#include <kiconloader.h>

#include "kprofwidget.h"
#include "cprofileinfo.h"
#include "cprofileviewitem.h"

static bool blockInval = false;

CProfileViewItem::CProfileViewItem (QListView *parent, CProfileInfo *profile, bool diff)
	:	QListViewItem (parent),
		mProfile (profile),
		mDiff (diff)
{
  	setRecursiveIcon ();
}

CProfileViewItem::CProfileViewItem (QListViewItem *parent, CProfileInfo *profile, bool diff)
	:	QListViewItem (parent),
		mProfile (profile),
		mDiff (diff)
{
  	setRecursiveIcon ();
}

CProfileViewItem::CProfileViewItem (QListView *parent, QListViewItem *after, CProfileInfo *profile, bool diff)
	:	QListViewItem (parent, after),
		mProfile (profile),
		mDiff (diff)
{
  	setRecursiveIcon ();
}


CProfileViewItem::CProfileViewItem (QListViewItem *parent, QListViewItem *after, CProfileInfo *profile, bool diff)
	:	QListViewItem (parent, after),
		mProfile (profile),
		mDiff (diff)
{
  	setRecursiveIcon ();
}

CProfileViewItem::~CProfileViewItem ()
{
}

#ifdef TEST_DIFF
void CProfileViewItem::setup ()
{
	QListViewItem::setup ();
	if (mProfile->previous)
		setHeight (2 * height ());
}
#endif

void CProfileViewItem::setRecursiveIcon ()
{
  	if (mProfile && mProfile->recursive)
	{
		KIconLoader *loader = KGlobal::iconLoader ();
		setPixmap (KProfWidget::col_recursive, loader->loadIcon ("redo", KIcon::Small));
  	}
}

QString CProfileViewItem::text (int column) const
{
	if (mProfile == NULL)
	{
		// we are a top level element of the object view: just return the
		// name, being the class name
		CProfileViewItem *child = (CProfileViewItem *) firstChild ();
		return (child == NULL || column != KProfWidget::col_function) ? QString("") : child->mProfile->object;
  	}

	if (mDiff == false)
	{
		switch (column)
		{
			case KProfWidget::col_function:
			{
				CProfileViewItem *p = dynamic_cast<CProfileViewItem *> (parent ());
				if (p && p->mProfile == NULL)
				{
					// we are in a method of an object in the object
					if (mProfile->multipleSignatures)
						return mProfile->name.right (mProfile->name.length () - mProfile->object.length() - 2);
					return mProfile->method;
				}
				return mProfile->simplifiedName;
			}

			case KProfWidget::col_count:
				return QString::number (mProfile->calls);

			case KProfWidget::col_total:
				return formatFloat (mProfile->cumSeconds, 3);

			case KProfWidget::col_totalPercent:
				return formatFloat (mProfile->cumPercent, 3);

			case KProfWidget::col_self:
				return formatFloat (mProfile->selfSeconds, 3);

			case KProfWidget::col_totalMsPerCall:
				return formatFloat (mProfile->totalMsPerCall, 3);

			default:
				if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_GPROF)
				{
					if (column == KProfWidget::col_selfMsPerCall)
						return formatFloat (mProfile->custom.gprof.selfMsPerCall, 3);
				}
				else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_FNCCHECK)
				{
					if (column == KProfWidget::col_minMsPerCall)
						return formatFloat (mProfile->custom.fnccheck.minMsPerCall, 3);
					if (column == KProfWidget::col_maxMsPerCall)
						return formatFloat (mProfile->custom.fnccheck.maxMsPerCall, 3);
				}
				else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_POSE)
				{
					if (column == KProfWidget::col_selfCycles)
						return QString::number (mProfile->custom.pose.selfCycles);
					if (column == KProfWidget::col_cumCycles)
						return QString::number (mProfile->custom.pose.cumCycles);
				}
				return "";
	   }
	} else {
		// if diff-mode
		switch (column)
		{
			case KProfWidget::diff_col_function:
			{
				CProfileViewItem *p = dynamic_cast<CProfileViewItem *> (parent ());
				if (p && p->mProfile == NULL)
				{
					// we are in a method of an object in the object
					if (mProfile->multipleSignatures)
						return mProfile->name.right (mProfile->name.length () - mProfile->object.length() - 2);
					return mProfile->method;
				}
				return mProfile->simplifiedName;
			}

			case KProfWidget::diff_col_new_count:
				return QString::number (mProfile->calls);
			
			case KProfWidget::diff_col_count:
				return mProfile->previous ? QString::number (mProfile->previous->calls) : "";

			case KProfWidget::diff_col_new_total:
				return formatFloat (mProfile->cumSeconds, 3);

			case KProfWidget::diff_col_total:
				return mProfile->previous ? formatFloat (mProfile->previous->cumSeconds, 3) : "";

			case KProfWidget::diff_col_new_totalPercent:
				return formatFloat (mProfile->cumPercent, 3);

			case KProfWidget::diff_col_totalPercent:
				return mProfile->previous ? formatFloat (mProfile->previous->cumPercent, 3) : "";

			case KProfWidget::diff_col_new_self:
				return formatFloat (mProfile->selfSeconds, 3);

			case KProfWidget::diff_col_self:
				return mProfile->previous ? formatFloat (mProfile->previous->selfSeconds, 3) : "";

			case KProfWidget::diff_col_new_totalMsPerCall:
				return formatFloat (mProfile->totalMsPerCall, 3);

			case KProfWidget::diff_col_totalMsPerCall:
				return mProfile->previous ? formatFloat (mProfile->previous->totalMsPerCall, 3) : "";

			default:
				if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_GPROF)
				{
					if (column == KProfWidget::diff_col_new_selfMsPerCall)
						return formatFloat (mProfile->custom.gprof.selfMsPerCall, 3);
					if (column == KProfWidget::diff_col_selfMsPerCall)
						return mProfile->previous ? formatFloat (mProfile->previous->custom.gprof.selfMsPerCall, 3) : "";
				}
				else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_FNCCHECK)
				{
					if (column == KProfWidget::diff_col_new_minMsPerCall)
						return formatFloat (mProfile->custom.fnccheck.minMsPerCall, 3);
					if (column == KProfWidget::diff_col_minMsPerCall)
						return mProfile->previous ? formatFloat (mProfile->previous->custom.fnccheck.minMsPerCall, 3) : "";
					if (column == KProfWidget::diff_col_new_maxMsPerCall)
						return formatFloat (mProfile->custom.fnccheck.maxMsPerCall, 3);
					if (column == KProfWidget::diff_col_maxMsPerCall)
						return mProfile->previous ? formatFloat (mProfile->previous->custom.fnccheck.maxMsPerCall, 3) : "";
				}
				else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_POSE)
				{
					if (column == KProfWidget::diff_col_new_selfCycles)
						return QString::number (mProfile->custom.pose.selfCycles);
					if (column == KProfWidget::diff_col_selfCycles)
						return mProfile->previous ? QString::number (mProfile->previous->custom.pose.selfCycles) : "";
					if (column == KProfWidget::diff_col_new_cumCycles)
						return QString::number (mProfile->custom.pose.cumCycles);
					if (column == KProfWidget::diff_col_cumCycles)
						return mProfile->previous ? QString::number (mProfile->previous->custom.pose.cumCycles) : "";
				}
				return "";
		}
	}
}

QString CProfileViewItem::key (int column, bool) const
{
	QString s;

	if (mProfile == NULL)
	{
		// we are a top level element of the object view: just return the
		// name, being the class name
		CProfileViewItem *child = (CProfileViewItem *) firstChild ();
		if (child == NULL || column != KProfWidget::col_function)
			return "";

		return child->mProfile->object;
  	}

	switch (column)
	{
		case KProfWidget::col_function:
			s = mProfile->name;
			break;

		case KProfWidget::col_count:
			s.sprintf ("%014ld", mProfile->calls);
			break;

   		case KProfWidget::col_total:
			s.sprintf ("%014ld", (long) (mProfile->cumSeconds * 100.0));
			break;

		case KProfWidget::col_totalPercent:
			s.sprintf ("%05ld",  (long) (mProfile->cumPercent * 100.0));
			break;

   		case KProfWidget::col_self:
			s.sprintf ("%014ld", (long) (mProfile->selfSeconds * 100.0));
			break;

		case KProfWidget::col_totalMsPerCall:
			s.sprintf ("%014ld", (long) (mProfile->totalMsPerCall * 100.0));
			break;
		
		default:	
			if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_GPROF)
			{
				if (column == KProfWidget::col_selfMsPerCall)
					s.sprintf ("%014ld", (long) (mProfile->custom.gprof.selfMsPerCall * 100.0));
			}
			else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_FNCCHECK)
			{
				if (column == KProfWidget::col_minMsPerCall)
					s.sprintf ("%014ld", (long) (mProfile->custom.fnccheck.minMsPerCall * 100.0));
				else if (column == KProfWidget::col_maxMsPerCall)
					s.sprintf ("%014ld", (long) (mProfile->custom.fnccheck.maxMsPerCall * 100.0));
			}
			else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_POSE)
			{
				if (column == KProfWidget::col_selfCycles)
					s.sprintf ("%014ld", mProfile->custom.pose.selfCycles);
				else if (column == KProfWidget::col_cumCycles)
					s.sprintf ("%014ld", mProfile->custom.pose.cumCycles);
			}
			break;
	}

	return s;
}

QString CProfileViewItem::formatFloat (float n, int precision)
{
	// format a float with parameterized precision
	char buffer[32], format[16];
	sprintf (format, "%%.0%df", precision);
	sprintf (buffer, format, n);
	return QString (buffer);
}

#ifdef TEST_DIFF
void CProfileViewItem::invalidateHeight ()
{
	if (blockInval == false)
		QListViewItem::invalidateHeight ();
}

void CProfileViewItem::paintCell (QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
	if (p == NULL)
		return;
	
	// no diff: paint the cell normally
	if (mProfile==NULL || mProfile->previous==NULL)
	{
		QListViewItem::paintCell (p, cg, column, width, align);
		return;
	}

	// paint the cell once, normally (current profile)
	blockInval = true;
	QFont f = p->font ();
	int h = height ();
	int h1 = h / 2;
	int h2 = h - h1;
	setHeight (h1);
	if (column != KProfWidget::col_function)
	{
		f.setBold (true);
		p->setFont (f);
	}
	
	QListViewItem::paintCell (p, cg, column, width, align);

	// paint the cell a second time, below, with different attribute (previous profile)
	if (column != KProfWidget::col_function && column != KProfWidget::col_recursive)
	{
		f.setBold (false);
		p->setFont (f);
	}
	p->translate (0, h1);
	setHeight (h2);

	CProfileInfo *current = mProfile;			// dirty trick...
	mProfile = mProfile->previous;
	
	QListViewItem::paintCell (p, cg, (column==KProfWidget::col_function || column==KProfWidget::col_recursive) ? -1 : column, width, align);
	
	mProfile = current;
	p->translate (0, -h1);
	setHeight (h);
	blockInval = false;
}

#endif // TEST_DIFF
