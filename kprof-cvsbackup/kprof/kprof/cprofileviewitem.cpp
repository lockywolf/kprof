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
#include <math.h>
#include <qstring.h>
#include <klocale.h>
#include <kiconloader.h>

#include "kprofwidget.h"
#include "cprofileinfo.h"
#include "cprofileviewitem.h"

CProfileViewItem::CProfileViewItem (QListView *parent, CProfileInfo *profile)
	:	QListViewItem (parent),
		mProfile (profile)
{
  	setRecursiveIcon ();
}

CProfileViewItem::CProfileViewItem (QListViewItem *parent, CProfileInfo *profile)
	:	QListViewItem (parent),
		mProfile (profile)
{
  	setRecursiveIcon ();
}

CProfileViewItem::CProfileViewItem (QListView *parent, QListViewItem *after, CProfileInfo *profile)
	:	QListViewItem (parent, after),
		mProfile (profile)
{
  	setRecursiveIcon ();
}


CProfileViewItem::CProfileViewItem (QListViewItem *parent, QListViewItem *after, CProfileInfo *profile)
	:	QListViewItem (parent, after),
		mProfile (profile)
{
  	setRecursiveIcon ();
}

CProfileViewItem::~CProfileViewItem ()
{
}

void CProfileViewItem::setRecursiveIcon ()
{
  	if (mProfile && mProfile->recursive)
	{
		// here we do not need to test diff mode because col_function is
		// always the first column
		KIconLoader *loader = KGlobal::iconLoader ();
		setPixmap (KProfWidget::col_function, loader->loadIcon ("redo", KIcon::Small));
  	}
}

QString CProfileViewItem::formatFloat (float n, int precision)
{
	// format a float with parameterized precision
	char buffer[32], format[16];
	sprintf (format, "%%.0%df", precision);
	sprintf (buffer, format, n);
	return QString (buffer);
}

