/*
 * cprofileviewitem.h
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

#ifndef __CPROFILEVIEWITEM_H__
#define __CPROFILEVIEWITEM_H__

#include <qlistview.h>
#include <qregexp.h>

class CProfileInfo;

class Q_EXPORT CProfileViewItem : public QListViewItem
{
protected:
	CProfileInfo*		mProfile;

public:
	CProfileViewItem (QListView *parent, 			CProfileInfo *profile);
	CProfileViewItem (QListViewItem *parent, 	CProfileInfo *profile);
	CProfileViewItem (QListView *parent, 			QListViewItem *after, CProfileInfo *profile);
	CProfileViewItem (QListViewItem *parent, 	QListViewItem *after, CProfileInfo *profile);
	virtual ~CProfileViewItem ();

	virtual void paintCell (QPainter * p, const QColorGroup & cg, int column, int width, int align);
	virtual QString text (int column) const;
	virtual QString key (int column, bool ascending) const;

	inline CProfileInfo* getProfile () const { return mProfile; }

private:
	void setRecursiveIcon ();
	static QString formatFloat (float n, int precision);
	static QString formatSpeedDiff (float newSpeed, float oldSpeed);
};

#endif
