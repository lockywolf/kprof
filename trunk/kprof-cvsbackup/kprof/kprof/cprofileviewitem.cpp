/*
 * cprofileviewitem.cpp
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

#include <stdlib.h>

#include <qstring.h>

#include <kiconloader.h>

#include "cprofileviewitem.h"

QRegExp *CProfileViewItem::sClassRegExp = NULL;


CProfileViewItem::CProfileViewItem (QListView *parent, CProfileInfo *profile)
	:	QListViewItem (parent),
		mProfile (profile)
{
	if (sClassRegExp == NULL)
		sClassRegExp = new QRegExp ("::[^\\(:]*\\(");

  	setRecursiveIcon ();
}

CProfileViewItem::CProfileViewItem (QListViewItem *parent, CProfileInfo *profile)
	:	QListViewItem (parent),
		mProfile (profile)
{
	if (sClassRegExp == NULL)
		sClassRegExp = new QRegExp ("::[^\\(:]*\\(");
  	setRecursiveIcon ();
}

CProfileViewItem::CProfileViewItem (QListView *parent, QListViewItem *after, CProfileInfo *profile)
	:	QListViewItem (parent, after),
		mProfile (profile)
{
	if (sClassRegExp == NULL)
		sClassRegExp = new QRegExp ("::[^\\(:]*\\(");
  	setRecursiveIcon ();
}


CProfileViewItem::CProfileViewItem (QListViewItem *parent, QListViewItem *after, CProfileInfo *profile)
	:	QListViewItem (parent, after),
		mProfile (profile)
{
	if (sClassRegExp == NULL)
		sClassRegExp = new QRegExp ("::[^\\(:]*\\(");
  	setRecursiveIcon ();
}

CProfileViewItem::~CProfileViewItem ()
{
}

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
		if (child == NULL || column != KProfWidget::col_function)
			return "";

		return child->mProfile->name.left (sClassRegExp->find (child->mProfile->name, 0));
  	}

	switch (column)
	{
		case KProfWidget::col_function:
		{
			CProfileViewItem *p = dynamic_cast<CProfileViewItem *> (parent ());
			if (p && p->mProfile == NULL)
			{
				// we are in a method of an object in the object
				return mProfile->name.right (mProfile->name.length () - sClassRegExp->find (mProfile->name, 0) - 2);
    		}
			return mProfile->name;
   		}

   		case KProfWidget::col_count:
			return QString::number (mProfile->calls);

   		case KProfWidget::col_total:
			return QString::number (mProfile->cumSeconds);

		case KProfWidget::col_totalPercent:
			return QString::number (mProfile->cumPercent);

   		case KProfWidget::col_self:
			return QString::number (mProfile->selfSeconds);

   		case KProfWidget::col_totalPerCall:
			return QString::number (mProfile->totalTsPerCall);

   		case KProfWidget::col_selfPerCall:
			return QString::number (mProfile->selfTsPerCall);

   		default:
			return "";
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

		return child->mProfile->name.left (sClassRegExp->find (child->mProfile->name, 0));
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

		case KProfWidget::col_totalPerCall:
			s.sprintf ("%014ld", (long) (mProfile->totalTsPerCall * 100.0));
			break;

   		case KProfWidget::col_selfPerCall:
			s.sprintf ("%014ld", (long) (mProfile->selfTsPerCall * 100.0));
			break;
	}

	return s;
}