QString CProfileViewItem::formatSpeedDiff (float newSpeed, float oldSpeed)
{
	// given two values which are speeds (usually execution speeds),
	// return a string formated as "x% faster" or "x% slower", with special cases
	// like "< 1% faster" or "< 1% slower"

	QString s;

	// make sure we don't have values < 1.0 which make 1/x miscalculations
	if (newSpeed > 0.0 && oldSpeed > 0.0)
	{
		while (newSpeed < 1.0 || oldSpeed < 1.0)
		{
			newSpeed *= 10.0;
			oldSpeed *= 10.0;
		}

		double pct = (double)1.0 / ((double)oldSpeed / (double)newSpeed / (double)100.0);

		if (pct < 100.0)
		{
			pct = (double)100.0 - pct;
			if (pct < (double)1.0)
				s = "< 1% faster";
			else
				s.sprintf (i18n ("%d%% faster"), (int) rint (pct));
		}
		else if (newSpeed < oldSpeed)
		{
			pct -= (double)100.0;
			if (pct < (double)1.0)
				s = "< 1% slower";
			else
				s.sprintf (i18n ("%d%% slower"), (int) rint (pct));
		}
	}
	return s;
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

	if (KProfWidget::sDiffMode == false)
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

			case KProfWidget::diff_col_status:
				if (mProfile->previous)
				{
					// try generating a meaningful string about the
					// changes that occured between two profiles of the same call
					if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_GPROF)
						return formatSpeedDiff (mProfile->custom.gprof.selfMsPerCall, mProfile->previous->custom.gprof.selfMsPerCall);
					if (mProfile->calls && mProfile->previous->calls)
					{
						float n = mProfile->selfSeconds;
						float o = mProfile->previous->selfSeconds;
						if (o > 0.0 && n > 0.0)
						{
							while (o < 1.0 || n < 1.0) {
								o *= 10.0;		// get values up a bit to minimize precision loss errors
								n *= 10.0;
							}
							return formatSpeedDiff ((float)((double)n / (double)mProfile->calls), (float)((double)o / (double)mProfile->previous->calls));
						}
					}
					return QString ("");
				}
				return mProfile->deleted ? i18n("deleted") : i18n("new");

			case KProfWidget::diff_col_new_count:
				return QString::number (mProfile->calls);
			
			case KProfWidget::diff_col_count:
				return mProfile->previous ? QString::number (mProfile->previous->calls) : QString ("");

			case KProfWidget::diff_col_new_total:
				return formatFloat (mProfile->cumSeconds, 3);

			case KProfWidget::diff_col_total:
				return mProfile->previous ? formatFloat (mProfile->previous->cumSeconds, 3) : QString ("");

			case KProfWidget::diff_col_new_totalPercent:
				return formatFloat (mProfile->cumPercent, 3);

			case KProfWidget::diff_col_totalPercent:
				return mProfile->previous ? formatFloat (mProfile->previous->cumPercent, 3) : QString ("");

			case KProfWidget::diff_col_new_self:
				return formatFloat (mProfile->selfSeconds, 3);

			case KProfWidget::diff_col_self:
				return mProfile->previous ? formatFloat (mProfile->previous->selfSeconds, 3) : QString ("");

			case KProfWidget::diff_col_new_totalMsPerCall:
				return formatFloat (mProfile->totalMsPerCall, 3);

			case KProfWidget::diff_col_totalMsPerCall:
				return mProfile->previous ? formatFloat (mProfile->previous->totalMsPerCall, 3) : QString ("");

			default:
				if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_GPROF)
				{
					if (column == KProfWidget::diff_col_new_selfMsPerCall)
						return formatFloat (mProfile->custom.gprof.selfMsPerCall, 3);
					if (column == KProfWidget::diff_col_selfMsPerCall)
						return mProfile->previous ? formatFloat (mProfile->previous->custom.gprof.selfMsPerCall, 3) : QString ("");
				}
				else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_FNCCHECK)
				{
					if (column == KProfWidget::diff_col_new_minMsPerCall)
						return formatFloat (mProfile->custom.fnccheck.minMsPerCall, 3);
					if (column == KProfWidget::diff_col_minMsPerCall)
						return mProfile->previous ? formatFloat (mProfile->previous->custom.fnccheck.minMsPerCall, 3) : QString ("");
					if (column == KProfWidget::diff_col_new_maxMsPerCall)
						return formatFloat (mProfile->custom.fnccheck.maxMsPerCall, 3);
					if (column == KProfWidget::diff_col_maxMsPerCall)
						return mProfile->previous ? formatFloat (mProfile->previous->custom.fnccheck.maxMsPerCall, 3) : QString ("");
				}
				else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_POSE)
				{
					if (column == KProfWidget::diff_col_new_selfCycles)
						return QString::number (mProfile->custom.pose.selfCycles);
					if (column == KProfWidget::diff_col_selfCycles)
						return mProfile->previous ? QString::number (mProfile->previous->custom.pose.selfCycles) : QString ("");
					if (column == KProfWidget::diff_col_new_cumCycles)
						return QString::number (mProfile->custom.pose.cumCycles);
					if (column == KProfWidget::diff_col_cumCycles)
						return mProfile->previous ? QString::number (mProfile->previous->custom.pose.cumCycles) : QString ("");
				}
				return QString ("");
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

	if (KProfWidget::sDiffMode == false)
	{
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
	} else {
		switch (column)
		{
			case KProfWidget::diff_col_function:
				s = mProfile->name;
				break;

			case KProfWidget::diff_col_new_count:
				s.sprintf ("%014ld", mProfile->calls);
				break;

			case KProfWidget::diff_col_count:
				if (mProfile->previous)
					s.sprintf ("%014ld", mProfile->previous->calls);
				break;

			case KProfWidget::diff_col_new_total:
				s.sprintf ("%014ld", (long) (mProfile->cumSeconds * 100.0));
				break;
			
			case KProfWidget::diff_col_total:
				if (mProfile->previous)
					s.sprintf ("%014ld", (long) (mProfile->previous->cumSeconds * 100.0));
				break;

			case KProfWidget::diff_col_new_totalPercent:
				s.sprintf ("%05ld",  (long) (mProfile->cumPercent * 100.0));
				break;

			case KProfWidget::diff_col_totalPercent:
				if (mProfile->previous)
					s.sprintf ("%05ld",  (long) (mProfile->previous->cumPercent * 100.0));
				break;

			case KProfWidget::diff_col_new_self:
				s.sprintf ("%014ld", (long) (mProfile->selfSeconds * 100.0));
				break;

			case KProfWidget::diff_col_self:
				if (mProfile->previous)
					s.sprintf ("%014ld", (long) (mProfile->previous->selfSeconds * 100.0));
				break;

			case KProfWidget::diff_col_new_totalMsPerCall:
				s.sprintf ("%014ld", (long) (mProfile->totalMsPerCall * 100.0));
				break;

			case KProfWidget::diff_col_totalMsPerCall:
				if (mProfile->previous)
					s.sprintf ("%014ld", (long) (mProfile->previous->totalMsPerCall * 100.0));
				break;

			default:	
				if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_GPROF)
				{
					if (column == KProfWidget::diff_col_new_selfMsPerCall)
						s.sprintf ("%014ld", (long) (mProfile->custom.gprof.selfMsPerCall * 100.0));
					if (mProfile->previous && column == KProfWidget::diff_col_selfMsPerCall)
						s.sprintf ("%014ld", (long) (mProfile->previous->custom.gprof.selfMsPerCall * 100.0));
				}
				else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_FNCCHECK)
				{
					if (column == KProfWidget::diff_col_new_minMsPerCall)
						s.sprintf ("%014ld", (long) (mProfile->custom.fnccheck.minMsPerCall * 100.0));
					else if (column == KProfWidget::diff_col_new_maxMsPerCall)
						s.sprintf ("%014ld", (long) (mProfile->custom.fnccheck.maxMsPerCall * 100.0));
					else if (mProfile->previous)
					{
						if (column == KProfWidget::diff_col_minMsPerCall)
							s.sprintf ("%014ld", (long) (mProfile->previous->custom.fnccheck.minMsPerCall * 100.0));
						else if (column == KProfWidget::diff_col_maxMsPerCall)
							s.sprintf ("%014ld", (long) (mProfile->previous->custom.fnccheck.maxMsPerCall * 100.0));
					}
				}
				else if (KProfWidget::sLastFileFormat == KProfWidget::FORMAT_POSE)
				{
					if (column == KProfWidget::diff_col_new_selfCycles)
						s.sprintf ("%014ld", mProfile->custom.pose.selfCycles);
					else if (column == KProfWidget::diff_col_new_cumCycles)
						s.sprintf ("%014ld", mProfile->custom.pose.cumCycles);
					else if (mProfile->previous)
					{
						if (column == KProfWidget::diff_col_selfCycles)
							s.sprintf ("%014ld", mProfile->previous->custom.pose.selfCycles);
						else if (column == KProfWidget::diff_col_cumCycles)
							s.sprintf ("%014ld", mProfile->previous->custom.pose.cumCycles);
					}
				}
				break;
		}
	}

	return s;
}

void CProfileViewItem::paintCell (QPainter * p, const QColorGroup & cg, int column, int width, int align)
{
	// call original drawing method
	QListViewItem::paintCell (p, cg, column, width, align);
	if (p == NULL)
		return;
	
	// if in diff mode, paint a border line on the right of the cell:
	// - light for the edge of a "previous" cell
	// - dark for the edge of a "new" cell
	// this gives a display where old and new entries are grouped by blocks of two
	p->save ();
	if (KProfWidget::sDiffMode == false)
		p->setPen (QPen (cg.foreground(), 1, DotLine));
	else
	{
		bool solid = false;
		if (column==KProfWidget::diff_col_status ||
			column==KProfWidget::diff_col_new_count ||
			column==KProfWidget::diff_col_new_total ||
			column==KProfWidget::diff_col_new_totalPercent ||
			column==KProfWidget::diff_col_new_self ||
			column==KProfWidget::diff_col_new_totalMsPerCall ||
			(KProfWidget::sLastFileFormat==KProfWidget::FORMAT_GPROF && column==KProfWidget::diff_col_new_selfMsPerCall) ||
			(KProfWidget::sLastFileFormat==KProfWidget::FORMAT_FNCCHECK &&
			 (column==KProfWidget::diff_col_new_minMsPerCall || column==KProfWidget::diff_col_new_maxMsPerCall)) ||
			(KProfWidget::sLastFileFormat==KProfWidget::FORMAT_POSE &&
			 (column==KProfWidget::diff_col_new_selfCycles || column==KProfWidget::diff_col_new_cumCycles)))
		{
			solid = true;
		}
		p->setPen (QPen (cg.foreground(), 1, solid ? SolidLine : DotLine));
	}
QColor c (cg.foreground());
printf ("R=%d, g=%d, b=%d, after lighting: ", c.red(), c.green(), c.blue());
c = c.light();
printf ("R=%d, g=%d, b=%d\n", c.red(), c.green(), c.blue());
p->setPen (QPen (c, 1, SolidLine));
	p->drawLine (width-1, 0, width-1, 20);
	p->restore ();
}

